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
	private:
		size_t countFiles;
		size_t downloadedFiles;
		size_t downloadedBytes;
		size_t prevSizeDownload;
		double progressDownloaded;	/* 0 - 1.0 */
		wxWindow *parent;
	protected:

		void OnCancel( wxCommandEvent& event ) wxOVERRIDE; 
		void OnUpdateProgress(wxTimerEvent &event);
		wxTimer timerUpdate;
		wxStopWatch timeRun;
		wxMutex mutex;
	public:
		OPolyglotProgressInstallLanguage(wxWindow *parent,size_t countFiles);
		~OPolyglotProgressInstallLanguage();
		void SetDownloadProgress(size_t download,size_t AllSize);
		void FinishDownloadFile();

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
		void OnCancelUser(wxThreadEvent &event);
		wxWebRequest CreateRequest(wxString url);
	private:
		void ScanLangs();
		wxWindow *parent;
		wxWebRequest 	fileRequest;
		wxMutex 		mutexFileRequest;
		wxMemoryBuffer 	*dataReceiv;
		wxStopWatch		timeDownload;
		ArrayXmlNode  urlsXML;
		wxXmlDocument document;
		wxArrayString idListLanguage;
		OPolyglotProgressInstallLanguage *progress = NULL;
};

