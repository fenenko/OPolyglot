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


WX_DEFINE_ARRAY_PTR(wxXmlNode *,ArrayXmlNode);

class OPolyglotProgressInstallLanguage : public GUIOPolyglotProgressInstallLanguage
{
	protected:

		void OnCancel( wxCommandEvent& event ) wxOVERIDE; 
		void OnUpdateProgress(wxTimerEvent &event);
		wxTimer timerUpdate;
		wxStopWatch timeRun;
		wxMutex mutex;
	public:
		OPolyglotProgressInstallLanguage(wxWindow *parent,size_t countFiles);
		~OPolyglotProgressInstallLanguage();
		void SetFileProgress(size_t download,size_t AllSize);

};

class OPolyglotDownloadLanguage : public GUIOPolyglotDownloadLanguage 
{
	public:
		OPolyglotDownloadLanguage(wxWindow *parent);
		~OPolyglotDownloadLanguage();
		void OnStartDownload(wxCommandEvent& event);
		void OnFileDownload(wxWebRequestEvent& event);
		void OnDataDownload(wxWebRequestEvent& event);
		void OnTimerProgressUpdate(wxTimerEvent &event);
		wxWebRequest CreateRequest(wxString url);
	private:
		void ScanLangs();
		wxWindow *parent;
		wxWebRequest 	fileRequest;
		wxMutex 		mutexFileRequest;
		wxMemoryBuffer 	*dataReceiv;
		int 			progressReceived = 0; /* 0 ... 1000 */
		wxString		messageProgress =wxEmptyString;
		wxStopWatch		timeDownload;
		ArrayXmlNode  urlsXML;
		wxXmlDocument document;
		wxArrayString idListLanguage;
		wxProgressDialog	*progress = nullptr;
		wxTimer 	*timeUpdate = nullptr;
		long timeStartDownload;
};

