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
	pos = wxPoint((geom.width-this->GetSize().GetWidth())/2,(geom.height-this->GetSize().GetHeight())/2);
	timeUpdate = new wxTimer();
	timeUpdate->SetOwner(this,TIMER_ID);
	OPOLYGLOT_MESSAGE(wxT("user-agent %s"),OPOLYGLOT_USER_AGENT);
	this->v_box->Layout();
	this->Refresh();
	this->SetPosition(pos);
#if 1
	if(!document.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("Load %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("load data xml %s"),OPOLYGLOT_GET_XML_DATA_FILE);
	}
#endif
	this->Bind(wxEVT_WEBREQUEST_STATE,&OPolyglotDownloadLanguage::OnFileDownload,this);
#if 1 /* тимчасово */
	this->Bind(wxEVT_WEBREQUEST_DATA,&OPolyglotDownloadLanguage::OnFileData,this);
#endif
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_START_DOWNLOAD,&OPolyglotDownloadLanguage::OnStartDownloadFile,this);
	this->Bind(wxEVT_TIMER,&OPolyglotDownloadLanguage::OnTimerProgressUpdate,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_FAILED_DOWNLOAD_LANGUAGE,&OPolyglotDownloadLanguage::OnFailedDownloadLanguage,this);
	this->ScanLangs();
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
	OPOLYGLOT_MESSAGE();
	//wxXmlDocument doc;
	this->Show( false );
	progress = new wxProgressDialog(wxT("OPolyglot install languages"),_("deleting unused language files before installation"),1000,this,wxPD_APP_MODAL|wxPD_CAN_ABORT);
	progress->Show();
	//this->ListLanguage->GetCheckedItems(selections);
#if 0
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("Load %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
#endif
	wxArrayString listIdForInstalled;
	wxArrayString listIdInstalled;
	wxArrayString listIdForNotInstall;
	for(size_t i =0; i < this->ListLanguage->GetCount();i++)
	{
		wxXmlNode *node = OPolyglotGetNodeFromId(&document,idListLanguage.Item(i));
		if(!node->GetName().IsSameAs(wxS("Language")))
		{
			OPOLYGLOT_ERROR(wxT("node is id %s not \"Language\" - %s"),idListLanguage.Item(i),node->GetName());
			wxMessageDialog msg(this,wxString::Format(wxT("%s %s %s"),_("error Language id "),idListLanguage.Item(i),node->GetName()),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			progress->Destroy();
			this->Show(true);
			return;
		}
			OPOLYGLOT_DEBUG(wxT("Select %s : %s"),idListLanguage.Item(i),this->ListLanguage->GetStrings().Item(i));
			for(wxXmlNode *id=node->GetChildren();id;id=id->GetNext())
			{
				if(id->GetName().IsSameAs(wxT("Id")))
				{
					if(this->ListLanguage->IsChecked(i))
					{
						listIdForInstalled.Add(id->GetNodeContent());
					} else
					{
						listIdForNotInstall.Add(id->GetNodeContent());
					}
				}
			}

	}
	//wxXmlNode *nodeInstalled = OPolyglotGetNodeFromName(&document,wxS("Installed"));
	for(wxXmlNode *child = OPolyglotGetNodeFromName(&document,wxS("Installed"))->GetChildren();child;child=child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("IdInstalled")))
		{
			listIdInstalled.Add(child->GetAttribute(wxS("id")));
		}
	}
	for(size_t i =0;i < listIdInstalled.GetCount();i++)
	{
		if(listIdForInstalled.Index(listIdInstalled.Item(i)) == wxNOT_FOUND)
		{
			OPOLYGLOT_DEBUG(wxT("%s need remove"),listIdInstalled.Item(i));

			wxXmlNode *child= OPolyglotGetNodeFromName(&document,wxS("Installed"))->GetChildren();
			while(child)
			{
				if(child->GetName().IsSameAs(wxS("IdInstalled")))
				{
					if(child->GetAttribute(wxS("id")).IsSameAs(listIdInstalled.Item(i)))
					{
						for(wxXmlNode* fileToRemove=child->GetChildren();fileToRemove;)
						{

						}
					}
				} else{
					OPOLYGLOT_WARNING(wxT("node is not \"IdInstalled\" \"%s\""),child->GetName());
					child->GetNext();
				}
			}
		}
	}

	

	OPOLYGLOT_DEBUG(wxT("files to download %ld"),listIdForInstalled.GetCount());
	for(;0 < listIdForInstalled.GetCount();listIdForInstalled.RemoveAt(0))
	{
		wxXmlNode *node = OPolyglotGetNodeFromId(&document,listIdForInstalled.Item(0));
		if(listIdInstalled.Index(node->GetAttribute(wxS("id"))) == wxNOT_FOUND)
		{
			urlsXML.Add(node);
		}
		if(node == NULL)
		{
			OPOLYGLOT_ERROR(wxT("error not find node %s"),listIdForInstalled.Item(0));
			wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("error not find nod"),idListLanguage.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			progress->Destroy();
			this->Show(true);
			return;

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
		this->Show(true);
	}
	//doc.~wxXmlDocument();
#if 0
	/*
	 * remove not select languages
	 */
	OPOLYGLOT_MESSAGE(wxT("start deleting unused language files"));
	progress = new wxProgressDialog(wxT("OPolyglot install languages"),_("deleting unused language files before installation"),1000,this,wxPD_APP_MODAL|wxPD_CAN_ABORT);
	progress->Show();
	wxArrayString filesToInstalled;
	for(size_t i = 0; i < this->ListLanguage->GetCount();i++)
	{
		if(this->ListLanguage->IsChecked(i))
		{
			for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child=child->GetNext())
			{
				if((child->GetName().Cmp(wxT("Language")) == 0)
						&&(this->ListLanguage->GetString(i).Cmp(OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(child)) == 0))
				{
					OPOLYGLOT_DEBUG(wxT("save %s"),OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(child));
					for(wxXmlNode *saveNode = child->GetChildren();saveNode;saveNode = saveNode->GetNext())
					{
						if(saveNode->GetName().IsSameAs(wxT("File")))
						{
							if(filesToInstalled.Index(saveNode->GetNodeContent()) == wxNOT_FOUND)
							{
								filesToInstalled.Add(saveNode->GetNodeContent());
							}
						}
					}

				}
			}

		}
	}
	OPOLYGLOT_DEBUG(wxT("filesToInstalled.GetCount(%ld)"),filesToInstalled.GetCount());
	for(size_t i =0; i < filesToInstalled.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("saveFile.Item(%ld) = %s"),i,filesToInstalled.Item(i));
	}
#if 0
	progress->Destroy();
	this->Enable(true);
	return;
#endif
#if 1
	/* the cycle of removal of unused languages */
	for(size_t i = 0; i < this->ListLanguage->GetCount();i++)
	{
		if(!this->ListLanguage->IsChecked(i))
		{
			for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child=child->GetNext())
			{

				if((child->GetName().Cmp(wxT("Language")) == 0)
						&&(this->ListLanguage->GetString(i).Cmp(OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(child)) == 0))
				{
					for(wxXmlNode *removeNode=child->GetChildren();removeNode;removeNode = removeNode->GetNext())
			//		{
						if(removeNode->GetName().IsSameAs(wxT("File")))
						{
							OPOLYGLOT_DEBUG(wxT("check for remove %s"),removeNode->GetNodeContent());
							if((filesToInstalled.Index(removeNode->GetNodeContent()) == wxNOT_FOUND)&&(OPolyglotCheckForInstallFile(removeNode)))
							{
								OPOLYGLOT_DEBUG(wxT("removeNode %s : %s"),removeNode->GetName(),removeNode->GetNodeContent());
								if(removeNode->GetNodeContent().Find(wxT("tessdata")) != wxNOT_FOUND)
								{
									wxString file = removeNode->GetAttribute(wxT("fileCheckForInstall"));
									OPOLYGLOT_DEBUG(wxT("remove file %s"),wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,file));
									if(!wxRemoveFile(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,file)))
									{
										OPOLYGLOT_WARNING(wxT("can`t delete the file %s"),file);
										wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("warning can't delete the file"),file),wxT("OPolyglot"),wxOK|wxICON_ERROR);
										msg.ShowModal();
									}
									file.Replace(wxT("fast"),wxT("best"));
									OPOLYGLOT_DEBUG(wxT("remove file %s/%s"),OPOLYGLOT_USER_DATA,file);
									if(!wxRemoveFile(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,file)))
									{
										OPOLYGLOT_WARNING(wxT("can`t delete the file %s/%s"),OPOLYGLOT_USER_DATA,file);
										wxMessageDialog msg(this,wxString::Format(wxT("%s %s/%s"),_("warning can't delete the file"),OPOLYGLOT_USER_DATA,file),wxT("OPolyglot"),wxOK|wxICON_ERROR);
										msg.ShowModal();
									}
								} else
								{
									wxString path = wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,removeNode->GetAttribute(wxT("fileCheckForInstall")).BeforeFirst(wxT('/')));
									OPOLYGLOT_DEBUG(wxT("remove dir %s"),path);
									if(!wxDir::Remove(path,wxPATH_RMDIR_RECURSIVE))
									{
										OPOLYGLOT_WARNING(wxT("can`t delete the dir %s"),path);
										wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("warning can`t delete the dir"),path),wxT("OPolyglot"),wxOK|wxICON_ERROR);
										msg.ShowModal();
									}

								}
							} 
						}
					}
				}
			} /* for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child=child->GetNext()) */
		} /* if(!this->ListLanguage->IsChecked(i)) */
	} /* for(size_t i = 0; i < this->ListLanguage->GetCount();i++) */
	//filesUrl.Clear();
	/*
	 * start create list download files
	 */
#if 0
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
							if(!OPolyglotCheckForInstallLanguage(child))
							{
								OPOLYGLOT_DEBUG(wxT("add download file %s"),c->GetNodeContent());
								filesDownload.Add(c->GetNodeContent());
							}

						} /* if(c->GetName().Cmp(wxT("File")) == 0) */
					}
				}
			}
		}
	}
#endif
	OPOLYGLOT_INFO(wxT("filesDownload.GetCount %ld"),filesToInstalled.GetCount());

	urlsXML.Clear();
	for(;0 < filesToInstalled.GetCount();filesToInstalled.RemoveAt(0))
	{
		if(!OPolyglotCheckForInstallFile(OPolyglotGetNodeFile(&doc,filesToInstalled.Item(0))))
		{
			bool flagNotFind = true; /* do not remove, is a check for the corrctness od download.xml */
			for(wxXmlNode *urlNode = doc.GetRoot()->GetChildren();(urlNode!= NULL)&&flagNotFind;urlNode = urlNode->GetNext())
			{
				if(urlNode->GetName().IsSameAs(wxT("Url")))
				{
					if(urlNode->GetAttribute(wxT("file")).IsSameAs(filesToInstalled.Item(0)))
					{
						urlsXML.Add(urlNode);
						flagNotFind = false;
					}
				}
			}
			if(flagNotFind)
			{
				OPOLYGLOT_ERROR(wxT("url for file %s not found"),filesToInstalled.Item(0)); 
				wxMessageDialog msg(this,wxString::Format(wxT("url for file :%s not found"),filesToInstalled.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
				msg.ShowModal();

				progress->Destroy();
				this->Enable(true);
				return;
			}
		} else
		{
			OPOLYGLOT_DEBUG(wxT("this file installed %s"),filesToInstalled.Item(0));
		}
	}
#if 0
	for(;0 <filesDownload.GetCount();filesDownload.RemoveAt(0))
	{
		bool flagAdd = true; /* do not remove, is a check for the correctness of download.xml */
		for(wxXmlNode *filesUrl=doc.GetRoot()->GetChildren();filesUrl&&flagAdd;filesUrl=filesUrl->GetNext())
		{
			if(filesUrl->GetName().IsSameAs(wxT("Url")))
			{
#if 0	
				for(wxXmlNode *urlXml = filesUrl->GetChildren();urlXml;urlXml = urlXml->GetNext())
				{
#endif
					OPOLYGLOT_DEBUG(wxT("%s - %s"),filesDownload.Item(0),filesUrl->GetAttribute(wxT("file")));
					if(filesDownload.Item(0).IsSameAs(filesUrl->GetAttribute(wxT("file"))))
					{
						urlsXML.Add(filesUrl);
						flagAdd = false;

					}
#if 0
				}
#endif

			}
		} /* for(wxXmlNode *filesUrl=doc.GetRoot()->GetChildren();filesUrl;filesUrl=filesUrl->GetNext()) */
		/* do not remove, is a check for the correctness of download.xml */
		if(flagAdd)
		{
			OPOLYGLOT_ERROR(wxT("url for file %s not found"),filesDownload.Item(0)); 
			wxMessageDialog msg(this,wxString::Format(wxT("url for file :%s not found"),filesDownload.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			return;
		}
	}
#endif
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
#endif
#endif
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
	//wxXmlDocument doc;
	OPOLYGLOT_MESSAGE();
#if 0
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR("Load %s",OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
#endif
	this->ListLanguage->Clear();
	//wxXmlNode *nodeInstalled = OPolyglotGetNodeFromName(&doc,wxS("Installed"));
	for(wxXmlNode *child=document.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().Cmp(wxS("Language")) == 0)
		{
			bool flagFind = true;
			this->ListLanguage->InsertItems(1,(new wxString(OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(&document,child))),this->ListLanguage->GetCount());
			idListLanguage.Add(child->GetAttribute(wxS("id")));
			wxXmlNode *nodeInstalled = OPolyglotGetNodeFromName(&document,wxS("Installed"));
			for(wxXmlNode *idFile = child->GetChildren();idFile&&flagFind;idFile = idFile->GetNext())
			{
				if(idFile->GetName().IsSameAs(wxT("Id")))
				{
					flagFind = false;
					for(wxXmlNode *idInstalled=nodeInstalled->GetChildren();idInstalled&&(!flagFind);idInstalled = idInstalled->GetNext())
					{
						if(idInstalled->GetName().IsSameAs(wxS("IdInstalled")))
						{
							if(idFile->GetNodeContent().IsSameAs(idInstalled->GetAttribute(wxS("id"))))
							{
								flagFind = true;
							}
						}
					}
				}
			}
			this->ListLanguage->Check(this->ListLanguage->GetCount()-1,flagFind);
		}
	}
	OPOLYGLOT_DEBUG(wxT("finish create this->ListLanguage"));
	//doc.~wxXmlDocument();
#if 0
	while(child)
	{
		if(child->GetName().Cmp(wxT("Language")) == 0)
		{
			this->ListLanguage->InsertItems(1,(new wxString(OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(child))),this->ListLanguage->GetCount());
#if 0
			if(wxFileName::FileExists(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,child->GetAttribute(wxT("configfile"))))
					&&wxFileName::FileExists(wxString::Format(wxT("%s/%s.traineddata"),OPOLYGLOT_USER_DATA,child->GetAttribute(wxT("ocrfile")))))
#endif
			if(OPolyglotCheckForInstallLanguage(child))
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
#endif
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
			OPOLYGLOT_DEBUG(wxT("wxWebRequestEvent::State_Active finish"));
			break;
		case wxWebRequest::State_Completed:
			{
				wxArrayString newFiles;
				wxArrayString newDirs;
				wxXmlNode 		 *nodeInstalled;
				bool flagZipOk = true;
				//wxXmlDocument doc;
				timeDownload.Pause();
				OPOLYGLOT_DEBUG(wxS("wxWebRequest::State_Completed"));
#if 0
				if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
				{
					OPOLYGLOT_ERROR("Load %s",OPOLYGLOT_GET_XML_DATA_FILE);
					wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
					msg.ShowModal();
					return;
				}
#endif
				nodeInstalled = OPolyglotGetNodeFromName(&document,wxS("Installed"));
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
								this->Show(true);
								//wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES));
								return;
							}
						}
						

					} 

				}
				wxMemoryInputStream min(dataReceiv->GetData(),dataReceiv->GetDataLen());
				wxZipInputStream zip(min);
				wxZipEntry *entry =zip.GetNextEntry();
#if 1
				wxXmlNode *node = new wxXmlNode(nodeInstalled,wxXML_ELEMENT_NODE  ,(const wxString)wxString("IdInstalled"));
				node->AddAttribute(wxS("id"),urlsXML.Item(0)->GetAttribute(wxT("id")));
#endif
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
								this->Show(true);
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
								wxXmlNode *file = new wxXmlNode(node,wxXML_ELEMENT_NODE  ,(const wxString)wxString("FileInstalled"));
								file->AddAttribute(wxS("file"),OPOLYGLOT_FILE_FROM_STRING(entry->GetName()));

								newFiles.Add(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,entry->GetName()));
							} else
							{
								OPOLYGLOT_WARNING(wxT("file exist %s"),OPOLYGLOT_FILE_FROM_STRING(entry->GetName()));
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
					OPOLYGLOT_DEBUG(wxT("file download %p %s: %s"),urlsXML.Item(0)->GetParent(),urlsXML.Item(0)->GetAttribute(wxT("id")),urlsXML.Item(0)->GetAttribute(wxT("file")));
					for(; 0 < newFiles.GetCount();newFiles.RemoveAt(0))
					{

						OPOLYGLOT_DEBUG(wxT("add node file %s"),newFiles.Item(0));
					}

#if 0
					if(!doc.Save(OPOLYGLOT_GET_XML_DATA_FILE))
					{
						OPOLYGLOT_ERROR(wxS("error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
						wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error save file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
						msg.ShowModal();
						return;
					}
#endif
					urlsXML.RemoveAt(0);
					if(0 < urlsXML.GetCount())
					{
						fileRequest = this->CreateRequest(urlsXML.Item(0)->GetNodeContent());
						fileRequest.Start();
					} else
					{
						progress->Destroy();
						this->ScanLangs();
						this->Show(true);
						//wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES));
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
						this->Show(true);
						//wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES));
					}
				} 



			}
			break;
		case wxWebRequest::State_Failed:
			{
				OPOLYGLOT_ERROR(wxT("State_Failed %s %s"),(wxString)event.GetErrorDescription(),urlsXML.Item(0)->GetNodeContent());
				wxString strError = wxString::Format(wxT("download %s\n%s"),event.GetErrorDescription(),urlsXML.Item(0)->GetNodeContent());
				progress->Destroy();
				this->Show(true);
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
			this->Show(true);
			this->ScanLangs();
			break;
	}
}

OPolyglotDownloadLanguage::~OPolyglotDownloadLanguage()
{
	OPOLYGLOT_MESSAGE();
	if(!document.Save(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxS("error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error save file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	wxMilliSleep(200);
	wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES));
}
