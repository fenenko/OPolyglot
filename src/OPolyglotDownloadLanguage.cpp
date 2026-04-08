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

OPolyglotProgressInstallLanguage::OPolyglotProgressInstallLanguage(wxWindow *parent,size_t size) : GUIOPolyglotProgressInstallLanguage(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgressInstallLanguage"));
	this->SetTitle(wxString::Format(wxT("OPolyglot %s"),_("install languages")));
	OPOLYGLOT_MESSAGE(wxT("MESSAGE TEST"));
	this->parent = parent;
	timerUpdate.SetOwner(this,TIMER_ID);
	sizeToDownload = size;
	prevSizeDownload = 0;
	downloadedBytes = 0;
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	this->Bind(wxEVT_TIMER,&OPolyglotProgressInstallLanguage::OnUpdateProgress,this);
	this->SizeAll->SetLabel(convertSizeToLabelHuman(sizeToDownload));
	timeRun.Start();
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
	wxString prefix = _("B/s    ");
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
	//OPOLYGLOT_DEBUG(wxT("%zu : %zu"),download,allSize);
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
	urlsXML = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Root"));
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

#if 0
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

	

	OPOLYGLOT_DEBUG(wxT("files to download %zu"),listIdToInstallation.GetCount());
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
			urlsXML->AddChild(new wxXmlNode(*node));
		}
		if(node == NULL)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::OnApply node not found %s"),listIdToInstallation.Item(0));
			wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Error: Node not found"),listIdToInstallation.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			this->Show(true);
			return;

		}
	}
	OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::OnApply start download"));
	if(urlsXML->GetChildren())
	{
		progress = new OPolyglotProgressInstallLanguage(this,sizeToDownload);
		this->Show(false);
		mutexFileRequest.Lock();
		fileRequest = this->CreateRequest(urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
		fileRequest.Start();
		mutexFileRequest.Unlock();
	} else
	{
		//progress->Destroy();
	}
}
#endif

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
	int scrollX,scrollY;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::ScanLangs"));
	this->ListLanguages->GetViewStart(&scrollX,&scrollY);
	OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::ScanLangs scroll %d %d"),scrollX,scrollY);
	/* This loop is for unbinding all buttons. */
	for(size_t i = 0; i < box->GetItemCount();i++)
	{
		//wxButton *button = (wxButton *)((wxBoxSizer *)(box->GetItem(i)->GetUserData())->GetItem(2)->GetUserData();
		wxButton *button = (wxButton *)(((wxBoxSizer *)(box->GetItem(i)->GetUserData()))->GetItem(2)->GetUserData());
		if(button->GetLabel().IsSameAs(_("Download")))
		{
		} else
		{
		}
	}
	this->box->Clear();
	wxArrayString labelFullLanguages;
	wxArrayString labelNotFullLanguages;
	wxArrayString idLanguagesAdd;
	delete xmlLanguages;
	xmlLanguages = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Languages"));
	OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::ScanLangs start build labelFullLanguages"));
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
						&&childToEng->GetAttribute(wxS("ver")).IsSameAs(childFromEng->GetAttribute(wxS("ver")))
						&&(idLanguagesAdd.Index(childFromEng->GetAttribute(wxS("id"))) == wxNOT_FOUND))
				{
					flagFindTwoLanguages = true;
					wxString label = wxString::Format(wxS("%s %s %s")
							,OPolyglotGetTranslateLanguage(childToEng->GetAttribute(wxS("from")))
							,childToEng->GetAttribute(wxS("type"))
							,childToEng->GetAttribute(wxS("ver")));
					if(labelFullLanguages.Index(label) == wxNOT_FOUND)
					{
						labelFullLanguages.Add(label);
						wxXmlNode *xmlLang = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Label"));
						xmlLang->AddAttribute(wxS("label"),label);
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
					} else
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::ScanLangs duplicate language label %s"),label);
						wxMessageDialog msg(this,wxString::Format(wxT("Error: for model %s find duplicate"),label),wxT("OPolyglot"),wxICON_ERROR|wxOK);
						msg.ShowModal();
					}


				}
			}
		}

	}
	labelFullLanguages.Sort();
	for(wxXmlNode *child = document.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Language"))
				&&(idLanguagesAdd.Index(child->GetAttribute(wxS("id"))) == wxNOT_FOUND))
		{
			idLanguagesAdd.Add(child->GetAttribute(wxS("id")));
			wxString label = wxString::Format(wxS("%s  \"%s -> %s\" %s %s")
					,OPolyglotGetTranslateLanguage(child->GetAttribute(wxS("language")))
					,OPolyglotGetTranslateLanguage(child->GetAttribute(wxS("from")))
					,OPolyglotGetTranslateLanguage(child->GetAttribute(wxS("to")))
					,child->GetAttribute(wxS("type"))
					,child->GetAttribute(wxS("ver")));
			if(labelNotFullLanguages.Index(label) == wxNOT_FOUND)	
			{
				labelNotFullLanguages.Add(label);
				wxXmlNode *xmlLang = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Label"));
				xmlLang->AddAttribute(wxS("label"),label);
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
			} else
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::ScanLangs duplicate language label %s"),label);
				wxMessageDialog msg(this,wxString::Format(wxT("Error: for model %s find duplicate"),label),wxT("OPolyglot"),wxICON_ERROR|wxOK);
				msg.ShowModal();
			}
		}
	}
	labelNotFullLanguages.Sort();
	for(size_t i =0; i < labelNotFullLanguages.GetCount();labelFullLanguages.Add(labelNotFullLanguages.Item(i)),i++);
	OPOLYGLOT_DEBUG(wxT("debug"));
	wxXmlNode *xmlInstalled = NULL;
	for(wxXmlNode *child = document.GetRoot()->GetChildren();child&&(xmlInstalled == NULL);child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Installed")))
		{
			xmlInstalled = child;
		}
	}
	for(size_t i = 0; i  <labelFullLanguages.GetCount();i++)
	{
		for(wxXmlNode *childLang = xmlLanguages->GetChildren();childLang;childLang = childLang->GetNext())
		{
			if(childLang->GetName().IsSameAs(wxS("Label"))
					&&(childLang->GetAttribute(wxS("label")).IsSameAs(labelFullLanguages.Item(i))))
			{
				bool flagInstalled = true;
				for(wxXmlNode *childId = childLang->GetChildren();childId&&flagInstalled;childId = childId->GetNext())
				{
					if(childId->GetName().IsSameAs(wxS("Id")))
					{
						flagInstalled = false;
						for(wxXmlNode *childIdInstalled = xmlInstalled->GetChildren();childIdInstalled&&(!flagInstalled);childIdInstalled = childIdInstalled->GetNext())
						{
							if(childIdInstalled->GetName().IsSameAs(wxS("IdInstalled")))
							{
								if(childIdInstalled->GetAttribute(wxS("id")).IsSameAs(childId->GetAttribute(wxS("id"))))
								{
									flagInstalled = true;
								}
							}
						}
					}
				}
				wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
				wxStaticText *label = new wxStaticText(this->ListLanguages,wxID_ANY,childLang->GetAttribute(wxS("label")),wxDefaultPosition,wxDefaultSize,0);
				sizer->Add(label,0,wxALL|wxEXPAND,2);
				sizer->Add( 0, 0, 1, wxEXPAND, 2 );
				if(flagInstalled)
				{
					wxButton *button = new wxButton(ListLanguages,wxID_ANY,_("Remove"),wxDefaultPosition,wxDefaultSize,0);
					childLang->AddAttribute(wxS("idButton"),wxString::Format(wxT("%d"),button->GetId()));
					OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::ScanLangs %s installed %d"),childLang->GetAttribute(wxS("label")),button->GetId());
					Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotDownloadLanguage::OnLanguageRemove,this,button->GetId(),button->GetId());
					sizer->Add(button,0,wxALL,2);
				} else
				{
					wxButton *button = new wxButton(ListLanguages,wxID_ANY,_("Download"),wxDefaultPosition,wxDefaultSize,0);
					childLang->AddAttribute(wxS("idButton"),wxString::Format(wxT("%d"),button->GetId()));
					OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::ScanLangs %s not install %d"),childLang->GetAttribute(wxS("label")),button->GetId());
					Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotDownloadLanguage::OnLanguageDownload,this,button->GetId(),button->GetId());
					sizer->Add(button,0,wxALL,2);
				}
				sizer->Layout();
				box->Add(sizer,0,wxALL|wxEXPAND,0);
				box->Layout();

			}
		}
	}
}


void OPolyglotDownloadLanguage::OnLanguageDownload(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnLanguageDownload %d"),event.GetId());
}

void OPolyglotDownloadLanguage::OnLanguageRemove(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnLanguageRemove %d"),event.GetId());
}

void OPolyglotDownloadLanguage::OnCancelUser(wxThreadEvent &event)
{
	wxMutexLocker lock(mutexFileRequest);
	OPOLYGLOT_WARNING("OPolyglotDownloadLanguage::OnCancelUser");
	fileRequest.Cancel();
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
			OPOLYGLOT_INFO(wxT("wxWebRequestEvent::State_Active %s"),urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
			dataReceiv->Clear();
			break;
		case wxWebRequest::State_Completed:
			{
				wxArrayString newFiles;
				wxArrayString newDirs;
				wxXmlNode 		 *nodeInstalled;
				bool flagZipOk = true;
				timeDownload.Pause();
				OPOLYGLOT_MESSAGE(wxS("wxWebRequest::State_Completed %s download time %.1f S, size %zu Bytes"),urlsXML->GetChildren()->GetAttribute(wxT("file")) ,((double)timeDownload.Time())/1000.0,dataReceiv->GetDataLen());
				nodeInstalled = OPolyglotGetNodeFromName(&document,wxS("Installed"));
				if(dataReceiv->GetDataLen() != (size_t)fileRequest.GetBytesExpectedToReceive())
				{
					OPOLYGLOT_ERROR(wxT("received data %zu != %zu"),dataReceiv->GetDataLen(),fileRequest.GetBytesExpectedToReceive());
				}
				if(urlsXML->GetChildren()->GetAttribute(wxT("sha1sum")).IsEmpty())
				{
					OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::OnFileDownload for file %s not sha1sum"),urlsXML->GetChildren()->GetAttribute(wxT("file")));
					wxMessageDialog msg(this
							,wxString::Format(wxT("%s: %s")
								,_("Warning: No SHA1 checksum for file")
								,urlsXML->GetChildren()->GetAttribute(wxT("file")))
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
						OPOLYGLOT_DEBUG(wxT("%s sha1sum %s"),urlsXML->GetChildren()->GetAttribute(wxT("file")),hexString);
						if(!urlsXML->GetChildren()->GetAttribute(wxT("sha1sum")).IsSameAs(hexString))
						{
							OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::OnFileDownload sha1sum failed for file %s %s %s"),urlsXML->GetChildren()->GetAttribute(wxT("file")),urlsXML->GetChildren()->GetAttribute(wxT("sha1sum")),hexString);
							fileRequest = this->CreateRequest(urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
							fileRequest.Start();
							return;
						}
					} 

				}
				wxMemoryInputStream min(dataReceiv->GetData(),dataReceiv->GetDataLen());
				wxZipInputStream zip(min);
				wxZipEntry *entry =zip.GetNextEntry();
				wxXmlNode *node = new wxXmlNode(nodeInstalled,wxXML_ELEMENT_NODE  ,(const wxString)wxString("IdInstalled"));
				node->AddAttribute(wxS("id"),urlsXML->GetChildren()->GetAttribute(wxT("id")));
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
									for(;urlsXML->GetChildren();urlsXML->RemoveChild(urlsXML->GetChildren()));


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
						OPOLYGLOT_ERROR(wxT("zip failed %s"),urlsXML->GetChildren()->GetAttribute(wxT("file")));
						flagZipOk = false;
					}
					zip.CloseEntry();
					entry = zip.GetNextEntry();
				}
				zip.CloseEntry();
				if(flagZipOk)
				{
					OPOLYGLOT_DEBUG(wxT("file download %zu %s: %s"),newFiles.GetCount(),urlsXML->GetChildren()->GetAttribute(wxT("id")),urlsXML->GetChildren()->GetAttribute(wxT("file")));
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

					urlsXML->RemoveChild(urlsXML->GetChildren());
					if(urlsXML->GetChildren())
					{
						fileRequest = this->CreateRequest(urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
						fileRequest.Start();
					} else
					{
						progress->Destroy();
						this->ScanLangs();
						this->Show(true);
						progress = NULL;	
						//wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
					}
				} else
				{
					OPOLYGLOT_ERROR(wxT("error bad zip file %s"),urlsXML->GetChildren()->GetAttribute(wxT("file")));
					for(;0< newFiles.GetCount();newFiles.RemoveAt(0))
					{
						if(!wxRemoveFile(newFiles.Item(0)))
						{
							OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::OnFileDownload can`t delete the file %s"),newFiles.Item(0));
							wxMessageDialog msg(this,wxString::Format(wxT("%s: %s"),_("Warning: Could not delete file."),newFiles.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
							msg.ShowModal();
						}
					}
					for(;0<newDirs.GetCount();newDirs.RemoveAt(0))
					{
						if(!wxDir::Remove(newDirs.Item(0)))
						{
							OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::OnFileDownload can`t delete the dir %s"),newDirs.Item(0));
							wxMessageDialog msg(this,wxString::Format(wxT("%s: %s"),_("Warning: Could not delete directory"),newDirs.Item(0)),wxT("OPolyglot"),wxOK|wxICON_ERROR);
							msg.ShowModal();
						}
					}
					OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::OnFileDownload redownload file %s"),urlsXML->GetChildren()->GetAttribute(wxT("file")));
					fileRequest = this->CreateRequest(urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
					fileRequest.Start();
				} 
			}
			break;
		case wxWebRequest::State_Failed:
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::OnFileDownload State_Failed %s redownload %s"),(wxString)event.GetErrorDescription(),urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
				fileRequest = this->CreateRequest(urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
				fileRequest.Start();
			}
			break;
		case wxWebRequest::State_Cancelled:
			OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::OnFileDownload cancelled by user download"));
			progress->Destroy();
			progress = NULL;
			for(;urlsXML->GetChildren();urlsXML->RemoveChild(urlsXML->GetChildren()));
			this->Show(true);
			this->ScanLangs();
			break;
	}
}

OPolyglotDownloadLanguage::~OPolyglotDownloadLanguage()
{
	OPOLYGLOT_MESSAGE("OPolyglotDownloadLanguage::~OPolyglotDownloadLanguage");
	mutexFileRequest.Lock();
	if(!IS_NULLPTR(progress))
	{
		wxQueueEvent(this,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER));
		while(fileRequest.GetState() != wxWebRequest::State_Cancelled)
		{
			OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::~OPolyglotDownloadLanguage %d"),fileRequest.GetState());
			wxMilliSleep(1000);
			wxSafeYield();
		}

	}
	if(!document.Save(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxS("error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error save file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	dataReceiv->Clear();
	delete dataReceiv;
	delete urlsXML;
	mutexFileRequest.Unlock();
	wxMilliSleep(200);
}
