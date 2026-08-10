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
#include "Utils.h"
#include <wx/thread.h>


class OPolyglotEditTranslating : public GUIOpolyglotEditTranslating ,wxThreadHelper
{
	private:
		wxString idText;
		int startViewX = 0;
		int startViewY = 0;
		bool onlyOCR;
		wxBitmap bitmap;
		wxWindow *handler;
		wxString oldText;
		wxString oldOCR;
		wxString filesBergamot;
		int oldLineCount;
		wxString fileName;
		OPolyglotDialogProgress *progress;
	public:
		OPolyglotEditTranslating(wxWindow* parent,wxString fileName,wxString &id,int oldLine);
		~OPolyglotEditTranslating();
		void OnPaint(wxPaintEvent& event);
		void OnVScroll( wxScrollEvent& event ) wxOVERRIDE;
		void OnHScroll( wxScrollEvent& event ) wxOVERRIDE;
		void OnTextOCR( wxCommandEvent& event ) ;
		void OnTextTranslate(wxCommandEvent& event) wxOVERRIDE;
		void OnSave(wxCommandEvent& event) wxOVERRIDE;
		void OnClose(wxCloseEvent& event) wxOVERRIDE;
		void OnSize(wxSizeEvent& event);
		void OnSaveAndTranslating( wxCommandEvent& event );
		void OnFinishTranslating(wxThreadEvent& event);
		virtual wxThread::ExitCode Entry();
};
