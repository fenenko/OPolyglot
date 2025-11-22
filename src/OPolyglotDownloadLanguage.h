/* $Id: OPolyglotDownloadLanguage.h,v 1.9 2025/11/17 09:20:30 oleksandr Exp oleksandr $ */
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
#define OPOLYGLOT_DOWNLOAD_LANGUAGE_H_VERSION	"$Id: OPolyglotDownloadLanguage.h,v 1.9 2025/11/17 09:20:30 oleksandr Exp oleksandr $"


//WX_DECLRARE_ARRAY_PTR(wxXmlNode *,ArrayUlrXml);
WX_DEFINE_ARRAY_PTR(wxXmlNode *,ArrayXmlNode);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_START_DOWNLOAD, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_FAILED_DOWNLOAD_LANGUAGE, wxThreadEvent);

class OPolyglotDownloadLanguage : public GUIOPolyglotDownloadLanguage 
								  /*, public wxThreadHelper*/
{
	public:
		OPolyglotDownloadLanguage(wxWindow *parent);
		~OPolyglotDownloadLanguage();
		void OnFailedDownloadLanguage(wxThreadEvent& event);
		void OnStartDownload(wxCommandEvent& event);
		void OnStartDownloadFile(wxThreadEvent &event);
		void OnFileDownload(wxWebRequestEvent& event);
		void OnFileData(wxWebRequestEvent& event);
		void OnTimerProgressUpdate(wxTimerEvent &event);
		wxWebRequest CreateRequest(wxString url);
	private:
		//wxArrayString filesUrl;
		void ScanLangs();
#if 0
		virtual wxThread::ExitCode	Entry();
#endif
		wxWindow *parent;
		//wxArrayString *languages;
		wxWebRequest 	fileRequest;
		wxMutex 		mutexFileRequest;
		wxMemoryBuffer 	*dataReceiv;
		int 			progressReceived = 0; /* 0 ... 1000 */
		wxString		messageProgress =wxEmptyString;
		wxStopWatch		timeDownload;
		ArrayXmlNode  urlsXML;
		wxXmlDocument doc;
		wxArrayString idListLanguage;
#if 0
		wxString 		urlFile;
#endif
		wxProgressDialog	*progress = nullptr;
		wxTimer 	*timeUpdate = nullptr;
#if 0
		wxSemaphore	downloadFinish;
		wxMessageQueue<wxString> downloadMessage;
		wxXmlNode *currentNode;
		wxMutex mutexProgress;
#endif
		long timeStartDownload;
};

