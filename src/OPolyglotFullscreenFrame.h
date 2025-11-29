#pragma once
#include "GuiOPolyglot.h"
#include "OPolyglotEvent.h"
#include "OPolyglotType.h"
#include <wx/uiaction.h>
#include <wx/timer.h>
#include <wx/thread.h>


class OPolyglotFullscreenFrame : public GUIFullscreen
{
	public:
		OPolyglotFullscreenFrame(wxWindow *parent,OPolyglotImage *img);
		~OPolyglotFullscreenFrame();
		void OnTimeMouseState(wxTimerEvent &event);
		void OnPaint(wxPaintEvent &event);
		void OnMouseLeftUp( wxMouseEvent& event ) wxOVERRIDE; 
	private:
		wxUIActionSimulator action;
		wxTimer *timer;
		wxMutex mutex;
		int startX;
		int startY;
		int oldX;
		int oldY;
		wxWindow *parent;
		wxBitmap bitmap;
		OPolyglotImage *image;
		int timePressedLeft = 0;
};
