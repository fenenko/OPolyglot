/* $Id: OPolyglotDownloadLanguage.cpp,v 1.17 2025/11/17 09:20:30 oleksandr Exp oleksandr $ */
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
#include "../res/icon.xpm"
#include "OPolyglot.h"
#include <tomcrypt.h>

#if 0
#include <valgrind/valgrind.h>
#include <valgrind/memcheck.h>
#endif

#define OPOLYGLOT_TIMEOUT_START_DOWNLOAD		120 /* in second */
#define OPOLYGLOT_FILE_FROM_STRING(NAME)		wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,NAME)
#define OPOLYGLOT_DIR_FROM_STRING(NAME)			wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,NAME)
#define OPOLYGLOT_MESSAGE_DOWNLOAD(FILES_LEFT)	wxString::Format(wxT("%s %ld"),_("download new languages, files left "),FILES_LEFT)
#define OPOLYGLOT_MESSAGE_DOWNLOAD_AND_SPEAD(FILES_LEFT,SPEED_IN_FLOAT_KBYTES,PREFIX)	\
		wxString::Format(wxT("%s %ld %s %.1f %s") \
			,_("download new languages, files left ") \
			,FILES_LEFT \
			,_(" speed ") \
			,SPEED_IN_FLOAT_KBYTES \
			,PREFIX)
#define OPOLYGLOT_USER_AGENT	wxString::Format(wxT("TEST OPolyglot PLEASE NOT BLOCKING %s"), wxGetOsDescription())
enum{
	TIMER_ID,
};

#include <wx/arrimpl.cpp> 
//WX_DEFINE_ARRAY_PTR(wxXmlNode *,ArrayUrlXml);

wxDEFINE_EVENT(wxEVT_COMMAND_OPOLYGLOT_START_DOWNLOAD, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_COMMAND_OPOLYGLOT_FAILED_DOWNLOAD_LANGUAGE, wxThreadEvent);

OPolyglotDownloadLanguage::OPolyglotDownloadLanguage(wxWindow *parent):GUIOPolyglotDownloadLanguage(parent)
{
	OPOLYGLOT_DEBUG(wxT(OPOLYGLOT_DOWNLOAD_LANGUAGE_H_VERSION));
	OPOLYGLOT_DEBUG(wxT("$Id: OPolyglotDownloadLanguage.cpp,v 1.17 2025/11/17 09:20:30 oleksandr Exp oleksandr $"));
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxPoint pos;
	OPOLYGLOT_MESSAGE();
	SetIcon(wxICON(icon));
	dataReceiv = new wxMemoryBuffer(1024);
	//languages= new wxArrayString();
	this->parent = parent;
#if 0
	urlFile = wxEmptyString;
#endif
	this->ScanLangs();
	pos = wxPoint((geom.width-this->GetSize().GetWidth())/2,(geom.height-this->GetSize().GetHeight())/2);
	timeUpdate = new wxTimer();
	timeUpdate->SetOwner(this,TIMER_ID);
	OPOLYGLOT_MESSAGE(wxT("user-agent %s"),OPOLYGLOT_USER_AGENT);
	this->v_box->Layout();
	this->Refresh();
	this->SetPosition(pos);
	this->Bind(wxEVT_WEBREQUEST_STATE,&OPolyglotDownloadLanguage::OnFileDownload,this);
#if 1 /* тимчасово */
	this->Bind(wxEVT_WEBREQUEST_DATA,&OPolyglotDownloadLanguage::OnFileData,this);
#endif
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_START_DOWNLOAD,&OPolyglotDownloadLanguage::OnStartDownloadFile,this);
	this->Bind(wxEVT_TIMER,&OPolyglotDownloadLanguage::OnTimerProgressUpdate,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_FAILED_DOWNLOAD_LANGUAGE,&OPolyglotDownloadLanguage::OnFailedDownloadLanguage,this);
}


void OPolyglotDownloadLanguage::OnFailedDownloadLanguage(wxThreadEvent& event)
{
	OPOLYGLOT_ERROR(wxT("%s"),event.GetString());
	this->ScanLangs();
	wxMessageDialog msg(this,wxString::Format(wxT("Error %s"),event.GetString()),_("OPolyglot"),wxOK|wxICON_ERROR);
	msg.ShowModal();
	this->Destroy();
}

void OPolyglotDownloadLanguage::OnStartDownload(wxCommandEvent& event)
{
	wxArrayString filesDownload;
	OPOLYGLOT_MESSAGE();
	this->Enable( false );
	//this->ListLanguage->GetCheckedItems(selections);
	if(!doc.Load(OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE))
	{
		OPOLYGLOT_ERROR(wxT("Load %s"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	/*
	 * remove not select languages
	 */
	OPOLYGLOT_MESSAGE(wxT("start deleting unused language files"));
	progress = new wxProgressDialog(wxT("OPolyglot install languages"),_("deleting unused language files before installation"),1000,this,wxPD_APP_MODAL|wxPD_CAN_ABORT);
	progress->Show();
	wxArrayString saveFile;
	wxArrayString saveDir;
	for(size_t i = 0; i < this->ListLanguage->GetCount();i++)
	{
		if(this->ListLanguage->IsChecked(i))
		{
			for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child=child->GetNext())
			{
				if((child->GetName().Cmp(wxT("Language")) == 0)
						&&(this->ListLanguage->GetString(i).Cmp(OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(child)) == 0))
				{
					if(OPOLYGLOT_CHECKING_INSTALLE_LANGUAGE_FROM_NODE_XML(child))
					{
						saveFile.Add(OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(child));
						saveFile.Add(OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(child));

					}
				}
			}

		}
	}
	for(size_t i = 0; i < this->ListLanguage->GetCount();i++)
	{
		if(!this->ListLanguage->IsChecked(i))
		{
			for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child=child->GetNext())
			{
				if((child->GetName().Cmp(wxT("Language")) == 0)
						&&(this->ListLanguage->GetString(i).Cmp(OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(child)) == 0))
				{

					if(wxNOT_FOUND == saveFile.Index(OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(child)))
					{
						if(wxFileName::FileExists(OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(child)))
						{
							OPOLYGLOT_DEBUG(wxT("remove traineddata file %s"),OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(child));
							if(!wxRemoveFile(OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(child)))
							{
								OPOLYGLOT_WARNING(wxT("can`t delete the file %s"),OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(child));
								wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("warning can't delete the file"),OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(child)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
								msg.ShowModal();
							}
						}
					}
					if(wxNOT_FOUND == saveFile.Index(OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(child)))
					{
						if(wxFileName::FileExists(OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(child)))
						{
							OPOLYGLOT_DEBUG(wxT("remove traineddata file %s"),OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(child));
							if(!wxRemoveFile(OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(child)))
							{
								OPOLYGLOT_WARNING(wxT("can`t delete the file %s"),OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(child));
								wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("warning can't delete the file"),OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(child)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
								msg.ShowModal();
							}
						}
					}
					if(wxFileName::FileExists(OPOLYGLOT_CONFIG_FILE_TRANSLATOR_FOR_NODE_XML(child)))
					{
						OPOLYGLOT_DEBUG(wxT("remove dir translation models %s"),OPOLYGLOT_GET_DIR_TRANSLATOR_FOR_NODE_XML(child));
						if(!wxDir::Remove(OPOLYGLOT_GET_DIR_TRANSLATOR_FOR_NODE_XML(child),wxPATH_RMDIR_RECURSIVE))
						{
							OPOLYGLOT_WARNING(wxT("can`t delete the dir %s"),OPOLYGLOT_GET_DIR_TRANSLATOR_FOR_NODE_XML(child));
							wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("warning can`t delete the dir"),OPOLYGLOT_GET_DIR_TRANSLATOR_FOR_NODE_XML(child)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
							msg.ShowModal();
						}

					}
				}
			}
		}
	}
	//filesUrl.Clear();
	urlsXML.Clear();
	/*
	 * start create list url download files
	 */
	for(size_t i =0; i < this->ListLanguage->GetCount();i++)
	{
		if(this->ListLanguage->IsChecked(i))
		{
			OPOLYGLOT_DEBUG(wxT("%s"),this->ListLanguage->GetString(i));
			for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child=child->GetNext())
			{
				if((child->GetName().Cmp(wxT("Language")) == 0)
						&&(this->ListLanguage->GetString(i).Cmp(OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(child)) == 0))
				{
					for(wxXmlNode *c = child->GetChildren();c;c=c->GetNext())
					{
						if(c->GetName().Cmp(wxT("File")) == 0)
						{

							//filesDownload.Add(c->GetNodeContent());
							
							if(c->GetNodeContent().Find(wxT("tessdata")) != wxNOT_FOUND)
							{
								OPOLYGLOT_DEBUG(wxT("this file is tesseract traineddata %s"),c->GetNodeContent());
								if((!wxFileName::FileExists(OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(child)))
										&&(!wxFileName::FileExists(OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(child))))
								{

									if(filesDownload.Index(c->GetNodeContent()) == wxNOT_FOUND)
									{
										OPOLYGLOT_DEBUG(wxT("add tesseract url %s"),c->GetNodeContent());
										filesDownload.Add(c->GetNodeContent());
									}
								} 

							} else
							{
								OPOLYGLOT_DEBUG(wxT("this file is firefox-translations-model %s"),c->GetNodeContent());
								if(!wxFileName::FileExists(OPOLYGLOT_CONFIG_FILE_TRANSLATOR_FOR_NODE_XML(child)))
								{
									if(filesDownload.Index(c->GetNodeContent()) == wxNOT_FOUND)
									{
										OPOLYGLOT_DEBUG(wxT("add translation models %s"),c->GetNodeContent());
										filesDownload.Add(c->GetNodeContent());
									}

								} 
							}
						} /* if(c->GetName().Cmp(wxT("File")) == 0) */
					}
				}
			}
		}
	}
	OPOLYGLOT_DEBUG(wxT("filesDownload.GetCount %ld"),filesDownload.GetCount());
	for(wxXmlNode *filesUrl=doc.GetRoot()->GetChildren();filesUrl;filesUrl=filesUrl->GetNext())
	{
		if(filesUrl->GetName().IsSameAs(wxT("FilesUrl")))
		{
			for(;0 <filesDownload.GetCount();filesDownload.RemoveAt(0))
			{
				bool flagAdd = true; /* do not remove, is a check for the correctness of download.xml */
				
				for(wxXmlNode *urlXml = filesUrl->GetChildren();urlXml;urlXml = urlXml->GetNext())
				{
					if(filesDownload.Item(0).IsSameAs(urlXml->GetAttribute(wxT("file"))))
					{
						urlsXML.Add(urlXml);
						flagAdd = false;

					}
				}

				/* do not remove, is a check for the correctness of download.xml */
				if(flagAdd)
				{
					OPOLYGLOT_ERROR(wxT("url for file %s not found"),filesDownload.Item(0)); 
					wxMessageDialog msg(this,wxString::Format(wxT("url for file :%s not found"),filesDownload.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
					msg.ShowModal();
					return;
				}
			}
		}
	}
	OPOLYGLOT_DEBUG(wxT("finish create urlsXML %ld"),urlsXML.GetCount());
	if(0 < urlsXML.GetCount())
	{
		progress->Update(0,wxString::Format(wxT("%s %ld"),_("start download files"),urlsXML.GetCount()));
		for(size_t i = 0; i < urlsXML.GetCount();i++)
		{
			OPOLYGLOT_DEBUG(wxT("%ld url %s"),i+1,urlsXML.Item(i)->GetNodeContent());
		}
		OPOLYGLOT_DEBUG(wxT("start download %s"),urlsXML.Item(0)->GetNodeContent());
		mutexFileRequest.Lock();
		fileRequest = this->CreateRequest(urlsXML.Item(0)->GetNodeContent());
		fileRequest.Start();
		mutexFileRequest.Unlock();
	} else
	{
		progress->Destroy();
		this->Enable(true);
	}
}


wxWebRequest OPolyglotDownloadLanguage::CreateRequest(wxString url)
{
	wxWebRequest ret;
	OPOLYGLOT_MESSAGE(wxT("%s"),url);
	ret = wxWebSession::GetDefault().CreateRequest(this,url);
	ret.SetHeader(wxT("User-Agent"), OPOLYGLOT_USER_AGENT);
	//ret.SetStorage(wxWebRequest::Storage_Memory);
	ret.SetStorage(wxWebRequest::Storage_None);
	/* fileRequest.SetMethod(wxT("GET")); */
	return ret;
}


void OPolyglotDownloadLanguage::OnStartDownloadFile(wxThreadEvent &event)
{

	OPOLYGLOT_MESSAGE();
	//fileRequest.Start();
}

void OPolyglotDownloadLanguage::OnTimerProgressUpdate(wxTimerEvent &event)
{
	//static int speedCounting ;
	wxMutexLocker lock(mutexFileRequest);
	if(progressReceived == 0)
	{
		if(!progress->Pulse(messageProgress))
		{
			OPOLYGLOT_WARNING(wxT("user cancel"));
			fileRequest.Cancel();
		}
	} else
	{
		if(!progress->Update(progressReceived,messageProgress))
		{
			OPOLYGLOT_WARNING(wxT("user cancel"));
			fileRequest.Cancel();
		}
	}
	messageProgress = wxEmptyString;
#if 0
	if(fileRequest.IsOk())
	{
		OPOLYGLOT_DEBUG(wxT("%ld %ld"),fileRequest.GetBytesExpectedToReceive(),fileRequest.GetBytesReceived());
	} else
	{
		OPOLYGLOT_DEBUG(wxT("failed request"));
	}
#if 1
	if(0 < fileRequest.GetBytesExpectedToReceive())
	{
		if(speedCounting < 10)
		{
			speedCounting++;
			if(!progress->Update((fileRequest.GetBytesReceived()*1000)/fileRequest.GetBytesExpectedToReceive()))
			{
				OPOLYGLOT_WARNING(wxT("user cancel"));
				fileRequest.Cancel();
			}
		} else
		{
			speedCounting = 0;
			double speed = ((double)fileRequest.GetBytesReceived())/(wxGetUTCTime()-timeStartDownload);
			wxString prefix = wxT("Bytes");
			if(512.0 < speed )
			{
				speed = speed/1024.0;
				prefix = wxT("KB");
				if(512.0 < speed)
				{
					speed = speed /1024.0;
					prefix = wxT("MB");
				}
			}
			if(!progress->Update(
						(fileRequest.GetBytesReceived()*1000)/fileRequest.GetBytesExpectedToReceive()
						,OPOLYGLOT_MESSAGE_DOWNLOAD_AND_SPEAD(urlsXML.GetCount(),speed,prefix)))
			{
				OPOLYGLOT_WARNING(wxT("user cancel"));
				timeUpdate->Stop();
				fileRequest.Cancel();
			}
		}
	} else
	{
		speedCounting = 0;
		if(OPOLYGLOT_TIMEOUT_START_DOWNLOAD < ( 	wxGetUTCTime () - timeStartDownload))
		{
			OPOLYGLOT_WARNING(wxT("timeout start download %ld for file %s"),(wxGetUTCTime()-timeStartDownload),urlsXML.Item(0));
			wxMessageDialog msg(this,wxString::Format(wxT("%s %ld %s %s"),_("timeout start download"),(wxGetUTCTime()-timeStartDownload),_("second\n"),urlsXML.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			timeUpdate->Stop();
			fileRequest.Cancel();
		}
		if(!progress->Update(0))
		{
			OPOLYGLOT_WARNING(wxT("user cancel not started download %s"),urlsXML.Item(0));
			timeUpdate->Stop();
			fileRequest.Cancel();
		}

	}
#endif
	OPOLYGLOT_DEBUG(wxT("finish"));
#endif
}

void OPolyglotDownloadLanguage::OnFileData(wxWebRequestEvent& event)
{
	double speed;
	wxString prefix = wxT("Bytes");
	wxMutexLocker lock(mutexFileRequest);
	dataReceiv->AppendData(event.GetDataBuffer(),event.GetDataSize());
	timeDownload.Pause();
	speed = (double)1000*dataReceiv->GetDataLen()/timeDownload.Time();
	if(512 < speed)
	{
		speed = speed/1024.0;
		prefix = wxT("KB");
		if(512 < speed)
		{
			speed = speed/ 1024.0;
				prefix = wxT("MB");
		}
	}
	progressReceived = (int)((1000*dataReceiv->GetDataLen())/fileRequest.GetBytesExpectedToReceive());
	messageProgress = wxString::Format(wxT("%s %ld , %s  %s  %.1f %s")
			,_("files need to be downloaded")
			,urlsXML.GetCount()
			,_("download file")
			,urlsXML.Item(0)->GetAttribute(wxT("file"))
			,speed
			,prefix);
	timeDownload.Resume();
}

void OPolyglotDownloadLanguage::ScanLangs()
{
	wxXmlDocument doc;
	OPOLYGLOT_MESSAGE();
	if(!doc.Load(OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE))
	{
		OPOLYGLOT_ERROR("Load %s",OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	this->ListLanguage->Clear();
	wxXmlNode *child = doc.GetRoot()->GetChildren();
	while(child)
	{
		if(child->GetName().Cmp(wxT("Language")) == 0)
		{
			this->ListLanguage->InsertItems(1,(new wxString(OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(child))),this->ListLanguage->GetCount());
#if 0
			if(wxFileName::FileExists(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,child->GetAttribute(wxT("configfile"))))
					&&wxFileName::FileExists(wxString::Format(wxT("%s/%s.traineddata"),OPOLYGLOT_USER_DATA,child->GetAttribute(wxT("ocrfile")))))
#endif
			if(OPOLYGLOT_CHECKING_INSTALLE_LANGUAGE_FROM_NODE_XML(child))
			{
				this->ListLanguage->Check(this->ListLanguage->GetCount()-1,true);
			} else
			{
				OPOLYGLOT_DEBUG(wxT("not install %s"),OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(child));
				if(!wxFileName::FileExists(OPOLYGLOT_CONFIG_FILE_TRANSLATOR_FOR_NODE_XML(child)))
				{
					OPOLYGLOT_DEBUG(wxT("not find %s"),OPOLYGLOT_CONFIG_FILE_TRANSLATOR_FOR_NODE_XML(child));
				}
				if((!wxFileName::FileExists(OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(child)))
						||(!wxFileName::FileExists(OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(child))))
				{
					OPOLYGLOT_DEBUG(wxT("not find %s or")
							,OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(child)
							,OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(child));
				}
			}
		} 
		child = child->GetNext();
	}
}


void OPolyglotDownloadLanguage::OnFileDownload(wxWebRequestEvent& event)
{
	wxMutexLocker lock(mutexFileRequest);
#if 1
	OPOLYGLOT_INFO();
#endif
	timeUpdate->Stop();
	switch(event.GetState())
	{
		case wxWebRequest::State_Idle:
			OPOLYGLOT_MESSAGE(wxT("State_Idle"));
			break;
		case wxWebRequest::State_Unauthorized:
			OPOLYGLOT_MESSAGE(wxT("State_Unauthorized"));
			break;
		case wxWebRequest::State_Active:
			OPOLYGLOT_INFO(wxT("wxWebRequestEvent::State_Active %s"),urlsXML.Item(0)->GetNodeContent());
			timeStartDownload = wxGetUTCTime();
			messageProgress = wxString::Format(wxT("%s %ld , %s %s")
					,_("files need to be downloaded")
					,urlsXML.GetCount()
					,_("start downloaded file")
					,urlsXML.Item(0)->GetAttribute("file"));
			timeUpdate->Start(200);
			dataReceiv->Clear();
			progressReceived = 0;
			timeDownload.Start();
			break;
		case wxWebRequest::State_Completed:
			{
				wxArrayString newFiles;
				wxArrayString newDirs;
				bool flagZipOk = true;
				if(dataReceiv->GetDataLen() != (size_t)fileRequest.GetBytesExpectedToReceive())
				{
					OPOLYGLOT_ERROR(wxT("received data %ld != %ld"),dataReceiv->GetDataLen(),fileRequest.GetBytesExpectedToReceive());
				}
#if 0
				OPOLYGLOT_DEBUG(wxT("size %ld %ld")
						,event.GetResponse().GetStream()->GetLength()
						,event.GetResponse().GetStream()->GetSize());
#endif
				if(urlsXML.Item(0)->GetAttribute(wxT("sha1sum")).IsEmpty())
				{
					OPOLYGLOT_WARNING(wxT("for file %s not sha1sum"),urlsXML.Item(0)->GetAttribute(wxT("file")));
					wxMessageDialog msg(this
							,wxString::Format(wxT("%s %s")
								,_("warning not haved sha1sum for file: ")
								,urlsXML.Item(0)->GetAttribute(wxT("file")))
							,wxT("OPolyglot"),wxOK|wxICON_WARNING);
					msg.ShowModal();
				} else
				{
					OPOLYGLOT_INFO(wxT("dataReceiv->GetDataLen(%ld)"),dataReceiv->GetDataLen());
					{
						int err;
						unsigned char sum_sha1[20];
						wxString hexString = wxEmptyString;
						hash_state sha1;
						if((err = sha1_init(&sha1)) != CRYPT_OK)
						{
							OPOLYGLOT_ERROR(wxT("tomcrypt error sha1_init %s"),wxString(error_to_string(err)));
							wxMessageDialog msg(this
									,wxString::Format(wxT("%s\n%s")
										,_("error tomcrypt sha1_init ")
										,error_to_string(err))
									,wxT("OPolyglot"),wxOK|wxICON_WARNING);
							msg.ShowModal();
						}
						if((err = sha1_process(&sha1,(unsigned char *)dataReceiv->GetData(),dataReceiv->GetDataLen())) != CRYPT_OK)
						{
							OPOLYGLOT_ERROR(wxT("tomcrypt error sha1_process %s"),wxString(error_to_string(err)));
							wxMessageDialog msg(this,wxString::Format(wxT("%s\n%s"),_("error tomcrypt sha1_process "),error_to_string(err)),wxT("OPolyglot"),wxOK|wxICON_WARNING);
							msg.ShowModal();
						}
						if((err = sha1_done(&sha1,sum_sha1)) != CRYPT_OK)
						{
							OPOLYGLOT_ERROR(wxT("tomcrypt error sha1_done %s"),wxString(error_to_string(err)));
							wxMessageDialog msg(this,wxString::Format(wxT("%s\n%s"),_("error tomcrypt sha1_done "),error_to_string(err)),wxT("OPolyglot"),wxOK|wxICON_WARNING);
							msg.ShowModal();
						}
						for(size_t i = 0; i < sizeof(sum_sha1);i++)
						{
							hexString += wxString::Format(wxT("%02x"),sum_sha1[i]);
						}
						OPOLYGLOT_DEBUG(wxT("%s sha1sum %s"),urlsXML.Item(0)->GetAttribute(wxT("file")),hexString);
						if(!urlsXML.Item(0)->GetAttribute(wxT("sha1sum")).IsSameAs(hexString))
						{
							OPOLYGLOT_WARNING(wxT("sha1sum failed for file %s %s %s"),urlsXML.Item(0)->GetAttribute(wxT("file")),urlsXML.Item(0)->GetAttribute(wxT("sha1sum")),hexString);
							wxMessageDialog msg(this,wxString::Format(wxT("%s %s %s\n%s"),_("error sha1sum : "),urlsXML.Item(0)->GetAttribute(wxT("file")),hexString,_("redownload this_file")),wxT("OPolyglot"),wxYES_NO|wxICON_WARNING);
							if(msg.ShowModal() == wxID_YES)
							{
								OPOLYGLOT_MESSAGE(wxT("redownload file %s"),urlsXML.Item(0)->GetAttribute(wxT("file")));
								progress->Update(0,OPOLYGLOT_MESSAGE_DOWNLOAD(urlsXML.GetCount()));
								fileRequest = this->CreateRequest(urlsXML.Item(0)->GetNodeContent());
								fileRequest.Start();
								return;
							} else
							{
								progress->Destroy();
								this->ScanLangs();
								this->Enable(true);
								wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES));
							}
						}
						

					} 
#if 0
					else
					{
						OPOLYGLOT_ERROR(wxT("error read download files %d %ld %ld")
								,event.GetResponse().GetStream()->GetLastError()
								,event.GetResponse().GetStream()->LastRead()
								,event.GetResponse().GetStream()->GetSize());
						wxMessageDialog msg(this
								,wxString::Format(wxT("%s %s %d"),_("error read file")
									,urlsXML.Item(0)->GetAttribute(wxT("file"))
									,event.GetResponse().GetStream()->GetLastError())
								,wxT("OPolyglot"),wxOK|wxICON_WARNING);
						msg.ShowModal();

					}
#endif

				}
				wxMemoryInputStream min(dataReceiv->GetData(),dataReceiv->GetDataLen());
				wxZipInputStream zip(min);
				wxZipEntry *entry =zip.GetNextEntry();

				while(entry&&flagZipOk)
				{
					if(zip.IsOk())
					{
						if(entry->IsDir())
						{
							OPOLYGLOT_DEBUG(wxT("mkdir %s"),OPOLYGLOT_DIR_FROM_STRING(entry->GetName()));
							if(!wxFileName::DirExists(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,entry->GetName())))
							{
							if(!wxDir::Make(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,entry->GetName())))
							{
								OPOLYGLOT_ERROR(wxT("create dir %s/%s"),OPOLYGLOT_USER_DATA,entry->GetName());
								timeUpdate->Stop();
								urlsXML.Clear();
								progress->Destroy();
								this->Enable(true);
								this->ScanLangs();
								wxMessageDialog msg(this
										,wxString::Format(wxT("%s %s/%s")
											,_("error create dir")
											,OPOLYGLOT_USER_DATA
											,entry->GetName())
										,wxT("OPolyglot"),wxOK|wxICON_ERROR);
								msg.ShowModal();
								return;
							} else
							{
								newDirs.Add(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,entry->GetName()));
							}
							} else
							{
								OPOLYGLOT_WARNING(wxT("dir exists %s/%s"),OPOLYGLOT_USER_DATA,entry->GetName());
							}

						} else
						{
							OPOLYGLOT_DEBUG(wxT("file zip: %s"),OPOLYGLOT_FILE_FROM_STRING(entry->GetName()));
							if(!wxFileName::FileExists(OPOLYGLOT_FILE_FROM_STRING(entry->GetName())))
							{
								wxFileOutputStream out(OPOLYGLOT_FILE_FROM_STRING(entry->GetName()));
								zip.Read(out);
								newFiles.Add(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,entry->GetName()));
							} else
							{
								OPOLYGLOT_INFO(wxT("file exist %s"),OPOLYGLOT_FILE_FROM_STRING(entry->GetName()));
							}
						}
					} else
					{
						OPOLYGLOT_ERROR(wxT("zip failed %s"),urlsXML.Item(0)->GetAttribute(wxT("file")));
						flagZipOk = false;
					}
					entry = zip.GetNextEntry();
				}
				if(flagZipOk)
				{
					OPOLYGLOT_DEBUG(wxT("file download %s"),urlsXML.Item(0)->GetAttribute(wxT("file")));
					urlsXML.RemoveAt(0);
					if(0 < urlsXML.GetCount())
					{
						fileRequest = this->CreateRequest(urlsXML.Item(0)->GetNodeContent());
						fileRequest.Start();
					} else
					{
						progress->Destroy();
						this->ScanLangs();
						this->Enable(true);
						wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES));
					}
				} else
				{
					OPOLYGLOT_ERROR(wxT("error bad zip file %s"),urlsXML.Item(0)->GetAttribute(wxT("file")));
					for(;0< newFiles.GetCount();newFiles.RemoveAt(0))
					{
						if(!wxRemoveFile(newFiles.Item(0)))
						{
							OPOLYGLOT_WARNING(wxT("can`t delete the file %s"),newFiles.Item(0));
							wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("warning can't delete the file"),newFiles.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
							msg.ShowModal();
						}
					}
					for(;0<newDirs.GetCount();newDirs.RemoveAt(0))
					{
						if(!wxDir::Remove(newDirs.Item(0)))
						{
							OPOLYGLOT_WARNING(wxT("can`t delete the dir %s"),newDirs.Item(0));
							wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("warning can`t delete the dir"),newDirs.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
							msg.ShowModal();
						}
					}
					wxMessageDialog msg(this
							,wxString::Format(wxT("%s %s\n%s")
								,_("error bad zip file: ")
								,urlsXML.Item(0)->GetAttribute(wxT("file"))
								,_("redownload this_file"))
							,wxT("OPolyglot"),wxYES_NO|wxICON_WARNING);
					if(msg.ShowModal() == wxID_YES)
					{
						OPOLYGLOT_MESSAGE(wxT("redownload file %s"),urlsXML.Item(0)->GetAttribute(wxT("file")));
						progress->Update(0,OPOLYGLOT_MESSAGE_DOWNLOAD(urlsXML.GetCount()));
						fileRequest = this->CreateRequest(urlsXML.Item(0)->GetNodeContent());
						fileRequest.Start();
					} else
					{
						progress->Destroy();
						this->ScanLangs();
						this->Enable(true);
						wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES));
					}
				} 



			}
			break;
		case wxWebRequest::State_Failed:
			{
				OPOLYGLOT_ERROR(wxT("State_Failed %s %s"),(wxString)event.GetErrorDescription(),urlsXML.Item(0)->GetNodeContent());
				wxString strError = wxString::Format(wxT("download %s\n%s"),event.GetErrorDescription(),urlsXML.Item(0)->GetNodeContent());
				progress->Destroy();
				this->Enable(true);
				this->ScanLangs();
				urlsXML.Clear();
				wxMessageDialog msg(this,wxString::Format(wxT("%s"),strError),_("OPolyglot"),wxOK|wxICON_ERROR);
				msg.ShowModal();
			}
			break;
		case wxWebRequest::State_Cancelled:
			OPOLYGLOT_WARNING(wxT("user cancel"));
			wxString strState = wxString(wxString::Format(wxT("%s"),_("user cancel")));
			progress->Destroy();
			urlsXML.Clear();
			this->Enable(true);
			this->ScanLangs();
			break;
	}
}

OPolyglotDownloadLanguage::~OPolyglotDownloadLanguage()
{
	OPOLYGLOT_MESSAGE();
}
