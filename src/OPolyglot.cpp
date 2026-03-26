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


#include <wx/log.h>
#include "OPolyglot.h"
#include "OPolyglotEvent.h"
#include "Utils.h"
#include "Config.h"
#include <wx/clipbrd.h>
#ifndef __WXMSW__
#include "../res/icon.xpm"
#endif
#include <wx/panel.h>
#include <wx/rawbmp.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotSetup.h"
#include <wx/arrimpl.cpp> 
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/config.h>
#include <wx/display.h>
#include <wx/regex.h>
#include <wx/dcmemory.h>
#include <wx/uri.h>
#include <wx/sstream.h>
#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#if defined(__SNAP) || defined(__FLATPAK)
#pragma message "COMPILE LIBPORTAL"
#include <libportal/portal.h>
#include <libportal-gtk3/portal-gtk3.h>

#endif

enum{
	TIMER_ID,
	TIMER_MOUSE_ID,
	TIMER_PROGRESS_OCR_TRANSLATION_ID,
};

#if defined(__SNAP) || defined(__FLATPAK)
	static wxMutex 		mutex;
	static wxString 	fileName;
	static XdpPortal	*portal;
	static wxWindow		*parent;
	static int			countRun;
static void portal_screenshot_ready(GObject *source_object,GAsyncResult *res, gpointer user_data)
{
	OPOLYGLOT_DEBUG(wxT("OPolyglot::portal_screenshot_ready"));
	GError	*error = NULL;
	XdpPortal *portal = XDP_PORTAL(source_object);
	char *uri = xdp_portal_take_screenshot_finish(portal,res,&error);
	if(error)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot::portal_screenshot_ready %s %s"),error->message,OPOLYGLOT_BOOL_TO_STRING(uri != NULL));
		OPOLYGLOT_DEBUG(wxT("ERROR OPolyglot::portal_screenshot_ready %s %s"),error->message,OPOLYGLOT_BOOL_TO_STRING(uri != NULL));
		g_error_free(error);
		return;
	}
	OPOLYGLOT_DEBUG(wxT("%s"),uri);
	if(0 < countRun)
	{
		wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SCREENSHOT_FINISH);
		event->SetInt(countRun);
		event->SetString(wxString(uri));
		wxQueueEvent(parent,event);
	}
	countRun+=1;
	g_free(uri);
}
static void PortalInit()
{
	portal = xdp_portal_new();
	countRun = 0;
}

static void PortalTakeScreenshot(wxWindow *w)
{
	parent = w;
	xdp_portal_take_screenshot(portal
			,xdp_parent_new_gtk(GTK_WINDOW(w->GetHandle()))
			,XDP_SCREENSHOT_FLAG_NONE
			,NULL
			,portal_screenshot_ready
			,NULL);
}

#endif

OPolyglotProgress::OPolyglotProgress(wxWindow *parent) : GUIOPolyglotProgressOCRTranslator(NULL)
{
	int w,h;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgress"));
	this->parent = parent;
	timerUpdate.SetOwner(this,TIMER_ID);
	this->Bind(wxEVT_TIMER,&OPolyglotProgress::OnUpdateProgress,this);
	timerUpdate.Start(200);
	this->vBox->Fit(this);
	this->vBox->Layout();
	this->GetSize(&w,&h);
	this->SetSize(480,h);
	this->Raise();
}

OPolyglotProgress::~OPolyglotProgress()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgress::~OPolyglotProgress"));
}

void OPolyglotProgress::OnCancel(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgress::OnCancel"));
	wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER));
}

void OPolyglotProgress::OnUpdateProgress(wxTimerEvent &event)
{
	Progress->Pulse();
}

void OPolyglotProgress::Finish()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgress::Finish"));
	this->Destroy();
}

#if 1
enum{
	STYLE_TRANSLATE = 1,
	STYLE_NOT_TRANSLATE = 2
};

OPolyglotViewTextTranslate::OPolyglotViewTextTranslate(wxWindow *parent)
	: GUIOPolyglotViewTextTranslate(parent)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
#else
	SetIcon(wxICON(icon));
#endif
	this->parent = parent;
	SetTitle(wxString::Format(wxT("OPolyglot %s"),_("view translate")));
	wxBitmap copyIcon = wxArtProvider::GetBitmap(wxART_COPY, wxART_BUTTON);
	buttonCopy->SetBitmap(copyIcon);
	wxBitmap quitIcon = wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_BUTTON);
	buttonExit->SetBitmap(quitIcon);
	textTranslate->StyleSetForeground(STYLE_TRANSLATE,wxColour(wxS("black")));
	textTranslate->StyleSetBold(STYLE_TRANSLATE,true);
	textTranslate->StyleSetForeground(STYLE_NOT_TRANSLATE,wxColour(wxS("gray")));
	textTranslate->StyleSetBold(STYLE_NOT_TRANSLATE,true);
	textTranslate->SetWrapMode( wxSTC_WRAP_WORD);
	//this->Show();
}

OPolyglotViewTextTranslate::~OPolyglotViewTextTranslate()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotViewTextTranslate"));
}

void OPolyglotViewTextTranslate::OnClose( wxCloseEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::OnClose"));
	Show(false);
}

void OPolyglotViewTextTranslate::OnCopy( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::OnCopy"));
}

void OPolyglotViewTextTranslate::OnExit( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::OnExit"));
	Show(false);
}


bool OPolyglotViewTextTranslate::LoadXML(wxString xml)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::LoadXML"));
	textTranslate->ClearAll();
	wxStringInputStream sis(xml);
	wxXmlDocument doc(sis);
	if(!doc.GetRoot()->GetName().IsSameAs(wxT("TranslationTexts")))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::LoadXML not valid root %s not \"TranslationTexts\n"),doc.GetRoot()->GetName());
		return false;
	}
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxT("Text")))
		{
			int start= textTranslate->GetCurrentPos();

			if(child->GetAttribute(wxS("notTranslate")).IsEmpty())
			{
				textTranslate->AppendText(child->GetAttribute(wxT("text")));
				int end = textTranslate->GetCurrentPos();
				textTranslate->StartStyling(start);
				textTranslate->SetStyling(end-start,STYLE_TRANSLATE);
			} else
			{
				textTranslate->AppendText(child->GetAttribute(wxT("text")));
				int end = textTranslate->GetCurrentPos();
				textTranslate->StartStyling(start);
				textTranslate->SetStyling(end-start,STYLE_NOT_TRANSLATE);
			}
		}
	}
	Show(true);
	wxRect rect = this->parent->GetRect();
	wxPoint pos = GetPosition();
	pos.y = (rect.GetY()+rect.GetHeight()+5);
	SetPosition(pos);
	return true;
}

#endif

OPolyglot::OPolyglot(wxEvtHandler *handler) 
	: GuiOPolyglot(NULL) 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
#else
	SetIcon(wxICON(icon));
#endif

	this->handler = handler;
#if 0
	this->ButtonCopyTranslate->SetBitmap(wxICON(icon_copy));
	this->ButtonCopyTranslate->SetToolTip(_("Copies the translation text to the clipboard."));
#endif
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxSize s = this->GetSize();
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT))
	{
		this->SetWindowStyle(this->GetWindowStyle()|wxSTAY_ON_TOP);
	} else
	{
		this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	}

	wxPoint pos((geom.width - s.GetWidth())/2,64);
	this->SetPosition(pos);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SCREENSHOT_FINISH,&OPolyglot::OnScreenshot,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_OCR_START,&OPolyglot::OnStartOCR,this);

	this->ScanLangs();
	imageForOCR = NULL;
	if( (0 == this->LanguageFrom->GetCount())||(0 == this->LanguageTo->GetCount()))
	{
		frameDownload = new OPolyglotDownloadLanguage(this);
		this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglot::OnFinishSetupLanguages,this);
		wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
		frameDownload->Show();
	}
	MainVBox->Fit(this);
	MainVBox->Layout();
	this->Show();
	wxScreenDC dc;
	int w,h;
	dc.GetSize(&w,&h);
	if((w == 0)||(h == 0))
	{
#if defined(__SNAP) || defined(__FLATPAK)
		OPOLYGLOT_MESSAGE(wxT("OPolyglot use libportal to capture the screen"));
		PortalInit();
		PortalTakeScreenshot(this);
#else
		buttonCaptureScreen->Enable(false);
		OPOLYGLOT_WARNING(wxT("OPolyglot not supported screen capture"));
#endif
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglot available screen capture"));
	}

	viewTextTranslate = new OPolyglotViewTextTranslate(this);
}



OPolyglot::~OPolyglot()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglot"));
	delete LanguageFrom;
}

void OPolyglot::OnMenuSetup( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnMenuSetup"));
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}

void OPolyglot::OnMenuAbout( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnMenuAbout"));
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_ABOUT));
}

void OPolyglot::OnCancelTranslation(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OnCancelTranslation"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnExitThreadTranslation,this);
	if(threadTranslator->IsRunning())
	{
		threadTranslator->Delete();
		threadTranslator = NULL;
		progress->Finish();
	}
}

void OPolyglot::OnCancelOCR(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OnCancelOCR"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglot::OnCancelOCR,this);
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnOCRFinish,this);
	if(threadOCR->IsRunning())
	{
		threadOCR->Delete();
		threadOCR = NULL;
		progress->Finish();
	}
}

void OPolyglot::OnExitThreadTranslation(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OnExitThreadTranslation"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglot::OnCancelTranslation,this);
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnExitThreadTranslation,this);
	threadTranslator = NULL;
	progress->Finish();
	this->Enable(true);
	if(event.GetString().IsEmpty())
	{
		OPOLYGLOT_WARNING(wxT("OnExitThreadTranslation return value IsEmpty"));
		return;
	}
	wxStringInputStream sis(event.GetString());
	wxXmlDocument doc(sis);
	if(doc.GetRoot()->GetName().IsSameAs(wxS("Error")))
	{
		wxMessageDialog msg(this
				,wxString::Format(wxT("%s"),doc.GetRoot()->GetAttribute(wxS("value")))
				,wxT("OPolyglot")
				,wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	OPOLYGLOT_DEBUG(wxT("OnExitThreadTranslation\n%s"),event.GetString());
	//wxStringInputStream sis(event.GetString());
	//wxXmlDocument doc(sis);
	/*НЕОБХІДНО НАПИСАТИ ПОСТ ОБРОБКУ ТЕКСТУ   */
	viewTextTranslate->LoadXML(event.GetString());
}


void OPolyglot::OnCaptureScreen(wxCommandEvent& event)
{
	wxScreenDC dc;
	int w,h;
	dc.GetSize(&w,&h);
	if((0 < w)&&(0 < h))
	{
		OPOLYGLOT_MESSAGE(wxT("OnCaptureScreen(%dx%d)"),w,h);
		wxBitmap bitmap(w,h);
		wxMemoryDC memDC;
		memDC.SelectObject(bitmap);
		memDC.Blit(0,0,w,h,&dc,0,0);
		memDC.SelectObject(wxNullBitmap);
		wxString fileName = wxFileName::GetTempDir();
#if defined(__WXMSW__)
		fileName.Append(wxS("\\screen.bmp"));
#else
		fileName.Append(wxS("/screen.bmp"));
#endif
		OPOLYGLOT_DEBUG(wxT("OPolyglot::OnCaptureScreen screenshot %s"),fileName);
		if(!bitmap.SaveFile(fileName,wxBITMAP_TYPE_BMP))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglot::OnCaptureScreen not save screenshot %s"),fileName);
			wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("error saving screenshot"),fileName),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			return;
		}
		wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SCREENSHOT_FINISH);
		event->SetInt(-1);
		event->SetString(fileName);
		wxQueueEvent(this,event);
	} else
	{
#if defined(__FLATPAK) || defined(__SNAP)
		OPOLYGLOT_MESSAGE(wxT("OnCaptureScreen using libportal"));
		PortalTakeScreenshot(this);
#else
		OPOLYGLOT_ERROR(wxT("OPolyglot::OnCaptureScreen error creating screenshot"));
		wxMessageDialog msg(this,wxString::Format(wxT("%s %dx%d"),_("Error creating screenshot"),w,h),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
#endif
	}
}


void OPolyglot::OnOCRFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnOCRFinish %d"),event.GetInt());
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglot::OnCancelOCR,this);
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnOCRFinish,this);
	this->Enable(true);
	progress->Finish();
	OPOLYGLOT_DEBUG(wxT("OPolyglot::OnOCRFinish \n%s"),event.GetString());
	if(event.GetString().IsEmpty())
	{
		OPOLYGLOT_WARNING(wxT("OnOCRFinish return value IsEmpty"));
		return;
	}
	wxStringInputStream sis(event.GetString());
	wxXmlDocument doc(sis);
	if(doc.GetRoot()->GetName().IsSameAs(wxS("Error")))
	{
		wxMessageDialog msg(this
				,wxString::Format(wxT("%s"),doc.GetRoot()->GetAttribute(wxS("value")))
				,wxT("OPolyglot")
				,wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	bool flagPreprocessing = config.ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING
			,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT);
	wxXmlNode *rootNode = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("TextForTranslating"));
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxT("Text")))
		{
			wxXmlNode *childNew = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Text"));
			childNew->AddAttribute(wxS("langCode"),child->GetAttribute(wxS("langCode")));
			if(flagPreprocessing)
			{
				wxString text = child->GetAttribute(wxS("original"));
				for(size_t i =0; (i < preProcessingRegex.GetCount());i++)
				{
					OPOLYGLOT_DEBUG(wxT("OnOCRFinish rule %ld %s %s"),i+1,preProcessingRegex.Item(i),preProcessingReplace.Item(i));
					wxRegEx regex(preProcessingRegex.Item(i));
					wxString replace = preProcessingReplace.Item(i);
					replace.Replace(wxS("\\a"),"\a");
					replace.Replace(wxS("\\b"),"\b");
					replace.Replace(wxS("\\n"),"\n");
					replace.Replace(wxS("\\r"),"\r");
					replace.Replace(wxS("\\t"),"\t");
					replace.Replace(wxS("\\v"),"\v");
					replace.Replace(wxS("\\f"),"\f");
					OPOLYGLOT_MESSAGE(wxT("OnOCRFinish pre processing replace %ld %d"),i,regex.ReplaceAll(&text,replace));
				}
				childNew->AddAttribute(wxS("original"),text);
			} else
			{
				childNew->AddAttribute(wxS("original"),child->GetAttribute(wxS("original")));
			}
			rootNode->AddChild(childNew);
		}
	}
	wxString outXMl = wxEmptyString;
	wxStringOutputStream sos(&outXMl);
	wxXmlDocument outputDoc;
	outputDoc.SetRoot(rootNode);
	outputDoc.Save(sos);
	OPOLYGLOT_DEBUG(wxT("OnOCRFinish %s\n%s"),doc.GetRoot()->GetAttribute(wxS("fileName")),outXMl);
	if(!wxRemoveFile(doc.GetRoot()->GetAttribute(wxS("fileName"))))
	{
		OPOLYGLOT_WARNING(wxT("OnOCRFinish it's not critical,can not delete the file %s"),doc.GetRoot()->GetAttribute(wxS("fileName")));
	}
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglot::OnCancelTranslation,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnExitThreadTranslation,this);
	threadTranslator = new OPolyglotThreadTranslator(this,&configTranslatorFileYml,outXMl);
	progress = new OPolyglotProgress(this);
	progress->Show();
	this->Enable(false);
}

void OPolyglot::ScanLangs()
{
	OPOLYGLOT_MESSAGE(wxT("ScanLangs"));
	wxXmlDocument doc;
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("load file download language %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	preProcessingRegex.Clear();
	preProcessingReplace.Clear();
	postProcessingRegex.Clear();
	postProcessingReplace.Clear();
	for(wxXmlNode *node = doc.GetRoot()->GetChildren();node;node = node->GetNext())
	{
		if(node->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_PREPROCESSING))
			//				&&config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT))
		{
			for(wxXmlNode *rule = node->GetChildren();rule;rule = rule->GetNext())
			{
				if(rule->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_RULE))
				{
					preProcessingRegex.Add(rule->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_REGULAR));
					preProcessingReplace.Add(rule->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_REPLACEMENT));
				}
			}
		}	
		if(node->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_POSTPROCESSING))
			//				&&config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT))
		{
			for(wxXmlNode *rule = node->GetChildren();rule;rule = rule->GetNext())
			{
				if(rule->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_RULE))
				{
					postProcessingRegex.Add(rule->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_REGULAR));
					postProcessingReplace.Add(rule->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_REPLACEMENT));
				}
			}
		}
	}
	delete config;
	installCodeTranslator.Clear();
	installLanguageFrom.Clear();
	installLanguageTo.Clear();
	for(wxXmlNode *language=doc.GetRoot()->GetChildren();language;language = language->GetNext())
	{
		if(language->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_LANGUAGE))
		{
			if(OPolyglotCheckThatLanguageInstalled(&doc,language))
			{
				wxString code = language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_CODE_FROM)+language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_CODE_TO);
				wxString valueFrom = wxString::Format(wxS("%s|%s"),language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_FROM),language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_CODE_FROM));
				wxString valueTo = wxString::Format(wxS("%s|%s"),language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_TO),language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_CODE_TO));
				if(installCodeTranslator.Index(code) == wxNOT_FOUND)
				{
					installCodeTranslator.Add(code);
				}
				if(installLanguageFrom.Index(valueFrom) == wxNOT_FOUND)
				{
					installLanguageFrom.Add(valueFrom);
				}
				if(installLanguageTo.Index(valueTo) == wxNOT_FOUND)
				{
					installLanguageTo.Add(valueTo);
				}
			}
		}
	}
	installLanguageFrom.Sort(false);
	installLanguageTo.Sort(false);
	OPOLYGLOT_DEBUG(wxT("installed languageFrom %ld"),installLanguageFrom.GetCount());
	for(size_t i =0; i < installLanguageFrom.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("%ld : %s %s %s")
				,i
				,installLanguageFrom.Item(i)
				,installLanguageFrom.Item(i).SubString(0,installLanguageFrom.Item(i).Length()-5)
				,installLanguageFrom.Item(i).SubString(
					installLanguageFrom.Item(i).Length()-3
					,installLanguageFrom.Item(i).Length()-1));
	}
	OPOLYGLOT_DEBUG(wxT("installed languageTo %ld"),installLanguageTo.GetCount());
	for(size_t i =0; i < installLanguageTo.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("%ld : %s"),i,installLanguageTo.Item(i));
	}
	OPOLYGLOT_DEBUG(wxT("installed translator %ld"),installCodeTranslator.GetCount());
	for(size_t i =0; i < installCodeTranslator.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("%ld : %s"),i,installCodeTranslator.Item(i));
	}

	this->ScanLanguageFrom();
	this->ScanLanguageTo();
	this->CreateTranslatorConfig();
}


void OPolyglot::ScanLanguageFrom()
{
#define GET_CODE_FROM	installLanguageFrom.Item(i).SubString(installLanguageFrom.Item(i).Length()-3,installLanguageFrom.Item(i).Length()-1)
#define GET_NAME_FROM 	installLanguageFrom.Item(i).SubString(0,installLanguageFrom.Item(i).Length()-5)
	OPOLYGLOT_MESSAGE(wxT("ScanLanguageFrom"));
	this->LanguageFrom->Clear();
	for(size_t i = 0; i < installLanguageFrom.GetCount();i++)
	{
		this->LanguageFrom->Append(GET_NAME_FROM);
	}
	if(0 < this->LanguageFrom->GetCount())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		int find = this->LanguageFrom->FindString(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT));
		if(find != wxNOT_FOUND)
		{
			this->LanguageFrom->SetSelection(find);
		} else
		{
			this->LanguageFrom->SetSelection(0);
		}
		config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,this->LanguageFrom->GetStringSelection());
	}
	OPOLYGLOT_DEBUG(wxT("LanguageFrom %s %d"),this->LanguageFrom->GetStringSelection(),this->LanguageFrom->GetSelection());
}

void OPolyglot::ScanLanguageTo()
{
#define GET_CODE_TO	installLanguageTo.Item(i).SubString(installLanguageTo.Item(i).Length()-3,installLanguageTo.Item(i).Length()-1)
#define GET_NAME_TO	installLanguageTo.Item(i).SubString(0,installLanguageTo.Item(i).Length()-5)
	wxXmlDocument doc;
	OPOLYGLOT_MESSAGE(wxT("ScanLanguageTo"));
	wxString selectCodeLanguageFrom = wxEmptyString;
	this->LanguageTo->Clear();
	if(0 <= this->LanguageFrom->GetSelection() )
	{
		for(size_t i =0; (i < installLanguageFrom.GetCount())&&selectCodeLanguageFrom.IsEmpty();i++)
		{
			if(this->LanguageFrom->GetStringSelection().IsSameAs(GET_NAME_FROM))
			{
				selectCodeLanguageFrom =GET_CODE_FROM;
			}
		}
	}
	this->LanguageTo->Clear();
	for(size_t i = 0; i < installLanguageTo.GetCount();i++)
	{
		if(!selectCodeLanguageFrom.IsSameAs(GET_CODE_TO))
		{
			//if(codeLanguageTo.Index(GET_CODE_TO) == wxNOT_FOUND)
			if(this->LanguageTo->GetStrings().Index(GET_NAME_TO) == wxNOT_FOUND)
			{
				this->LanguageTo->Append(GET_NAME_TO);
			}

		}
	}
	/* start find available cross translate exmple POLISH -> UKRAINIAN : POLISH -> ENGLISH, ENGLISH -> UKRAINIAN */
	wxString codeToEng = selectCodeLanguageFrom+wxS("eng");
	if(installCodeTranslator.Index(codeToEng) != wxNOT_FOUND)
	{
		for(size_t i = 0; i < installLanguageTo.GetCount();i++)
		{
			wxString codeFromEng = wxS("eng")+GET_CODE_TO;
			if((installCodeTranslator.Index(codeFromEng) != wxNOT_FOUND)&&(!selectCodeLanguageFrom.IsSameAs(GET_CODE_TO)))
			{
				//if(codeLanguageTo.Index(GET_CODE_TO) == wxNOT_FOUND)
				if(this->LanguageTo->GetStrings().Index(GET_NAME_TO) == wxNOT_FOUND)
				{
					this->LanguageTo->Append(GET_NAME_TO);
				}
			}
		}
	}

	OPOLYGLOT_DEBUG(wxT("select this->LanguageTo"));
	if(0 < this->LanguageTo->GetCount())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		int find = this->LanguageTo->FindString(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT));
		if(find != wxNOT_FOUND)
		{
			this->LanguageTo->SetSelection(find);
		} else
		{
			this->LanguageTo->SetSelection(0);
		}
		config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,this->LanguageTo->GetStringSelection());
	}
}

void OPolyglot::OnSelectLanguageFrom( wxCommandEvent& event )
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("OnSelectLanguageFrom"));
	this->ScanLanguageTo();
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,this->LanguageFrom->GetStringSelection());
	CreateTranslatorConfig();
}


void OPolyglot::OnSelectLanguageTo( wxCommandEvent& event )
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("OnSelectLanguageTo"));
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,this->LanguageTo->GetStringSelection());
	CreateTranslatorConfig();
}


void OPolyglot::OnFinishSetupLanguages(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OnFinishSetupLanguages"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglot::OnFinishSetupLanguages,this);
	delete frameDownload;
	frameDownload = NULL;
	this->ScanLangs();
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
}



void OPolyglot::OnStartOCR(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnStartOCR"));
	OPOLYGLOT_DEBUG(wxT("OPolyglot::OnStartOCR\n%s"),event.GetString());
	wxString langCode = wxEmptyString;
	for(size_t i =0; (i < installLanguageFrom.GetCount())&&langCode.IsEmpty();i++)
	{
		if(this->LanguageFrom->GetStringSelection().IsSameAs(GET_NAME_FROM))
		{
			langCode = GET_CODE_FROM;
		}
	}
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	wxString dirTraineddata = wxEmptyString;
	if(config.Read(OPOLYGLOT_CONFIG_STRING_OCR_METHOD,OPOLYGLOT_CONFIG_STRING_OCR_METHOD_DEFAULT).IsSameAs(wxT("BEST")))
	{
		OPOLYGLOT_DEBUG(wxT("select BEST OCR %s"),OPOLYGLOT_GET_DIR_BEST_TRAINEDDATA);
		dirTraineddata = OPOLYGLOT_GET_DIR_BEST_TRAINEDDATA;
	} else
	{
		OPOLYGLOT_DEBUG(wxT("select FAST OCR %s"),OPOLYGLOT_GET_DIR_FAST_TRAINEDDATA);
		dirTraineddata = OPOLYGLOT_GET_DIR_FAST_TRAINEDDATA;
	}
	if(!wxFileName::FileExists(wxString::Format(wxT("%s/%s.traineddata"),dirTraineddata,langCode)))
	{
		OPOLYGLOT_ERROR(wxT("OCR config error not find :%s/%s.traineddata"),dirTraineddata,langCode);
		return ;
	}
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglot::OnCancelOCR,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnOCRFinish,this);
#if 0
	/* Додати в майбутніх версіях */
	if(!langCode.IsSameAs(wxS("eng")))
	{
		langCode = langCode + "+eng";
	}
#endif
	threadOCR = new OPolyglotThreadOCR(this,dirTraineddata,langCode,event.GetString());
	progress = new OPolyglotProgress(this);
	progress->Show();
	this->Enable(false);
}


void OPolyglot::SetShow(bool flag)
{
	OPOLYGLOT_MESSAGE(wxT("SetShow(%s) %s"),OPOLYGLOT_BOOL_TO_STRING(flag),OPOLYGLOT_BOOL_TO_STRING(this->IsShown()));
	if(flag)
	{

		this->Show(true);
		//viewTextTranslate->Show(true);
		flagShow = true;
	} else
	{
		this->Show(false);
		viewTextTranslate->Show(false);
		flagShow = false;
	}
}



void OPolyglot::OnClose( wxCloseEvent& event ) { 
	OPOLYGLOT_MESSAGE(wxT("OnClose"));
	if(wxTaskBarIcon::IsAvailable())
	{
		OPOLYGLOT_MESSAGE(wxT("hide to tray"));
		wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("close window"));
		wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT));
	}
}

wxString StringToHex(const wxString& input) {
	wxString result;
	for (const auto& ch : input) {
		// Перетворення кожного символу в hex-представлення
		result += wxString::Format(wxT("%02x"), static_cast<unsigned int>(ch));
	}
	return result;
}


void OPolyglot::CreateTranslatorConfig()
{
	OPOLYGLOT_MESSAGE(wxT("CreateTranslatorConfig(%s -> %s)"),this->LanguageFrom->GetStringSelection(),this->LanguageTo->GetStringSelection());
	configTranslatorFileYml.Clear();
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	if((0 <= this->LanguageFrom->GetSelection() )||(0 <= this->LanguageTo->GetSelection() ))
	{
		wxString code = wxEmptyString;//codeLanguageFrom.Item(this->LanguageFrom->GetSelection())+codeLanguageTo.Item(this->LanguageTo->GetSelection());
		for(size_t i = 0; (i < installLanguageFrom.GetCount())&&code.IsEmpty();i++)
		{
			if(this->LanguageFrom->GetStringSelection().IsSameAs(GET_NAME_FROM))
			{
				code = GET_CODE_FROM;
			}
		}
		for(size_t i =0; (i < installLanguageTo.GetCount())&&(code.Length() == 3);i++)
		{
			if(this->LanguageTo->GetStringSelection().IsSameAs(GET_NAME_TO))
			{
				code = code + GET_CODE_TO;
			}
		}
		if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(wxT("BEST")))
		{
			OPOLYGLOT_MESSAGE(wxT("BEST:%s"),code);
			if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,code)))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
			if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,code))&&(configTranslatorFileYml.GetCount()==0))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
			if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,code))&&(configTranslatorFileYml.GetCount()==0))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
		} else /*if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST"))) */
		{
			OPOLYGLOT_MESSAGE(wxT("FAST:%s"),code);
			if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,code)))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
			if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,code))&&(configTranslatorFileYml.GetCount()==0))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
			if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,code))&&(configTranslatorFileYml.GetCount()==0))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
		} /*  else if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST"))) */
		if(0 == configTranslatorFileYml.GetCount())
		{
			/* start find cross English translation */
			wxString codeToEng = wxEmptyString;//codeLanguageFrom.Item(this->LanguageFrom->GetSelection())+wxS("eng");
			for(size_t i = 0; (i < installLanguageFrom.GetCount())&&codeToEng.IsEmpty();i++)
			{
				if(this->LanguageFrom->GetStringSelection().IsSameAs(GET_NAME_FROM))
				{
					codeToEng = GET_CODE_FROM+wxS("eng");
				}
			}
			wxString codeFromEng = wxEmptyString;//wxS("eng")+codeLanguageTo.Item(this->LanguageTo->GetSelection());
			for(size_t i =0;(i < installLanguageTo.GetCount())&&codeFromEng.IsEmpty();i++)
			{
				if(this->LanguageTo->GetStringSelection().IsSameAs(GET_NAME_TO))
				{
					codeFromEng = wxS("eng")+GET_CODE_TO;
				}
			}
			if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(wxT("BEST")))
			{
				OPOLYGLOT_MESSAGE(wxT("start find cross translation for BEST method : %s -> %s"),codeToEng,codeFromEng);
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng)))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng))&&(configTranslatorFileYml.GetCount()==0))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng))&&(configTranslatorFileYml.GetCount()==0))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount()==1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount()==1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount()==1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(configTranslatorFileYml.GetCount() < 2)
				{
					OPOLYGLOT_ERROR(wxT("not find for BEST full method translation"));
					configTranslatorFileYml.Clear();
				}
			} else /* if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST"))) */
			{
				OPOLYGLOT_MESSAGE(wxT("start find cross translation for FAST method : %s -> %s"),codeToEng,codeFromEng);
				if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng)))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng))&&(configTranslatorFileYml.GetCount()==0))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng))&&(configTranslatorFileYml.GetCount()==0))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount() == 1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount()==1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount()==1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(configTranslatorFileYml.GetCount() < 2)
				{
					OPOLYGLOT_ERROR(wxT("not find for FAST full method translation"));
					configTranslatorFileYml.Clear();
				}
			} /* else if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST"))) */
		} /*  if(0 == configTranslatorFileYml.GetCount()) */
	} /* if((0 <= this->LanguageFrom->GetSelection() )||(0 <= this->LanguageTo->GetSelection() )) */
	OPOLYGLOT_MESSAGE(wxT("Select config file : %ld"),configTranslatorFileYml.GetCount());
	for(size_t i = 0; i < configTranslatorFileYml.GetCount();i++)
	{
		OPOLYGLOT_MESSAGE(wxT("%ld : %s"),i,configTranslatorFileYml.Item(i));
	}
	delete config;
}


void OPolyglot::OnScreenshot(wxThreadEvent &event)
{
	wxString fileName = event.GetString();
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnScreenshot %s"),fileName);
#ifndef __WXMSW__
	wxURI uri(fileName);
	if(uri.HasScheme())
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglot::OnScreenshot %s is URI %s"),fileName,uri.GetPath());
		fileName = uri.GetPath();
	} else
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglot::OnScreenshot %s is not URI"),fileName);
	}
#endif
#if defined(__FLATPAK)||defined(__SNAP)
#endif
	if(event.GetInt() != 0)
	{

		this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
		imageForOCR = new OPolyglotImage();
		fullscreen = new OPolyglotFullscreenFrame(this,fileName);
#if defined(__FLATPAK)||defined(__WXMSW__)
		fullscreen->Raise();
#endif
	}
}


