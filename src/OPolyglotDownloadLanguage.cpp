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

enum {OPOLYGLOT_RET_SUCCESS=0,OPOLYGLOT_RET_ERROR=1,OPOLYGLOT_RET_CRITICAL_ERROR=-1};


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
	downloadedFiles = 0;
	sizeToDownload = 0;
	countFiles = 0;
	wxArrayString idsInstalled;
	wxArrayString idsToInstall;
	urlsXML.SetRoot(new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Urls")));
	wxXmlDocument doc;
	this->xmlLanguages = xmlLanguages;
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages failed to load the XML file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this
				,wxString::Format(wxS("%s: %s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE)
				,wxT("OPolyglot"),wxICON_ERROR|wxOK);
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
	FileProgress->SetToolTip(urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file")));
	timerUpdateProgress.SetOwner(this,wxID_ANY);
	downloadTimeout.SetOwner(this,wxID_ANY);
	this->Bind(wxEVT_TIMER,&OPolyglotInstallLanguages::OnUpdateProgress,this,timerUpdateProgress.GetId());
	this->Bind(wxEVT_TIMER,&OPolyglotInstallLanguages::OnDownloadTimeout,this,downloadTimeout.GetId());
	this->Bind(wxEVT_WEBREQUEST_STATE,&OPolyglotInstallLanguages::OnDownloadState,this);
	
	languageDownload = CreateRequest(this,urlsXML.GetRoot()->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
	languageDownload.Start();
	timeRun.Start();
	Show();
}

OPolyglotInstallLanguages::~OPolyglotInstallLanguages()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotProgressInstallLanguage"));
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
	wxMutexLocker lock(mutex);
	OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::OnCancel"));
	languageDownload.Cancel();
}


void OPolyglotInstallLanguages::OnClose( wxCloseEvent& event ) 
{
	wxMutexLocker lock(mutex);
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgressInstallLanguage::OnClose"));
	wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER));
}

void OPolyglotInstallLanguages::OnDownloadTimeout(wxTimerEvent& event)
{
	wxMutexLocker lock(mutex);
	OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::OnDownloadTimeout"));
	languageDownload.Cancel();
}

void OPolyglotInstallLanguages::OnUpdateProgress(wxTimerEvent &event)
{
	double speed;
	double timeRemaining;
	double timeElapsed;
	static wxFileOffset prevBytesReceived = 0;
	wxString prefix = _("B/s    ");
	wxString prefixTime = _("s    ");
	wxMutexLocker lock(mutex);
	FileProgress->SetValue((int)((languageDownload.GetBytesReceived()*FileProgress->GetRange())/sizeFile));
	SizeFile->SetLabel(convertSizeToLabelHuman(sizeFile-languageDownload.GetBytesReceived()));
	timeRun.Pause();
	speed = (double)((downloadedBytes+languageDownload.GetBytesReceived())*1000) / (double)(timeRun.Time() ); /* per second */
	timeElapsed = ((double)timeRun.Time())/1000.0;
	timeRun.Resume();
	timeRemaining =  ((double)(sizeToDownload-downloadedBytes-languageDownload.GetBytesReceived()))/(double)speed;
	
	if(prevBytesReceived != languageDownload.GetBytesReceived())
	{
		downloadTimeout.StartOnce(OPOLYGLOT_TIMEOUT_DOWNLOAD);
	}
	prevBytesReceived = languageDownload.GetBytesReceived();
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
	if(0 < (downloadedBytes+languageDownload.GetBytesReceived()))
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



void OPolyglotInstallLanguages::FinishDownloadFile()
{
	static size_t currentFile = 0;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgressInstallLanguage::FinishDownloadFile"));
	wxMutexLocker lock(mutex);
	this->AllProgress->SetValue((int)((downloadedBytes*(this->AllProgress->GetRange()))/sizeToDownload));
	currentFile++;
	AllProgress->SetToolTip(wxString::Format(wxT("%s %zu:%zu"),_("Total progress"),currentFile,countFiles));
	this->FileProgress->SetValue(0);
	this->SizeAll->SetLabel(convertSizeToLabelHuman(sizeToDownload-downloadedBytes));
	this->HBox1->Layout();
	this->HBox2->Layout();
	this->HBox3->Layout();
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
	for(;xmlLanguages.GetRoot()->GetChildren();xmlLanguages.GetRoot()->RemoveChild(xmlLanguages.GetRoot()->GetChildren()));
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

wxWebRequest OPolyglotInstallLanguages::CreateRequest(wxEvtHandler* handler,wxString url)
{
	wxWebRequest ret;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::CreateRequest %s"),url);
	ret = wxWebSession::GetDefault().CreateRequest(handler,url);
	ret.SetHeader(wxT("User-Agent"), OPOLYGLOT_USER_AGENT);
	ret.SetStorage(wxWebRequest::Storage_File);
	return ret;
}


int OPolyglotInstallLanguages::UnpackAndInstall(const wxString& fileName)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::UnpackAndInstall"));
	wxXmlNode 		 *nodeInstalled = nullptr;
	messageError = wxEmptyString;
	wxXmlDocument document;
	wxFileInputStream fis(fileName);
	if(!fis.IsOk())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall not load %s"),fileName);
		return OPOLYGLOT_RET_CRITICAL_ERROR;
	}
	if(!document.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall not load %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		messageError = wxString::Format(wxS("%s: %s"),_("not load file"),OPOLYGLOT_GET_XML_DATA_FILE);
		return OPOLYGLOT_RET_CRITICAL_ERROR;
	}
	for(wxXmlNode *child = document.GetRoot()->GetChildren();child&&(IS_NULLPTR(nodeInstalled));child=child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Installed")))
		{
			nodeInstalled = child;
		}
	}
	if(IS_NULLPTR(nodeInstalled))
	{
		nodeInstalled = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Installed"));
		document.GetRoot()->AddChild(nodeInstalled);
	}
	if(languageDownload.GetBytesReceived() < languageDownload.GetBytesExpectedToReceive())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall received data %zu < %zu"),languageDownload.GetBytesReceived(),languageDownload.GetBytesExpectedToReceive());
		return OPOLYGLOT_RET_ERROR;
	}
	if(urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("sha1sum")).IsEmpty())
	{
		OPOLYGLOT_WARNING(wxT("OPolyglotInstallLanguages::UnpackAndInstall for file %s not sha1sum"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("file")));
	} else
	{
		{
			OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::UnpackAndInstall %s"),fileName);
			int err;
			unsigned char sum_sha1[20];
			const size_t CHUNK_SIZE = 4096;
			unsigned char chunk[CHUNK_SIZE];
			wxString hexString = wxEmptyString;
			hash_state sha1;
#if 0
			if(!response->GetStream()->ReadAll(buff,response->GetStream()->GetLength()))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall not read languageDownload"));
				delete[] buff;
				messageError = wxString::Format(wxT("%s languageDownload"),_("not read"));
				return OPOLYGLOT_RET_CRITICAL_ERROR;
			}
#endif
			if((err = sha1_init(&sha1)) != CRYPT_OK)
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall tomcrypt error sha1_init %s"),wxString(error_to_string(err)));
				messageError = wxString::Format(wxT("%s\n%s"),_("error tomcrypt sha1_init "),error_to_string(err));
				return OPOLYGLOT_RET_CRITICAL_ERROR;
			}
			fis.SeekI(0,wxFromStart);
			while (!fis.Eof())
			{
				fis.Read(chunk, CHUNK_SIZE);
				size_t bytesRead = fis.LastRead(); 

				if (bytesRead > 0)
				{
					if ((err = sha1_process(&sha1, chunk, bytesRead)) != CRYPT_OK)
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall tomcrypt error sha1_process %s"), wxString(error_to_string(err)));
						messageError = wxString::Format(wxT("%s\n%s"), _("error tomcrypt sha1_process "), error_to_string(err));
						return OPOLYGLOT_RET_CRITICAL_ERROR;
					}
				}
			}			if((err = sha1_done(&sha1,sum_sha1)) != CRYPT_OK)
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall tomcrypt error sha1_done %s"),wxString(error_to_string(err)));
				messageError = wxString::Format(wxT("%s\n%s"),_("error tomcrypt sha1_done "),error_to_string(err));
				return OPOLYGLOT_RET_CRITICAL_ERROR;
			}
			for(size_t i = 0; i < sizeof(sum_sha1);i++)
			{
				hexString += wxString::Format(wxT("%02x"),sum_sha1[i]);
			}
			if(!urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("sha1sum")).IsSameAs(hexString))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall sha1sum failed for file %s %s %s"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("file")),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("sha1sum")),hexString);
				messageError = wxString::Format(wxS("sha1sum failed for file %s"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("file")));
				return OPOLYGLOT_RET_ERROR;
			}
		} 

	}
	fis.SeekI(0,wxFromStart);
	wxZipInputStream zip(fis);
	wxZipEntry *entry =zip.GetNextEntry();
	wxXmlNode *node = new wxXmlNode(nodeInstalled,wxXML_ELEMENT_NODE  ,(const wxString)wxString("IdInstalled"));
	node->AddAttribute(wxS("id"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("id")));
	while(entry)
	{
		if(zip.IsOk())
		{
			if(entry->IsDir())
			{
				wxString dirPath = wxString::Format(wxS("%s%c%s"),OPOLYGLOT_USER_DATA,wxFileName::GetPathSeparator(),entry->GetName());
				wxXmlNode *dir = new wxXmlNode(node,wxXML_ELEMENT_NODE,(const wxString)wxString("DirCreated"));
				dir->AddAttribute(wxS("dir"),dirPath);
				if(!wxFileName::DirExists(dirPath))
				{
					if(!wxDir::Make(dirPath))
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall cannot create directory %s"),dirPath);
						messageError = wxString::Format(wxT("%s %s"),_("cannot create dir"),dirPath);
						return OPOLYGLOT_RET_CRITICAL_ERROR;
					} 
				} else
				{
					OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::UnpackAndInstall dir exists %s/%s"),OPOLYGLOT_USER_DATA,entry->GetName());
				}

			} else
			{
				wxString fileName = wxString::Format(wxS("%s%c%s"),OPOLYGLOT_USER_DATA,wxFileName::GetPathSeparator(),entry->GetName());
				wxXmlNode *file = new wxXmlNode(node,wxXML_ELEMENT_NODE,(const wxString)wxString("FileInstalled"));
				file->AddAttribute(wxS("file"),fileName);
				if(!wxFileName::FileExists(fileName))
				{
					wxFileOutputStream out(fileName);
					if(!out.IsOk())
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall cannot create wxFileOutputStream(%s)"),fileName);
						messageError = wxString::Format(wxT("%s %s"),_("cannot create"),fileName);
						return OPOLYGLOT_RET_CRITICAL_ERROR;
					}
					zip.Read(out);

				} else
				{
					OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::UnpackAndInstall file exist %s"),fileName);
				}
			}
		} else
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::UnpackAndInstall bad zip file %s"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("file")));
			messageError = wxString::Format(wxS("%s %s"),_("bad zip file"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxT("file")));
			return OPOLYGLOT_RET_ERROR;
		}
		zip.CloseEntry();
		entry = zip.GetNextEntry();
	}
	zip.CloseEntry();
	if(!document.Save(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxS("error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		messageError = wxString::Format(wxT("%s :%s"),_("Error save file"),OPOLYGLOT_GET_XML_DATA_FILE);
		return OPOLYGLOT_RET_CRITICAL_ERROR;
	}
	urlsXML.GetRoot()->RemoveChild(urlsXML.GetRoot()->GetChildren());
	return OPOLYGLOT_RET_SUCCESS;
}

void OPolyglotInstallLanguages::OnDownloadState(wxWebRequestEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::OnDownloadState"));
	wxMutexLocker lock(mutex);
	wxString messageError;
	wxString tmpStr;
	unsigned long tmpValue;
	switch(event.GetState())
	{
		case wxWebRequest::State_Idle:
			OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::OnDownloadStatus wxWebRequest::State_Idle"));
			break;
		case wxWebRequest::State_Unauthorized:
			OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::OnDownloadStatus wxWebRequest::State_Unauthorized"));
			break;
		case wxWebRequest::State_Active:
			OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::OnDownloadStatus wxWebRequestEvent::State_Active %s %s"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file")),urlsXML.GetRoot()->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
			FileProgress->SetValue(0);
			tmpStr = urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))+wxS(" ")
				+urlsXML.GetRoot()->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL);
			FileProgress->SetToolTip(tmpStr);
			if(!urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("size")).ToULong(&tmpValue,10))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::OnDownloadStatus wxWebRequestEvent::State_Active not convert file %s size %s")
						,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
						,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("size")));
				tmpValue = -1;
			}
			sizeFile = tmpValue;
			SizeFile->SetLabel(convertSizeToLabelHuman(tmpValue));
			downloadTimeout.StartOnce(OPOLYGLOT_TIMEOUT_DOWNLOAD);
			timerUpdateProgress.Start(300);
			break;
		case wxWebRequest::State_Completed:
			OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::OnDownloadStatus wxWebRequest::State_Completed"));	
			downloadTimeout.Stop();
			OPOLYGLOT_DEBUG(wxT("OPolyglotInstallLanguages::OnDownloadStatus wxWebRequest::State_Completed %s"),event.GetDataFile());
			switch(UnpackAndInstall(event.GetDataFile()))
			{
				case OPOLYGLOT_RET_CRITICAL_ERROR:
					{
						wxMessageDialog msg(this,messageError,wxT("OPolyglot"),wxICON_ERROR|wxOK);
						msg.ShowModal();
						wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
						break;
					}
				case OPOLYGLOT_RET_ERROR:
					{
						OPOLYGLOT_WARNING(wxT("OPolyglotInstallLanguages::OnDownloadStatus wxWebRequest::State_Completed error"));
						break;
					}
				case OPOLYGLOT_RET_SUCCESS:
					{
						OPOLYGLOT_MESSAGE(wxT("OPolyglotInstallLanguages::OnDownloadStatus wxWebRequest::State_Completed SUCCESS"));
						break;
					}
			}

			downloadedBytes += languageDownload.GetBytesReceived();
			if(urlsXML.GetRoot()->GetChildren())
			{
				if(messageError.IsEmpty())
				{
					AllProgress->SetValue((int)(downloadedBytes*(this->AllProgress->GetRange())/sizeToDownload));
					downloadedFiles++;
					tmpStr = _("Total progress")+" "+wxString::Format(wxT("%zu:%s"),downloadedFiles,urlsXML.GetRoot()->GetAttribute(wxS("count")));
					AllProgress->SetToolTip(tmpStr);
					SizeAll->SetLabel(convertSizeToLabelHuman(sizeToDownload-downloadedBytes));
				}

				languageDownload = CreateRequest(this,urlsXML.GetRoot()->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
				languageDownload.Start();
			} else
			{
				downloadTimeout.Stop();
				wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
			}
			break;
		case wxWebRequest::State_Failed:
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotInstallLanguages::OnDownloadStatus State_Failed %s redownload %s %s")
						,event.GetErrorDescription()
						,urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file"))
						,urlsXML.GetRoot()->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
				languageDownload = CreateRequest(this,urlsXML.GetRoot()->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
				languageDownload.Start();
			}
			break;
		case wxWebRequest::State_Cancelled:
			if(downloadTimeout.IsRunning())
			{
				OPOLYGLOT_WARNING(wxT("OPolyglotInstallLanguages::OnDownloadStatus cancelled by user download"));
				downloadTimeout.Stop();
				wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
			} else
			{
				OPOLYGLOT_WARNING(wxT("OPolyglotInstallLanguages::OnDownloadStatus timeout download %s"),urlsXML.GetRoot()->GetChildren()->GetAttribute(wxS("file")));
				languageDownload = CreateRequest(this,urlsXML.GetRoot()->GetChildren()->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
				languageDownload.Start();
			}
			break;
	}
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
		wxMessageDialog msg(this,messageError,wxT("OPolyglot"),wxICON_ERROR|wxOK);
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
		box->GetItem((size_t)0)->GetSizer()->Hide(buttonDownloadAll);
		box->GetItem((size_t)0)->GetSizer()->Layout();
	}
	if(!flagShowRemoveAll)
	{
		box->GetItem((size_t)0)->GetSizer()->Hide(buttonRemoveAll);
		box->GetItem((size_t)0)->GetSizer()->Layout();
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
		wxMessageDialog msg(this,messageError,wxT("OPolyglot"),wxICON_ERROR|wxOK);
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
		wxMessageDialog msg(this,messageError,wxT("OPolyglot"),wxICON_ERROR|wxOK);
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
