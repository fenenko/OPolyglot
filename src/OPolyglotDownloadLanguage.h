#pragma once

#include "GuiOPolyglot.h"
#include "OPolyglotType.h"
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
		void OnStartDownload(wxCommandEvent& event);
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
		ArrayXmlNode  urlsXML;
		wxXmlDocument document;
		wxArrayString idListLanguage;
		wxArrayString	listLanguages;
		OPolyglotProgressInstallLanguage *progress = NULL;
};

