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
#include "../res/icon.xpm"
#include "../res/icon_copy.xpm"
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
#ifdef __WXMSW__
    #include <wx/msw/private.h>
#endif



enum{
	TIMER_ID,
	TIMER_MOUSE_ID,
	TIMER_PROGRESS_OCR_TRANSLATION_ID,
};


OPolyglotProgress::OPolyglotProgress(wxWindow *parent) : GUIOPolyglotProgressOCRTranslator(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgress"));
	this->parent = parent;
	timerUpdate.SetOwner(this,TIMER_ID);
	this->Bind(wxEVT_TIMER,&OPolyglotProgress::OnUpdateProgress,this);
	timerUpdate.Start(200);
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

OPolyglot::OPolyglot(wxEvtHandler *handler) 
	: GuiOPolyglot(NULL)  
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot"));
	SetIcon(wxICON(icon));
	this->handler = handler;
	this->ButtonCopyTranslate->SetBitmap(wxICON(icon_copy));
	this->ButtonCopyTranslate->SetToolTip(_("Copies the translation text to the clipboard."));
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxSize s = this->GetSize();
	viewDialogTranslator = false;
	OPOLYGLOT_MESSAGE(wxT("OPolyglot display(%dx%d)"),geom.GetWidth(),geom.GetHeight());
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
	timerClipboardChecking = new wxTimer(this,TIMER_ID);
	timerMouseState = new wxTimer(this,TIMER_MOUSE_ID);
	//timerProgressOcrTranslation = new wxTimer(this,TIMER_PROGRESS_OCR_TRANSLATION_ID);
	this->Bind(wxEVT_TIMER,wxTimerEventHandler(OPolyglot::OnTimeCheckClipboard),this,TIMER_ID);
	this->Bind(wxEVT_TIMER,wxTimerEventHandler(OPolyglot::OnTimeCheckMouseState),this,TIMER_MOUSE_ID);
	//this->Bind(wxEVT_TIMER,wxTimerEventHandler(OPolyglot::OnTimerProgressOCRTranslation),this,TIMER_PROGRESS_OCR_TRANSLATION_ID);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SEND_IMAGE,&OPolyglot::OnReceivImage,this);
	this->Bind(wxEVT_RIGHT_DOWN,&OPolyglot::OnRightClick,this);	

	if(this->EnableAutoTranslate->IsChecked())
	{
		timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
	}
	if (wxTheClipboard->Open()&&wxTheClipboard->IsSupported(wxDF_TEXT))
	{
		wxTextDataObject data;
		wxTheClipboard->GetData( data );
		lastClipboardText =  data.GetText();
	}  else
	{
		lastClipboardText = wxEmptyString;
	}
	wxTheClipboard->Close();
	mouseLeftButtonPressed = false;
	coordStartX = -1;
	coordStartY = -1;
	countLeftPress = 0;
	this->ScanLangs();
	//this->MainVBox->Layout();
	//this->Layout();
	//this->Refresh();
	MainVBox->Fit(this);
	MainVBox->Layout();
	this->Update();
	this->Show();
	imageForOCR = NULL;
	if( (0 == this->LanguageFrom->GetCount())||(0 == this->LanguageTo->GetCount()))
	{
		frameDownload = new OPolyglotDownloadLanguage(this);
		this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglot::OnFinishSetupLanguages,this);
		wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
		frameDownload->Show();
	}
	wxScreenDC dc;
	int w,h;
	dc.GetSize(&w,&h);
	if((w == 0)||(h == 0))
	{
		OCRTranslate->Enable(false);
	}
	this->GetSize(&w,&h);
	if(w != OPOLYGLOT_CONFIG_INT_WIDTH_DEFAULT)
	{
		config.Write(OPOLYGLOT_CONFIG_INT_WIDTH,w);
	}
	this->GetSize(&w,&h);
	OPOLYGLOT_DEBUG(wxT("OPolyglot::OPolyglot size %dx%d"),w,h);
#if OPOLYGLOT_DEBUG_ENABLED
	wxDynamicLibrary l(OPOLYGLOT_LIBRARY);
	if(!l.IsLoaded())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot error load library"));
	}
#endif
	//this->SetSize(w,40);

}



OPolyglot::~OPolyglot()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglot"));
	//timerClipboardChecking->~wxTimer();
	delete timerClipboardChecking;
	//timerMouseState->~wxTimer();
	//delete timerProgressOcrTranslation;
	delete LanguageFrom;
	delete ButtonCopyTranslate;
}

void OPolyglot::OnMenuSetup( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnMenuSetup"));
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}

/*
void OPolyglot::OnSize(wxSizeEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnSize %dx%d"),event.GetSize().GetWidth(),event.GetSize().GetHeight());
	this->SetClientSize(event.GetSize().GetWidth(),event.GetSize().GetHeight());
}
*/

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
		FinishThread();
	}
}

void OPolyglot::OnCancelOCR(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OnCancelOCR"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnExitThreadOCR,this);
	if(threadOCR->IsRunning())
	{
		threadOCR->Delete();
		threadOCR = NULL;
		progress->Finish();
		FinishThread();
	}
}

void OPolyglot::FinishThread()
{

	OPOLYGLOT_MESSAGE(wxT("FinishThread"));
	progress->Finish();
	if(this->IsShown())
	{
		if(this->EnableAutoTranslate->IsChecked())
		{
			if (wxTheClipboard->Open())
			{
				// This data objects are held by the clipboard,
				// so do not delete them in the app.
				//lastClipboardText = this->textTranslation->GetValue();
				OPOLYGLOT_DEBUG(wxT("translate "));
				wxTheClipboard->SetData( new wxTextDataObject(lastClipboardText) );
				wxTheClipboard->Close();
			} else
			{
				OPOLYGLOT_ERROR(wxT("error open clipboard"));
			}
			timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
		} 
		if(this->OCRTranslate->IsChecked())
		{
			timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
		}
	}
	this->Enable(true);
	this->ButtonCopyTranslate->Enable(true);
	/* imitate pressed on buttonShowTranslate */
	this->buttonShowTranslate->SetValue(true);
	wxPostEvent(this->buttonShowTranslate,wxCommandEvent(wxEVT_TOGGLEBUTTON));
}


void OPolyglot::OnExitThreadTranslation(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OnExitThreadTranslation %d"),event.GetInt());
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnExitThreadTranslation,this);
	threadTranslator = NULL;
	if((event.GetInt()!=0)&&(!event.GetString().IsEmpty()))
	{
		OPOLYGLOT_ERROR(wxT("error thread translation%s"),event.GetString());
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),event.GetString()),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
	}
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	wxString text = event.GetString();
	if(!text.IsEmpty())
	{

	}
	delete config;
	if (wxTheClipboard->Open())
	{
		lastClipboardText = text;
		wxTheClipboard->SetData(new wxTextDataObject(text));
		wxTheClipboard->Close();
	}
	if(!text.IsEmpty())
	{
		this->textTranslation->Clear();
		this->textTranslation->AppendText(text);
	}
	FinishThread();
	this->Raise();
	OPOLYGLOT_DEBUG(wxT("postprocessing"));
}


void OPolyglot::OnShowTranslate(wxCommandEvent &event)
{
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("OnShowTranslate(%s)"),OPOLYGLOT_BOOL_TO_STRING(this->buttonShowTranslate->GetValue()));
	if(this->buttonShowTranslate->GetValue())
	{
		int width,height;
		if(!this->translatePanel->IsShown())
		{
			this->GetSize(&width,&height);
			this->translatePanel->Show(true);
			if(height < 360)
			{
				this->SetSize((int)(config->ReadLong(OPOLYGLOT_CONFIG_INT_WIDTH,OPOLYGLOT_CONFIG_INT_WIDTH_DEFAULT)),(int)config->ReadLong(OPOLYGLOT_CONFIG_INT_HEIGHT,OPOLYGLOT_CONFIG_INT_HEIGHT_DEFAULT));
			}
			this->buttonShowTranslate->SetLabel(_("hide the translation"));
		}
		if(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_SHOW_ORIGINAL,OPOLYGLOT_CONFIG_BOOL_SHOW_ORIGINAL_DEFAULT))
		{
			this->buttonShowOriginal->SetValue(true);
			wxPostEvent(this->buttonShowOriginal,wxCommandEvent(wxEVT_TOGGLEBUTTON));
		}
	} else
	{
		int width,height;
		this->translatePanel->Show(false);
		this->buttonShowTranslate->SetLabel(_("show translation"));
		this->MainVBox->Layout();
		MainVBox->Fit(this);
		this->Refresh();
		this->GetSize(&width,&height);
		this->SetSize((int)config->Read(OPOLYGLOT_CONFIG_INT_WIDTH,OPOLYGLOT_CONFIG_INT_WIDTH_DEFAULT),height);

	}
	delete config;
}


void OPolyglot::OnCopyTextTranslate( wxCommandEvent& event ) 
{
	bool flagTimerClipboard = timerClipboardChecking->IsRunning();
	OPOLYGLOT_MESSAGE(wxT("OnCopyTextTranslate"));
	timerClipboardChecking->Stop();
	if (wxTheClipboard->Open())
	{
		// This data objects are held by the clipboard,
		// so do not delete them in the app.
		lastClipboardText = this->textTranslation->GetValue();
		wxTheClipboard->SetData( new wxTextDataObject(lastClipboardText) );
		//lastClipboardText = this->textTranslation->GetValue();
		wxTheClipboard->Close();
	} else
	{
		OPOLYGLOT_ERROR(wxT("error open clipboard"));
	}
	if(flagTimerClipboard)
	{
		timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
	}

}

void OPolyglot::AddOrSetOriginalText(wxString text)
{
	OPOLYGLOT_MESSAGE(wxT("AddOrSetOriginalText text length %ld"),text.Length());
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	bool flag = config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT);
	wxString result = text;
	OPOLYGLOT_MESSAGE(wxT("AddOrSetOriginalText preProcessingRegex %s %ld"),OPOLYGLOT_BOOL_TO_STRING(flag),preProcessingRegex.GetCount());
	for(size_t i =0; (i < preProcessingRegex.GetCount())&&flag;i++)
	{
		// Регулярний вираз для знаходження переносу рядка між двома маленькими буквами в Unicode
		wxRegEx regex(preProcessingRegex.Item(i));
		// Заміна переносу рядка на пробіл
		wxString replace = preProcessingReplace.Item(i);
		replace.Replace(wxS("\\a"),"\a");
		replace.Replace(wxS("\\b"),"\b");
		replace.Replace(wxS("\\n"),"\n");
		replace.Replace(wxS("\\r"),"\r");
		replace.Replace(wxS("\\t"),"\t");
		replace.Replace(wxS("\\v"),"\v");
		replace.Replace(wxS("\\f"),"\f");
		OPOLYGLOT_MESSAGE(wxT("AddOrSetOriginalText replace %ld %d"),i,regex.ReplaceAll(&result,wxString::Format(wxS("%s"),preProcessingReplace.Item(i).c_str())));
		//OPOLYGLOT_DEBUG(wxT("%ld\t'%s' '%s' count Replace %ld"),i,preProcessingRegex.Item(i),preProcessingReplace.Item(i),regex.ReplaceAll(&result, wxString::Format(wxS("%s"),preProcessingReplace.Item(i).c_str())));
	}
	flag = config->ReadBool(OPOLYGLOT_CONFIG_BOOL_METHOD_CREATION_TEXT_NEW,OPOLYGLOT_CONFIG_BOOL_METHOD_CREATION_TEXT_DEFAULT);
	if(flag)
	{
		this->textOriginal->Clear();
	}
	delete config;
	this->textOriginal->AppendText(result);
	this->textOriginal->AppendText(wxS("\n"));
}

void OPolyglot::OnExitThreadOCR(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnExitThreadOCR %d"),event.GetInt());
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnExitThreadOCR,this);
	if((event.GetInt()!=0)&&(!event.GetString().IsEmpty()))
	{
		OPOLYGLOT_ERROR(wxT("error thread ocr %s"),event.GetString());
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),event.GetString()),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		FinishThread();
		threadOCR = NULL;
		return;
	}
	if(!event.GetString().IsEmpty())
	{
		AddOrSetOriginalText(event.GetString());
		threadTranslator = new OPolyglotThreadTranslator(this,&configTranslatorFileYml,this->textOriginal->GetValue());
		this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnExitThreadTranslation,this);
	} else
	{
		OPOLYGLOT_DEBUG(wxT("FinishThread"));
		threadOCR = NULL;
		FinishThread();	
	}
}

void OPolyglot::OnShowOriginal(wxCommandEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OnShowOriginal(%s)"),OPOLYGLOT_BOOL_TO_STRING(this->buttonShowOriginal->GetValue()));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	if(this->buttonShowOriginal->GetValue())
	{
		this->textOriginal->Show(true);
		this->buttonShowOriginal->SetLabel(_("Hide the text of the original"));
		config->Write(OPOLYGLOT_CONFIG_BOOL_SHOW_ORIGINAL,true);
	} else
	{
		this->textOriginal->Show(false);
		this->buttonShowOriginal->SetLabel(_("Show the text of the original"));
		config->Write(OPOLYGLOT_CONFIG_BOOL_SHOW_ORIGINAL,false);
	}
	delete config;
	this->MainVBox->Layout();
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



void OPolyglot::OnRightClick(wxMouseEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OnRightClick"));
}

void OPolyglot::OnReceivImage(wxThreadEvent &event)
{

	if(event.GetInt() == 0)
	{
		OPOLYGLOT_MESSAGE(wxT("OnReceivImage non select area"));
		if((this->IsShown()))
		{
			if(this->OCRTranslate->IsChecked())
			{
				timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
			}
			if(this->EnableAutoTranslate->IsChecked())
			{
				timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
			} 
		}
		return;
	}
	coordStartX = -1;
	coordStartY = -1;
	countLeftPress = 0;
	//imageForOCR = event.GetPayload<OPolyglotImage *>();
	StartThreadTranslation();
	this->Raise();
}
void OPolyglot::OnOCRTranslate( wxCommandEvent& event )
{
	if(this->OCRTranslate->IsChecked())
	{
		OPOLYGLOT_MESSAGE(wxT("OnOCRTranslate checked"));
		timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("OnOCRTranslate not checked"));
		timerMouseState->Stop();
	}
}

void OPolyglot::OnEnableClipboard( wxCommandEvent& event ) {
	OPOLYGLOT_MESSAGE(wxT("OnEnableClipboard(%s)"),OPOLYGLOT_BOOL_TO_STRING(this->EnableAutoTranslate->IsChecked()));
	if(this->EnableAutoTranslate->IsChecked())
	{
		timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
	} else
	{
		timerClipboardChecking->Stop();
	}
}

void OPolyglot::SetShow(bool flag)
{
	OPOLYGLOT_MESSAGE(wxT("SetShow(%s) %s"),OPOLYGLOT_BOOL_TO_STRING(flag),OPOLYGLOT_BOOL_TO_STRING(this->IsShown()));
	if(flag)
	{

		this->Show(true);
		flagShow = true;
		if(this->EnableAutoTranslate->IsChecked())
		{
			timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
		} 
		if(this->OCRTranslate->IsChecked())
		{
			timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
		}
	} else
	{
		this->Show(false);
		flagShow = false;
		timerClipboardChecking->Stop();
		timerMouseState->Stop();
	}
}


void OPolyglot::OnSize( wxSizeEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnSize(%dx%d)"),event.GetSize().GetWidth(),event.GetSize().GetHeight());
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	config->Write(OPOLYGLOT_CONFIG_INT_WIDTH,event.GetSize().GetWidth());
	if(this->translatePanel->IsShown())
	{
		config->Write(OPOLYGLOT_CONFIG_INT_HEIGHT,event.GetSize().GetHeight());
	}
	delete config;
	event.Skip();
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

void OPolyglot::OnTimeCheckClipboard(wxTimerEvent &event)
{
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxString text;
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			text = data.GetText();
			text.Replace(wxS("\r"),wxS(""),true);
			if(!(lastClipboardText.IsSameAs(text)))
			{
				OPOLYGLOT_DEBUG(wxT("start translate %s"),OPOLYGLOT_BOOL_TO_STRING(lastClipboardText.IsSameAs(text)));
				lastClipboardText = text;
				AddOrSetOriginalText(lastClipboardText);
				timerClipboardChecking->Stop();
				timerMouseState->Stop();
				imageForOCR = nullptr;
				StartThreadTranslation();

			}
		}
		wxTheClipboard->Close();
	} else
	{
		OPOLYGLOT_ERROR(wxT("error open clipboards"));
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Error open clipboards")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
	}
}


void OPolyglot::OnStartTranslate(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnStartTranslate(%s -> %s)"),this->LanguageFrom->GetStringSelection(),this->LanguageTo->GetStringSelection());
	imageForOCR = NULL;
	StartThreadTranslation();
}


void OPolyglot::StartThreadTranslation()
{
	OPOLYGLOT_MESSAGE(wxT("StartThreadTranslation(ocr %s)"),OPOLYGLOT_BOOL_TO_STRING(!IS_NULLPTR(imageForOCR)));
	if(configTranslatorFileYml.GetCount() == 0)
	{
		OPOLYGLOT_ERROR(wxT("error config files translator %ld"),configTranslatorFileYml.GetCount());
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Error not find config translator")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}

	if(!IS_NULLPTR(imageForOCR))
	{
		wxString langCode = wxEmptyString; //codeLanguageFrom.Item(this->LanguageFrom->GetSelection());
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
		this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnExitThreadOCR,this);
		threadOCR = new OPolyglotThreadOCR(this,dirTraineddata,langCode,imageForOCR);
		delete imageForOCR;
		imageForOCR = NULL;
		OPOLYGLOT_DEBUG(wxT("start threadOCR"));
		threadTranslator = NULL;
	} else
	{
		this->Bind(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,&OPolyglot::OnCancelTranslation,this);
		this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglot::OnExitThreadTranslation,this);
		threadTranslator = new OPolyglotThreadTranslator(this,&configTranslatorFileYml,wxString(this->textOriginal->GetValue()));
		threadOCR = NULL;
	}
	messageProgressThreadTranslation = wxEmptyString;
	progress = new OPolyglotProgress(this);
	progress->Show();
	this->Enable(false);
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

void OPolyglot::StartTranslation()
{
	OPOLYGLOT_MESSAGE(wxT("StartTranslator configuration %s -> %s"),this->LanguageFrom->GetStringSelection(),this->LanguageTo->GetStringSelection());
	for(size_t i =0; i < configTranslatorFileYml.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("\t%ld : %s"),i,configTranslatorFileYml.Item(i));
	}
	StartThreadTranslation();
}



void OPolyglot::OnTimeCheckMouseState(wxTimerEvent &event)
{
	wxSize s = wxGetDisplaySize();
	wxMouseState mouseState = wxGetMouseState();
#if __WXMSW__
#pragma message "check mouse in window"
	// 1. Отримуємо HWND нашого вікна wxWidgets
    HWND myHwnd = (HWND)this->GetHWND();

    // 2. Отримуємо HWND вікна, яке зараз на передньому плані в Windows
    HWND foregroundHwnd = ::GetForegroundWindow();

    // 3. Порівнюємо
    if (myHwnd == foregroundHwnd) {
        // Користувач зараз дивиться саме на ваше вікно і воно поверх інших
		event.Skip();
		return;
    } else {
        // Користувач переключився на іншу програму (браузер, провідник тощо)
    }
#if 0
	HWND hwndNext = ::GetWindow((HWND)this->GetHWND(), GW_HWNDPREV);
	if (hwndNext == NULL) {
		// Перед цим вікном у Z-порядку більше нікого немає
		event.Skip();
		return;
	}
	if(this->GetScreenRect().Contains(wxGetMousePosition())&&this->IsActive())//&&this->IsShownOnScreen()&&this->HasFocus())
	{
		event.Skip();
		return;
	}
#endif
#endif
	if(mouseState.LeftIsDown())
	{
		OPOLYGLOT_MESSAGE(wxT("OnTimeCheckMouseState %dx%d %s"),s.GetWidth(),s.GetHeight(),OPOLYGLOT_BOOL_TO_STRING(mouseState.LeftIsDown()));
		if((coordStartX == -1)&&(coordStartY == -1))
		{
			coordStartX = mouseState.GetX();
			coordStartY = mouseState.GetY();
		}
		imageForOCR = new OPolyglotImage();
		fullscreen = new OPolyglotFullscreenFrame(this,imageForOCR);
#if __FLATPAK
		fullscreen->Raise();
#endif
#if __WXMSW__
#pragma message "compile fullscreen->Raise()"			
		fullscreen->Raise();
#endif
#if __WXGTK__
#pragma message "COMPILE FOR WXGTK"
#endif
		timerMouseState->Stop();
	} else
	{
	}
	event.Skip();

}

