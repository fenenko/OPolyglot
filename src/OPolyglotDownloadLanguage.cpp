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
#ifndef __WXMSW__
#include "../res/icon.xpm"
#endif
#include "OPolyglotEvent.h"
#include <tomcrypt.h>


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
#define OPOLYGLOT_USER_AGENT	wxString::Format(wxT("OPolyglot offline translator %s"), wxGetOsDescription())
enum{
	TIMER_ID,
};

#include <wx/arrimpl.cpp> 




wxString convertSizeToLabelHuman(size_t size)
{
	wxString ret = wxEmptyString;
	double value = (double)size;
	ret = wxString::Format(wxS("%ld B    "),size);
	if(1024.0 < value)	
	{
		value = value/1024.0;
		ret = wxString::Format(wxS("%.1f KB   "),value);
		if(1024.0 < value)
		{
			value = value/1024;
			ret = wxString::Format(wxS("%.1f MB   "),value);
			if(1024.0 < value)
			{
				value = value/1024;
				ret = wxString::Format(wxS("%.1f GB   "),value);
			}
		}
	}
	return ret;
}

OPolyglotProgressInstallLanguage::OPolyglotProgressInstallLanguage(wxWindow *parent,size_t size) : GUIOPolyglotProgressInstallLanguage(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgressInstallLanguage"));
	this->parent = parent;
	timerUpdate.SetOwner(this,TIMER_ID);
	sizeToDownload = size;
	prevSizeDownload = 0;
	downloadedBytes = 0;
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
#else
	SetIcon(wxICON(icon));
#endif
	this->Bind(wxEVT_TIMER,&OPolyglotProgressInstallLanguage::OnUpdateProgress,this);
	this->SizeAll->SetLabel(convertSizeToLabelHuman(sizeToDownload));
	timeRun.Start();
	//this->LabelProgress->SetLabel(wxString::Format(wxS("%s %ld\t:\t%ld"),_("Progress"),downloadedFiles,countFiles));
	timerUpdate.Start(500);
	this->Show();
	this->HBox1->Layout();
	this->HBox2->Layout();
	this->HBox3->Layout();
	this->Refresh();
	this->MainBox->Fit(this);
	this->MainBox->Layout();
	int w,h;
	this->GetSize(&w,&h);
	this->SetSize(640,h);
}

OPolyglotProgressInstallLanguage::~OPolyglotProgressInstallLanguage()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotProgressInstallLanguage"));
}


void OPolyglotProgressInstallLanguage::OnCancel( wxCommandEvent& event )
{
	wxMutexLocker lock(mutex);
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgressInstallLanguage::OnCancel"));
	wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER));
}


void OPolyglotProgressInstallLanguage::OnClose( wxCloseEvent& event ) 
{
	wxMutexLocker lock(mutex);
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgressInstallLanguage::OnClose"));
	wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER));
}

void OPolyglotProgressInstallLanguage::OnUpdateProgress(wxTimerEvent &event)
{
	double speed;
	double timeRemaining;
	double timeElapsed;
	wxString prefix = _("Bytes/S");
	wxString prefixTime = _("s    ");
	wxMutexLocker lock(mutex);
	timeRun.Pause();
	speed = (double)(downloadedBytes*1000) / (double)(timeRun.Time() ); /* per second */
	timeElapsed = ((double)timeRun.Time())/1000.0;
	timeRun.Resume();
	timeRemaining =  ((double)(sizeToDownload-downloadedBytes))/(double)speed;
	if(512.0 < speed)
	{
		speed = speed / 1024.0;
		prefix = _("KiB/S");
		if(512.0 < speed)
		{
			speed = speed /1024.0;
			prefix = _("MiB/S");
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
	if(0 < downloadedBytes)
	{
		this->TimeRemaining->SetLabel(wxString::Format(wxS("%0.1f %s"),timeRemaining,prefixTime));
	} else
	{
		this->TimeRemaining->SetLabel(wxS("infinity"));
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
	this->MainBox->Layout();
	this->Refresh();
	//OPOLYGLOT_DEBUG(wxS("%0.2f time remaining %0.1f"),progressDownloaded,timeRemaining);
}

void OPolyglotProgressInstallLanguage::SetDownloadProgress(size_t download,size_t allSize)
{
	wxMutexLocker lock(mutex);
	//OPOLYGLOT_DEBUG(wxT("%ld : %ld"),download,allSize);
	downloadedBytes += (download - prevSizeDownload);
	prevSizeDownload = download;
	this->FileProgress->SetValue((int)((download*(this->FileProgress->GetRange()))/allSize));
	this->SizeFile->SetLabel(convertSizeToLabelHuman(allSize-download));
	this->HBox1->Layout();
	this->HBox2->Layout();
	this->HBox3->Layout();
	this->MainBox->Layout();
	this->Refresh();
}

void OPolyglotProgressInstallLanguage::FinishDownloadFile()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgressInstallLanguage::FinishDownloadFile"));
	wxMutexLocker lock(mutex);
	this->AllProgress->SetValue((int)(downloadedBytes*(this->AllProgress->GetRange())/sizeToDownload));
	prevSizeDownload = 0;
	this->FileProgress->SetValue(0);
	this->SizeAll->SetLabel(convertSizeToLabelHuman(sizeToDownload-downloadedBytes));
	this->HBox1->Layout();
	this->HBox2->Layout();
	this->HBox3->Layout();
	this->MainBox->Layout();
	this->Refresh();
}



OPolyglotDownloadLanguage::OPolyglotDownloadLanguage(wxEvtHandler *handler):GUIOPolyglotDownloadLanguage(NULL)
{
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxPoint pos;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OPolyglotDownloadLanguage"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
#else
	SetIcon(wxICON(icon));
#endif
	this->handler = handler;
	dataReceiv = new wxMemoryBuffer(1024);
	pos = wxPoint((geom.width-this->GetSize().GetWidth())/2,(geom.height-this->GetSize().GetHeight())/2);
	OPOLYGLOT_MESSAGE(wxT("user-agent %s"),OPOLYGLOT_USER_AGENT);
	this->v_box->Layout();
	this->Refresh();
	this->SetPosition(pos);
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
	this->Bind(wxEVT_WEBREQUEST_STATE,&OPolyglotDownloadLanguage::OnFileDownload,this);
	this->Bind(wxEVT_WEBREQUEST_DATA,&OPolyglotDownloadLanguage::OnDataDownload,this);
	this->Bind(wxEVT_TIMER,&OPolyglotDownloadLanguage::OnTimerProgressUpdate,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglotDownloadLanguage::OnCancelUser,this);
	xmlLanguages = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Languages"));
	this->ScanLangs();
	this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	//wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_HIDE));
}

void OPolyglotDownloadLanguage::OnClose(wxCloseEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnClose"));
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}

void OPolyglotDownloadLanguage::OnApply(wxCommandEvent& event)
{
	bool flagNotCancelUser = true;
	size_t sizeToDownload = 0;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnApply "));
	OPOLYGLOT_DEBUG(wxT("%s"),xmlLanguages->GetName());
	wxArrayString listIdToInstallation;
	wxArrayString listIdInstalled;
	for(size_t i=0; i < this->ListLanguage->GetCount();i++)
	{
		if(this->ListLanguage->IsChecked(i))
		{
			OPOLYGLOT_DEBUG(wxT("%s"),ListLanguage->GetStrings().Item(i));
			for(wxXmlNode *child = xmlLanguages->GetChildren();child;child = child->GetNext())
			{
				if(child->GetName().IsSameAs("Label")
						&&(ListLanguage->GetStrings().Item(i).IsSameAs(child->GetAttribute(wxS("label")))))
				{
					for(wxXmlNode *childId = child->GetChildren();childId;childId = childId->GetNext())
					{
						if(childId->GetName().IsSameAs(wxS("Id")))
						{
							if(listIdToInstallation.Index(childId->GetAttribute(wxS("id"))) == wxNOT_FOUND)
							{
								OPOLYGLOT_DEBUG(wxT("%s %s"),child->GetAttribute(wxS("label")),childId->GetAttribute(wxS("id")));
								listIdToInstallation.Add(childId->GetAttribute(wxS("id")));
							}
						}
					}
				}
			}
		}
	}
	for(wxXmlNode *child = OPolyglotGetNodeFromName(&document,wxS("Installed"))->GetChildren();child;child=child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("IdInstalled")))
		{
			listIdInstalled.Add(child->GetAttribute(wxS("id")));
		}
	}
	/*
	 * cycle for remove language
	 */
	wxProgressDialog removeProgress(wxS("OPolyglot"),_("remove languages"),1000,this);
	removeProgress.Show();
	wxArrayString filesToRemove;
	wxArrayString dirsToRemove;
	for(size_t i =0;(i < listIdInstalled.GetCount())&&(flagNotCancelUser);i++)
	{
		if(listIdToInstallation.Index(listIdInstalled.Item(i)) == wxNOT_FOUND)
		{
			/*
			 * if id is not found in the id list for installation, then the deletion is started 
			 */
			OPOLYGLOT_MESSAGE(wxT("%s need remove"),listIdInstalled.Item(i));
			flagNotCancelUser= removeProgress.Pulse();
			wxXmlNode *child= OPolyglotGetNodeFromName(&document,wxS("Installed"))->GetChildren();
			while(child)
			{
				if(child->GetName().IsSameAs(wxS("IdInstalled")))
				{
					if(child->GetAttribute(wxS("id")).IsSameAs(listIdInstalled.Item(i)))
					{
						for(wxXmlNode* fileToRemove=child->GetChildren();fileToRemove;fileToRemove = fileToRemove->GetNext())
						{
							if(fileToRemove->GetName().IsSameAs(wxS("FileInstalled")))
							{
								filesToRemove.Add(fileToRemove->GetAttribute(wxS("file")));
							}
							if(fileToRemove->GetName().IsSameAs(wxS("DirCreated")))
							{
								dirsToRemove.Add(fileToRemove->GetAttribute(wxS("dir")));
							}
						}
						wxXmlNode *next = child->GetNext();
						if(!OPolyglotGetNodeFromName(&document,wxS("Installed"))->RemoveChild(child))
						{
							OPOLYGLOT_ERROR(wxT("could not remove Node %s"),child->GetAttribute(wxT("id")));
						} else
						{
						}
						child = next;
						
					} else
					{
						child = child->GetNext();
					}
				} else{
					OPOLYGLOT_WARNING(wxT("node is not \"IdInstalled\" != \"%s\""),child->GetName());
					child = child->GetNext();
				}
			}
		}
		removeProgress.Pulse();
	}
	for(wxXmlNode *child = OPolyglotGetNodeFromName(&document,wxS("Installed"))->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("IdInstalled"))&&(listIdToInstallation.Index(child->GetAttribute(wxS("id"))) != wxNOT_FOUND))
		{
			for(wxXmlNode *file = child->GetChildren();file;file = file->GetNext())
			{
				if(file->GetName().IsSameAs(wxS("FileInstalled")))
				{
					while(filesToRemove.Index(file->GetAttribute(wxS("file"))) != wxNOT_FOUND)
					{
						filesToRemove.Remove(file->GetAttribute(wxS("file")));
					}
				} 
				if(file->GetName().IsSameAs(wxS("DirCreated")))
				{
					while(dirsToRemove.Index(file->GetAttribute(wxS("dir"))) != wxNOT_FOUND)
					{
						dirsToRemove.Remove(file->GetAttribute(wxS("dir")));
					}
				}

			}
		}
		removeProgress.Pulse();
	}
	for(size_t i =0 ; i < filesToRemove.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("file to remove:%s"),filesToRemove.Item(i));
		if(wxFileName::FileExists(filesToRemove.Item(i)))
		{
			if(!wxRemoveFile(filesToRemove.Item(i)))
			{
				OPOLYGLOT_WARNING(wxT("OnApply not remove %s file"),filesToRemove.Item(i));
			} 
		} else
		{
			OPOLYGLOT_WARNING(wxT("OnApply remove file,file %s not find"),filesToRemove.Item(i));
		}
		removeProgress.Pulse();
	}
	for(size_t i = 0; i < dirsToRemove.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("dir to remove:%s"),dirsToRemove.Item(i));
		if(wxFileName::DirExists(dirsToRemove.Item(i)))
		{
			if(!wxFileName::Rmdir(dirsToRemove.Item(i)))
			{
				OPOLYGLOT_WARNING(wxT("OnApply not remove %s dir"),dirsToRemove.Item(i));
			}
		} else
		{
			OPOLYGLOT_WARNING(wxT("OnApply remove dir,dir %s not find"),dirsToRemove.Item(i));
		}
		removeProgress.Pulse();
	}

	if(!document.Save(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxS("error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error save file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	removeProgress.Destroy();

	

	OPOLYGLOT_DEBUG(wxT("files to download %ld"),listIdToInstallation.GetCount());
	for(;0 < listIdToInstallation.GetCount();listIdToInstallation.RemoveAt(0))
	{
		wxXmlNode *node = OPolyglotGetNodeFromId(&document,listIdToInstallation.Item(0));
		/* check that the file is not installed */
		if(listIdInstalled.Index(node->GetAttribute(wxS("id"))) == wxNOT_FOUND)
		{
			long size;
			if(node->GetAttribute(wxS("size")).ToLong(&size,10))
			{
				sizeToDownload += size;
			}
			urlsXML.Add(node);
		}
		if(node == NULL)
		{
			OPOLYGLOT_ERROR(wxT("error not find node %s"),listIdToInstallation.Item(0));
			wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("error not find node"),listIdToInstallation.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			this->Show(true);
			return;

		}
	}
	OPOLYGLOT_INFO(wxT("finish create urlsXML %ld"),urlsXML.GetCount());
	if(0 < urlsXML.GetCount())
	{
		progress = new OPolyglotProgressInstallLanguage(this,sizeToDownload);
		this->Show(false);
		for(size_t i = 0; i < urlsXML.GetCount();i++)
		{
			OPOLYGLOT_DEBUG(wxT("%ld url %s"),i+1,urlsXML.Item(i)->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
		}
		OPOLYGLOT_DEBUG(wxT("start download %s"),urlsXML.Item(0)->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
		mutexFileRequest.Lock();
		fileRequest = this->CreateRequest(urlsXML.Item(0)->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
		fileRequest.Start();
		mutexFileRequest.Unlock();
	} else
	{
		//progress->Destroy();
	}
}


wxWebRequest OPolyglotDownloadLanguage::CreateRequest(wxString url)
{
	wxWebRequest ret;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::CreateRequest %s"),url);
	ret = wxWebSession::GetDefault().CreateRequest(this,url);
	ret.SetHeader(wxT("User-Agent"), OPOLYGLOT_USER_AGENT);
	ret.SetStorage(wxWebRequest::Storage_None);
	return ret;
}

void OPolyglotDownloadLanguage::OnTimerProgressUpdate(wxTimerEvent &event)
{
	wxMutexLocker lock(mutexFileRequest);
}

void OPolyglotDownloadLanguage::OnDataDownload(wxWebRequestEvent& event)
{
	wxString prefix = wxT("Bytes");
	wxMutexLocker lock(mutexFileRequest);
	dataReceiv->AppendData(event.GetDataBuffer(),event.GetDataSize());
	progress->SetDownloadProgress(dataReceiv->GetDataLen(),fileRequest.GetBytesExpectedToReceive());
}

void OPolyglotDownloadLanguage::ScanLangs()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::ScanLangs"));
	this->ListLanguage->Clear();
	//listLanguages.Clear();
	wxArrayString idLanguagesAdd;
	delete xmlLanguages;
	xmlLanguages = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Languages"));
	for(wxXmlNode *childToEng=document.GetRoot()->GetChildren();childToEng;childToEng = childToEng->GetNext())
	{
		if(childToEng->GetName().IsSameAs(wxS("Language"))
				&&childToEng->GetAttribute(wxS("to")).IsSameAs(wxS("English"))
				&&(idLanguagesAdd.Index(childToEng->GetAttribute(wxS("id"))) == wxNOT_FOUND))
		{
				/*
				   *Створення Назви Spanish | tiny
				   * для прикладу з двох мов Spanish->English|tiny , English->Spanish|tiny
				   */
			bool flagFindTwoLanguages=false;
			for(wxXmlNode *childFromEng=document.GetRoot()->GetChildren();childFromEng&&(!flagFindTwoLanguages);childFromEng=childFromEng->GetNext())
			{
				if(childFromEng->GetName().IsSameAs(wxS("Language"))
						&&childFromEng->GetAttribute(wxS("from")).IsSameAs(wxS("English"))
						&&childToEng->GetAttribute(wxS("type")).IsSameAs(childFromEng->GetAttribute(wxS("type")))
						&&childToEng->GetAttribute(wxS("from")).IsSameAs(childFromEng->GetAttribute(wxS("to")))
						&&(idLanguagesAdd.Index(childFromEng->GetAttribute(wxS("id"))) == wxNOT_FOUND))
				{
					flagFindTwoLanguages = true;
					wxXmlNode *xmlLang = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Label"));
					xmlLang->AddAttribute(wxS("label"),wxString::Format(wxS("%s | %s"),childToEng->GetAttribute(wxS("from")),childToEng->GetAttribute(wxS("type"))));
					idLanguagesAdd.Add(childToEng->GetAttribute(wxS("id")));
					idLanguagesAdd.Add(childFromEng->GetAttribute(wxS("id")));
					for(wxXmlNode *childId = childToEng->GetChildren();childId;childId = childId->GetNext())
					{
						if(childId->GetName().IsSameAs(wxS("Id")))
						{
							wxXmlNode *xmlId = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Id"));	
							xmlId->AddAttribute(wxS("id"),childId->GetAttribute(wxT("id")));
							xmlLang->AddChild(xmlId);
						}
					}
					for(wxXmlNode *childId = childFromEng->GetChildren();childId;childId = childId->GetNext())
					{
						if(childId->GetName().IsSameAs(wxS("Id")))
						{
							wxXmlNode *xmlId = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Id"));
							xmlId->AddAttribute(wxS("id"),childId->GetAttribute(wxS("id")));
							xmlLang->AddChild(xmlId);
						}
					}
					xmlLanguages->AddChild(xmlLang);

					
				}
			}
		}

	}
	for(wxXmlNode *child = document.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Language"))
				&&(idLanguagesAdd.Index(child->GetAttribute(wxS("id"))) == wxNOT_FOUND))
		{
			idLanguagesAdd.Add(child->GetAttribute(wxS("id")));
			wxXmlNode *xmlLang = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Label"));
			xmlLang->AddAttribute(wxS("label"),wxString::Format(wxS("%s\t%s -> %s | %s"),child->GetAttribute(wxS("language")),child->GetAttribute(wxS("from")),child->GetAttribute(wxS("to")),child->GetAttribute(wxS("type"))));
			for(wxXmlNode *childId = child->GetChildren();childId;childId=childId->GetNext())
			{
				if(childId->GetName().IsSameAs(wxS("Id")))
				{
					wxXmlNode *xmlId = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Id"));
					xmlId->AddAttribute(wxS("id"),childId->GetAttribute(wxS("id")));
					xmlLang->AddChild(xmlId);
				}
			}
			xmlLanguages->AddChild(xmlLang);
		}
	}
	OPOLYGLOT_DEBUG(wxT("debug"));
	wxXmlNode *xmlInstalled = NULL;
	for(wxXmlNode *child = document.GetRoot()->GetChildren();child&&(xmlInstalled == NULL);child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Installed")))
		{
			xmlInstalled = child;
		}
	}
	for(wxXmlNode *child = xmlLanguages->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Label")))
		{
			ListLanguage->Append(child->GetAttribute(wxS("label")));
			bool flagCheck = true;
			OPOLYGLOT_DEBUG(wxT("%s -------------------"),child->GetAttribute(wxS("label")));
			for(wxXmlNode *child1 = child->GetChildren();child1&&flagCheck;child1=child1->GetNext())
			{
				OPOLYGLOT_DEBUG(wxT("%s"),child1->GetAttribute(wxS("id")));
				flagCheck = false;
				for(wxXmlNode *child2 = xmlInstalled->GetChildren();child2&&(!flagCheck);child2 = child2->GetNext())
				{
					if(child2->GetName().IsSameAs(wxS("IdInstalled")))
					{
						if(child1->GetAttribute(wxS("id")).IsSameAs(child2->GetAttribute(wxS("id"))))
						{
							flagCheck = true;
						}
					}
				}
			}
			if(flagCheck)
			{
				ListLanguage->Check(ListLanguage->GetCount()-1);
			}

		}
	}
}


void OPolyglotDownloadLanguage::OnCancelUser(wxThreadEvent &event)
{
	wxMutexLocker lock(mutexFileRequest);
	OPOLYGLOT_WARNING("OPolyglotDownloadLanguage::OnCancelUser");
	fileRequest.Cancel();
	progress->Destroy();
}


void OPolyglotDownloadLanguage::OnFileDownload(wxWebRequestEvent& event)
{
	wxMutexLocker lock(mutexFileRequest);
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnFileDownload"));
	switch(event.GetState())
	{
		case wxWebRequest::State_Idle:
			OPOLYGLOT_MESSAGE(wxT("State_Idle"));
			break;
		case wxWebRequest::State_Unauthorized:
			OPOLYGLOT_MESSAGE(wxT("State_Unauthorized"));
			break;
		case wxWebRequest::State_Active:
			OPOLYGLOT_INFO(wxT("wxWebRequestEvent::State_Active %s"),urlsXML.Item(0)->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
			dataReceiv->Clear();
			break;
		case wxWebRequest::State_Completed:
			{
				wxArrayString newFiles;
				wxArrayString newDirs;
				wxXmlNode 		 *nodeInstalled;
				bool flagZipOk = true;
				timeDownload.Pause();
				OPOLYGLOT_MESSAGE(wxS("wxWebRequest::State_Completed %s download time %.1f S, size %ld Bytes"),urlsXML.Item(0)->GetAttribute(wxT("file")) ,((double)timeDownload.Time())/1000.0,dataReceiv->GetDataLen());
				nodeInstalled = OPolyglotGetNodeFromName(&document,wxS("Installed"));
				if(dataReceiv->GetDataLen() != (size_t)fileRequest.GetBytesExpectedToReceive())
				{
					OPOLYGLOT_ERROR(wxT("received data %ld != %ld"),dataReceiv->GetDataLen(),fileRequest.GetBytesExpectedToReceive());
				}
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
								fileRequest = this->CreateRequest(urlsXML.Item(0)->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
								fileRequest.Start();
								return;
							} else
							{
								progress->Destroy();
								this->ScanLangs();
								this->Show(true);
								return;
							}
						}
					} 

				}
				wxMemoryInputStream min(dataReceiv->GetData(),dataReceiv->GetDataLen());
				wxZipInputStream zip(min);
				wxZipEntry *entry =zip.GetNextEntry();
				wxXmlNode *node = new wxXmlNode(nodeInstalled,wxXML_ELEMENT_NODE  ,(const wxString)wxString("IdInstalled"));
				node->AddAttribute(wxS("id"),urlsXML.Item(0)->GetAttribute(wxT("id")));
				while(entry&&flagZipOk)
				{
					if(zip.IsOk())
					{
						if(entry->IsDir())
						{
							OPOLYGLOT_DEBUG(wxT("mkdir %s"),OPOLYGLOT_DIR_FROM_STRING(entry->GetName()));
							wxXmlNode *dir = new wxXmlNode(node,wxXML_ELEMENT_NODE,(const wxString)wxString("DirCreated"));
							dir->AddAttribute(wxS("dir"),wxString::Format(wxS("%s/%s"),OPOLYGLOT_USER_DATA,entry->GetName()));
							if(!wxFileName::DirExists(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,entry->GetName())))
							{
								if(!wxDir::Make(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,entry->GetName())))
								{
									OPOLYGLOT_ERROR(wxT("create dir %s/%s"),OPOLYGLOT_USER_DATA,entry->GetName());
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
							newFiles.Add(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,entry->GetName()));
							if(!wxFileName::FileExists(OPOLYGLOT_FILE_FROM_STRING(entry->GetName())))
							{
								wxFileOutputStream out(OPOLYGLOT_FILE_FROM_STRING(entry->GetName()));
								zip.Read(out);

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
					zip.CloseEntry();
					entry = zip.GetNextEntry();
				}
				zip.CloseEntry();
				if(flagZipOk)
				{
					OPOLYGLOT_DEBUG(wxT("file download %ld %s: %s"),newFiles.GetCount(),urlsXML.Item(0)->GetAttribute(wxT("id")),urlsXML.Item(0)->GetAttribute(wxT("file")));
					for(; 0 < newFiles.GetCount();newFiles.RemoveAt(0))
					{

						OPOLYGLOT_DEBUG(wxT("add node file %s"),newFiles.Item(0));
						wxXmlNode *file = new wxXmlNode(node,wxXML_ELEMENT_NODE  ,(const wxString)wxString("FileInstalled"));
						file->AddAttribute(wxS("file"),newFiles.Item(0));
					}
					if(!document.Save(OPOLYGLOT_GET_XML_DATA_FILE))
					{
						OPOLYGLOT_ERROR(wxS("error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
						wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error save file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
						msg.ShowModal();
						return;
					}
					progress->FinishDownloadFile();

					urlsXML.RemoveAt(0);
					if(0 < urlsXML.GetCount())
					{
						fileRequest = this->CreateRequest(urlsXML.Item(0)->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
						fileRequest.Start();
					} else
					{
						progress->Destroy();
						this->ScanLangs();
						this->Show(true);
						
						//wxQueueEvent(this,new wxCloseEvent());
						wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
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
						fileRequest = this->CreateRequest(urlsXML.Item(0)->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
						fileRequest.Start();
					} else
					{
						this->ScanLangs();
						this->Show(true);
					}
				} 
			}
			break;
		case wxWebRequest::State_Failed:
			{
				OPOLYGLOT_ERROR(wxT("State_Failed %s %s"),(wxString)event.GetErrorDescription(),urlsXML.Item(0)->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
				wxString strError = wxString::Format(wxT("download %s\n%s"),event.GetErrorDescription(),urlsXML.Item(0)->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
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
	OPOLYGLOT_MESSAGE("OPolyglotDownloadLanguage::~OPolyglotDownloadLanguage");
	mutexFileRequest.Lock();
	if(!document.Save(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxS("error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error save file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	dataReceiv->Clear();
	delete dataReceiv;
	mutexFileRequest.Unlock();
	wxMilliSleep(200);
}
