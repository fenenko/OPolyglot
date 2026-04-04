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
#include <wx/panel.h>
#include <wx/rawbmp.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotSettings.h"
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
#include "LibOPolyglot.h"

#if __WXGTK__
	#include "../res/icon.xpm"
	#include <libportal/portal.h>
	#include <libportal-gtk3/portal-gtk3.h>
#endif

enum{
	TIMER_ID,
	TIMER_MOUSE_ID,
	TIMER_PROGRESS_OCR_TRANSLATION_ID,
};

#if __WXGTK__
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

OPolyglotProgress::OPolyglotProgress(wxWindow *parent,wxString label) : GUIOPolyglotProgressOCRTranslator(NULL)
{
	int w,h;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgress"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	this->SetTitle(wxT("OPolyglot"));
	this->parent = parent;
	timerUpdate.SetOwner(this,TIMER_ID);
	this->Bind(wxEVT_TIMER,&OPolyglotProgress::OnUpdateProgress,this);
	timerUpdate.Start(200);
	ProgressLabel->SetLabel(label);
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
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	wxBitmap copyIcon = wxArtProvider::GetBitmap(wxART_COPY, wxART_BUTTON);
	wxBitmap clearIcon = wxArtProvider::GetBitmap(OPOLYGLOT_ART_CLEAR,wxART_BUTTON,copyIcon.GetSize());
	buttonCopy->SetBitmap(copyIcon);
	buttonClear->SetBitmap(clearIcon);
	this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	this->parent = parent;
	SetTitle(wxString::Format(wxT("OPolyglot %s"),_("screen translation text")));
	textTranslate->Clear();
	textTranslate->SetLexer(wxSTC_LEX_CONTAINER);
	textTranslate->AnnotationClearAll();
	textTranslate->StyleSetForeground(STYLE_TRANSLATE,wxColour(wxS("black")));
	textTranslate->StyleSetBold(STYLE_TRANSLATE,true);
	textTranslate->StyleSetForeground(STYLE_NOT_TRANSLATE,wxColour(wxS("gray")));
	textTranslate->StyleSetBold(STYLE_NOT_TRANSLATE,true);
	textTranslate->SetWrapMode( wxSTC_WRAP_WORD);
	textTranslate->SetLayoutCache(wxSTC_CACHE_DOCUMENT);
	textTranslate->SetEndAtLastLine(false);
	textTranslate->Bind(wxEVT_STC_DOUBLECLICK,&OPolyglotViewTextTranslate::OnDoubleClickText,this);
	LoadXML();
	wxRect rect = this->parent->GetRect();
	wxPoint pos = GetPosition();
	pos.y = (rect.GetY()+rect.GetHeight()+5);
	SetPosition(pos);
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
	if(wxTheClipboard->Open())
	{
		    wxTheClipboard->SetData( new wxTextDataObject(textTranslate->GetText()) );
		    wxTheClipboard->Close();
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::OnCopy not open clipboard"));
	}
}

void OPolyglotViewTextTranslate::OnClear(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::OnClear"));
	wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Are you sure you want to clear the translation text?")),wxT("OPolyglot"),wxOK|wxCANCEL|wxICON_QUESTION);
	if(msg.ShowModal() == wxID_OK)
	{
		wxXmlDocument doc;
		if(!doc.Load(OPOLYGLOT_GET_XML_FILE_TRANSLATE))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::OnClear not load %s"),OPOLYGLOT_GET_XML_FILE_TRANSLATE);
			return;
		}
		for(;doc.GetRoot()->GetChildren();doc.GetRoot()->RemoveChild(doc.GetRoot()->GetChildren()));
		if(!doc.Save(OPOLYGLOT_GET_XML_FILE_TRANSLATE))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::OnClear not saved %s"),OPOLYGLOT_GET_XML_FILE_TRANSLATE);
			return;
		}
	}
	LoadXML();
}


void OPolyglotViewTextTranslate::OnDoubleClickText(wxStyledTextEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::OnDoubleClickText"));
	OPOLYGLOT_DEBUG(wxT("OPolyglotViewTextTranslate::OnDoubleClickText %d"),event.GetLine());

}

void OPolyglotViewTextTranslate::LoadXML()
{

	int countLines = 0;
	textTranslate->SetReadOnly(false);
	textTranslate->AnnotationClearAll();
	int oldLineCount = textTranslate->GetLineCount();
	textTranslate->AnnotationSetVisible(wxSTC_ANNOTATION_STANDARD);
	for(int i =0; i < textTranslate->GetLineCount();i++)
	{
		countLines += textTranslate->WrapCount(i);
	}
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::ViewTranslate"));
	OPOLYGLOT_DEBUG(wxT("OPolyglotViewTextTranslate::ViewTranslate old document %d visible lines %d"),textTranslate->GetLineCount(),countLines);
	wxRect rect = this->parent->GetRect();
	wxPoint pos = GetPosition();
	pos.y = (rect.GetY()+rect.GetHeight()+5);
	SetPosition(pos);
	textTranslate->Clear();
	textTranslate->ClearAll();
	wxXmlDocument *doc = new wxXmlDocument();
	if(!doc->Load(OPOLYGLOT_GET_XML_FILE_TRANSLATE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::ViewTranslate not load %s"),OPOLYGLOT_GET_XML_FILE_TRANSLATE);
		return;
	}
	if(!doc->GetRoot()->GetName().IsSameAs(wxT("Texts")))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::ViewTranslate not valid root %s not \"TranslationTexts\n"),doc->GetRoot()->GetName());
		return;
	}
	for(wxXmlNode *child = doc->GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxT("Text")))
		{
			int start= textTranslate->GetTextLength();

			if(!child->GetAttribute(wxS("text")).IsEmpty())
			{
				textTranslate->AppendText(child->GetAttribute(wxT("text")));
				int end = textTranslate->GetTextLength();
				textTranslate->StartStyling(start);
				textTranslate->SetStyling(end-start,STYLE_TRANSLATE);
			} else
			{
				textTranslate->AppendText(child->GetAttribute(wxT("original")));
				int end = textTranslate->GetTextLength();
				textTranslate->StartStyling(start);
				textTranslate->SetStyling(end-start,STYLE_TRANSLATE);
			}
		}
	}
	delete doc;
	if((countLines != 0)&&(oldLineCount < textTranslate->GetLineCount())&&(0 <= (oldLineCount-2)))
	{
		textTranslate->AnnotationSetText(oldLineCount-2,wxS("----------------------------------------------------"));
		textTranslate->AnnotationSetStyle(oldLineCount-2,STYLE_NOT_TRANSLATE);

	}
	if(textTranslate->GetLineCount() != 0)
	{
		buttonCopy->Enable(true);
		buttonClear->Enable(true);
	}
	textTranslate->Update();
	textTranslate->SetFirstVisibleLine(countLines);
	textTranslate->SetReadOnly(true);

}

bool OPolyglotViewTextTranslate::ViewTranslate()
{

	LoadXML();
	Show(true);
	this->Raise();
	return true;
}


OPolyglot::OPolyglot(wxEvtHandler *handler) 
	: GuiOPolyglot(NULL) 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	panelMain->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif

	this->handler = handler;
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
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_CLOSE_TRANSLATOR,&OPolyglot::OnCloseTranslator,this);
	this->ScanLanguageFrom();
	this->ScanLanguageTo();
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
#if __WXGTK__
		OPOLYGLOT_MESSAGE(wxT("OPolyglot use libportal to capture the screen"));
		PortalInit();
		PortalTakeScreenshot(this);
#else
		OPOLYGLOT_ERROR(wxT("OPolyglot not supported screen capture"));
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
	if(!IS_NULLPTR(frameDownload ))
	{
		delete frameDownload;
	}
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
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglot::OnCancelTranslation,this);
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH,&OPolyglot::OnExitThreadTranslation,this);
	this->Enable(true);
	if((!IS_NULLPTR(threadTranslator))&&(threadTranslator->IsRunning()))
	{
		threadTranslator->Delete();
		threadTranslator = NULL;
		progress->Finish();
	}
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT))
	{
		this->SetWindowStyle(this->GetWindowStyle()|wxSTAY_ON_TOP);
	} else
	{
		this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	}
}

void OPolyglot::OnCancelOCR(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OnCancelOCR"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglot::OnCancelOCR,this);
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnOCRFinish,this);
	this->Enable(true);
	if((!IS_NULLPTR(threadOCR))||(threadOCR->IsRunning()))
	{
		threadOCR->Delete();
		threadOCR = NULL;
		progress->Finish();
	}
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT))
	{
		this->SetWindowStyle(this->GetWindowStyle()|wxSTAY_ON_TOP);
	} else
	{
		this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
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
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT))
	{
		this->SetWindowStyle(this->GetWindowStyle()|wxSTAY_ON_TOP);
	} else
	{
		this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	}
	if(event.GetString().IsEmpty())
	{
		OPOLYGLOT_WARNING(wxT("OnExitThreadTranslation return value IsEmpty"));
		return;
	}
	bool flagPostprocessing= config.ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING
			,OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT);
	wxArrayString postProcessingRegex;
	wxArrayString postProcessingReplace;
	wxXmlDocument docRegex;
	if(flagPostprocessing)
	{
		if(!docRegex.Load(OPOLYGLOT_GET_XML_DATA_FILE))
		{
			OPOLYGLOT_ERROR(wxT("OnExitThreadTranslation not load %s"),OPOLYGLOT_GET_XML_DATA_FILE);
			return;
		}
		for(wxXmlNode *node = docRegex.GetRoot()->GetChildren();node;node = node->GetNext())
		{
			if(node->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_POSTPROCESSING))
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
	wxXmlDocument *xmlTranslate = new wxXmlDocument();
	if(!xmlTranslate->Load(OPOLYGLOT_GET_XML_FILE_TRANSLATE))
	{
		OPOLYGLOT_WARNING(wxT("OnExitThreadTranslation not load %s"),OPOLYGLOT_GET_XML_FILE_TRANSLATE);
		wxXmlNode *root = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Texts"));
		xmlTranslate->SetRoot(root);
	} else
	{
	}

	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Text")))
		{
			wxXmlNode *childNew = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Text"));
			for(wxXmlAttribute *attr = child->GetAttributes();attr;attr=attr->GetNext())
			{
				if(attr->GetName().IsSameAs(wxS("text")))
				{
					wxString text = attr->GetValue();
					for(size_t i =0; (i < postProcessingRegex.GetCount())&&(!text.IsEmpty());i++)
					{
						OPOLYGLOT_DEBUG(wxT("OnExitThreadTranslation %zu %s %s"),i+1,postProcessingRegex.Item(i),postProcessingReplace.Item(i));
						wxRegEx regex(postProcessingRegex.Item(i));
						wxString replace = postProcessingReplace.Item(i);
						replace.Replace(wxS("\\a"),"\a");
						replace.Replace(wxS("\\b"),"\b");
						replace.Replace(wxS("\\n"),"\n");
						replace.Replace(wxS("\\r"),"\r");
						replace.Replace(wxS("\\t"),"\t");
						replace.Replace(wxS("\\v"),"\v");
						replace.Replace(wxS("\\f"),"\f");
						(void)regex.ReplaceAll(&text,replace);
					}
					childNew->AddAttribute(wxS("text"),text);
				} else
				{
					childNew->AddAttribute(attr->GetName(),attr->GetValue());
				}
			}
			xmlTranslate->GetRoot()->AddChild(childNew);
		}
	}
	if(!xmlTranslate->Save(OPOLYGLOT_GET_XML_FILE_TRANSLATE))
	{
		OPOLYGLOT_ERROR(wxT("OnExitThreadTranslation not saved  %s"),OPOLYGLOT_GET_XML_FILE_TRANSLATE);
	}
	delete xmlTranslate;
	viewTextTranslate->ViewTranslate();
}


void OPolyglot::OnCaptureScreen(wxCommandEvent& event)
{
	this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	this->Lower();
	this->Update();
	{
		int w,h;
		wxScreenDC dc;
		dc.GetSize(&w,&h);
		if((0 < w)&&(0 < h))
		{
			OPOLYGLOT_MESSAGE(wxT("OnCaptureScreen(%dx%d)"),w,h);
			wxBitmap bitmap(w,h);
			wxMemoryDC memDC;
			memDC.SelectObject(bitmap);
			memDC.Blit(0,0,w,h,&dc,0,0);
			memDC.SelectObject(wxNullBitmap);
			wxString fileName = wxString::Format(wxS("%s%s%s"),wxFileName::GetTempDir(),wxString(wxFileName::GetPathSeparator()),wxT("screen.png"));
			OPOLYGLOT_DEBUG(wxT("OPolyglot::OnCaptureScreen screenshot %s"),fileName);
			if(!bitmap.SaveFile(fileName,wxBITMAP_TYPE_PNG))
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
#if __WXGTK__
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
}


void OPolyglot::OnOCRFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnOCRFinish"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglot::OnCancelOCR,this);
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH,&OPolyglot::OnOCRFinish,this);
	this->Enable(true);
	progress->Finish();
	threadOCR = NULL;
	if(event.GetString().IsEmpty())
	{
		OPOLYGLOT_WARNING(wxT("OPolyglot::OnOCRFinish return value IsEmpty"));
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT))
		{
			this->SetWindowStyle(this->GetWindowStyle()|wxSTAY_ON_TOP);
		} else
		{
			this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
		}
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
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT))
		{
			this->SetWindowStyle(this->GetWindowStyle()|wxSTAY_ON_TOP);
		} else
		{
			this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
		}
		return;
	}
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	bool flagPreprocessing = config.ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING
			,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT);

	wxXmlNode *rootNode = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Texts"));
	wxArrayString preProcessingRegex;
	wxArrayString preProcessingReplace;
	if(flagPreprocessing)
	{
		wxXmlDocument doc;
		if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
		{
			OPOLYGLOT_ERROR(wxT("OnOCRFinish not load %s for read preProcessing rules"),OPOLYGLOT_GET_XML_DATA_FILE);
			return;
		}
		for(wxXmlNode *node = doc.GetRoot()->GetChildren();node;node = node->GetNext())
		{
			if(node->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_PREPROCESSING))
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
		}
	}

	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxT("Text")))
		{
			wxXmlNode *childNew = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Text"));
			childNew->AddAttribute(wxS("codeOCR"),child->GetAttribute(wxS("codeOCR")));
			if(flagPreprocessing)
			{

				wxString text = child->GetAttribute(wxS("original"));
				for(size_t i =0; (i < preProcessingRegex.GetCount());i++)
				{
					OPOLYGLOT_DEBUG(wxT("OnOCRFinish rule %zu %s %s"),i+1,preProcessingRegex.Item(i),preProcessingReplace.Item(i));
					wxRegEx regex(preProcessingRegex.Item(i));
					wxString replace = preProcessingReplace.Item(i);
					replace.Replace(wxS("\\a"),"\a");
					replace.Replace(wxS("\\b"),"\b");
					replace.Replace(wxS("\\n"),"\n");
					replace.Replace(wxS("\\r"),"\r");
					replace.Replace(wxS("\\t"),"\t");
					replace.Replace(wxS("\\v"),"\v");
					replace.Replace(wxS("\\f"),"\f");
					int count = regex.ReplaceAll(&text,replace);
					OPOLYGLOT_MESSAGE(wxT("OnOCRFinish pre processing replace %zu %d"),i,count);
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
	OPOLYGLOT_DEBUG(wxT("OnOCRFinish %s"),doc.GetRoot()->GetAttribute(wxS("fileName")));
	if(!wxRemoveFile(doc.GetRoot()->GetAttribute(wxS("fileName"))))
	{
		OPOLYGLOT_WARNING(wxT("OnOCRFinish it's not critical,can not delete the file %s"),doc.GetRoot()->GetAttribute(wxS("fileName")));
	}
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglot::OnCancelTranslation,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH,&OPolyglot::OnExitThreadTranslation,this);
	wxArrayString configs = OPolyglotCreateConfigsFromBergamot(OPolyglotGetOriginalLanguage(this->LanguageFrom->GetStringSelection())
			,OPolyglotGetOriginalLanguage(this->LanguageTo->GetStringSelection()));
	threadTranslator = new OPolyglotThreadTranslator(this,configs,outXMl);
	threadTranslator->Run();
	progress = new OPolyglotProgress(this,_("Translating..."));
	progress->Show();
	this->Enable(false);
}



void OPolyglot::ScanLanguageFrom()
{
	OPOLYGLOT_MESSAGE(wxT("ScanLanguageFrom"));
	this->LanguageFrom->Clear();
	this->LanguageFrom->Append(OPolyglotGetTranslatedLanguages(OPolyglotGetInstalledLanguagesFrom()));
	if(0 < this->LanguageFrom->GetCount())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		if(!LanguageFrom->SetStringSelection(OPolyglotGetTranslateLanguage(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT))))
		{
			LanguageFrom->SetSelection(0);
		}
#if 0
		int find = this->LanguageFrom->FindString(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT));
		if(find != wxNOT_FOUND)
		{
			this->LanguageFrom->SetSelection(find);
		} else
		{
			this->LanguageFrom->SetSelection(0);
		}
#endif
		config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPolyglotGetOriginalLanguage(this->LanguageFrom->GetStringSelection()));
	}
	OPOLYGLOT_DEBUG(wxT("LanguageFrom %s %d"),this->LanguageFrom->GetStringSelection(),this->LanguageFrom->GetSelection());
}

void OPolyglot::ScanLanguageTo()
{
	OPOLYGLOT_MESSAGE(wxT("ScanLanguageTo"));
	this->LanguageTo->Clear();
	this->LanguageTo->Append(
			OPolyglotGetTranslatedLanguages(
				OPolyglotGetInstalledLanguagesTo(
					OPolyglotGetOriginalLanguage(this->LanguageFrom->GetStringSelection()))));
	if(0 < this->LanguageTo->GetCount())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		if(!LanguageTo->SetStringSelection(OPolyglotGetTranslateLanguage(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT))))
		{
			LanguageTo->SetSelection(0);
		}
#if 0
		int find = this->LanguageTo->FindString(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT));
		if(find != wxNOT_FOUND)
		{
			this->LanguageTo->SetSelection(find);
		} else
		{
			this->LanguageTo->SetSelection(0);
		}
#endif
		config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPolyglotGetOriginalLanguage(this->LanguageTo->GetStringSelection()));
	}

}

void OPolyglot::OnSelectLanguageFrom( wxCommandEvent& event )
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("OnSelectLanguageFrom"));
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPolyglotGetOriginalLanguage(this->LanguageFrom->GetStringSelection()));
	this->ScanLanguageTo();
}


void OPolyglot::OnSelectLanguageTo( wxCommandEvent& event )
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("OnSelectLanguageTo"));
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPolyglotGetOriginalLanguage(this->LanguageTo->GetStringSelection()));
}


void OPolyglot::OnFinishSetupLanguages(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnFinishSetupLanguages"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglot::OnFinishSetupLanguages,this);
	delete frameDownload;
	frameDownload = NULL;
	ScanLanguageFrom();
	ScanLanguageTo();
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
}


void OPolyglot::OnOpenTranslator( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnOpenTranslator"));
	buttonCaptureScreen->Enable(false);
	frameTranslator = new OPolyglotTranslator(this,LanguageFrom->GetStringSelection(),LanguageTo->GetStringSelection());
	frameTranslator->Show();
}



void OPolyglot::OnStartOCR(wxThreadEvent &event)
{
	if(event.GetString().IsEmpty())
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnStartOCR CANCEL"));
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT))
		{
			this->SetWindowStyle(this->GetWindowStyle()|wxSTAY_ON_TOP);
		} else
		{
			this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
		}
		return;
	}
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnStartOCR"));
	this->Raise();
	wxString langCode = OPolyglotGetCodeFromLanguage(OPolyglotGetOriginalLanguage(this->LanguageFrom->GetStringSelection()));
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
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH,&OPolyglot::OnOCRFinish,this);
#if 1
	if(!config.Read(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR_DEFAULT).IsSameAs(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR_DEFAULT))
	{
		if(!OPolyglotGetCodeFromLanguage(OPolyglotGetOriginalLanguage(config.Read(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR))).IsSameAs(langCode))
		{
			langCode = langCode+"+"+OPolyglotGetCodeFromLanguage(OPolyglotGetOriginalLanguage(config.Read(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR_DEFAULT)));
		}
	}
#endif
	OPOLYGLOT_DEBUG(wxT("OPolyglot::OnStartOCR lang code %s"),langCode);
	threadOCR = new OPolyglotThreadOCR(this,dirTraineddata,langCode,event.GetString());
	progress = new OPolyglotProgress(this,_("Processing OCR...."));
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
	if(event.GetInt() != 0)
	{

		this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
		fullscreen = new OPolyglotFullscreenFrame(this,fileName);
		fullscreen->Raise();
	}
}


void OPolyglot::OnShowTranslation( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnShowTranslation"));
	viewTextTranslate->Show(true);
	viewTextTranslate->Raise();
}

void OPolyglot::OnCloseTranslator(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnCloseTranslator"));
	frameTranslator->Destroy();
	frameTranslator = NULL;
	buttonCaptureScreen->Enable(true);
	this->SetFocus();
}

OPolyglotTranslator::OPolyglotTranslator(wxWindow *parent,wxString languageFrom,wxString languageTo) : GUIOPolyglotTranslator(parent)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator %s -> %s"),languageFrom,languageTo);
	this->SetTitle(wxString::Format(wxT("OPolyglot %s"),_("translator")));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	Bind(wxEVT_THREAD,&OPolyglotTranslator::OnThreadTranslatorFinish,this);
	Bind(wxEVT_TIMER,&OPolyglotTranslator::OnStartTranslator,this);
	startTranslation = new wxTimer(this);
	wxBitmap copyIcon = wxArtProvider::GetBitmap(wxART_COPY,wxART_BUTTON);
	wxBitmap rechangeIcon = wxArtProvider::GetBitmap(OPOLYGLOT_ART_RECHANGE,wxART_BUTTON,copyIcon.GetSize());
	buttonCopy->SetBitmap(copyIcon);
	buttonRechange->SetBitmap(rechangeIcon);
	LanguageFrom->Append(OPolyglotGetTranslatedLanguages(OPolyglotGetInstalledLanguagesFrom()));
	if(LanguageFrom->GetStrings().Index(languageFrom) != wxNOT_FOUND)
	{
		LanguageFrom->Select(LanguageFrom->GetStrings().Index(languageFrom));
	} else
	{
		LanguageFrom->Select(0);
	}
	LanguageTo->Append(OPolyglotGetTranslatedLanguages(OPolyglotGetInstalledLanguagesTo(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()))));
	if(LanguageTo->GetStrings().Index(languageTo) != wxNOT_FOUND)
	{
		LanguageTo->Select(LanguageTo->GetStrings().Index(languageTo));
	} else
	{
		LanguageTo->Select(0);
	}
	wxRect rect = parent->GetRect();
	wxPoint pos = GetPosition();
	pos.y = (rect.GetY()+rect.GetHeight()+5);
	SetPosition(pos);
	buttonCopy->Enable(false);
	this->parent = parent;
	configsTranslator = OPolyglotCreateConfigsFromBergamot(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()),OPolyglotGetOriginalLanguage(LanguageTo->GetStringSelection()));
	OPOLYGLOT_DEBUG(wxT("OPolyglotTranslator %zu"),configsTranslator.GetCount());
}

OPolyglotTranslator::~OPolyglotTranslator()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::~OPolyglotTranslator"));
	if(startTranslation->IsRunning())
	{
		startTranslation->Stop();
	}
	delete startTranslation;
}

void OPolyglotTranslator::OnClose(wxCloseEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnClose"));
	wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CLOSE_TRANSLATOR));
}

void OPolyglotTranslator::OnRechange(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnRechange"));
	wxString oldLangFrom = LanguageFrom->GetStringSelection();
	wxString oldLangTo = LanguageTo->GetStringSelection();
	OPOLYGLOT_DEBUG(wxT("OPolyglotTranslator::OnRechange old languages \"%s -> %s\""),oldLangFrom,oldLangTo);
	if(LanguageFrom->GetStrings().Index(oldLangTo) != wxNOT_FOUND)
	{
		LanguageFrom->Select(LanguageFrom->GetStrings().Index(oldLangTo));
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnRechange not found \"%s -> %s\" in installed languages"),oldLangTo,oldLangFrom);
		LanguageFrom->Select(LanguageFrom->GetStrings().Index(oldLangFrom));
		wxMessageDialog msg(this,wxString::Format(wxS("%s \"%s -> %s\""),_("Not found in installed languages"),oldLangTo,oldLangFrom),wxT("OPolyglot"),wxICON_ERROR|wxOK);
		msg.ShowModal();
		return;
	}
	LanguageTo->Clear();
	LanguageTo->Append(OPolyglotGetTranslatedLanguages(OPolyglotGetInstalledLanguagesTo(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()))));
	if(LanguageTo->GetStrings().Index(oldLangFrom) != wxNOT_FOUND)
	{
		LanguageTo->Select(LanguageTo->GetStrings().Index(oldLangFrom));
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnRechange not found \"%s -> %s\" in installed languages"),oldLangTo,oldLangFrom);
		LanguageFrom->Select(LanguageFrom->GetStrings().Index(oldLangFrom));
		LanguageTo->Clear();
		LanguageTo->Append(OPolyglotGetTranslatedLanguages(OPolyglotGetInstalledLanguagesTo(OPolyglotGetOriginalLanguage(oldLangFrom))));
		LanguageTo->Select(LanguageTo->GetStrings().Index(oldLangTo));
		wxMessageDialog msg(this,wxString::Format(wxS("%s \"%s -> %s\""),_("Not found in installed languages"),oldLangTo,oldLangFrom),wxT("OPolyglot"),wxICON_ERROR|wxOK);
		msg.ShowModal();
		return;
	}
	buttonCopy->Enable(false);
	if((IS_NULLPTR(GetThread()))||(!GetThread()->IsRunning()))
	{
		if(textTranslate->GetValue().Length() != 0)
		{
			textOriginal->SetValue(textTranslate->GetValue());
		}
	}
	configsTranslator = OPolyglotCreateConfigsFromBergamot(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()),OPolyglotGetOriginalLanguage(LanguageTo->GetStringSelection()));
	buttonCopy->Enable(false);
	buttonRechange->Enable(false);
	startTranslation->Start(50,wxTIMER_ONE_SHOT);
}

void OPolyglotTranslator::OnLanguageFrom(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnLanguageFrom(%s)"),LanguageFrom->GetStringSelection());
	wxString oldLangTo = LanguageTo->GetStringSelection();
	LanguageTo->Clear();
	LanguageTo->Append(OPolyglotGetTranslatedLanguages(OPolyglotGetInstalledLanguagesTo(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()))));
	if(LanguageTo->GetStrings().Index(oldLangTo) != wxNOT_FOUND)
	{
		LanguageTo->Select(LanguageTo->GetStrings().Index(oldLangTo));
	} else
	{
		LanguageTo->Select(0);
	}
	configsTranslator.Clear();
	configsTranslator = OPolyglotCreateConfigsFromBergamot(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()),OPolyglotGetOriginalLanguage(LanguageTo->GetStringSelection()));

}

void OPolyglotTranslator::OnLanguageTo(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnLanguageTo(%s)"),LanguageTo->GetStringSelection());
	configsTranslator = OPolyglotCreateConfigsFromBergamot(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()),OPolyglotGetOriginalLanguage(LanguageTo->GetStringSelection()));
}

wxThread::ExitCode OPolyglotTranslator::Entry()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::Entry"));
	wxXmlNode *rootNode = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Texts"));
	wxXmlNode *textNode = new wxXmlNode(rootNode,wxXML_ELEMENT_NODE,wxS("Text"));
	textNode->AddAttribute(wxS("original"),textOriginal->GetValue());
	wxString outXML = wxEmptyString;
	wxStringOutputStream sos(&outXML);
	wxXmlDocument docXML;
	docXML.SetRoot(rootNode);
	docXML.Save(sos);
	wxString configYml = configsTranslator.Item(0);
	wxString configYmlSecond = wxEmptyString;
	if(configsTranslator.GetCount() == 2)
	{
		configYmlSecond = configsTranslator.Item(1);
	}
	wxString result = LibOPolyglotTranslator(outXML,configYml,configYmlSecond);
	wxThreadEvent *event = new wxThreadEvent();
	event->SetString(wxString(result));
	wxQueueEvent(GetEventHandler(),event);
	return (wxThread::ExitCode)0;
}

void OPolyglotTranslator::OnThreadTranslatorFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnThreadTranslatorFinish"));
	textOriginal->Enable(true);
	textTranslate->Enable(true);
	buttonRechange->Enable(true);
	textTranslate->SetValue(wxT(""));
	if(event.GetString().IsEmpty())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnThreadTranslatorFinish return IsNull"));
		wxMessageDialog msg(this,wxT("Error: \"OPolyglotTranslator\" return thread isNull"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
		msg.ShowModal();
		return;
	}
	wxStringInputStream sis(event.GetString());
	wxXmlDocument doc(sis);
	if((!doc.IsOk())||(doc.GetRoot()->GetName().IsSameAs(wxS("Error"))))
	{
		if(!doc.IsOk())
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnThreadTranslatorFinish return not valid Xml Document"));
			wxMessageDialog msg(this,_("Error: thread OPolyglotTranslator return not valid Xml Document"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
			msg.ShowModal();
			return;
		} else
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnThreadTranslatorFinish return \"Error\" %s"),doc.GetRoot()->GetAttribute(wxS("value")));
			wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Error: thread OPolyglotTranslator"),doc.GetRoot()->GetAttribute(wxS("value"))),wxT("OPolyglot"),wxICON_ERROR|wxOK);
			msg.ShowModal();
		}
	}
	wxString value = wxEmptyString;
	if(doc.GetRoot()->GetName().IsSameAs(wxS("Texts")))
	{
		for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child=child->GetNext())
		{
			if(child->GetName().IsSameAs(wxS("Text")))
			{
				value = wxString::Format(wxT("%s%s\n"),value,child->GetAttribute(wxS("text")));
			}
		}
		textTranslate->SetValue(value);
		if(0 < value.Length())
		{
			buttonCopy->Enable(true);
		}
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnThreadTranslatorFinish not valid root xml node \"%s\" != \"Texts\""),doc.GetRoot()->GetName());
	}
}

void OPolyglotTranslator::OnTextSource(wxCommandEvent& event) 
{
	startTranslation->Start(1500,wxTIMER_ONE_SHOT);
	buttonCopy->Enable(false);
	buttonRechange->Enable(false);
}

void OPolyglotTranslator::OnStartTranslator(wxTimerEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnStartTranslator %zu"),configsTranslator.GetCount());
	if((IS_NULLPTR(GetThread()))||(!(GetThread()->IsRunning())))
	{
		textTranslate->Enable(false);
		textTranslate->SetValue(_("translation process..."));
		if(CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnStartTranslator could not create the worker thread!"));
			wxMessageDialog msg(this,_("Could not create the worker thread!"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
			msg.ShowModal();
			return;
		}
		OPOLYGLOT_DEBUG(wxT("OPolyglotTranslator::OnStartTranslator thread created"));
		if(GetThread()->Run() != wxTHREAD_NO_ERROR)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnStartTranslator could not run the worker thread!"));
			wxMessageDialog msg(this,_("Could not run the worker thread!"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
			msg.ShowModal();
			return;
		}
		OPOLYGLOT_DEBUG(wxT("OPolyglotTranslator::OnStartTranslator thread runing"));
	} else
	{
		startTranslation->Start(100,wxTIMER_ONE_SHOT);
	}
}


void OPolyglotTranslator::OnCopy( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnCopy"));
	if(wxTheClipboard->Open())
	{
		    wxTheClipboard->SetData( new wxTextDataObject(textTranslate->GetValue()) );
		    wxTheClipboard->Close();
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnCopy not open clipboard"));
	}
}
