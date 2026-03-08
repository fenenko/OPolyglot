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


class OPolyglot : public GuiOPolyglot 
{
	public:
		OPolyglot(wxEvtHandler *handler);
		~OPolyglot();
		void OnEnableClipboard( wxCommandEvent& event ) wxOVERRIDE;
		void OnShowTranslate( wxCommandEvent& event ) wxOVERRIDE;
		void OnShowOriginal(wxCommandEvent& event) wxOVERRIDE;
		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnTimeCheckClipboard(wxTimerEvent &event);
		void OnTimeCheckMouseState(wxTimerEvent &event);
		void OnOCRTranslate( wxCommandEvent& event ) wxOVERRIDE;
		void OnPaint(wxPaintEvent &event);
		void OnReceivImage(wxThreadEvent &event);
		void OnFinishSetupLanguages(wxThreadEvent &event);
		void OnRightClick(wxMouseEvent &event);
		void OnExitThreadTranslation(wxThreadEvent &event);
		void OnExitThreadOCR(wxThreadEvent &event);
		void OnCancelTranslation(wxThreadEvent &event);
		void OnCancelOCR(wxThreadEvent &event);
		void OnSelectLanguageFrom( wxCommandEvent& event ) wxOVERRIDE;
		void OnSelectLanguageTo( wxCommandEvent& event ) wxOVERRIDE;
		void OnStartTranslate(wxCommandEvent& event) wxOVERRIDE;
		void OnSize( wxSizeEvent& event ) wxOVERRIDE; 
		//void OnSize(wxSizeEvent& event) wxOVERRIDE;
		void ScanLangs();
		void ScanLanguageFrom();
		void ScanLanguageTo();
		void SetShow(bool flag);
		void FinishThread();
		void OnCopyTextTranslate( wxCommandEvent& event ) ;
		void AddOrSetOriginalText(wxString text);
		void StartTranslation();
		void StartThreadTranslation();
		void CreateTranslatorConfig();
		void OnMenuSetup( wxCommandEvent& event )wxOVERRIDE;		
		void OnMenuAbout( wxCommandEvent& event )wxOVERRIDE;
	protected:
	private:
		wxEvtHandler *handler;
		wxTimer			*timerClipboardChecking;
		wxTimer			*timerMouseState;
		bool 			mouseLeftButtonPressed;
		//wxProgressDialog *progressThreadTranslation;
		OPolyglotProgress *progress;
		wxString		messageProgressThreadTranslation;
		wxMutex 		mutexProgressThreadTranslation;
		OPolyglotThreadTranslator	*threadTranslator = NULL;
		OPolyglotThreadOCR			*threadOCR = NULL;
		OPolyglotDownloadLanguage	*frameDownload = NULL;
		int coordStartX;
		int coordStartY;
		int countLeftPress;
		wxString lastClipboardText;
		bool viewDialogTranslator;
		wxString textForTranslate;
		wxArrayString codeTranslateLanguageFrom;
		bool flagShow = true;

		OPolyglotFullscreenFrame *fullscreen = nullptr;
		wxArrayString configTranslatorFileYml;
		wxArrayString installLanguageFrom;
		wxArrayString installLanguageTo;
		wxArrayString installCodeTranslator;
		wxArrayString preProcessingRegex;
		wxArrayString preProcessingReplace;
		wxArrayString postProcessingRegex;
		wxArrayString postProcessingReplace;
		OPolyglotImage	*imageForOCR;
};

