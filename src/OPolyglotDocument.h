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
#include "OPolyglotViewTextTranslate.h"
#include "Utils.h"
#include <wx/dynarray.h>

class OPolyglotDocument : public GUIOPolyglotDocumentView ,wxThreadHelper
{
	public:
		OPolyglotDocument(wxEvtHandler *handler,wxString file,wxString languageFrom,wxString languageTo);
		~OPolyglotDocument();
		void ScanLanguageFrom();
		void ScanLanguageTo();
		
	private:
		virtual wxThread::ExitCode Entry();
		OPolyglotDialogProgress *progress = NULL;
		OPolyglotViewTextTranslate *viewTextTranslate = nullptr;
		wxBitmap pageDocument;
		int selectedPage;
		int startVSelectedPage;
		int startHSelectedPage;
		int startX;
		int startY;
		int pageStartX;
		int pageStartY;
		int pageNumber;
		int pageWidth;
		int pageHeight;
		int pageDocumentZoomMul;
		int pageDocumentZoomDiv;
		void OnPaint(wxPaintEvent& event);
		void OnVScroll(wxScrollEvent& event);
		void OnHScroll(wxScrollEvent& event);
		void OnMotion( wxMouseEvent& event );
		void OnMouseWheel(wxMouseEvent& event);
		void OnSetCurrentPage( wxCommandEvent& event );
		void OnRenderPage( wxSizeEvent& event ) ;						//
		void OnLeftDown( wxMouseEvent& event );					//start draw box from translate
		void OnStartTranslate( wxMouseEvent& event );				 	//using for end box from translate and start translate
		void OnSelectLanguageFrom( wxCommandEvent& event );
		void OnSelectLanguageTo( wxCommandEvent& event );
		void OnFinishThreadTranslator(wxThreadEvent &event);
		void OnShowTranslation( wxCommandEvent& event ) wxOVERRIDE;
		void OnDocumentZoom( wxCommandEvent& event ) wxOVERRIDE;
		wxEvtHandler *handler;
		wxString fileName;
		void  *pdfDoc = nullptr;
		uint8_t *buffBitmapDocument300;
		int heightBitmapDocument300;
		int widthBitmapDocument300;
};
