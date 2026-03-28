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
#include "OPolyglotType.h"
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotFullscreenFrame.h"
#include "OPolyglotThread.h"
#include <wx/dynarray.h>
#include <wx/dynlib.h>

/*
 * SetString(file for OCR)
 * SetString(wxEmptyString) not select aread
 */


class OPolyglotProgress : public GUIOPolyglotProgressOCRTranslator
{
	protected:
		wxWindow *parent;
		void OnUpdateProgress(wxTimerEvent &event);
		void OnCancel( wxCommandEvent& event) wxOVERRIDE;
		wxTimer timerUpdate;
		wxMutex mutex;
	public:
		OPolyglotProgress(wxWindow *parent);
		~OPolyglotProgress();
		void Finish();
};

#if 1
class OPolyglotViewTextTranslate : public GUIOPolyglotViewTextTranslate
{
	protected:
		wxWindow *parent;
		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnCopy( wxCommandEvent& event ) wxOVERRIDE;
		void OnExit( wxCommandEvent& event ) wxOVERRIDE;
	public:
		OPolyglotViewTextTranslate(wxWindow *parent);
		~OPolyglotViewTextTranslate();
		bool LoadXML(wxString xml);
};
#endif


class OPolyglot : public GuiOPolyglot 
{
	public:
		OPolyglot(wxEvtHandler *handler);
		~OPolyglot();
		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnFinishSetupLanguages(wxThreadEvent &event);
		void OnExitThreadTranslation(wxThreadEvent &event);
		void OnOCRFinish(wxThreadEvent &event);
		void OnCancelTranslation(wxThreadEvent &event);
		void OnCancelOCR(wxThreadEvent &event);
		void OnStartOCR(wxThreadEvent &event);
		void OnScreenshot(wxThreadEvent &event);
		void OnSelectLanguageFrom( wxCommandEvent& event ) wxOVERRIDE;
		void OnSelectLanguageTo( wxCommandEvent& event ) wxOVERRIDE;
		void OnCaptureScreen(wxCommandEvent& event) wxOVERRIDE;
		void ScanLanguageFrom();
		void ScanLanguageTo();
		void SetShow(bool flag);
		//void FinishThread();
		void OnCopyTextTranslate( wxCommandEvent& event ) ;
		//void AddOrSetOriginalText(wxString text);
		//void StartTranslation();
		//void StartThreadTranslation();
		void OnMenuSetup( wxCommandEvent& event )wxOVERRIDE;		
		void OnMenuAbout( wxCommandEvent& event )wxOVERRIDE;
	protected:
	private:
		wxEvtHandler *handler;
		//wxProgressDialog *progressThreadTranslation;
		OPolyglotProgress *progress;
		wxString		messageProgressThreadTranslation;
		wxMutex 		mutexProgressThreadTranslation;
		OPolyglotThreadTranslator	*threadTranslator = NULL;
		OPolyglotThreadOCR			*threadOCR = NULL;
		OPolyglotDownloadLanguage	*frameDownload = NULL;
		wxString textForTranslate;
		wxArrayString codeTranslateLanguageFrom;
		bool flagShow = true;
		OPolyglotViewTextTranslate *viewTextTranslate;
		OPolyglotFullscreenFrame *fullscreen = nullptr;
};

