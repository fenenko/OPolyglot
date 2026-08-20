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


#include "OPolyglotSettings.h"
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotEvent.h"
#include "OPolyglotDialogError.h"
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
	this->SetTitle(wxString::Format(wxT("OPolyglot %s"),_("log view")));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
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
	if(!file.Open(OPOLYGLOT_LOG_FILENAME))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotViewLog not find %s check option OPOLYGLOT_DEBUG_ENABLED == 0"),OPOLYGLOT_LOG_FILENAME);
		OPolyglotDialogError msg(this,wxString::Format(wxT("Will not find a file \"%s\""),OPOLYGLOT_LOG_FILENAME));
		this->Destroy();
		return;
	}
	for(wxString str = file.GetFirstLine();!file.Eof();str = file.GetNextLine())
	{
		int start = Log->GetTextLength();
		str = wxString::Format(wxS("%s\n"),str);
		if(str.Contains(wxS("Error:"))
				||(str.Contains(wxString::Format(wxT("%s:"),_("Error")))))
		{
			Log->AppendText(str);
			int end = Log->GetTextLength();
			Log->StartStyling(start);
			Log->SetStyling(end-start,STYLE_ERROR);
		} else
		{
			if(str.Contains(wxS("Warning:"))
					||(str.Contains(wxString::Format(wxT("%s:"),_("Warning")))))
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

OPolyglotSettings::OPolyglotSettings(wxEvtHandler *parent) : GUIOPolyglotSettings(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings"));
	this->SetTitle(wxString::Format(wxT("OPolyglot %s"),_("settings")));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxPoint position;
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	handler = parent;
	this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	this->SetPosition(wxPoint((geom.width-this->GetSize().GetWidth())/2,(geom.height -this->GetSize().GetHeight())/2));
	this->StyleStayOnTop->SetValue(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT));
	wxString logLevel = config->Read(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT);
	if(logLevel.IsSameAs(wxT("ERROR")))
	{
		this->LogLevel->SetStringSelection(_("ERROR"));
	} else
	{
		if(logLevel.IsSameAs(wxT("WARNING")))
		{
			this->LogLevel->SetStringSelection(_("WARNING"));
		} else
		{
			this->LogLevel->SetStringSelection(_("MESSAGE"));
		}
	}
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
	this->sauvolaEnabled->SetValue(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_SAUVOLA,OPOLYGLOT_CONFIG_BOOL_ENABLED_SAUVOLA_DEFAULT));
	this->sauvolaMindiff->Show(sauvolaEnabled->GetValue());
	this->sauvolaMindiff->SetValue(static_cast<int>(config->ReadLong(OPOLYGLOT_CONFIG_INT_SAUVOLA_MINDIFF,OPOLYGLOT_CONFIG_INT_SAUVOLA_MINDIFF_DEFAULT)));
	this->EnablePreprocessing->SetValue(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT));
	this->RulesPreprocessing->Show(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT));
	this->EnablePostprocessing->SetValue(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT));
	this->RulesPostprocessing->Show(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT));
	wxDir dir(OPOLYGLOT_LOCALE_DIR);
	wxString filename;
	if(!dir.IsOpened())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotSettings locale catalog not found %s"),OPOLYGLOT_LOCALE_DIR);
		OPolyglotDialogError msg(this,wxString::Format(wxT("%s %s"),_("Locale catalog not found"),OPOLYGLOT_LOCALE_DIR));
		this->Destroy();
		return;
	} else
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotSettings dir %s"),dir.GetName());
	}
	bool cont = dir.GetFirst(&filename,wxEmptyString,wxDIR_NO_FOLLOW|wxDIR_DIRS);
	OPOLYGLOT_DEBUG(wxT("OPolyglotSettings System language %d %d %s"),wxLANGUAGE_DEFAULT,wxLocale::GetSystemLanguage(),wxLocale::GetLanguageName(wxLocale::GetSystemLanguage()).BeforeFirst(' '));
	if(!cont)
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotSettings not find dir"));
	} 
	wxArrayString interfaceLangs;
	while(cont)
	{
#if __SNAP
		wxString fileMo = wxString::Format(wxS("%s/%s/LC_MESSAGES/opolyglot.mo"),dir.GetName(),filename);
#else
		wxString fileMo = wxString::Format(wxS("%s%c%s%copolyglot.mo"),dir.GetName(),wxFileName::GetPathSeparator(),filename,wxFileName::GetPathSeparator());
#endif
		if(wxFileName::FileExists(fileMo))
		{
			const wxLanguageInfo *info = wxLocale::FindLanguageInfo(filename);
			OPOLYGLOT_DEBUG(wxT("OPolyglotSettings dir %s %s %d"),filename,info->Description,info->Language);
			interfaceLangs.Add(wxString::Format(wxT("%s"),info->Description));
		} else
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotSettings localization file not found %s"),filename);
		}
		cont = dir.GetNext(&filename);
		OPOLYGLOT_DEBUG(wxT("OPolyglotSettings dir next %s"),dir.GetName());
	}
	interfaceLangs.Sort();
	SelectInterfaceLanguage->Append(OPolyglotGetTranslatedLanguages(interfaceLangs));
	OPOLYGLOT_DEBUG(wxT("OPolyglotSettings count interface languages %zu %zu"),SelectInterfaceLanguage->GetStrings().GetCount(),interfaceLangs.GetCount());
	int index;
	if( 0 == (int)config->ReadLong(OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE,OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE_DEFAULT))
	{
		index = SelectInterfaceLanguage->GetStrings().Index(OPolyglotGetTranslateLanguage(wxLocale::GetLanguageName(wxLocale::GetSystemLanguage()).BeforeFirst(' ')));
	} else
	{
		index = SelectInterfaceLanguage->GetStrings().Index(OPolyglotGetTranslateLanguage(wxLocale::GetLanguageName((int)config->ReadLong(OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE,OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE_DEFAULT)).BeforeFirst(' ')));
	}
	xmlLanguages.SetRoot(new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Languages")));
	SelectInterfaceLanguage->Select(index);
	additionalLanguageOCR->Clear();
	additionalLanguageOCR->Append(_("NONE"));
	OPOLYGLOT_DEBUG(wxT("OPolyglotSettings select OCR Language"));
	wxArrayString listInstalled = OPolyglotGetTranslatedLanguages(OPolyglotGetInstalledLanguagesFrom());
	listInstalled.Sort(CompareLocaleNoCase);
	additionalLanguageOCR->Append(listInstalled);
	OPOLYGLOT_DEBUG(wxT("OPolyglotSettings select OCR finish %zu"),additionalLanguageOCR->GetStrings().GetCount());
	wxString lang = config->Read(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR_DEFAULT);
	index = additionalLanguageOCR->GetStrings().Index(OPolyglotGetTranslateLanguage(lang));
	if(index != wxNOT_FOUND)
	{
		additionalLanguageOCR->Select(index);
	} else
	{
		additionalLanguageOCR->Select(0);
	}
	this->HBox0->Layout();
	HBoxSauvola->Layout();
	delete config;
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH,&OPolyglotSettings::OnDownloadFinish,this);
	ScanLangs();
}

OPolyglotSettings::~OPolyglotSettings()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotSettings"));
	if(!IS_NULLPTR(listRules))
	{
		delete listRules;
	}
	if(!IS_NULLPTR(download))
	{
		download->Destroy();
		download = nullptr;
	}
}



void OPolyglotSettings::OnSauvolaEnabled( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings::OnSauvolaEnabled %s"),OPOLYGLOT_BOOL_TO_STRING(sauvolaEnabled->GetValue()));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	config->Write(OPOLYGLOT_CONFIG_BOOL_ENABLED_SAUVOLA,sauvolaEnabled->GetValue());
	sauvolaMindiff->Show(sauvolaEnabled->GetValue());
	//sauvolaWhsize->Show(sauvolaEnabled->GetValue());
	//sauvolaFactor->Show(sauvolaEnabled->GetValue());
	if(sauvolaEnabled->GetValue())
	{
		HBoxSauvola->Layout();
	}
	delete config;
}


void OPolyglotSettings::OnSauvolaMindiff( wxSpinEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings::OnSauvolaMindiff %d"),sauvolaMindiff->GetValue());
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	config->Write(OPOLYGLOT_CONFIG_INT_SAUVOLA_MINDIFF,sauvolaMindiff->GetValue());
	delete config;
	
}

void OPolyglotSettings::OnClose( wxCloseEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings::OnClose"));
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}

void OPolyglotSettings::OnAdditionalLanguage(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings::OnAdditionalLanguage %s"),OPolyglotGetOriginalLanguage(additionalLanguageOCR->GetStringSelection()));
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	if(!additionalLanguageOCR->GetStringSelection().IsSameAs(_("NONE")))
	{
		config.Write(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,OPolyglotGetOriginalLanguage(additionalLanguageOCR->GetStringSelection()));
	} else
	{
		config.Write(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,wxT("NONE"));
	}
}


void OPolyglotSettings::OnChangeLogLevel( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnChangeLogLevel %s"),this->LogLevel->GetStringSelection());
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	if(LogLevel->GetStringSelection().IsSameAs(_("ERROR")))
	{
		config.Write(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,wxT("ERROR"));
		wxLog::SetLogLevel(OPolyglotGetLogLevel(wxT("ERROR")));
	} else
	{
		if(LogLevel->GetStringSelection().IsSameAs(_("WARNING")))
		{
			config.Write(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,wxT("WARNING"));
			wxLog::SetLogLevel(OPolyglotGetLogLevel(wxT("WARNING")));
		} else
		{
			config.Write(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,wxT("MESSAGE"));
			wxLog::SetLogLevel(OPolyglotGetLogLevel(wxT("MESSAGE")));
		}
	}

}


void OPolyglotSettings::OnChangeStayOnTop( wxCommandEvent& event ) 
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


void OPolyglotSettings::OnSelectMethodTranslation( wxCommandEvent& event )
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


void OPolyglotSettings::OnSelectMethodOCR( wxCommandEvent& event )
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

void OPolyglotSettings::OnEnablePreprocessing( wxCommandEvent& event )
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
	}
}

void OPolyglotSettings::OnEnablePostprocessing( wxCommandEvent& event )
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
	}
}


void OPolyglotSettings::OnRulesPreprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnRulesPreprocessing "));
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSettings::OnRulesPreprocessingFinish,this);
	listRules = new OPolyglotListProcessingRules(this,wxS("RulesPreProcessing"));
	this->Show(false);
}

void OPolyglotSettings::OnRulesPreprocessingFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnRulesPostprocessingFinish"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSettings::OnRulesPreprocessingFinish,this);
	delete listRules;
	listRules = NULL;
	this->Show(true);
}

void OPolyglotSettings::OnRulesPostprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnRulesPostprocessing "));
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSettings::OnRulesPostprocessingFinish,this);
	listRules = new OPolyglotListProcessingRules(this,wxS("RulesPostProcessing"));
	this->Show(false);
}

void OPolyglotSettings::OnRulesPostprocessingFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnRulesPostprocessingFinish"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSettings::OnRulesPostprocessingFinish,this);
	delete listRules;
	listRules = NULL;
	this->Show(true);
}

void OPolyglotSettings::OnViewLog(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnViewLog"));
	view = new OPolyglotViewLog(this);

}


void OPolyglotSettings::OnSelectInterfaceLanguage( wxCommandEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("OnSelectInterfaceLanguage(%s)"),OPolyglotGetOriginalLanguage(SelectInterfaceLanguage->GetStringSelection()));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	if((int)config->ReadLong(OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE,OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE_DEFAULT)
			!= wxLocale::FindLanguageInfo(OPolyglotGetOriginalLanguage(SelectInterfaceLanguage->GetStringSelection()))->Language)
	{
		config->Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE,wxLocale::FindLanguageInfo(OPolyglotGetOriginalLanguage(SelectInterfaceLanguage->GetStringSelection()))->Language);
		wxMessageDialog msg(
				this
				,_("To apply the interface language changes, you must restart OPolyglot.")
				,wxT("OPolyglot"),wxOK);
		msg.ShowModal();

	}
	delete config;
}

void OPolyglotSettings::ScanLangs()
{
	int scrollX,scrollY;
	wxString messageError;
	wxArrayString labelLanguages;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings::ScanLangs"));
	if(!OPolyglotInstallLanguages::CreateXmlLanguages(messageError,labelLanguages,xmlLanguages))
	{
		OPolyglotDialogError msg(this,messageError);
		return;
	}
	ListLanguages->Freeze();
	ListLanguages->GetViewStart(&scrollX,&scrollY);
	ListLanguages->Scroll(0,0);
	boxLanguages->Clear(true);
	bool flagShowDownloadAll = false;
	bool flagShowRemoveAll = false;
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *label = new wxStaticText(ListLanguages,wxID_ANY,_("Download languages for offline translation"),wxDefaultPosition,wxDefaultSize,0);
	sizer->Add(label,0,wxALL|wxEXPAND,2);
	sizer->Add(0,0,1,wxEXPAND,2);
	wxButton *buttonDownloadAll = new wxButton(ListLanguages,wxID_ANY,_("Download All"),wxDefaultPosition,wxDefaultSize,0);
	buttonDownloadAll->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotSettings::OnLanguagesDownloadAll,this,buttonDownloadAll->GetId(),buttonDownloadAll->GetId());
	sizer->Add(buttonDownloadAll,0,wxALL,2);
	wxButton  *buttonRemoveAll = new wxButton(ListLanguages,wxID_ANY,_("Remove All"),wxDefaultPosition,wxDefaultSize,0);
	buttonRemoveAll->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotSettings::OnLanguagesRemoveAll,this,buttonRemoveAll->GetId(),buttonRemoveAll->GetId());
	sizer->Add(buttonRemoveAll,0,wxALL,2);
	boxLanguages->Add(sizer,0,wxALL|wxEXPAND,3);
	wxStaticLine *line = new wxStaticLine( ListLanguages, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	boxLanguages->Add( line, 0, wxEXPAND | wxALL, 0 );
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
					button->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotSettings::OnLanguageRemove,this,button->GetId(),button->GetId());
					sizer->Add(button,0,wxALL,2);
					if(!flagShowRemoveAll)
					{
						flagShowRemoveAll = true;
					}
				} else
				{
					wxButton *button = new wxButton(ListLanguages,wxID_ANY,_("Download"),wxDefaultPosition,wxDefaultSize,0);
					childLang->AddAttribute(wxS("idButton"),wxString::Format(wxT("%d"),button->GetId()));
					button->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotSettings::OnLanguageDownload,this,button->GetId(),button->GetId());
					sizer->Add(button,0,wxALL,2);
					if(!flagShowDownloadAll)
					{
						flagShowDownloadAll = true;
					}
				}
				sizer->Layout();
				boxLanguages->Add(sizer,0,wxALL|wxEXPAND,3);

			}
		}
	}
	if(!flagShowDownloadAll)
	{
		boxLanguages->GetItem((size_t)0)->GetSizer()->Hide(buttonDownloadAll);
		boxLanguages->GetItem((size_t)0)->GetSizer()->Layout();
	}
	if(!flagShowRemoveAll)
	{
		boxLanguages->GetItem((size_t)0)->GetSizer()->Hide(buttonRemoveAll);
		boxLanguages->GetItem((size_t)0)->GetSizer()->Layout();
	}
	boxLanguages->Layout();
	ListLanguages->Thaw();
	ListLanguages->Scroll(scrollX,scrollY);
}


void OPolyglotSettings::OnLanguagesDownloadAll(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings::OnLanguagesDownloadAll"));
	download = new OPolyglotInstallLanguages(this,xmlLanguages,OPOLYGLOT_ID_ALL);
	this->Show(false);
}
void OPolyglotSettings::OnLanguagesRemoveAll(wxCommandEvent& event)
{
	wxString messageError;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings::OnLanguagesRemoveAll"));
	if(!OPolyglotInstallLanguages::RemoveLanguage(messageError,xmlLanguages,OPOLYGLOT_ID_ALL))
	{
		OPolyglotDialogError msg(this,messageError);
	}
	ScanLangs();
}

void OPolyglotSettings::OnLanguageDownload(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings::OnLanguageDownload"));
	download = new OPolyglotInstallLanguages(this,xmlLanguages,event.GetId());
	this->Show(false);
}

void OPolyglotSettings::OnLanguageRemove(wxCommandEvent& event)
{
	wxString messageError;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings::OnLanguageRemove %d"),event.GetId());
	if(!OPolyglotInstallLanguages::RemoveLanguage(messageError,xmlLanguages,event.GetId()))
	{
		OPolyglotDialogError msg(this,messageError);
	}
	ScanLangs();
}

void OPolyglotSettings::OnDownloadFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotSettings::OnDownloadFinish"));
	download->Destroy();
	download = nullptr;
	ScanLangs();
	Show(true);
}
