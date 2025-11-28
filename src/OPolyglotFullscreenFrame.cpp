#include "OPolyglotFullscreenFrame.h"
#include "OPolyglotEvent.h"
#include "Utils.h"
#include "Config.h"
#include <wx/dcscreen.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/display.h>

enum{
	TIMER_ID,
};

OPolyglotFullscreenFrame::OPolyglotFullscreenFrame(wxWindow *parent) : GUIFullscreen(parent)
{
	OPOLYGLOT_MESSAGE();
	this->Show(false);
	wxScreenDC dc;
	int w,h;
	dc.GetSize(&w,&h);
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
	wxRect r1 = dis.GetGeometry();
	timePressedLeft = 0;
	OPOLYGLOT_DEBUG(wxT("mouse %d %d display %d %d %dx%d"),startX,startY,r1.GetX(),r1.GetY(),r1.GetWidth(),r1.GetHeight());
	timer->Start(TIMEOUT_FULLSCREAN_CHECK_MOUSE_STATE);
}


OPolyglotFullscreenFrame::~OPolyglotFullscreenFrame()
{
	wxMutexLocker lock(mutex);
	timer->Stop();
	OPOLYGLOT_MESSAGE();
}


void OPolyglotFullscreenFrame::OnMouseLeftUp( wxMouseEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("FullscreenFrame"));
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
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SELECT_AREA);
			event->SetString(wxEmptyString);
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
				str.Append(wxT("/area.png"));
				OPolyglotImage *image = new OPolyglotImage(&bitmapArea);
				bitmapArea.SaveFile(str,wxBITMAP_TYPE_PNG);
				wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SELECT_AREA);
				event->SetString(str);
				wxQueueEvent(this->parent,event);
				this->Destroy();
				return;

			}

		} else
		{
			OPOLYGLOT_DEBUG(wxT("is small select AREA %dx%d"),w,h);
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SELECT_AREA);
			event->SetString(wxEmptyString);
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
}
