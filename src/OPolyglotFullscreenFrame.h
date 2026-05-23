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
#include <wx/uiaction.h>
#include <wx/timer.h>
#include <wx/thread.h>
#include <wx/dynarray.h>

WX_DECLARE_OBJARRAY(wxRect,OPolyglotArrayRect);


class OPolyglotFullscreenFrame : public GUIFullscreen
{
	public:
		OPolyglotFullscreenFrame(wxWindow *parent,wxString fileName);
		~OPolyglotFullscreenFrame();
		void OnPaint(wxPaintEvent& event);
		void OnMouseLeftUp( wxMouseEvent& event ) ; 
		void OnMouseLeftDown( wxMouseEvent& event);
		void OnMouseMotion(wxMouseEvent& event);
		void OnMouseRightDown(wxMouseEvent& event);
		void OnMouseRightUp(wxMouseEvent& event);
		void OnCharHook(wxKeyEvent& event);
		void OnItemDelete(wxCommandEvent& event);
		void OnItemOnlyOCR(wxCommandEvent& event);
	private:
		wxMutex mutex;
		wxWindow *parent;
		wxBitmap bitmapFile = wxNullBitmap;
		wxBitmap bitmapDC = wxNullBitmap;
		int startX = -1;
		int startY = -1;
		int endX = -1;
		int endY = -1;
		size_t selectBoxResize=-1;
		int selectLineResize = 0;
		size_t selectBoxMenu = -1;
		wxPanel *Panel;
		OPolyglotArrayRect boxs;
		wxArrayInt boxsOption;
		wxXmlNode *nodeScreenshot;
};
