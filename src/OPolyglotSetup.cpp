#include "OPolyglotSetup.h"
#include "OPolyglotDownloadLanguage.h"
#include <wx/display.h>
#include "../res/icon.xpm"
#include "Utils.h"
#include <wx/config.h>
#include "Config.h"

OPolyglotSetup::OPolyglotSetup(OPolyglot *parent) : GUIOPolyglotSetup(parent)
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxPoint position;
	OPOLYGLOT_MESSAGE();
	SetIcon(wxICON(icon));
	this->SetPosition(wxPoint((geom.width-this->GetSize().GetWidth())/2,(geom.height -this->GetSize().GetHeight())/2));
	this->parent =parent;
	this->StyleStayOnTop->SetValue(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT));
	this->LogLevel->SetStringSelection(config.Read(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT));
	this->MethodTranslation->SetStringSelection(config.Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT));

}

OPolyglotSetup::~OPolyglotSetup()
{
	OPOLYGLOT_MESSAGE();
}


void OPolyglotSetup::OnSetupLanguages( wxCommandEvent& event ) 
{
	OPOLYGLOT_MESSAGE();
	OPolyglotDownloadLanguage *download = new OPolyglotDownloadLanguage(this->parent);
	download->Show();
	this->Destroy();
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
	OPOLYGLOT_MESSAGE(wxT("%s"),OPOLYGLOT_BOOL_TO_STRING(this->StyleStayOnTop->IsChecked()));
	long style = this->parent->GetWindowStyle();
	if(this->StyleStayOnTop->IsChecked())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		config.Write(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,true);
		style = style | wxSTAY_ON_TOP;

	} else
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		config.Write(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,false);
		style = style ^ wxSTAY_ON_TOP;
	}
	this->parent->SetWindowStyle(style);
	this->parent->Layout();
	this->parent->Refresh();
	this->parent->Update();
}


void OPolyglotSetup::OnSelectMethodTranslation( wxCommandEvent& event )
{
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("%s"),this->MethodTranslation->GetStringSelection());
	config->Write(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD
			,this->MethodTranslation->GetStringSelection());
	delete config; 																		/* when deleting, the configuration file is recorded */
	this->parent->CreateTranslatorConfig(); 											/* re-create the translator configuration for the selected method  */ 
	
}
