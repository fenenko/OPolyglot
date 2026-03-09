/*
 * Copyright 2026 Fenenko Oleksandr.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "OPolyglotFullscreenFrame.h"
#include "OPolyglotEvent.h"
#include "OPolyglotType.h"
#include "Utils.h"
#include "Config.h"
#include <wx/dcscreen.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/display.h>

enum{
	TIMER_ID,
};

OPolyglotFullscreenFrame::OPolyglotFullscreenFrame(wxWindow *parent,OPolyglotImage *img) : GUIFullscreen(parent)
{
	int tW,tH;
	this->ShowFullScreen(true);
	wxWindowDC wDC(parent);
	wDC.GetSize(&tW,&tH);
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame(%dx%d)"),tW,tH);
	this->Show(false);
	wxSize s = wxGetDisplaySize();
	wxScreenDC dc;
	int w,h;
	image = img;
	//dc.GetSize(&w,&h);
	w = s.GetWidth();
	h = s.GetHeight();
	OPOLYGLOT_DEBUG(wxT("%dx%d"),w,h);
	bitmap = wxBitmap(w,h);
	wxMemoryDC memDC;
	memDC.SelectObject(bitmap);
	memDC.Blit(0,0,w,h,&dc,0,0);
	memDC.SelectObject(wxNullBitmap);
	this->parent = parent;
	wxMouseState state = wxGetMouseState();
	startX = state.GetX();
	startY = state.GetY();
	oldX = startX;
	oldY = startY;
	this->Bind(wxEVT_TIMER,&OPolyglotFullscreenFrame::OnTimeMouseState,this);
	timer = new wxTimer();
	timer->SetOwner(this,TIMER_ID);
	timer->Start(TIMEOUT_FULLSCREAN_CHECK_MOUSE_STATE);
	Bind(wxEVT_PAINT, &OPolyglotFullscreenFrame::OnPaint, this);
	wxDisplay dis(this);
	timePressedLeft = 0;
	OPOLYGLOT_DEBUG(wxT("mouse %d %d display %d %d %dx%d"),startX,startY,dis.GetGeometry().GetX(),dis.GetGeometry().GetY(),dis.GetGeometry().GetWidth(),dis.GetGeometry().GetHeight());
	timer->Start(TIMEOUT_FULLSCREAN_CHECK_MOUSE_STATE);
}


OPolyglotFullscreenFrame::~OPolyglotFullscreenFrame()
{
	wxMutexLocker lock(mutex);
	timer->Stop();
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotFullscreenFrame"));
}


void OPolyglotFullscreenFrame::OnMouseLeftUp( wxMouseEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnMouseLeftUp"));
}


void OPolyglotFullscreenFrame::OnTimeMouseState(wxTimerEvent &event)
{
	wxMutexLocker lock(mutex);
	timer->Stop();
	wxMouseState state = wxGetMouseState();
	if(!state.LeftIsDown())
	{
		int x,y,w,h;
		if(timePressedLeft  < TIME_PRESSED_LEFT )
		{

			OPOLYGLOT_MESSAGE(wxT("time pressed left is small %d"),timePressedLeft);
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SEND_IMAGE);
			event->SetPayload(NULL);
			wxQueueEvent(this->parent,event);
			this->Destroy();
			return;
		}
		if(state.GetX() < startX)
		{
			x = startX;
			startX = state.GetX();
		} else
		{
			x = state.GetX();
		}
		if(state.GetY() < startY)
		{
			y = startY;
			startY = state.GetY();
		} else
		{
			y = state.GetY();
		}
		w = x - startX;
		h = y - startY;
		if((32 < (w))&&(16 < (h)))
		{
			wxBitmap bitmapArea(w,h);
			wxMemoryDC screenDC;
			screenDC.SelectObject(bitmap);
			wxMemoryDC memDC;
			memDC.SelectObject(bitmapArea);
			if(!memDC.StretchBlit(0,0,w,h,&screenDC,startX,startY,w,h))
			{
				OPOLYGLOT_ERROR(wxT("memDC.StretchBlit(0,0,%d,%d,&dc,%d,%d,%d,%d)"),w,h,startX,startY,w,h);
			} else
			{
				OPOLYGLOT_DEBUG(wxT("select area %d %d %dx%d"),startX,startY,w,h);
				memDC.SelectObject(wxNullBitmap);
				wxString str = wxFileName::GetTempDir();
				image->SetData(bitmapArea);
				wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SEND_IMAGE);
				event->SetInt(-1);
				event->SetPayload<OPolyglotImage *>(NULL);
				wxQueueEvent(this->parent,event);
				image = NULL;
				this->Destroy();
				return;

			}

		} else
		{
			OPOLYGLOT_DEBUG(wxT("is small select AREA %dx%d"),w,h);
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SEND_IMAGE);
			event->SetInt(0);
			wxQueueEvent(this->parent,event);
			this->Destroy();
			return;
		}
	} else
	{
		if((startX != state.GetX())||(startY != state.GetY()))
		{
			this->Show(true);
			this->ShowFullScreen(true);
			this->SetFocus();
			this->Refresh();
		}
		timePressedLeft++;
		if((oldX != state.GetX())||(oldY != state.GetY()))
		{
			this->Refresh(true);
		}
		oldX = state.GetX();
		oldY = state.GetY();
	}
	timer->Start(TIMEOUT_FULLSCREAN_CHECK_MOUSE_STATE);
}


void OPolyglotFullscreenFrame::OnPaint(wxPaintEvent& event)
{
	wxStopWatch time;
	time.Start();
	wxMutexLocker lock(mutex);
	wxColour col;
	wxMouseState state = wxGetMouseState();
	int x1,y1,x2,y2;
	wxDisplay dis(this);
	if(startX < state.GetX())
	{
		x1 = startX;
		x2 = state.GetX();
	} else
	{
		x1 = state.GetX();
		x2 = startX;
	}
	if(startY < state.GetX())
	{
		y1 = startY;
		y2 = state.GetY();
	} else
	{
		y1 = state.GetY();
		y2 = startY;
	}
	wxPaintDC dc(this);
	dc.DrawBitmap(bitmap,0,0);
	dc.GetPixel(startX,startY,&col);
	col.Set(~col.GetRed(),~col.GetGreen(),~col.GetBlue());
	wxPen pen(col,2,wxPENSTYLE_SHORT_DASH );
	dc.SetPen(pen);
	dc.DrawLine(x1,y1,x2,y1);
	dc.DrawLine(x2,y1,x2,y2);
	dc.DrawLine(x1,y2,x2,y2);
	dc.DrawLine(x1,y2,x1,y1);
	time.Pause();
	OPOLYGLOT_DEBUG(wxT("time %ld"),time.Time());
}
