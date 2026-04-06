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
#include <wx/arrstr.h>
#include <wx/webrequest.h>
#include <wx/timer.h>
#include <wx/progdlg.h>
#include <wx/thread.h>
#include <wx/msgqueue.h>
#include <wx/xml/xml.h>
#include <wx/dynarray.h>



class OPolyglotProgressInstallLanguage : public GUIOPolyglotProgressInstallLanguage
{
	private:
		size_t sizeToDownload;
		size_t downloadedBytes;
		size_t prevSizeDownload;
		wxWindow *parent;
	protected:

		void OnCancel( wxCommandEvent& event ) wxOVERRIDE; 
		void OnClose( wxCloseEvent& event ) wxOVERRIDE; 
		void OnUpdateProgress(wxTimerEvent &event);
		wxTimer timerUpdate;
		wxStopWatch timeRun;
		wxMutex mutex;
	public:
		OPolyglotProgressInstallLanguage(wxWindow *parent,size_t sizeToDownload);
		~OPolyglotProgressInstallLanguage();
		void SetDownloadProgress(size_t download,size_t AllSize);
		void FinishDownloadFile();

};

class OPolyglotDownloadLanguage : public GUIOPolyglotDownloadLanguage 
{
	public:
		OPolyglotDownloadLanguage(wxEvtHandler *handler);
		~OPolyglotDownloadLanguage();
		void OnApply(wxCommandEvent& event) wxOVERRIDE;
		void OnFileDownload(wxWebRequestEvent& event);
		void OnDataDownload(wxWebRequestEvent& event);
		void OnTimerProgressUpdate(wxTimerEvent &event);
		void OnCancelUser(wxThreadEvent &event);
		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		wxWebRequest CreateRequest(wxString url);
	private:
		void ScanLangs();
		wxEvtHandler *handler;
		wxWebRequest 	fileRequest;
		wxMutex 		mutexFileRequest;
		wxMemoryBuffer 	*dataReceiv;
		wxStopWatch		timeDownload;
		wxXmlNode 		*urlsXML = NULL;
		wxXmlDocument document;
		wxXmlNode	*xmlLanguages;
		OPolyglotProgressInstallLanguage *progress = NULL;
};

