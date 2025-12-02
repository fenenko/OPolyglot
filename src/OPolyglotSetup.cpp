#include "OPolyglotSetup.h"
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotEvent.h"
#include <wx/display.h>
#include "../res/icon.xpm"
#include "Utils.h"
#include <wx/config.h>
#include "Config.h"

OPolyglotSetup::OPolyglotSetup(wxEvtHandler *parent) : GUIOPolyglotSetup(NULL)
{
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxPoint position;
	OPOLYGLOT_MESSAGE();
	SetIcon(wxICON(icon));
	handler = parent;
	this->ButtonSetupLanguages->SetToolTip(_("installation or removal of translator languages."));
	this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	this->SetPosition(wxPoint((geom.width-this->GetSize().GetWidth())/2,(geom.height -this->GetSize().GetHeight())/2));
	this->StyleStayOnTop->SetValue(config->ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT));
	this->LogLevel->SetStringSelection(config->Read(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT));
	this->MethodTranslation->SetStringSelection(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT));
	this->MethodOCR->SetStringSelection(config->Read(OPOLYGLOT_CONFIG_STRING_OCR_METHOD,OPOLYGLOT_CONFIG_STRING_OCR_METHOD_DEFAULT));
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
	this->MainBox->Layout();
	this->MainBox->Fit(this);
	delete config;
}

OPolyglotSetup::~OPolyglotSetup()
{
	OPOLYGLOT_MESSAGE();
	if(!IS_NULLPTR(listRules))
	{
		delete listRules;
	}
	if(!IS_NULLPTR(download))
	{
		delete download;
	}
}

void OPolyglotSetup::OnFinishSetupLanguage(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE();
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSetup::OnFinishSetupLanguage,this);
	delete download;
	download = NULL;
	this->Show(true);
}

void OPolyglotSetup::OnModeCreationText( wxCommandEvent& event ) 
{
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("%d"),this->ModeCreationText->GetSelection());
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
	OPOLYGLOT_MESSAGE();
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
	this->Destroy();
}

void OPolyglotSetup::OnSetupLanguages( wxCommandEvent& event ) 
{
	OPOLYGLOT_MESSAGE();
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSetup::OnFinishSetupLanguage,this);
	download = new OPolyglotDownloadLanguage(this);
	download->Show();
	this->Show(false);
}


void OPolyglotSetup::OnChangeLogLevel( wxCommandEvent& event )
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("%s"),this->LogLevel->GetStringSelection());
	config.Write(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,this->LogLevel->GetStringSelection());
	wxLog::SetLogLevel(OPolyglotGetLogLevel(this->LogLevel->GetStringSelection()));

}


void OPolyglotSetup::OnChangeStayOnTop( wxCommandEvent& event ) 
{
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("%s"),OPOLYGLOT_BOOL_TO_STRING(this->StyleStayOnTop->IsChecked()));
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
	OPOLYGLOT_MESSAGE(wxT("%s"),this->MethodTranslation->GetStringSelection());
	config->Write(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD
			,this->MethodTranslation->GetStringSelection());
	delete config; 																		/* when deleting, the configuration file is recorded */
}


void OPolyglotSetup::OnSelectMethodOCR( wxCommandEvent& event )
{
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("%s"),this->MethodOCR->GetStringSelection());
	config->Write(OPOLYGLOT_CONFIG_STRING_OCR_METHOD
			,this->MethodOCR->GetStringSelection());
	delete config; 																		/* when deleting, the configuration file is recorded */
}

void OPolyglotSetup::OnEnablePreprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("%s"),OPOLYGLOT_BOOL_TO_STRING(this->EnablePreprocessing->GetValue()));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	bool val = this->EnablePreprocessing->GetValue();
	config->Write(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,val);
	this->RulesPreprocessing->Show(val);
	delete config;
	this->HBox4->Layout();
}

void OPolyglotSetup::OnEnablePostprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("%s"),OPOLYGLOT_BOOL_TO_STRING(this->EnablePostprocessing->GetValue()));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	bool val = this->EnablePostprocessing->GetValue();
	config->Write(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,val);
	this->RulesPostprocessing->Show(val);
	delete config;
	this->HBox5->Layout();
}


void OPolyglotSetup::OnRulesPreprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE();
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSetup::OnRulesPreprocessingFinish,this);
	listRules = new OPolyglotListProcessingRules(this,wxS("RulesPreProcessing"));
	this->Show(false);
}

void OPolyglotSetup::OnRulesPreprocessingFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE();
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotSetup::OnRulesPreprocessingFinish,this);
	delete listRules;
	listRules = NULL;
	this->Show(true);
}

void OPolyglotSetup::OnRulesPostprocessing( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE();
}
