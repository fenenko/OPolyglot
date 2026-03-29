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


#include "OPolyglotSetup.h"
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotEvent.h"
#include <wx/display.h>
#ifndef __WXMSW__
#include "../res/icon.xpm"
#endif
#include "Utils.h"
#include <wx/config.h>
#include "Config.h"
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/msgdlg.h>
#include <wx/textfile.h>

enum{
	STYLE_MESSAGE=1,
	STYLE_WARNING=2,
	STYLE_ERROR=3,
};

OPolyglotViewLog::OPolyglotViewLog(wxFrame *parent) : GUIViewLog(parent)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewLog"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
#else
	SetIcon(wxICON(icon));
#endif
	OPOLYGLOT_DEBUG(wxT("%d %d %d"),STYLE_MESSAGE,STYLE_WARNING,STYLE_ERROR);
	Log->SetLexer(wxSTC_LEX_CONTAINER);
	Log->AnnotationClearAll();
	Log->Clear();
	Log->StyleSetForeground(STYLE_MESSAGE,wxColour(0,100,0)); /* dark green */
	Log->StyleSetBold(STYLE_MESSAGE,true);
	Log->StyleSetForeground(STYLE_ERROR,wxColour(255,0,0)); /* red */
	Log->StyleSetBold(STYLE_ERROR,true);
	Log->StyleSetForeground(STYLE_WARNING,wxColour(255,127,0)); /* Dark Orange */
	Log->StyleSetBold(STYLE_WARNING,true);
	//Log->SetWrapMode( wxSTC_WRAP_WORD);
	wxTextFile file;
	file.Open(OPOLYGLOT_LOG_FILENAME);
	for(wxString str = file.GetFirstLine();!file.Eof();str = file.GetNextLine())
	{
		int start = Log->GetTextLength();
		str = wxString::Format(wxS("%s\n"),str);
		if(str.Contains(wxS("Error:")))
		{
			Log->AppendText(str);
			int end = Log->GetTextLength();
			Log->StartStyling(start);
			Log->SetStyling(end-start,STYLE_ERROR);
		} else
		{
			if(str.Contains(wxS("Warning:")))
			{
				Log->AppendText(str);
				int end = Log->GetTextLength();
				Log->StartStyling(start);
				Log->SetStyling(end-start,STYLE_WARNING);
			} else
			{
				Log->AppendText(str);
				int end = Log->GetTextLength();
				Log->StartStyling(start);
				Log->SetStyling(end-start,STYLE_MESSAGE);
			}
		}
	}
	OPOLYGLOT_DEBUG(wxT("OPolyglotViewLog line count %d"),Log->GetLineCount());
	Log->SetFirstVisibleLine(Log->GetLineCount());
	Show();
}

OPolyglotViewLog::~OPolyglotViewLog()
{
	OPOLYGLOT_MESSAGE(wxT("~ViewLog"));
}

OPolyglotSetup::OPolyglotSetup(wxEvtHandler *parent) : GUIOPolyglotSetup(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSetup"));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxPoint position;
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
#else
	SetIcon(wxICON(icon));
#endif
	handler = parent;
	this->ButtonSetupLanguages->SetToolTip(_("installation or removal of translator languages."));
	this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	this->SetPosition(wxPoint((geom.width-this->GetSize().GetWidth())/2,(geom.height -this->GetSize().GetHeight())/2));
	this->StyleStayOnTop->SetValue(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT));
	this->LogLevel->SetStringSelection(config->Read(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT));
	wxString method;
	if(	config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(wxT("BEST")))
	{
		method = _("BEST");
	} else
	{
		method = _("FAST");
	}
	this->MethodTranslation->SetStringSelection(method);
	if(	config->Read(OPOLYGLOT_CONFIG_STRING_OCR_METHOD,OPOLYGLOT_CONFIG_STRING_OCR_METHOD_DEFAULT).IsSameAs(wxT("BEST")) )
	{
		method = _("BEST");
	} else
	{
		method = _("FAST");
	}
	this->MethodOCR->SetStringSelection(method);
	this->EnablePreprocessing->SetValue(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT));
	this->RulesPreprocessing->Show(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT));
	this->EnablePostprocessing->SetValue(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT));
	this->RulesPostprocessing->Show(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT));
	/* */
	wxDir dir(OPOLYGLOT_LOCALE_DIR);
	wxString filename;
	if(!dir.IsOpened())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotSetup no catalog for locales %s"),OPOLYGLOT_LOCALE_DIR);
		wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("no catalog for locales"),OPOLYGLOT_LOCALE_DIR),wxT("OPolyglot"),wxICON_ERROR|wxOK);
		msg.ShowModal();
		this->Destroy();
		return;
	} else
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotSetup dir %s"),dir.GetName());
	}
	bool cont = dir.GetFirst(&filename,wxEmptyString,wxDIR_NO_FOLLOW|wxDIR_DIRS);
	OPOLYGLOT_DEBUG(wxT("OPolyglotSetup System language %d %d %s"),wxLANGUAGE_DEFAULT,wxLocale::GetSystemLanguage(),wxLocale::GetLanguageName(wxLocale::GetSystemLanguage()).BeforeFirst(' '));
	if(!cont)
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotSetup not find dir"));
	} 
	while(cont)
	{
#if __SNAP
		wxString fileMo = wxString::Format(wxS("%s/%s/LC_MESSAGES/opolyglot.mo"),dir.GetName(),filename);
#else
		wxString fileMo = wxString::Format(wxS("%s/%s/opolyglot.mo"),dir.GetName(),filename);
#endif
		if(wxFileName::FileExists(fileMo))
		{
			const wxLanguageInfo *info = wxLocale::FindLanguageInfo(filename);
			OPOLYGLOT_DEBUG(wxT("OPolyglotSetup dir %s %s %d"),filename,info->Description,info->Language);
			interfaceLangs.Add(wxString::Format(wxT("%s"),info->Description));
		}
		cont = dir.GetNext(&filename);
		OPOLYGLOT_DEBUG(wxT("OPolyglotSetup dir next %s"),dir.GetName());
	}
	interfaceLangs.Sort();
	for(size_t i = 0; i < interfaceLangs.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotSetup %ld : %s"),i,interfaceLangs.Item(i));
		SelectInterfaceLanguage->Append(interfaceLangs.Item(i));
	}
	int index;
	if( 0 == (int)config->ReadLong(OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE,OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE_DEFAULT))
	{
		index = interfaceLangs.Index(wxLocale::GetLanguageName(wxLocale::GetSystemLanguage()).BeforeFirst(' '));
	} else
	{
		index = interfaceLangs.Index(wxLocale::GetLanguageName((int)config->ReadLong(OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE,OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE_DEFAULT)).BeforeFirst(' '));
	}
	OPOLYGLOT_DEBUG(wxT("code %s %d"),interfaceLangs.Item(index),wxLocale::FindLanguageInfo(interfaceLangs.Item(index))->Language);
	SelectInterfaceLanguage->Select(index);
	additionaLanguageOCR->Clear();
	additionaLanguageOCR->Append(wxT("NONE"));
	additionaLanguageOCR->Append(OPolyglotGetInstalledLanguagesFrom());
	wxString lang = config->Read(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR_DEFAULT);
	index = additionaLanguageOCR->GetStrings().Index(lang);
	if(index != wxNOT_FOUND)
	{
		additionaLanguageOCR->Select(index);
	} else
	{
		additionaLanguageOCR->Select(0);
	}
	this->HBox0->Layout();
	this->HBox0->Fit(this);
	this->MainBox->Layout();
	this->MainBox->Fit(this);
	delete config;
}

OPolyglotSetup::~OPolyglotSetup()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotSetup"));
	if(!IS_NULLPTR(listRules))
	{
		delete listRules;
	}
	if(!IS_NULLPTR(download))
	{
		delete download;
	}
}



void OPolyglotSetup::OnClose( wxCloseEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSetup::OnClose"));
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}

void OPolyglotSetup::OnAdditionalLanguage(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSetup::OnAdditionalLanguage %s"),additionaLanguageOCR->GetStringSelection());
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	config.Write(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,additionaLanguageOCR->GetStringSelection());
}


void OPolyglotSetup::OnFinishSetupLanguage(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnFinishSetupLanguages"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSetup::OnFinishSetupLanguage,this);
	delete download;
	download = NULL;
	this->Show(true);
}

void OPolyglotSetup::OnSetupLanguages( wxCommandEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("OnSetupLanguages"));
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSetup::OnFinishSetupLanguage,this);
	download = new OPolyglotDownloadLanguage(this);
	download->Show();
	this->Show(false);
}


void OPolyglotSetup::OnChangeLogLevel( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnChangeLogLevel %s"),this->LogLevel->GetStringSelection());
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	config.Write(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,this->LogLevel->GetStringSelection());
	wxLog::SetLogLevel(OPolyglotGetLogLevel(this->LogLevel->GetStringSelection()));

}


void OPolyglotSetup::OnChangeStayOnTop( wxCommandEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("OnChangeStayOnTop %s"),OPOLYGLOT_BOOL_TO_STRING(this->StyleStayOnTop->IsChecked()));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	if(this->StyleStayOnTop->IsChecked())
	{
		config->Write(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,true);
	} else
	{
		config->Write(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,false);
	}
	delete config;
}


void OPolyglotSetup::OnSelectMethodTranslation( wxCommandEvent& event )
{
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	wxString method;
	if(this->MethodTranslation->GetStringSelection().IsSameAs(_("BEST")))
	{
		method = wxT("BEST");
	} else
	{
		method = wxT("FAST");
	}
	OPOLYGLOT_MESSAGE(wxT("OnSelectMethodTranslation(%s)"),method);
	config->Write(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD
			,method);
	delete config; 																		/* when deleting, the configuration file is recorded */
}


void OPolyglotSetup::OnSelectMethodOCR( wxCommandEvent& event )
{
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	wxString method;
	if(this->MethodOCR->GetStringSelection().IsSameAs(_("BEST")))
	{
		method = wxT("BEST");
	} else
	{
		method = wxT("FAST");
	}
	OPOLYGLOT_MESSAGE(wxT("OnSelectMethodOCR(%s)"),method);
	config->Write(OPOLYGLOT_CONFIG_STRING_OCR_METHOD
			,method);
	delete config; 																		/* when deleting, the configuration file is recorded */
}

void OPolyglotSetup::OnEnablePreprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnEnablePreprocessing %s"),OPOLYGLOT_BOOL_TO_STRING(this->EnablePreprocessing->GetValue()));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	bool val = this->EnablePreprocessing->GetValue();
	config->Write(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,val);
	this->RulesPreprocessing->Show(val);
	delete config;
	if(val)
	{
		this->HBox4->Layout();
		this->HBox4->Fit(this);
		this->MainBox->Layout();
		this->MainBox->Fit(this);
	}
}

void OPolyglotSetup::OnEnablePostprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnEnablePostprocessing %s"),OPOLYGLOT_BOOL_TO_STRING(this->EnablePostprocessing->GetValue()));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	bool val = this->EnablePostprocessing->GetValue();
	config->Write(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,val);
	this->RulesPostprocessing->Show(val);
	delete config;
	if(val)
	{
		this->HBox5->Layout();
		this->HBox5->Fit(this);
		this->MainBox->Layout();
		this->MainBox->Fit(this);
	}
}


void OPolyglotSetup::OnRulesPreprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnRulesPreprocessing "));
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSetup::OnRulesPreprocessingFinish,this);
	listRules = new OPolyglotListProcessingRules(this,wxS("RulesPreProcessing"));
	this->Show(false);
}

void OPolyglotSetup::OnRulesPreprocessingFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnRulesPostprocessingFinish"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSetup::OnRulesPreprocessingFinish,this);
	delete listRules;
	listRules = NULL;
	this->Show(true);
}

void OPolyglotSetup::OnRulesPostprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnRulesPostprocessing "));
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSetup::OnRulesPostprocessingFinish,this);
	listRules = new OPolyglotListProcessingRules(this,wxS("RulesPostProcessing"));
	this->Show(false);
}

void OPolyglotSetup::OnRulesPostprocessingFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnRulesPostprocessingFinish"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSetup::OnRulesPostprocessingFinish,this);
	delete listRules;
	listRules = NULL;
	this->Show(true);
}

void OPolyglotSetup::OnViewLog(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnViewLog"));
	view = new OPolyglotViewLog(this);

}


void OPolyglotSetup::OnSelectInterfaceLanguage( wxCommandEvent& event ) 
{
#if 0
	OPOLYGLOT_DEBUG(wxT("code %s %d"),interfaceLangs.Item(index),wxLocale::FindLanguageInfo(interfaceLangs.Item(index))->Language);
#endif
	OPOLYGLOT_MESSAGE(wxT("OnSelectInterfaceLanguage(%s)"),SelectInterfaceLanguage->GetStringSelection());
#if 0
	int index = interfaceLangs.Index(SelectInterfaceLanguage->GetStringSelection());
#endif
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	if((int)config->ReadLong(OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE,OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE_DEFAULT)
			!= wxLocale::FindLanguageInfo(SelectInterfaceLanguage->GetStringSelection())->Language)
	{
		config->Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE,wxLocale::FindLanguageInfo(SelectInterfaceLanguage->GetStringSelection())->Language);
		wxMessageDialog msg(
				this
				,_("To apply the interface language changes, you must restart OPolyglot.")
				,wxT("OPolyglot"),wxOK);
		msg.ShowModal();
	}
	delete config;

	
}
