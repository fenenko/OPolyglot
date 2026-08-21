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
#include <wx/timer.h>
#include <wx/progdlg.h>
#include <wx/thread.h>
#include <wx/msgqueue.h>
#include <wx/xml/xml.h>
#include <wx/dynarray.h>
#include <atomic>
#define OPOLYGLOT_ID_ALL	0


class OPolyglotInstallLanguages : public GUIOPolyglotInstallLanguages , public wxThreadHelper
{
	private:
		size_t sizeToDownload;
		size_t sizeFile;
		size_t countFiles;
		size_t downloadedFilesCount;
		size_t downloadedBytes;
		size_t downloadedBytesFile;
		size_t downloadSpeed = 0;
		wxString unpackFile;
		wxString	nameFileDownload;
		bool sendFinishThread = true;
		wxMessageQueue<size_t> msgDownloadSpeed;

		unsigned char *ptrCertBlob = nullptr;
		wxWindow *parent;
	protected:
		virtual wxThread::ExitCode	Entry();
		void OnCancel( wxCommandEvent& event ) wxOVERRIDE; 
		void OnClose( wxCloseEvent& event ) wxOVERRIDE; 
		void OnUpdateProgress(wxTimerEvent &event);
		void OnReceivData(wxThreadEvent& event);
		wxTimer timerUpdateProgress;
		wxStopWatch timeRun;
		wxMutex mutex;
		wxXmlDocument xmlLanguages;
		wxXmlDocument urlsXML;
		wxString messageError;
	public:

		wxMessageQueue<bool> msgCancel;
		static bool CreateXmlLanguages(wxString& messageError,wxArrayString& labelLanguages,wxXmlDocument &xmlLanguages);
		static bool RemoveLanguage(wxString& messageError,wxXmlDocument& xmlLanguages,int id);
		OPolyglotInstallLanguages(wxWindow *parent,wxXmlDocument& xmlLanguages,int id);
		~OPolyglotInstallLanguages();

};

class OPolyglotDownloadLanguage : public GUIOPolyglotDownloadLanguage 
{
	public:
		OPolyglotDownloadLanguage(wxEvtHandler *handler);
		~OPolyglotDownloadLanguage();
		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnLanguageDownload(wxCommandEvent& event);
		void OnLanguageRemove(wxCommandEvent& event);
		void OnLanguagesDownloadAll(wxCommandEvent& event);
		void OnLanguagesRemoveAll(wxCommandEvent& event);
		void OnDownloadFinish(wxThreadEvent& event);
	private:
		void ScanLangs();
		wxEvtHandler *handler;
		wxXmlDocument xmlLanguages;
		OPolyglotInstallLanguages *progress = nullptr;
};

