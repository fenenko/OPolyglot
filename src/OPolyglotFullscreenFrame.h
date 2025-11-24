#pragma once
#include "GuiOPolyglot.h"
#include "OPolyglot.h"
#include <wx/uiaction.h>
#include <wx/timer.h>


class OPolyglotFullscreenFrame : public GUIFullscreen
{
	public:
		OPolyglotFullscreenFrame(wxWindow *parent);
		~OPolyglotFullscreenFrame();
		void OnTimeMouseState(wxTimerEvent &event);
		void OnPaint(wxPaintEvent &event);
		void OnMouseLeftUp( wxMouseEvent& event ) wxOVERRIDE; 
	private:
		wxUIActionSimulator action;
		wxTimer *timer;
		int startX;
		int startY;
		int oldX;
		int oldY;
		wxWindow *parent;
		wxBitmap bitmap;
		int timePressedLeft = 0;
};
