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

