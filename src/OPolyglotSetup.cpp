#include "OPolyglotSetup.h"
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotEvent.h"
#include <wx/display.h>
#include "../res/icon.xpm"
#include "Utils.h"
#include <wx/config.h>
#include "Config.h"
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/msgdlg.h>



OPolyglotViewLog::OPolyglotViewLog(wxFrame *parent) : GUIViewLog(parent)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewLog"));
	SetIcon(wxICON(icon));
	wxTextFile file;
	file.Open(OPOLYGLOT_LOG_FILENAME);
	Log->Clear();
	for(wxString str = file.GetFirstLine();!file.Eof();str = file.GetNextLine())
	{
		Log->AppendText(wxString::Format(wxT("%s\n"),str));
	}
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
	SetIcon(wxICON(icon));
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
	if(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_METHOD_CREATION_TEXT_NEW,OPOLYGLOT_CONFIG_BOOL_METHOD_CREATION_TEXT_DEFAULT))
	{
		this->ModeCreationText->SetSelection(0);
	} else
	{
		this->ModeCreationText->SetSelection(1);
	}
	this->EnablePreprocessing->SetValue(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT));
	this->RulesPreprocessing->Show(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT));
	this->EnablePostprocessing->SetValue(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT));
	this->RulesPostprocessing->Show(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT));
	/* */
	wxDir dir(OPOLYGLOT_LOCALE_DIR);
	wxString filename;
	bool cont = dir.GetFirst(&filename);
	OPOLYGLOT_DEBUG(wxT("OPolyglotSetup System language %d %d %s"),wxLANGUAGE_DEFAULT,wxLocale::GetSystemLanguage(),wxLocale::GetLanguageName(wxLocale::GetSystemLanguage()).BeforeFirst(' '));
	if(!cont)
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotSetup not find dir"));
	} 
	while(cont)
	{
		const wxLanguageInfo *info = wxLocale::FindLanguageInfo(filename);
		OPOLYGLOT_DEBUG(wxT("OPolyglotSetup dir %s %s %d"),filename,info->Description,info->Language);
		interfaceLangs.Add(wxString::Format(wxT("%s"),info->Description));
		cont = dir.GetNext(&filename);
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
	//this->LabelInterface->Hide();
	//this->SelectInterfaceLanguage->Hide();
	this->LabelPostprocessing->Hide();
	this->RulesPostprocessing->Hide();
	this->EnablePostprocessing->Hide();
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


void OPolyglotSetup::OnModeCreationText( wxCommandEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("OnModeCreationText %d"),this->ModeCreationText->GetSelection());
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	if(this->ModeCreationText->GetSelection() == 0)
	{
		config->Write(OPOLYGLOT_CONFIG_BOOL_METHOD_CREATION_TEXT_NEW,true);
	} else
	{

		config->Write(OPOLYGLOT_CONFIG_BOOL_METHOD_CREATION_TEXT_NEW,false);
	}
	delete config;
}

void OPolyglotSetup::OnClose( wxCloseEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnClose"));
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
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
	this->HBox4->Layout();
}

void OPolyglotSetup::OnEnablePostprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnEnablePostprocessing %s"),OPOLYGLOT_BOOL_TO_STRING(this->EnablePostprocessing->GetValue()));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	bool val = this->EnablePostprocessing->GetValue();
	config->Write(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,val);
	this->RulesPostprocessing->Show(val);
	delete config;
	this->HBox5->Layout();
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
	int index = interfaceLangs.Index(SelectInterfaceLanguage->GetStringSelection());
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
