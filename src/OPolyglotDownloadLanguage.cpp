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


#include <memory>
#include <wx/xml/xml.h>
#include <wx/display.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/mstream.h>
#include "OPolyglotDownloadLanguage.h"
#include "Utils.h"
#include <curl/curl.h>
#ifndef __WXMSW__
#include "../res/icon.xpm"
#endif
#include "OPolyglotEvent.h"
#include <openssl/evp.h>
#include <openssl/err.h>

#define OPOLYGLOT_TIMEOUT_START_DOWNLOAD		120 /* in second */
#define OPOLYGLOT_FILE_FROM_STRING(NAME)		wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,NAME)
#define OPOLYGLOT_DIR_FROM_STRING(NAME)			wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,NAME)
#define OPOLYGLOT_MESSAGE_DOWNLOAD(FILES_LEFT)	wxString::Format(wxT("%s %zu"),_("download new languages, files left "),FILES_LEFT)
#define OPOLYGLOT_MESSAGE_DOWNLOAD_AND_SPEAD(FILES_LEFT,SPEED_IN_FLOAT_KBYTES,PREFIX)	\
		wxString::Format(wxT("%s %zu %s %.1f %s") \
			,_("download new languages, files left ") \
			,FILES_LEFT \
			,_(" speed ") \
			,SPEED_IN_FLOAT_KBYTES \
			,PREFIX)
#define OPOLYGLOT_USER_AGENT	wxString::Format(wxT("OPolyglot offline translator %s"), wxGetOsDescription())
enum{
	TIMER_ID,
};

enum {OPOLYGLOT_RET_SUCCESS=0,OPOLYGLOT_RET_ERROR=1,OPOLYGLOT_RET_CRITICAL_ERROR=-1};


enum {OPOLYGLOT_PARAMETER_FILENAME=0,
	OPOLYGLOT_PARAMETER_FILESIZE,
	OPOLYGLOT_PARAMETER_FILE_DOWNLOADEDBYTES,
	OPOLYGLOT_PARAMET_FILE_UNPACK};


#include <wx/arrimpl.cpp> 



wxString convertSizeToLabelHuman(size_t size)
{
	wxString ret = wxEmptyString;
	double value = (double)size;
	ret = wxString::Format(wxS("%zu %s"),size,_("B    "));
	if(1024.0 < value)	
	{
		value = value/1024.0;
		ret = wxString::Format(wxS("%.1f %s"),value,_("KB   "));
		if(1024.0 < value)
		{
			value = value/1024;
			ret = wxString::Format(wxS("%.1f %s"),value,_("MB   "));
			if(1024.0 < value)
			{
				value = value/1024;
				ret = wxString::Format(wxS("%.1f %s"),value,_("GB   "));
			}
		}
	}
	return ret;
}

OPolyglotInstallLanguages::OPolyglotInstallLanguages(wxWindow *parent,wxXmlDocument& xmlLanguages, int id) : GUIOPolyglotInstallLanguages(nullptr)
{
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages"));
	this->SetTitle(wxString::Format(wxT("OPolyglot %s"),_("install languages")));
	OPOLYGLOT_MESSAGE(wxT("MESSAGE TEST"));
	this->parent = parent;
	downloadedBytes = 0;
	downloadedFilesCount = -1;
	sizeToDownload = 0;
	downloadedBytesFile = 0;
	sizeFile = 0;
	countFiles = 0;
	wxArrayString idsInstalled;
	wxArrayString idsToInstall;
	unpackFile = wxEmptyString;
	urlsXML.SetRoot(new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Urls")));
	wxXmlDocument doc;
	this->xmlLanguages = xmlLanguages;
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages failed to load the XML file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this
				,wxString::Format(wxS("%s: %s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE)
				,this->GetTitle(),wxICON_ERROR|wxOK);
		msg.ShowModal();
		wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
	}
	for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child=child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Installed")))
		{
			for(wxXmlNode *childId = child->GetChildren();childId;childId=childId->GetNext())
			{
				if(childId->GetName().IsSameAs(wxS("IdInstalled")))
				{
					if(idsInstalled.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND)
					{
						idsInstalled.Add(childId->GetAttribute(wxS("id")));
					}
				}
			}
		}
	}
	for(wxXmlNode *child = this->xmlLanguages.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Label")))
		{
			if(child->GetAttribute(wxS("idButton")).IsSameAs(wxString::Format("%d",id ))||(id == OPOLYGLOT_ID_ALL))
			{
				for(wxXmlNode *childId = child->GetChildren();childId;childId = childId->GetNext())
				{
					if(childId->GetName().IsSameAs(wxT("Id"))&&(idsInstalled.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND))
					{
						if(idsToInstall.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND)
						{
							idsToInstall.Add(childId->GetAttribute(wxS("id")));
						}
					}
				}
			}
			if(child->GetAttribute(wxS("label")).IsSameAs(_("English"))&&(child->GetAttribute(wxS("flagInstalled")).IsEmpty()))
			{
				OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages translate English <-> LocaleLanguage not installed add to install"));
				for(wxXmlNode *childId = child->GetChildren();childId;childId = childId->GetNext())
				{
					if(childId->GetName().IsSameAs(wxT("Id"))&&(idsInstalled.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND))
					{
						if(idsToInstall.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND)
						{
							idsToInstall.Add(childId->GetAttribute(wxS("id")));
						}
					}
				}
			}
		}
	}
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Url")))
		{
			if(idsToInstall.Index(child->GetAttribute(wxS("id"))) != wxNOT_FOUND)
			{
				idsToInstall.Remove(child->GetAttribute(wxS("id")));
				urlsXML.GetRoot()->AddChild(new wxXmlNode(*child));
				countFiles++;
				long size;
				if(child->GetAttribute(wxS("size")).ToLong(&size,10))
				{
					sizeToDownload += size;
				} else
				{
					OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages not converted size \"Url\" for %d"),child->GetAttribute(wxS("id")));
				}
			} 
		}
	}
	AllProgress->SetToolTip(wxString::Format(wxT("%s 0:%zu"),_("Total progress"),countFiles));
	this->SizeAll->SetLabel(convertSizeToLabelHuman(sizeToDownload));
	timerUpdateProgress.SetOwner(this,wxID_ANY);
	this->Bind(wxEVT_TIMER,&OPolyglotInstallLanguages::OnUpdateProgress,this,timerUpdateProgress.GetId());
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SEND_DATA,&OPolyglotInstallLanguages::OnReceivData,this);
	timeRun.Start();
	timerUpdateProgress.Start(750);
	Show();
	if(CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages could not create the worker thread"));
		wxMessageDialog msg(this,_("Could not create the worker thread!"),this->GetTitle(),wxICON_ERROR|wxOK);
		msg.ShowModal();
		wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
		return;
	}
	if(GetThread()->Run() != wxTHREAD_NO_ERROR)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages could not run the worker thread!"));
		wxMessageDialog msg(this,_("Could not run the worker thread!"),this->GetTitle(),wxICON_ERROR|wxOK);
		msg.ShowModal();
		wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
		return;
	}
#if __WXMSW__
	int w,h,t;
	this->GetSize(&w,&h);
	this->Fit();
	this->GetSize(&t,&h);
	this->SetSize(w,h);
	this->Refresh();
#endif
#if __WXGTK__
	this->CallAfter([this](){
			int w,h;
			this->GetSize(&w,&h);
			this->Fit();
			wxSize s = this->GetBestSize();
			s.SetWidth(w);
			OPOLYGLOT_INFO(wxT("OPolyglotInstallLanguages %dx%d"),s.GetWidth(),s.GetHeight());
			this->SetMinSize(s);
	});
#endif
	
}

void OPolyglotInstallLanguages::OnReceivData(wxThreadEvent& event)
{
	unsigned long tempValue;
	//OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::OnReceivData(%d)"),event.GetInt());
	switch(event.GetInt())
	{
		case OPOLYGLOT_PARAMETER_FILENAME:
			downloadedBytes += downloadedBytesFile;
			downloadedBytesFile = 0;
			downloadedFilesCount += 1;
			if(msgDownloadSpeed.Post(downloadSpeed)!= wxMSGQUEUE_NO_ERROR)
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::OnReceivData error post downloadSpeed"));
			}
			AllProgress->SetValue((int)((downloadedBytes*(this->AllProgress->GetRange()))/sizeToDownload));
			AllProgress->SetToolTip(wxString::Format(wxT("%s %zu:%zu"),_("Total progress"),downloadedFilesCount,countFiles));
			SizeAll->SetLabel(convertSizeToLabelHuman(sizeToDownload-downloadedBytes));
			FileProgress->SetToolTip(event.GetString());
			unpackFile = wxEmptyString;
			break;
		case OPOLYGLOT_PARAMETER_FILESIZE:
			if(!event.GetString().ToULong(&tempValue,10))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::OnReceivData(%d) Conversion from wxString(%s) to unsigned long failed.")
						,event.GetInt(),event.GetString());
				tempValue = -1;
			}
			downloadedBytesFile = 0;
			sizeFile = tempValue;
			unpackFile = wxEmptyString;
			break;
		case OPOLYGLOT_PARAMETER_FILE_DOWNLOADEDBYTES:
			downloadedBytesFile = event.GetExtraLong();
			break;
		case OPOLYGLOT_PARAMET_FILE_UNPACK:
			unpackFile = event.GetString();
			break;
		default:
			OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::OnReceivData(%d) Unknown state in case statement."),event.GetInt());
			break;
	}
}

static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	wxMemoryBuffer *mb = static_cast<wxMemoryBuffer*>(userp);
	size_t lastDataLen = mb->GetDataLen();
	try{
		mb->AppendData(contents,size*nmemb);
	}catch(const std::bad_alloc& e)
	{
		OPOLYGLOT_ERROR(wxT("CurlWriteCallback Critical error: Out of memory during download."));
		return 0;
	}
	if((mb->GetDataLen())!=(lastDataLen+size*nmemb))
	{
		OPOLYGLOT_ERROR(wxT("CurlWriteCallback not AppendData"));
		return 0;
	}
	return size*nmemb;
}

static int CurlProgressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	bool cancel;
	OPolyglotInstallLanguages *downloader = static_cast<OPolyglotInstallLanguages*>(clientp);
	if((downloader->msgCancel.ReceiveTimeout(10,cancel) == wxMSGQUEUE_TIMEOUT ))
	{
		cancel = false;
	}
	if(cancel)
	{
		return 1;
	}
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SEND_DATA);
	event->SetInt(OPOLYGLOT_PARAMETER_FILE_DOWNLOADEDBYTES);
	event->SetExtraLong(dlnow);
	wxQueueEvent(downloader,event);
	return 0;
}

wxThread::ExitCode OPolyglotInstallLanguages::Entry()
{
	char curl_errbuf[4096];
	bool cancel = false;
	int downloadRetries = 0;
	curl_blob certBlob;
	wxThreadEvent *event = nullptr;
	long reconnectTimeout = 100;
	bool flagResumeDownload = true;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::Entry"));
	wxXmlDocument document;
	wxXmlNode 		 *nodeInstalled = nullptr;
	if(!document.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry not load %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxS("%s: %s"),_("not load file"),OPOLYGLOT_GET_XML_DATA_FILE),this->GetTitle(),wxICON_ERROR|wxOK);
		msg.ShowModal();
		wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
		return (wxThread::ExitCode)0;
	}
	for(wxXmlNode *child = document.GetRoot()->GetChildren();child&&(IS_NULLPTR(nodeInstalled));child=child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Installed")))
		{
			nodeInstalled = child;
		}
	}
	wxString CERT = OPOLYGLOT_CERT_FILE_PATH;
	OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::Entry start read %s"),CERT);
	wxFileInputStream *fis = new wxFileInputStream(CERT);
	if (!fis->IsOk()) {
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry Unable to load cacert.pem %s"),CERT);
		delete fis;
		wxMessageDialog msg(this
				,wxString::Format(wxT("%s %s"),_("Unable to load"),CERT),this->GetTitle(),wxOK|wxICON_ERROR);
		msg.ShowModal();
		wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
		return (wxThread::ExitCode)0;
	}
	curl_global_init(CURL_GLOBAL_ALL);
	wxMemoryBuffer  *memBuf = new wxMemoryBuffer();
	ptrCertBlob = new unsigned char[fis->GetLength()+1];
	certBlob.data = static_cast<void *>(ptrCertBlob);
	certBlob.flags = CURL_BLOB_COPY;
	certBlob.len = fis->GetLength();
	fis->SeekI(0,wxFromStart);
	fis->ReadAll(certBlob.data,certBlob.len);
	ptrCertBlob[fis->GetLength()] = '\0';
	delete fis;
	OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::Entry finish read cacert.pem"));
	if(this->msgCancel.ReceiveTimeout(10,cancel) == wxMSGQUEUE_TIMEOUT )
	{
		cancel = false;
	}
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SEND_DATA);
	event->SetInt(OPOLYGLOT_PARAMETER_FILENAME);
	event->SetString(urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file")));
	wxQueueEvent(this,event);
	CURL *curl = nullptr; 
	while(urlsXML.GetRoot()->GetChildren()&&(!cancel))
	{
		size_t downloadSpeed;
		wxStopWatch downloadTime;
		if((msgDownloadSpeed.ReceiveTimeout(1000,downloadSpeed)!= wxMSGQUEUE_NO_ERROR))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry error receive downloadSpeed"));
			downloadSpeed = 0;
		}
		long limitSpeed = static_cast<long>(downloadSpeed)/5 + 10240L;
		OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::Entry start download file %s"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file")));
		OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::Entry limit speed %zu"),static_cast<size_t>(limitSpeed));
		if(!flagResumeDownload)
		{
			memBuf->Clear();
		}
		if(memBuf->GetDataLen() == 0)
		{
			event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SEND_DATA);
			event->SetInt(OPOLYGLOT_PARAMETER_FILESIZE);
			event->SetString(urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("size")));
			wxQueueEvent(this,event);
		}
		curl = curl_easy_init();
		if(!curl)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry Curl not initialized."));
			wxMessageDialog msg(this,_("Curl not initialized."),this->GetTitle(),wxICON_ERROR|wxOK);
			msg.ShowModal();
			cancel = true;
			curl_easy_cleanup(curl);
			curl = nullptr;
			continue;
		}
		OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::Entry configure curl"));
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
#if OPOLYGLOT_DEBUG_CURL_ENABLED==1
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
		curl_easy_setopt(curl, CURLOPT_URL, static_cast<const char *>(urlsXML.GetRoot()->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL).mb_str(wxConvUTF8)));
		curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &certBlob);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		if( 0 < memBuf->GetDataLen())
		{
			OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::Entry resume file download %s")
					,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file")));
			curl_easy_setopt(curl, CURLOPT_RESUME_FROM, static_cast<long>(memBuf->GetDataLen()));
		}
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, static_cast<const char*>(OPOLYGLOT_USER_AGENT.mb_str()));
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, memBuf);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, CurlProgressCallback);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
		curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, limitSpeed); 
		if(50000L < limitSpeed)
		{
			curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
		} else
		{
			curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 15L);
		}
		downloadTime.Start();
		CURLcode res = curl_easy_perform(curl);
		if(res == CURLE_OK)
		{
			
			bool zipOk = true;
			char *url = NULL;
			res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
		    if((res == CURLE_OK) && url) {
				downloadTime.Pause();
				size_t speed = static_cast<size_t>(memBuf->GetDataLen())*1000/downloadTime.Time()/1024;
				OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::Entry file %s, download speed %zu kB,mirror address %s")
						,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
						,speed
						,wxString(url));
			}
			curl_easy_cleanup(curl);
			curl = nullptr;
			reconnectTimeout = 100;
			downloadRetries = 0;
			if(!wxString::Format(wxS("%zu"),static_cast<size_t>(memBuf->GetDataLen()))
					.IsSameAs(urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("size"))))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry file(%s) sizes do not match %s!=%zu")
						,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
						,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("size"))
						,static_cast<size_t>(memBuf->GetDataLen()));
					wxMessageDialog msg(this
							,wxString::Format(wxS("%s(%s) %s %zu!=%s")
								,_("File")
								,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
								,_("sizes do not match")
								,static_cast<size_t>(memBuf->GetDataLen())
								,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("size")))
							,this->GetTitle()
							,wxICON_ERROR|wxOK);
				msg.ShowModal();
				continue;
			}
			event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SEND_DATA);
			event->SetInt(OPOLYGLOT_PARAMET_FILE_UNPACK);
			event->SetString(wxString::Format(wxT("%s %s")
						,_("extraction")
						,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))));
			wxQueueEvent(this,event);
			OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::Entry start extraction %s"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file")));
			wxMemoryInputStream *mis = new wxMemoryInputStream(memBuf->GetData(),memBuf->GetDataLen());
			{
				const size_t CHUNK_SIZE = 4096;
				unsigned char chunk[CHUNK_SIZE];
				wxString hexHash = wxEmptyString;
				EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
				if (mdctx == nullptr) 
				{

					OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry %s openssl error EVP_MD_CTX_new")
							,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file")));
					wxMessageDialog msg(this,wxString::Format(wxT("%s EVP_MD_CTX_new"),_("Error")),this->GetTitle(),wxICON_ERROR|wxOK);
					msg.ShowModal();
					cancel = true;
					delete mis;
					continue;
				}
				if (EVP_DigestInit_ex(mdctx, EVP_sha1(), nullptr) != 1) 
				{
				    EVP_MD_CTX_free(mdctx);
					OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry %s openssl error EVP_DigestInit_ex")
							,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file")));
					wxMessageDialog msg(this,wxString::Format(wxT("%s: openssl EVP_DigestInit_ex"),_("Error")),this->GetTitle(),wxICON_ERROR|wxOK);
					msg.ShowModal();
					cancel = true;
					delete mis;
					continue;
				}
				bool openssl_err = false;
				mis->SeekI(0,wxFromStart);
				while ((!mis->Eof()) && !openssl_err)
				{
				    mis->Read(chunk, CHUNK_SIZE);
				    size_t bytesRead = mis->LastRead(); 
	
    				if (bytesRead > 0)
				    {
				        if (EVP_DigestUpdate(mdctx, chunk, bytesRead) != 1)
					    {
            				char err_buf[1024];
				            ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));

				            OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry %s OpenSSL error SHA1 update %s")
                			    , urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
			                    , wxString::FromUTF8(err_buf));
            
            				wxMessageDialog msg(this, wxString::Format(wxT("%s OpenSSL SHA1 update \n%s"),_("Error"), wxString::FromUTF8(err_buf)), this->GetTitle(), wxICON_ERROR|wxOK);
				            msg.ShowModal();
				            cancel = true;
				            delete mis;
				            openssl_err = true;
				            continue; 
				        }
					}
				}
				if (!openssl_err) 
				{
					unsigned char hash[EVP_MAX_MD_SIZE];
					unsigned int hash_len = 0;
    				if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) 
					{
						char err_buf[1024];
						ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));

						OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry %s OpenSSL error EVP_DigestFinal_ex %s")
								, urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
								, wxString::FromUTF8(err_buf));
						
						EVP_MD_CTX_free(mdctx);
						wxMessageDialog msg(this, wxString::Format(wxT("%s OpenSSL EVP_DigestFinal_ex \n%s"),_("Error"), wxString::FromUTF8(err_buf)), this->GetTitle(), wxICON_ERROR|wxOK);
						msg.ShowModal();
						cancel = true;
						delete mis;
						openssl_err = true;
						continue; 
	    			} else 
					{
						wxString hexHash = wxEmptyString;
						EVP_MD_CTX_free(mdctx);
						for(size_t i =0; (i < hash_len);i++)
						{
							hexHash += wxString::Format(wxT("%02x"),hash[i]);
						}
						OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::Entry %s sha1sum %s"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("file")),hexHash);
						if((!cancel)&&(!urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("sha1sum")).IsSameAs(hexHash)))
						{
							OPOLYGLOT_WARNING(wxT("OPolyglotInstallLanguages::Entry sha1sum failed for file %s %s %s redownload")
									,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("file"))
									,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("sha1sum")),hexHash);
							memBuf->Clear();
							delete mis;
							continue;
						}
					}
				}
			}
			if((!cancel)&&zipOk)
			{
				mis->SeekI(0,wxFromStart);
				wxZipInputStream *zip = new wxZipInputStream(*mis);
				wxZipEntry *entry =zip->GetNextEntry();
				wxXmlNode *node = new wxXmlNode(nodeInstalled,wxXML_ELEMENT_NODE  ,static_cast<const wxString>(wxString("IdInstalled")));
				node->AddAttribute(wxS("id"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("id")));
				while((entry)&&(!cancel)&&zipOk)
				{
					if(zip->IsOk())
					{
						if(entry->IsDir())
						{
							wxString dirPath = wxString::Format(wxS("%s%c%s"),OPOLYGLOT_USER_DATA,wxFileName::GetPathSeparator(),entry->GetName());
							wxXmlNode *dir = new wxXmlNode(node,wxXML_ELEMENT_NODE,static_cast<const wxString>(wxString("DirCreated")));
							dir->AddAttribute(wxS("dir"),dirPath);
							if(!wxFileName::DirExists(dirPath))
							{
								if(!wxDir::Make(dirPath))
								{
									OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry cannot create directory %s"),dirPath);
									wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("cannot create dir"),dirPath),this->GetTitle(),wxICON_ERROR|wxOK);
									msg.ShowModal();
									cancel = true;
									zipOk = false;
								} 
							} else
							{
								OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::Entry dir exists %s/%s"),OPOLYGLOT_USER_DATA,entry->GetName());
							}

						} else
						{
							wxString fileName = wxString::Format(wxS("%s%c%s"),OPOLYGLOT_USER_DATA,wxFileName::GetPathSeparator(),entry->GetName());
							wxXmlNode *file = new wxXmlNode(node,wxXML_ELEMENT_NODE,static_cast<const wxString>(wxString("FileInstalled")));
							file->AddAttribute(wxS("file"),fileName);
							if(!wxFileName::FileExists(fileName))
							{
								wxFileOutputStream out(fileName);
								if(!out.IsOk())
								{
									OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry %s cannot create wxFileOutputStream(%s)")
											,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
											,fileName);
									wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("cannot create"),fileName),this->GetTitle(),wxICON_ERROR|wxOK);
									msg.ShowModal();
									cancel = true;
									zipOk = false;

								} else
								{
									zip->Read(out);
								}

							} else
							{
								OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::Entry file exist %s"),fileName);
							}
						}
					} else
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry bad zip file %s"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("file")));
						messageError = wxString::Format(wxS("%s %s"),_("bad zip file"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("file")));
						zipOk = false;
					}
					zip->CloseEntry();
					delete entry;
					entry = zip->GetNextEntry();
				}
				delete zip;

			}
			delete mis;

			memBuf->Clear();
			if(zipOk&&urlsXML.GetRoot()->GetChildren())
			{
				if(!document.Save(OPOLYGLOT_GET_XML_DATA_FILE))
				{
					OPOLYGLOT_ERROR(wxS("OPolyglotInstallLanguages::Entry %s error save file %s")
							,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
							,OPOLYGLOT_GET_XML_DATA_FILE);
					wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error save file"),OPOLYGLOT_GET_XML_DATA_FILE),this->GetTitle(),wxICON_ERROR|wxOK);
					msg.ShowModal();
					cancel = true;
				}
				urlsXML.GetRoot()->RemoveChild(urlsXML.GetRoot()->GetChildren());
				if(urlsXML.GetRoot()->GetChildren())
				{
					event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SEND_DATA);
					event->SetInt(OPOLYGLOT_PARAMETER_FILENAME);
					event->SetString(urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file")));
					wxQueueEvent(this,event);
				}
			}
		} else
		{
			char *url = NULL;
			wxString mirrorUrl =wxEmptyString;
		    if((curl_easy_getinfo(curl,CURLINFO_EFFECTIVE_URL,&url) == CURLE_OK) && url) {
				mirrorUrl = wxString(url);
			} else
			{
				mirrorUrl = wxS("not get url");
			}
			curl_easy_cleanup(curl);
			curl = nullptr;
			OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry %d File(%s) download error.\n\tError:\t%s\n\tError:\turl=%s\n\tError:\t%s")
					,res
					,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
					,wxString::FromUTF8(curl_easy_strerror(res))
					,mirrorUrl
					,wxString::FromUTF8(curl_errbuf));
			if((res == CURLE_RANGE_ERROR)||(res == CURLE_HTTP_RETURNED_ERROR))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry server not supported resume download"));
				flagResumeDownload = false;
				continue;
			}
			if(res == CURLE_WRITE_ERROR)
			{
				wxMessageDialog msg(this
						,wxString::Format(wxS("%s(%s).\n%s\n%s")
							,_("Error downloading the file")
							,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
							,wxString::FromUTF8(curl_easy_strerror(res))
							,urlsXML.GetRoot()->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL))
						,this->GetTitle()
						,wxICON_ERROR|wxOK);
				msg.ShowModal();
				cancel = true;
				continue;
			}
			if(res == CURLE_ABORTED_BY_CALLBACK)
			{
				cancel = true;
				continue;
			}
			downloadRetries++;
			if(9 < downloadRetries)
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::Entry Aborting %s download. Redownload count: %zu")
						,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
						,static_cast<size_t>(downloadRetries));
				wxMessageDialog msg(this
						,wxString::Format(wxS("%s(%s).\n%s\n%s")
							,_("Error downloading the file")
							,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
							,wxString::FromUTF8(curl_easy_strerror(res))
							,urlsXML.GetRoot()->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL))
						,this->GetTitle()
						,wxICON_ERROR|wxOK);
				msg.ShowModal();
				cancel = true;
				continue;
			}
			// Wait 30 seconds for the next download
			if((!cancel)&&(msgCancel.ReceiveTimeout(reconnectTimeout,cancel) == wxMSGQUEUE_TIMEOUT))
			{
				cancel = false;
			}
			reconnectTimeout += 10000;

		}
		if((!cancel)&&(this->msgCancel.ReceiveTimeout(100,cancel) == wxMSGQUEUE_TIMEOUT ))
		{
			cancel = false;
		}

	}
	if(!IS_NULLPTR(curl))
	{
		curl_easy_cleanup(curl);
		curl = nullptr;
	}
	delete memBuf;
	delete[] ptrCertBlob;
	ptrCertBlob = nullptr;
	curl_global_cleanup();
	if(sendFinishThread)
	{
		wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
	}
	return (wxThread::ExitCode)0;
}


OPolyglotInstallLanguages::~OPolyglotInstallLanguages()
{
	size_t second = 0;
	size_t minuten = 0;
	size_t hour = 0;

	timeRun.Pause();
	second = timeRun.Time()/1000;
	if(60 < second)
	{
		minuten = second/60;
		second = second%60;
	}
	if(60 < minuten)
	{
		hour = minuten/60;
		minuten = minuten%60;
	}

	OPOLYGLOT_MESSAGE(wxT("~OPolyglotProgressInstallLanguage download time %zu:%02zu:%02zu,avarage speed %zu kB"),hour,minuten,second,downloadSpeed/1024);
	if(GetThread()&&GetThread()->IsRunning())
	{
		sendFinishThread = false;
		msgCancel.Post(true);
		GetThread()->Wait();
	}

}


bool OPolyglotInstallLanguages::RemoveLanguage(wxString& messageError,wxXmlDocument& xmlLanguages,int id)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::RemoveLanguage %d"),id);
	wxArrayString idsNeeds;
	wxArrayString idsToRemove;
	wxArrayString filesNeed;
	wxArrayString dirsNeed;
	wxArrayString filesToRemove;
	wxArrayString dirsToRemove;
	wxXmlDocument doc;
	messageError = wxEmptyString;
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages failed to load the XML file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		messageError = wxString::Format(wxS("%s: %s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE);
		return false;
	}
	for(wxXmlNode *child=xmlLanguages.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Label")))	
		{
			if((child->GetAttribute(wxS("idButton")).IsSameAs(wxString::Format(wxT("%d"),id))&&(!child->GetAttribute(wxS("flagInstalled")).IsEmpty()))
					||((id == OPOLYGLOT_ID_ALL)&&(!child->GetAttribute(wxS("flagInstalled")).IsEmpty())))
			{
				child->DeleteAttribute(wxS("flagInstalled"));
				for(wxXmlNode *childId=child->GetChildren();childId;childId=childId->GetNext())
				{
					if(childId->GetName().IsSameAs(wxS("Id")))
					{
						if(idsToRemove.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND)
						{
							idsToRemove.Add(childId->GetAttribute(wxS("id")));
						}
					}
				}
			} else
			{
				if(!child->GetAttribute(wxS("flagInstalled")).IsEmpty())
				{
					for(wxXmlNode *childId = child->GetChildren();childId;childId = childId->GetNext())
					{
						if(childId->GetName().IsSameAs(wxS("Id")))
						{
							if(idsNeeds.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND)
							{
								idsNeeds.Add(childId->GetAttribute(wxS("id")));
								OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::RemoveLanguage %s need for %s"),childId->GetAttribute(wxS("id")),child->GetAttribute(wxS("label")));
							}
						}
					}
				}
			}
		}
	}
	for(wxXmlNode *childInstalled = doc.GetRoot()->GetChildren();childInstalled;childInstalled = childInstalled->GetNext())
	{
		if(childInstalled->GetName().IsSameAs(wxS("Installed")))
		{
			for(wxXmlNode *childIdInstalled = childInstalled->GetChildren();childIdInstalled;childIdInstalled=childIdInstalled->GetNext())
			{
				if(childIdInstalled->GetName().IsSameAs(wxS("IdInstalled")))
				{
					if(idsNeeds.Index(childIdInstalled->GetAttribute(wxS("id"))) != wxNOT_FOUND)
					{
						for(wxXmlNode *childId = childIdInstalled;childId;childId=childId->GetNext())
						{
							if(childId->GetName().IsSameAs(wxS("FileInstalled")))
							{
								if(filesNeed.Index(childId->GetAttribute(wxS("file"))) == wxNOT_FOUND)
								{
									filesNeed.Add(childId->GetAttribute(wxS("file")));
								}
							}
							if(childId->GetName().IsSameAs(wxS("DirCreated")))
							{
								if(dirsNeed.Index(childId->GetAttribute(wxS("dir"))) == wxNOT_FOUND)
								{
									dirsNeed.Add(childId->GetAttribute(wxS("dir")));
								}
							}
						}
					}

					if((idsToRemove.Index(childIdInstalled->GetAttribute(wxS("id"))) != wxNOT_FOUND)&&(idsNeeds.Index(childIdInstalled->GetAttribute(wxS("id"))) == wxNOT_FOUND))
					{
						for(wxXmlNode *child =childIdInstalled->GetChildren();child;child = child->GetNext())
						{
							if(child->GetName().IsSameAs(wxS("FileInstalled")))
							{
								if(filesToRemove.Index(child->GetAttribute(wxS("file"))) == wxNOT_FOUND)
								{
									filesToRemove.Add(child->GetAttribute(wxS("file")));
								}
							}
							if(child->GetName().IsSameAs(wxS("DirCreated")))
							{
								if(dirsToRemove.Index(child->GetAttribute(wxS("dir"))) == wxNOT_FOUND)
								{
									dirsToRemove.Add(child->GetAttribute(wxS("dir")));
								}
							}
						}
					}
				}
			}
		}
	}
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Installed")))
		{
			wxXmlNode *childId = child->GetChildren();
			while(childId)
			{
				if(childId->GetName().IsSameAs(wxS("IdInstalled")))
				{
					if((idsToRemove.Index(childId->GetAttribute(wxS("id"))) != wxNOT_FOUND)&&(idsNeeds.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND))
					{
						wxXmlNode *next = childId->GetNext();
						if(!child->RemoveChild(childId))
						{
							OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::RemoveLanguage Can't delete the tag \"IdInstalled\" with ID %s"),childId->GetAttribute(wxS("id")));
						}
						childId = next;
					} else
					{
						childId = childId->GetNext();
					}
				} else
				{
					OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::RemoveLanguage unknown tag for \"Installed\" %s"),childId->GetName());
					childId = childId->GetNext();
				}
			}
		}
	}
	if(!doc.Save(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxS("OPolyglotInstallLanguages::RemoveLanguage error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		return false;
	}
	for(size_t i =0; i < filesToRemove.GetCount();i++)
	{
		if(filesNeed.Index(filesToRemove.Item(i)) == wxNOT_FOUND)
		{
			OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::RemoveLanguage file %s to remove"),filesToRemove.Item(i));
			if(!wxRemoveFile(filesToRemove.Item(i)))
			{
				OPOLYGLOT_WARNING(wxT("OPolyglotInstallLanguages::RemoveLanguage can`t delete %s file"),filesToRemove.Item(i));
			}
		}
	}
	for(size_t i =0;i < dirsToRemove.GetCount();i++)
	{
		if(dirsNeed.Index(dirsToRemove.Item(i)) == wxNOT_FOUND)
		{
			OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::RemoveLanguage dir %s to remove"),dirsToRemove.Item(i));
			if(!wxRmdir(dirsToRemove.Item(i)))
			{
				OPOLYGLOT_WARNING(wxT("OPolyglotInstallLanguages::RemoveLanguage can`t delete %s dir"),filesToRemove.Item(i));
			}
		}
	}
	return true;
}

void OPolyglotInstallLanguages::OnCancel( wxCommandEvent& event )
{
	OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::OnCancel"));
	wxMessageDialog msg(this
			,_("Are you sure you want to cancel downloading languages?")
			,this->GetTitle()
			,wxICON_QUESTION|wxYES_NO);
	if(msg.ShowModal() == wxID_YES)
	{
		msgCancel.Post(true);
	}
}


void OPolyglotInstallLanguages::OnClose( wxCloseEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgressInstallLanguage::OnClose"));
	wxMessageDialog msg(this
			,_("Are you sure you want to cancel downloading languages?")
			,this->GetTitle()
			,wxICON_QUESTION|wxYES_NO);
	if(msg.ShowModal() == wxID_YES)
	{
		msgCancel.Post(true);
	}
}


void OPolyglotInstallLanguages::OnUpdateProgress(wxTimerEvent &event)
{
	double speed;
	double timeRemaining;
	double timeElapsed;
	wxString prefix = _("B/s    ");
	wxString prefixTime = _("s    ");
	wxMutexLocker lock(mutex);
	if( 0 < sizeFile)
	{
		FileProgress->SetValue((int)((downloadedBytesFile*FileProgress->GetRange())/sizeFile));
	}
	if(unpackFile.IsEmpty())
	{
		SizeFile->SetLabel(convertSizeToLabelHuman(sizeFile-downloadedBytesFile));
	} else
	{
		SizeFile->SetLabel(unpackFile);
	}
	timeRun.Pause();
	if( 0 < timeRun.Time())
	{
		downloadSpeed = (downloadedBytes+downloadedBytesFile)*1000/(timeRun.Time());
	} else
	{
		downloadSpeed = 0;
	}
	speed = (double)((downloadedBytes+downloadedBytesFile)*1000) / (double)(timeRun.Time() ); /* per second */
	timeElapsed = ((double)timeRun.Time())/1000.0;
	timeRun.Resume();
	timeRemaining =  ((double)(sizeToDownload-downloadedBytes-downloadedBytesFile))/(double)speed;

	if(512.0 < speed)
	{
		speed = speed / 1024.0;
		prefix = _("KB/s ");
		if(512.0 < speed)
		{
			speed = speed /1024.0;
			prefix = _("MB/s ");
		}
	}
	if(60 < timeRemaining)
	{
		timeRemaining /= 60.0;
		prefixTime = _("min  ");
		if(90 < timeRemaining)
		{
			timeRemaining /= 60.0;
			prefixTime = _("h    ");
		}
	}
	this->Speed->SetLabel(wxString::Format(wxS("%.1f %s"),speed,prefix));
	if(0 < (downloadedBytes+downloadedBytesFile))
	{
		this->TimeRemaining->SetLabel(wxString::Format(wxS("%0.1f %s"),timeRemaining,prefixTime));
	} else
	{
		TimeRemaining->SetLabel(wxS("∞"));
	}
	prefixTime = _("s    ");
	if(300 < timeElapsed)
	{
		prefixTime = _("min  ");
		timeElapsed /= 60.0;
		if( 60 < timeElapsed )
		{
			prefixTime = _("h    ");
			timeElapsed /= 60.0;
		}
	}
	this->TimeElapsed->SetLabel(wxString::Format(wxS("%0.1f %s"),timeElapsed,prefixTime));
	this->HBox1->Layout();
	this->HBox2->Layout();
	this->HBox3->Layout();
	this->HBox4->Layout();
	this->MainBox->Layout();
	this->Refresh();
}

bool OPolyglotInstallLanguages::CreateXmlLanguages(wxString& messageError,wxArrayString& labelLanguages,wxXmlDocument &xmlLanguages)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::CreateXmlLanguages"));
	wxArrayString idsInstalled;
	wxArrayString finishLanguages;
	wxString localeLanguage = wxLocale::FindLanguageInfo(wxGetLocale()->GetName())->Description.BeforeFirst(' '); 
	messageError = wxEmptyString;
	labelLanguages.Clear();
	while(xmlLanguages.GetRoot()->GetChildren())
	{
		wxXmlNode *child  = xmlLanguages.GetRoot()->GetChildren();
		xmlLanguages.GetRoot()->RemoveChild(child);
		delete child;
	}
	const wxXmlDocument doc(OPOLYGLOT_GET_XML_DATA_FILE);
	if(!doc.IsOk())//!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::CreateXmlLanguages failed to load the XML file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		messageError = wxString::Format(wxS("%s: %s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE);
		return false;
	}
	for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child=child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Installed")))
		{
			for(wxXmlNode *childId = child->GetChildren();childId;childId=childId->GetNext())
			{
				if(childId->GetName().IsSameAs(wxS("IdInstalled")))
				{
					if(idsInstalled.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND)
					{
						idsInstalled.Add(childId->GetAttribute(wxS("id")));
					}
				}
			}
		}
	}
	if(!localeLanguage.IsSameAs(wxS("English")))
	{
		for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child=child->GetNext())
		{
			if(child->GetName().IsSameAs(wxS("Language")))
			{
				if(child->GetAttribute(wxS("language")).IsSameAs(localeLanguage)&&(finishLanguages.Index(localeLanguage) == wxNOT_FOUND))
				{
					labelLanguages.Add(_("English"));
					finishLanguages.Add(localeLanguage);
					wxXmlNode *xmlLang = new wxXmlNode(xmlLanguages.GetRoot(),wxXML_ELEMENT_NODE,wxS("Label"));
					xmlLang->AddAttribute(wxS("label"),_("English"));
					bool flagInstalled = true;
					for(wxXmlNode *childLang = doc.GetRoot()->GetChildren();childLang;childLang = childLang->GetNext())
					{
						if(childLang->GetName().IsSameAs(wxS("Language")))
						{
							if((childLang->GetAttribute(wxS("from")).IsSameAs(wxS("English"))&&childLang->GetAttribute(wxS("to")).IsSameAs(localeLanguage))
									||((childLang->GetAttribute(wxS("from")).IsSameAs(localeLanguage))&&(childLang->GetAttribute(wxS("to")).IsSameAs(wxS("English")))))
							{
								for(wxXmlNode *childId = childLang->GetChildren();childId;childId = childId->GetNext())
								{
									if(childId->GetName().IsSameAs(wxS("Id")))
									{
										xmlLang->AddChild(new wxXmlNode(*childId));
										if(flagInstalled)
										{
											if(idsInstalled.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND)
											{
												flagInstalled = false;
											}
										}
									}
								}
							}

						}

					}
					if(flagInstalled)
					{
						xmlLang->AddAttribute(wxS("flagInstalled"),wxS("true"));
					}
				}

			}
		}
	} else
	{
		localeLanguage = wxEmptyString;
	}
	for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child= child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Language")))
		{
			if((finishLanguages.Index(child->GetAttribute(wxS("language"))) == wxNOT_FOUND)
					&&(!(child->GetAttribute(wxS("language")).IsSameAs(localeLanguage))))
			{
				bool flagFromEng = false;
				bool flagToEng = false;
				bool flagInstalled = true;
				wxString language = child->GetAttribute(wxS("language"));
				wxXmlNode *xmlLang = new wxXmlNode(xmlLanguages.GetRoot(),wxXML_ELEMENT_NODE,wxS("Label"));
				for(wxXmlNode *childLang = doc.GetRoot()->GetChildren();childLang;childLang=childLang->GetNext())
				{
					if(childLang->GetName().IsSameAs(wxS("Language"))
							&&childLang->GetAttribute(wxS("language")).IsSameAs(language))
					{
						if(childLang->GetAttribute(wxS("codeFrom")).IsSameAs(wxS("eng")))
						{
							flagFromEng = true;
						}
						if(childLang->GetAttribute(wxS("codeTo")).IsSameAs(wxS("eng")))
						{
							flagToEng = true;
						}
						for(wxXmlNode *childId = childLang->GetChildren();childId;childId =childId->GetNext())
						{
							if(childId->GetName().IsSameAs(wxS("Id")))
							{
								wxXmlNode *newId = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Id"));
								for(wxXmlAttribute *attr = childId->GetAttributes();attr;attr=attr->GetNext())
								{
									newId->AddAttribute(attr->GetName(),attr->GetValue());
								}
								xmlLang->AddChild(newId);
							}
							if(flagInstalled)
							{
								if(idsInstalled.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND)
								{
									flagInstalled = false;
								}
							}
						}
					}
				}
				if(flagInstalled)
				{
					xmlLang->AddAttribute(wxS("flagInstalled"),wxS("true"));
				}
				if(flagFromEng&&flagToEng)
				{
					xmlLang->AddAttribute(wxS("label"),OPolyglotGetTranslateLanguage(language));
				} else
				{
					if(flagFromEng&&(!flagToEng))
					{
						xmlLang->AddAttribute(wxS("label")
								,wxString::Format(wxT("%s | %s \"%s\"->\"%s\"")
									,OPolyglotGetTranslateLanguage(language)
									,_("only")
									,OPolyglotGetTranslateLanguage(_("English"))
									,OPolyglotGetTranslateLanguage(language)));
					}
					if(flagToEng&&(!flagFromEng))
					{
						xmlLang->AddAttribute(wxS("label")
								,wxString::Format(wxT("%s | %s \"%s\"->\"%s\"")
									,OPolyglotGetTranslateLanguage(language)
									,_("only")
									,OPolyglotGetTranslateLanguage(language)
									,OPolyglotGetTranslateLanguage(_("English"))));
					}
					if(xmlLang->GetAttribute(wxS("label")).IsEmpty())
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::CreateXmlLanguages not find label for %s %s"),child->GetAttribute(wxS("id")),OPolyglotGetTranslateLanguage(language));
						xmlLang->AddAttribute(wxS("label"),_("ERROR"));
					}
				}
				finishLanguages.Add(language);
				labelLanguages.Add(xmlLang->GetAttribute(wxS("label")));
			}
		}	
	}
	labelLanguages.Sort(CompareLocaleNoCase);
	return true;
}


OPolyglotDownloadLanguage::OPolyglotDownloadLanguage(wxEvtHandler *handler):GUIOPolyglotDownloadLanguage(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OPolyglotDownloadLanguage"));
	this->SetTitle(wxString::Format(wxT("OPolyglot %s"),_("setup languages")));
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxPoint pos;
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	this->handler = handler;
	pos = wxPoint((geom.width-this->GetSize().GetWidth())/2,(geom.height-this->GetSize().GetHeight())/2);
	ListLanguages->EnableScrolling(false,true);
	this->v_box->Layout();
	this->Refresh();
	this->SetPosition(pos);
	xmlLanguages.SetRoot(new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Languages")));
	this->ScanLangs();
	this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	Bind(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH,&OPolyglotDownloadLanguage::OnDownloadFinish,this);
}

void OPolyglotDownloadLanguage::OnClose(wxCloseEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnClose"));
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}



void OPolyglotDownloadLanguage::ScanLangs()
{
	int scrollX,scrollY;
	wxString messageError;
	wxArrayString labelLanguages;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::ScanLangs"));
	if(!OPolyglotInstallLanguages::CreateXmlLanguages(messageError,labelLanguages,xmlLanguages))
	{
		wxMessageDialog msg(this,messageError,this->GetTitle(),wxICON_ERROR|wxOK);
		msg.ShowModal();
		return;
	}
	ListLanguages->Freeze();
	ListLanguages->GetViewStart(&scrollX,&scrollY);
	ListLanguages->Scroll(0,0);
	box->Clear(true);
	OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::ScanLangs scroll %d %d"),scrollX,scrollY);
	bool flagShowDownloadAll = false;
	bool flagShowRemoveAll = false;
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *label = new wxStaticText(ListLanguages,wxID_ANY,_("Download languages for offline translation"),wxDefaultPosition,wxDefaultSize,0);
	sizer->Add(label,0,wxALL|wxEXPAND,2);
	sizer->Add(0,0,1,wxEXPAND,2);
	wxButton *buttonDownloadAll = new wxButton(ListLanguages,wxID_ANY,_("Download All"),wxDefaultPosition,wxDefaultSize,0);
	buttonDownloadAll->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotDownloadLanguage::OnLanguagesDownloadAll,this,buttonDownloadAll->GetId(),buttonDownloadAll->GetId());
	sizer->Add(buttonDownloadAll,0,wxALL,2);
	wxButton  *buttonRemoveAll = new wxButton(ListLanguages,wxID_ANY,_("Remove All"),wxDefaultPosition,wxDefaultSize,0);
	buttonRemoveAll->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotDownloadLanguage::OnLanguagesRemoveAll,this,buttonRemoveAll->GetId(),buttonRemoveAll->GetId());
	sizer->Add(buttonRemoveAll,0,wxALL,2);
	box->Add(sizer,0,wxALL|wxEXPAND,3);
	wxStaticLine *line = new wxStaticLine( ListLanguages, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	box->Add( line, 0, wxEXPAND | wxALL, 0 );
	for(size_t i = 0; i  <labelLanguages.GetCount();i++)
	{
		for(wxXmlNode *childLang = xmlLanguages.GetRoot()->GetChildren();childLang;childLang = childLang->GetNext())
		{
			if(childLang->GetName().IsSameAs(wxS("Label"))
					&&(childLang->GetAttribute(wxS("label")).IsSameAs(labelLanguages.Item(i))))
			{
				bool flagInstalled = !(childLang->GetAttribute(wxS("flagInstalled")).IsEmpty());
				wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
				wxStaticText *label = new wxStaticText(this->ListLanguages,wxID_ANY,childLang->GetAttribute(wxS("label")),wxDefaultPosition,wxDefaultSize,0);
				sizer->Add(label,0,wxALL|wxEXPAND,2);
				sizer->Add( 0, 0, 1, wxEXPAND, 2 );
				if(flagInstalled)
				{
					wxButton *button = new wxButton(ListLanguages,wxID_ANY,_("Remove"),wxDefaultPosition,wxDefaultSize,0);
					childLang->AddAttribute(wxS("idButton"),wxString::Format(wxT("%d"),button->GetId()));
					button->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotDownloadLanguage::OnLanguageRemove,this,button->GetId(),button->GetId());
					sizer->Add(button,0,wxALL,2);
					if(!flagShowRemoveAll)
					{
						flagShowRemoveAll = true;
					}
				} else
				{
					wxButton *button = new wxButton(ListLanguages,wxID_ANY,_("Download"),wxDefaultPosition,wxDefaultSize,0);
					childLang->AddAttribute(wxS("idButton"),wxString::Format(wxT("%d"),button->GetId()));
					button->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotDownloadLanguage::OnLanguageDownload,this,button->GetId(),button->GetId());
					sizer->Add(button,0,wxALL,2);
					if(!flagShowDownloadAll)
					{
						flagShowDownloadAll = true;
					}
				}
				sizer->Layout();
				box->Add(sizer,0,wxALL|wxEXPAND,3);

			}
		}
	}
	if(!flagShowDownloadAll)
	{
		box->GetItem(static_cast<size_t>(0))->GetSizer()->Hide(buttonDownloadAll);
		box->GetItem(static_cast<size_t>(0))->GetSizer()->Layout();
	}
	if(!flagShowRemoveAll)
	{
		box->GetItem(static_cast<size_t>(0))->GetSizer()->Hide(buttonRemoveAll);
		box->GetItem(static_cast<size_t>(0))->GetSizer()->Layout();
	}
	box->Layout();
	ListLanguages->Thaw();
	ListLanguages->Scroll(scrollX,scrollY);
}

void OPolyglotDownloadLanguage::OnLanguagesDownloadAll(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnLanguagesDownloadAll"));
	progress = new OPolyglotInstallLanguages(this,xmlLanguages,OPOLYGLOT_ID_ALL);
	this->Show(false);
}

void OPolyglotDownloadLanguage::OnLanguagesRemoveAll(wxCommandEvent& event)
{
	wxString messageError;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnLanguagesRemoveAll"));
	if(!OPolyglotInstallLanguages::RemoveLanguage(messageError,xmlLanguages,OPOLYGLOT_ID_ALL))
	{
		wxMessageDialog msg(this,messageError,this->GetTitle(),wxICON_ERROR|wxOK);
		msg.ShowModal();
	}
	ScanLangs();
}


void OPolyglotDownloadLanguage::OnLanguageDownload(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnLanguageDownload %d"),event.GetId());
	progress = new OPolyglotInstallLanguages(this,xmlLanguages,event.GetId());
	this->Show(false);
}


void OPolyglotDownloadLanguage::OnLanguageRemove(wxCommandEvent& event)
{
	wxString messageError;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnLanguageRemove %d"),event.GetId());
	if(!OPolyglotInstallLanguages::RemoveLanguage(messageError,xmlLanguages,event.GetId()))
	{
		wxMessageDialog msg(this,messageError,this->GetTitle(),wxICON_ERROR|wxOK);
		msg.ShowModal();
	}
	ScanLangs();
}


void OPolyglotDownloadLanguage::OnDownloadFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnDownloadFinish"));
	progress->Destroy();
	progress = nullptr;
	ScanLangs();
	this->Show(true);
}

OPolyglotDownloadLanguage::~OPolyglotDownloadLanguage()
{
	OPOLYGLOT_MESSAGE("OPolyglotDownloadLanguage::~OPolyglotDownloadLanguage");
	if(!IS_NULLPTR(progress))
	{
		progress->Destroy();
		progress = nullptr;
	}
}
