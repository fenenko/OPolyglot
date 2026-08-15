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
#include <wx/timer.h>
#include <wx/dcscreen.h>
#include <wx/taskbar.h>
#include "GuiOPolyglot.h"
#include "Utils.h"
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotFullscreenFrame.h"
#include "OPolyglotViewTextTranslate.h"
#include <wx/dynarray.h>
#include <wx/dynlib.h>
#include <wx/thread.h>
#include "OPolyglotDocument.h"




class OPolyglotTranslator : public GUIOPolyglotTranslator , public wxThreadHelper
{
	private:
		wxWindow *parent;
		wxTimer *startTranslation;
	protected:
		void OnClose(wxCloseEvent& event) wxOVERRIDE;
		void OnRechange(wxCommandEvent& event) wxOVERRIDE;
		void OnLanguageFrom( wxCommandEvent& event ) wxOVERRIDE;
		void OnLanguageTo( wxCommandEvent& event ) wxOVERRIDE;
		virtual wxThread::ExitCode Entry();
		void OnThreadTranslatorFinish(wxThreadEvent& event);
		void OnTextSource( wxCommandEvent& event ) wxOVERRIDE;
		void OnStartTranslator(wxTimerEvent& event) ;
		void OnCopy(wxCommandEvent& event )wxOVERRIDE;
	public:
		wxArrayString configsTranslator;
		OPolyglotTranslator(wxWindow* parent,wxString languageFrom,wxString languageTo);
		~OPolyglotTranslator();
};

class OPolyglot : public GuiOPolyglot , public wxThreadHelper 
{
	public:
		OPolyglot(wxEvtHandler *handler);
		~OPolyglot();
		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnFinishSetupLanguages(wxThreadEvent &event);
		void OnStartThreadTranslator(wxThreadEvent &event);
		void OnFinishThreadTranslator(wxThreadEvent &event);
		void OnScreenshot(wxThreadEvent &event);
		void OnSelectLanguageFrom( wxCommandEvent& event ) wxOVERRIDE;
		void OnSelectLanguageTo( wxCommandEvent& event ) wxOVERRIDE;
		void OnShowTranslation( wxCommandEvent& event ) wxOVERRIDE;
		void OnCaptureScreen(wxCommandEvent& event) wxOVERRIDE;
		void ScanLanguageFrom();
		void ScanLanguageTo();
		void SetShow(bool flag);
		void OnCopyTextTranslate( wxCommandEvent& event ) ;
		void OnMenuSetup( wxCommandEvent& event )wxOVERRIDE;		
		void OnMenuAbout( wxCommandEvent& event )wxOVERRIDE;
		void OnOpenTranslator( wxCommandEvent& event ) wxOVERRIDE;
		void OnDocumentTranslator( wxCommandEvent& event ) wxOVERRIDE;
		void OnCloseDocumentTranslator(wxThreadEvent& event);
		void OnCloseTranslator(wxThreadEvent& event);
	protected:
		virtual wxThread::ExitCode Entry();
	private:
		wxEvtHandler *handler;
		OPolyglotDialogProgress *progress;
		wxString		messageProgressThreadTranslation;
		wxMutex 		mutexProgressThreadTranslation;
		OPolyglotDownloadLanguage	*frameDownload = NULL;
		wxString textForTranslate;
		wxArrayString codeTranslateLanguageFrom;
		wxString valueXML;
		bool flagShow = true;
		bool flagCreateScreenshotOnlyPortal = true;
		OPolyglotViewTextTranslate *viewTextTranslate;
		OPolyglotFullscreenFrame *fullscreen = nullptr;
		OPolyglotTranslator	*frameTranslator = NULL;
};

