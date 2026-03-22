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
		OPolyglotFullscreenFrame(wxWindow *parent,wxString fileName,OPolyglotImage *img);
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
