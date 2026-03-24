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
#include <wx/accel.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

enum{
	TIMER_ID,
};
enum{
	ID_KEY_ESCAPE=1001,
};

OPolyglotFullscreenFrame::OPolyglotFullscreenFrame(wxWindow *parent,wxString fileName,OPolyglotImage *img) : GUIFullscreen(parent)
{
	int w,h;
	image = img;
#if defined(__SNAP)||defined(__FLATPAK)
	wxMilliSleep(500);
#endif
	Panel = new wxPanel(this);
	Panel->SetBackgroundStyle(wxBG_STYLE_PAINT);
	Panel->Bind(wxEVT_PAINT, &OPolyglotFullscreenFrame::OnPaint, this);
	//dc.GetSize(&w,&h);
	if(fileName.Contains(wxS("bmp")))
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame bitmap type BMP"));
		if(!bitmap.LoadFile(fileName,wxBITMAP_TYPE_BMP))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotFullscreenFrame not load screenshot %s"),fileName);
		}
	}
	if(fileName.Contains(wxS("png")))
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame bitmap type PNG"));
		if(!bitmap.LoadFile(fileName,wxBITMAP_TYPE_PNG))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotFullscreenFrame not load screenshot %s"),fileName);
		}
	}
	w = bitmap.GetWidth();
	h = bitmap.GetHeight();
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame(%dx%d) %s"),w,h,OPOLYGLOT_BOOL_TO_STRING(bitmap.IsOk()));
	this->parent = parent;
	wxMouseState state = wxGetMouseState();
	startX = state.GetX();
	startY = state.GetY();
	oldX = startX;
	oldY = startY;
#if 1
	this->Bind(wxEVT_TIMER,&OPolyglotFullscreenFrame::OnTimeMouseState,this);
	timer = new wxTimer();
	timer->SetOwner(this,TIMER_ID);
	timer->Start(15000);
#endif
	wxDisplay dis(this);
	timePressedLeft = 0;
	OPOLYGLOT_DEBUG(wxT("mouse %d %d display %d %d %dx%d"),startX,startY,dis.GetGeometry().GetX(),dis.GetGeometry().GetY(),dis.GetGeometry().GetWidth(),dis.GetGeometry().GetHeight());
	//timer->Start(30000);
	this->SetSize(w,h);
	this->Show(true);
	this->ShowFullScreen(true,wxFULLSCREEN_ALL);
	this->SetFocus();
	//this->Refresh();
	int x,y;
	this->GetPosition(&x,&y);
	this->GetSize(&w,&h);
	OPOLYGLOT_DEBUG(wxT("size(%dx%d) %d %d"),w,h,x,y);
	Bind(wxEVT_CHAR_HOOK, &OPolyglotFullscreenFrame::OnCharHook, this);
	Panel->Bind(wxEVT_LEFT_DOWN, &OPolyglotFullscreenFrame::OnMouseLeftDown, this);
	Panel->Bind(wxEVT_LEFT_UP, &OPolyglotFullscreenFrame::OnMouseLeftUp, this);
	Panel->Bind(wxEVT_MOTION, &OPolyglotFullscreenFrame::OnMouseMotion, this);
	Panel->Refresh();
}


OPolyglotFullscreenFrame::~OPolyglotFullscreenFrame()
{
	wxMutexLocker lock(mutex);
	//timer->Stop();
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotFullscreenFrame"));
}


void OPolyglotFullscreenFrame::OnCharHook(wxKeyEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnCharHook"));
	OPOLYGLOT_DEBUG(wxT("OnCharHook"));
	if(event.GetKeyCode() == WXK_ESCAPE)
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnCharHook(WXK_ESCAPE)"));
		OPOLYGLOT_DEBUG(wxT("OnCharHook(WXK_ESCAPE)"));
		timer->Stop();
		this->Destroy();
		return;
	}
}

void OPolyglotFullscreenFrame::OnMouseLeftUp( wxMouseEvent& event ) 
{

	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnMouseLeftUp"));
	OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame::OnMouseLeftUp"));
	Refresh();
}

void OPolyglotFullscreenFrame::OnMouseMotion( wxMouseEvent& event)
{
	Refresh();
}


void OPolyglotFullscreenFrame::OnMouseLeftDown( wxMouseEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnMouseLeftDown %dx%d"),event.GetX(),event.GetY());
	OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame::OnMouseLeftDown %dx%d"),event.GetX(),event.GetY());
	startX = event.GetX();
	startY = event.GetY();
	Refresh();
}

void OPolyglotFullscreenFrame::OnEscape(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnEscape"));
	OPOLYGLOT_DEBUG(wxT("OnEscape"));
}

void OPolyglotFullscreenFrame::OnKey(wxKeyEvent& event)
{
	OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame::OnKey"));
	if(event.GetKeyCode() == WXK_ESCAPE)
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnKey ESCAPE"));
	}
}

void OPolyglotFullscreenFrame::OnTimeMouseState(wxTimerEvent &event)
{
	timer->Stop();
	this->Destroy();
#if 0
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
#endif
}

void OPolyglotFullscreenFrame::OnPaint(wxPaintEvent& event)
{
	wxStopWatch time;
	int w,h;
	time.Start();
	wxMutexLocker lock(mutex);
#if 0
	wxColour col;
	wxMouseState state = wxGetMouseState();
	int x1,y1,x2,y2;
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
#endif
	wxAutoBufferedPaintDC dc(this->Panel);
	//wxPaintDC dc(this->Panel);
	//PrepareDC(dc);
	dc.Clear();
	//dc.DrawBitmap(bitmap,0,0);
#if defined(__SNAP) || defined(__FLATPAK)
	wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
	if(gc){
		//gc->Clear();
		gc->SetAntialiasMode(wxANTIALIAS_NONE);
		gc->SetInterpolationQuality(wxINTERPOLATION_NONE);
		gc->DrawBitmap(bitmap,0,0,bitmap.GetWidth(),bitmap.GetHeight());
		delete gc;
	}
#else
	dc.DrawBitmap(bitmap,0,0);
#endif

	
#if 0
	if(state.LeftIsDown())
	{
		dc.GetPixel(startX,startY,&col);
		col.Set(~col.GetRed(),~col.GetGreen(),~col.GetBlue());
		wxPen pen(col,2,wxPENSTYLE_SHORT_DASH );
		dc.SetPen(pen);
		dc.DrawLine(x1,y1,x2,y1);
		dc.DrawLine(x2,y1,x2,y2);
		dc.DrawLine(x1,y2,x2,y2);
		dc.DrawLine(x1,y2,x1,y1);
	}
#endif
	time.Pause();
	//OPOLYGLOT_DEBUG(wxT("%s %dx%d time %ld"),OPOLYGLOT_BOOL_TO_STRING(bitmap.IsOk()),bitmap.GetWidth(),bitmap.GetHeight(),time.Time());
	this->SetFocus();
}
