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

OPolyglotProgressInstallLanguage::OPolyglotProgressInstallLanguage(wxWindow *parent,const wxString& sizeToDownload,const wxString& countFilesToDownload) : GUIOPolyglotProgressInstallLanguage(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgressInstallLanguage"));
	this->SetTitle(wxString::Format(wxT("OPolyglot %s"),_("install languages")));
	OPOLYGLOT_MESSAGE(wxT("MESSAGE TEST"));
	this->parent = parent;
	timerUpdate.SetOwner(this,TIMER_ID);
	prevSizeDownload = 0;
	downloadedBytes = 0;
	if(!sizeToDownload.ToULong(&(this->sizeToDownload),10))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotProgressInstallLanguage error convert sizeToDownload(%s) to Long"),sizeToDownload);
		this->sizeToDownload = -1;
	}
	if(!countFilesToDownload.ToULong(&countFiles,10))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotProgressInstallLanguage error convert countFilesToDownload(%s) to Long"),countFilesToDownload);
		countFiles = -1;
	}
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	AllProgress->SetToolTip(wxString::Format(wxT("%s 0:%zu"),_("Total progress"),countFiles));
	this->Bind(wxEVT_TIMER,&OPolyglotProgressInstallLanguage::OnUpdateProgress,this);
	this->SizeAll->SetLabel(convertSizeToLabelHuman(this->sizeToDownload));
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

void OPolyglotProgressInstallLanguage::SetDownloadProgress(size_t downloaded,size_t sizeFile)
{
	wxMutexLocker lock(mutex);
	//OPOLYGLOT_DEBUG(wxT("%zu : %zu"),download,allSize);
	downloadedBytes += (downloaded - prevSizeDownload);
	prevSizeDownload = downloaded;
	this->FileProgress->SetValue((int)((downloaded*(this->FileProgress->GetRange()))/sizeFile));
	this->SizeFile->SetLabel(convertSizeToLabelHuman(sizeFile-downloaded));
	this->HBox1->Layout();
	this->HBox2->Layout();
	this->HBox3->Layout();
	this->MainBox->Layout();
	this->Refresh();
}

void OPolyglotProgressInstallLanguage::SetDownloadFile(const wxString& sizeFile,const wxString& fileNameToDownload)
{
	size_t size;
	FileProgress->SetToolTip(fileNameToDownload);
	if(!sizeFile.ToULong(&size,10))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotProgressInstallLanguage::SetDownloadFile error conver sizeFile(%s) to size_t"),sizeFile);
		size = -1;
	}
	SetDownloadProgress(0,size);

}

void OPolyglotProgressInstallLanguage::FinishDownloadFile()
{
	static size_t currentFile = 0;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgressInstallLanguage::FinishDownloadFile"));
	wxMutexLocker lock(mutex);
	this->AllProgress->SetValue((int)(downloadedBytes*(this->AllProgress->GetRange())/sizeToDownload));
	currentFile++;
	AllProgress->SetToolTip(wxString::Format(wxT("%s %zu:%zu"),_("Total progress"),currentFile,countFiles));
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
	dataReceiv = wxMemoryBuffer(1024);
	pos = wxPoint((geom.width-this->GetSize().GetWidth())/2,(geom.height-this->GetSize().GetHeight())/2);
	OPOLYGLOT_DEBUG(wxT("user-agent %s"),OPOLYGLOT_USER_AGENT);
	ListLanguages->EnableScrolling(false,true);
	this->v_box->Layout();
	this->Refresh();
	this->SetPosition(pos);
	if(!document.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage not load %s"),OPOLYGLOT_GET_XML_DATA_FILE);
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
	urlsXML = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Urls"));
	this->ScanLangs();
	this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	//wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_HIDE));
}

void OPolyglotDownloadLanguage::OnClose(wxCloseEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnClose"));
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
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
	dataReceiv.AppendData(event.GetDataBuffer(),event.GetDataSize());
	progress->SetDownloadProgress(dataReceiv.GetDataLen(),fileRequest.GetBytesExpectedToReceive());
}

wxArrayString OPolyglotDownloadLanguage::CreateXmlLanguages(const wxXmlDocument &document,wxXmlNode *xmlLanguages)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::CreateXmlLanguages"));
	wxArrayString labelLanguages;
	wxArrayString finishLanguages;
	wxArrayString idsInstalled = OPolyglotDownloadLanguage::GetIdsInstalled(document);

	wxString localeLanguage = wxLocale::FindLanguageInfo(wxGetLocale()->GetName())->Description.BeforeFirst(' '); 
	if(IS_NULLPTR(xmlLanguages))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::CreateXmlLanguages xmlLanguages is NULL"));
		return labelLanguages;
	}
	while(xmlLanguages->GetChildren())
	{
		xmlLanguages->RemoveChild(xmlLanguages->GetChildren());
	}
	OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::CreateXmlLanguages start build labelFullLanguages locale language %s"),localeLanguage);
	if(!localeLanguage.IsSameAs(wxS("English")))
	{
		for(wxXmlNode *child=document.GetRoot()->GetChildren();child;child=child->GetNext())
		{
			if(child->GetName().IsSameAs(wxS("Language")))
			{
				if(child->GetAttribute(wxS("language")).IsSameAs(localeLanguage)&&(finishLanguages.Index(localeLanguage) == wxNOT_FOUND))
				{
					labelLanguages.Add(_("English"));
					finishLanguages.Add(localeLanguage);
					wxXmlNode *xmlLang = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Label"));
					xmlLang->AddAttribute(wxS("label"),_("English"));
					bool flagInstalled = true;
					for(wxXmlNode *childLang = document.GetRoot()->GetChildren();childLang;childLang = childLang->GetNext())
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
					xmlLanguages->AddChild(xmlLang);

				}

			}
		}
	} else
	{
		localeLanguage = wxEmptyString;
	}
	for(wxXmlNode *child=document.GetRoot()->GetChildren();child;child= child->GetNext())
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
				wxXmlNode *xmlLang = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Label"));
				for(wxXmlNode *childLang = document.GetRoot()->GetChildren();childLang;childLang=childLang->GetNext())
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
								xmlLang->AddChild(new wxXmlNode(*childId));
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
				OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::CreateXmlLanguages %s %s"),language,xmlLang->GetAttribute(wxS("label")));
				finishLanguages.Add(language);
				xmlLanguages->AddChild(xmlLang);
				labelLanguages.Add(xmlLang->GetAttribute(wxS("label")));
			}
		}	

	}

	labelLanguages.Sort(CompareLocaleNoCase);
	return labelLanguages;
}

void OPolyglotDownloadLanguage::ScanLangs()
{
	int scrollX,scrollY;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::ScanLangs"));
	wxArrayString labelLanguages = OPolyglotDownloadLanguage::CreateXmlLanguages(document,xmlLanguages);
	this->ListLanguages->GetViewStart(&scrollX,&scrollY);
	this->box->Clear(true);
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
		for(wxXmlNode *childLang = xmlLanguages->GetChildren();childLang;childLang = childLang->GetNext())
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
		box->GetItem((size_t)0)->GetSizer()->Hide(buttonDownloadAll);
		box->GetItem((size_t)0)->GetSizer()->Layout();
	}
	if(!flagShowRemoveAll)
	{
		box->GetItem((size_t)0)->GetSizer()->Hide(buttonRemoveAll);
		box->GetItem((size_t)0)->GetSizer()->Layout();
	}
	box->Layout();
	ListLanguages->Scroll(scrollX,scrollY);
}

void OPolyglotDownloadLanguage::OnLanguagesDownloadAll(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnLanguagesDownloadAll"));
	wxArrayString idsToInstall = GetIdsToInstall(document,xmlLanguages,0);
	if(!OPolyglotDownloadLanguage::CreateUrlsToDownload(document,idsToInstall,urlsXML))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::OnLanguagesDownloadAll CreateUrlsToDownload"));
		return;
	}
	for(size_t i =0; i < idsToInstall.GetCount();i++)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::OnLanguagesDownloadAll not find url for %s"),idsToInstall.Item(i));
	}
	if(urlsXML->GetChildren())
	{
		progress = new OPolyglotProgressInstallLanguage(this,urlsXML->GetAttribute(wxT("size")),urlsXML->GetAttribute(wxT("count")));
		progress->SetDownloadFile(urlsXML->GetChildren()->GetAttribute(wxT("size")),urlsXML->GetChildren()->GetAttribute(wxT("file")));
		this->Show(false);
		mutexFileRequest.Lock();
		fileRequest = this->CreateRequest(urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
		fileRequest.Start();
		mutexFileRequest.Unlock();
	}
	ScanLangs();
}

void OPolyglotDownloadLanguage::OnLanguagesRemoveAll(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnLanguagesRemoveAll"));
	if(!OPolyglotDownloadLanguage::RemoveLanguage(0,document,xmlLanguages))
	{
	}
	ScanLangs();
}

wxArrayString OPolyglotDownloadLanguage::GetIdsInstalled(const wxXmlDocument &document)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::GetIdsInstalled start"));
	wxArrayString idsInstalled;
	for(wxXmlNode *child = document.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Installed")))
		{
			for(wxXmlNode *childId = child->GetChildren();childId;childId = childId->GetNext())
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
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::GetIdsInstalled(%zu)"),idsInstalled.GetCount());
	return idsInstalled;
}


wxArrayString OPolyglotDownloadLanguage::GetIdsToInstall(const wxXmlDocument &document,const wxXmlNode *xmlLanguages,const int idButton)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::GetIdsToInstall start"));
	wxArrayString idsToInstall;
	wxArrayString idsInstalled = OPolyglotDownloadLanguage::GetIdsInstalled(document);
	for(wxXmlNode *child = xmlLanguages->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Label")))
		{
			if(child->GetAttribute(wxS("idButton")).IsSameAs(wxString::Format("%d",idButton))||(idButton == 0))
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
				OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::GetIdsToInstall translate English <-> LocaleLanguage not installed add to install"));
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
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::GetIdsToInstall(%zu)"),idsToInstall.GetCount());
	return idsToInstall;
}

bool OPolyglotDownloadLanguage::CreateUrlsToDownload(const wxXmlDocument &document,wxArrayString &idsToInstall,wxXmlNode *urlsXML)
{
	size_t sizeToDownload = 0;
	size_t count = 0;
	if(IS_NULLPTR(urlsXML))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::CreateUrlsToDownload urlsXML is NULL"));
		return false;
	}
	for(wxXmlNode *child = document.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Url")))
		{
			if(idsToInstall.Index(child->GetAttribute(wxS("id"))) != wxNOT_FOUND)
			{
				OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::CreateUrlsToDownload find url for %s"),child->GetAttribute(wxS("id")));
				idsToInstall.Remove(child->GetAttribute(wxS("id")));
				urlsXML->AddChild(new wxXmlNode(*child));
				count++;
				long size;
				if(child->GetAttribute(wxS("size")).ToLong(&size,10))
				{
					sizeToDownload += size;
				} else
				{
					OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::CreateUrlsToDownload not converted size \"Url\" for %d"),child->GetAttribute(wxS("id")));
				}
			} else
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::CreateUrlsToDownload Url(%s) not find"),child->GetAttribute(wxS("id")));
			}
		}
	}
	urlsXML->DeleteAttribute(wxS("size"));
	urlsXML->DeleteAttribute(wxS("count"));
	urlsXML->AddAttribute(wxS("size"),wxString::Format(wxT("%zu"),sizeToDownload));
	urlsXML->AddAttribute(wxS("count"),wxString::Format(wxT("%zu"),count));
	return true;
}

void OPolyglotDownloadLanguage::OnLanguageDownload(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnLanguageDownload %d"),event.GetId());
	wxArrayString idsToInstall = GetIdsToInstall(document,xmlLanguages,event.GetId());
	if(!OPolyglotDownloadLanguage::CreateUrlsToDownload(document,idsToInstall,urlsXML))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::OnLanguageDownload CreateUrlsToDownload"));
		return;
	}
	for(size_t i =0; i < idsToInstall.GetCount();i++)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::OnLanguageDownload not find url for %s"),idsToInstall.Item(i));
	}
	if(urlsXML->GetChildren())
	{
		progress = new OPolyglotProgressInstallLanguage(this,urlsXML->GetAttribute(wxT("size")),urlsXML->GetAttribute(wxT("count")));
		progress->SetDownloadFile(urlsXML->GetChildren()->GetAttribute(wxT("size")),urlsXML->GetChildren()->GetAttribute(wxT("file")));
		this->Show(false);
		mutexFileRequest.Lock();
		fileRequest = this->CreateRequest(urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
		fileRequest.Start();
		mutexFileRequest.Unlock();
	}
	ScanLangs();
}

bool OPolyglotDownloadLanguage::RemoveLanguage(const int idButton,wxXmlDocument &document,wxXmlNode *xmlLanguages)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::RemoveLanguage %d"),idButton);
	wxArrayString idsNeeds;
	wxArrayString idsToRemove;
	wxArrayString filesNeed;
	wxArrayString dirsNeed;
	wxArrayString filesToRemove;
	wxArrayString dirsToRemove;
	for(wxXmlNode *child=xmlLanguages->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Label")))	
		{
			if((child->GetAttribute(wxS("idButton")).IsSameAs(wxString::Format(wxT("%d"),idButton))&&(!child->GetAttribute(wxS("flagInstalled")).IsEmpty()))
					||((idButton == 0)&&(!child->GetAttribute(wxS("flagInstalled")).IsEmpty())))
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
								OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::RemoveLanguage %s need for %s"),childId->GetAttribute(wxS("id")),child->GetAttribute(wxS("label")));
							}
						}
					}
				}
			}
		}
	}
#if 0
	for(size_t i =0; i < idsToRemove.GetCount();i++)
	{
		if(idsNeeds.Index(idsToRemove.Item(i)) != wxNOT_FOUND)
		{
			OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::RemoveLanguage %s is need"),idsToRemove.Item(i));
			idsToRemove.RemoveAt(i);
			i--;
		}
	}
#endif
	for(wxXmlNode *childInstalled = document.GetRoot()->GetChildren();childInstalled;childInstalled = childInstalled->GetNext())
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
	for(wxXmlNode *child = document.GetRoot()->GetChildren();child;child = child->GetNext())
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
							OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::RemoveLanguage Can't delete the tag \"IdInstalled\" with ID %s"),childId->GetAttribute(wxS("id")));
						}
						childId = next;
					} else
					{
						childId = childId->GetNext();
					}
				} else
				{
					OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::RemoveLanguage unknown tag for \"Installed\" %s"),childId->GetName());
					childId = childId->GetNext();
				}
			}
		}
	}
	if(!document.Save(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxS("OPolyglotDownloadLanguage::RemoveLanguage error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		return false;
	}
	for(size_t i =0; i < filesToRemove.GetCount();i++)
	{
		if(filesNeed.Index(filesToRemove.Item(i)) == wxNOT_FOUND)
		{
			OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::RemoveLanguage file %s to remove"),filesToRemove.Item(i));
			if(!wxRemoveFile(filesToRemove.Item(i)))
			{
				OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::RemoveLanguage can`t delete %s file"),filesToRemove.Item(i));
			}
		}
	}
	for(size_t i =0;i < dirsToRemove.GetCount();i++)
	{
		if(dirsNeed.Index(dirsToRemove.Item(i)) == wxNOT_FOUND)
		{
			OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::RemoveLanguage dir %s to remove"),dirsToRemove.Item(i));
			if(!wxRmdir(dirsToRemove.Item(i)))
			{
				OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::RemoveLanguage can`t delete %s dir"),filesToRemove.Item(i));
			}
		}
	}
	return true;
}

void OPolyglotDownloadLanguage::OnLanguageRemove(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnLanguageRemove %d"),event.GetId());
	if(!OPolyglotDownloadLanguage::RemoveLanguage(event.GetId(),document,xmlLanguages))
	{
	}
	this->ScanLangs();
}

void OPolyglotDownloadLanguage::OnCancelUser(wxThreadEvent &event)
{
	wxMutexLocker lock(mutexFileRequest);
	OPOLYGLOT_WARNING("OPolyglotDownloadLanguage::OnCancelUser");
	fileRequest.Cancel();
	this->ScanLangs();
}

OPolyglotDownloadLanguage::RetType OPolyglotDownloadLanguage::FinishProcessFile(wxString& messageError,wxXmlDocument& document,wxXmlNode *urlsXML,wxMemoryBuffer& dataReceiv,wxWebRequest& fileRequest)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::FinishProcessFile"));
	wxXmlNode 		 *nodeInstalled = nullptr;
	bool flagZipOk = true;
	messageError = wxEmptyString;
#if 0
	timeDownload.Pause();
	OPOLYGLOT_MESSAGE(wxS("wxWebRequest::State_Completed %s download time %.1f S, size %zu Bytes"),urlsXML->GetChildren()->GetAttribute(wxT("file")) ,((double)timeDownload.Time())/1000.0,dataReceiv->GetDataLen());
#endif
	for(wxXmlNode *child = document.GetRoot()->GetChildren();child&&(IS_NULLPTR(nodeInstalled));child=child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Installed")))
		{
			nodeInstalled = child;
		}
	}
	if(IS_NULLPTR(nodeInstalled))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::FinishProcessFile \"Installed\" tag not found in %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		messageError = wxString::Format(wxS("\"Installed\" %s %s"),_("tag not found"),OPOLYGLOT_GET_XML_DATA_FILE);
		return CRITICAL_ERROR;
	}
	if(dataReceiv.GetDataLen() < (size_t)fileRequest.GetBytesExpectedToReceive())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::FinishProcessFile received data %zu != %zu"),dataReceiv.GetDataLen(),fileRequest.GetBytesExpectedToReceive());
		messageError = wxString::Format(wxS("Received fewer bytes than expected %zu<%zu"),dataReceiv.GetDataLen(),fileRequest.GetBytesExpectedToReceive());
		return ERROR;
	}
	if(urlsXML->GetChildren()->GetAttribute(wxT("sha1sum")).IsEmpty())
	{
		OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::FinishProcessFile for file %s not sha1sum"),urlsXML->GetChildren()->GetAttribute(wxT("file")));
	} else
	{
		{
			int err;
			unsigned char sum_sha1[20];
			wxString hexString = wxEmptyString;
			hash_state sha1;
			if((err = sha1_init(&sha1)) != CRYPT_OK)
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::FinishProcessFile tomcrypt error sha1_init %s"),wxString(error_to_string(err)));
				messageError = wxString::Format(wxT("%s\n%s"),_("error tomcrypt sha1_init "),error_to_string(err));
				return ERROR;
			}
			if((err = sha1_process(&sha1,(unsigned char *)dataReceiv.GetData(),dataReceiv.GetDataLen())) != CRYPT_OK)
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::FinishProcessFile tomcrypt error sha1_process %s"),wxString(error_to_string(err)));
				messageError = wxString::Format(wxT("%s\n%s"),_("error tomcrypt sha1_process "),error_to_string(err));
				return ERROR;
			}
			if((err = sha1_done(&sha1,sum_sha1)) != CRYPT_OK)
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::FinishProcessFile tomcrypt error sha1_done %s"),wxString(error_to_string(err)));
				messageError = wxString::Format(wxT("%s\n%s"),_("error tomcrypt sha1_done "),error_to_string(err));
				return ERROR;
			}
			for(size_t i = 0; i < sizeof(sum_sha1);i++)
			{
				hexString += wxString::Format(wxT("%02x"),sum_sha1[i]);
			}
			OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::FinishProcessFile %s sha1sum %s"),urlsXML->GetChildren()->GetAttribute(wxT("file")),hexString);
			if(!urlsXML->GetChildren()->GetAttribute(wxT("sha1sum")).IsSameAs(hexString))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::FinishProcessFile sha1sum failed for file %s %s %s"),urlsXML->GetChildren()->GetAttribute(wxT("file")),urlsXML->GetChildren()->GetAttribute(wxT("sha1sum")),hexString);
				messageError = wxString::Format(wxS("sha1sum failed for file %s"),urlsXML->GetChildren()->GetAttribute(wxT("file")));
				return ERROR;
			}
		} 

	}
	wxMemoryInputStream min(dataReceiv.GetData(),dataReceiv.GetDataLen());
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
				wxString dirPath = wxString::Format(wxS("%s%c%s"),OPOLYGLOT_USER_DATA,wxFileName::GetPathSeparator(),entry->GetName());
				OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::FinishProcessFile mkdir %s"),dirPath);
				wxXmlNode *dir = new wxXmlNode(node,wxXML_ELEMENT_NODE,(const wxString)wxString("DirCreated"));
				dir->AddAttribute(wxS("dir"),dirPath);
				if(!wxFileName::DirExists(dirPath))
				{
					if(!wxDir::Make(dirPath))
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::FinishProcessFile cannot create directory %s"),dirPath);
						messageError = wxString::Format(wxT("%s %s"),_("cannot create dir"),dirPath);
						return CRITICAL_ERROR;
					} 
				} else
				{
					OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::FinishProcessFile dir exists %s/%s"),OPOLYGLOT_USER_DATA,entry->GetName());
				}

			} else
			{
				wxString fileName = wxString::Format(wxS("%s%c%s"),OPOLYGLOT_USER_DATA,wxFileName::GetPathSeparator(),entry->GetName());
				OPOLYGLOT_DEBUG(wxT("OPolyglotDownloadLanguage::FinishProcessFile file zip: %s"),fileName);
				wxXmlNode *file = new wxXmlNode(node,wxXML_ELEMENT_NODE,(const wxString)wxString("FileInstalled"));
				file->AddAttribute(wxS("file"),fileName);
				if(!wxFileName::FileExists(fileName))
				{
					wxFileOutputStream out(fileName);
					if(!out.IsOk())
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::FinishProcessFile cannot create wxFileOutputStream(%s)"),fileName);
						messageError = wxString::Format(wxT("%s %s"),_("cannot create"),fileName);
						return CRITICAL_ERROR;
					}
					zip.Read(out);

				} else
				{
					OPOLYGLOT_WARNING(wxT("OPolyglotDownloadLanguage::FinishProcessFile file exist %s"),fileName);
				}
			}
		} else
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::FinishProcessFile bad zip file %s"),urlsXML->GetChildren()->GetAttribute(wxT("file")));
			messageError = wxString::Format(wxS("%s %s"),_("bad zip file"),urlsXML->GetChildren()->GetAttribute(wxT("file")));
			return ERROR;
		}
		zip.CloseEntry();
		entry = zip.GetNextEntry();
	}
	zip.CloseEntry();
	if(!document.Save(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxS("error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		messageError = wxString::Format(wxT("%s :%s"),_("Error save file"),OPOLYGLOT_GET_XML_DATA_FILE);
		return CRITICAL_ERROR;
	}
	urlsXML->RemoveChild(urlsXML->GetChildren());
	return SUCCESS;
}

void OPolyglotDownloadLanguage::OnFileDownload(wxWebRequestEvent& event)
{
	wxMutexLocker lock(mutexFileRequest);
	wxString messageError;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnFileDownload"));
	switch(event.GetState())
	{
		case wxWebRequest::State_Idle:
			OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnFileDownload wxWebRequest::State_Idle"));
			break;
		case wxWebRequest::State_Unauthorized:
			OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnFileDownload wxWebRequest::State_Unauthorized"));
			break;
		case wxWebRequest::State_Active:
			OPOLYGLOT_MESSAGE(wxT("OPolyglotDownloadLanguage::OnFileDownload wxWebRequestEvent::State_Active %s"),urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
			progress->SetDownloadFile(urlsXML->GetChildren()->GetAttribute(wxT("size")),urlsXML->GetChildren()->GetAttribute(wxT("file")));
			dataReceiv.Clear();
			break;
		case wxWebRequest::State_Completed:
			
			if(OPolyglotDownloadLanguage::FinishProcessFile(messageError,document,urlsXML,dataReceiv,fileRequest)== OPolyglotDownloadLanguage::CRITICAL_ERROR)
			{
				wxMessageDialog msg(this,messageError,wxT("OPolyglot"),wxICON_ERROR|wxOK);
				msg.ShowModal();
			} 
			if(urlsXML->GetChildren())
			{
				progress->FinishDownloadFile();
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
			break;
		case wxWebRequest::State_Failed:
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotDownloadLanguage::OnFileDownload State_Failed %s redownload %s %s")
						,event.GetErrorDescription()
						,urlsXML->GetChildren()->GetAttribute(wxS("file"))
						,urlsXML->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
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
	dataReceiv.Clear();
	delete urlsXML;
	delete xmlLanguages;
	mutexFileRequest.Unlock();
	wxMilliSleep(200);
}
