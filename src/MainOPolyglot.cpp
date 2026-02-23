#include "MainOPolyglot.h"
#include "OPolyglotEvent.h"
#include "OPolyglotAbout.h"
#include "Utils.h"
#include "Config.h"
#include "Version.h"
#include <iostream>
#include <wx/dcscreen.h>
#include <wx/graphics.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/config.h>
#include <wx/translation.h>
#include <wx/uilocale.h>
#include "GuiOPolyglot.h"

wxIMPLEMENT_APP(MainOPolyglot);

bool MainOPolyglot::OnInit()
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	wxLog::SetLogLevel(OPolyglotGetLogLevel(config.Read(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT)));
	wxLog* logger = new wxLogStream(&(std::cout));
	wxLog::SetActiveTarget(logger);
	#line __LINE__	"src/MainOPolyglot.cpp|MainOPolyglot::OnInit"
	OPOLYGLOT_DEBUG(wxT("test log level DEBUG"));
	OPOLYGLOT_INFO(wxT("test log level INFO"));	
	OPOLYGLOT_MESSAGE(wxT("test log level MESSAGE"));
	OPOLYGLOT_WARNING(wxT("test log level WARNING"));
	OPOLYGLOT_ERROR(wxT("test log level ERROR"));
	OPOLYGLOT_ERROR(wxT("OPolyglot version git commit hash %s"),GIT_COMMIT_HASH); /* these messages such as error so that the software version is always displayed in the logs */
	OPOLYGLOT_MESSAGE(wxT("config dir %s"),OPOLYGLOT_USER_DIR);
	OPOLYGLOT_MESSAGE(wxT("download xml %s"),wxGetenv("DOWNLOAD_XML"));
	wxFileTranslationsLoader::AddCatalogLookupPathPrefix(wxS("."));
	wxTranslations* const trans = new wxTranslations();
    wxTranslations::Set(trans);
	if(!trans->AddCatalog("opolyglot"))
	{
		OPOLYGLOT_ERROR(wxT("language %s"),wxUILocale::GetLanguageName(wxLANGUAGE_DEFAULT));
	}
	if(!wxFileName::DirExists(OPOLYGLOT_USER_DIR))
	{
		OPOLYGLOT_WARNING(wxT("path %s is absent"),OPOLYGLOT_USER_DIR);
		if(wxDir::Make(OPOLYGLOT_USER_DIR))
		{
			OPOLYGLOT_MESSAGE(wxT("creating directory %s"),OPOLYGLOT_USER_DIR);
		} else
		{
			OPOLYGLOT_ERROR(wxT("creating dir %s"),OPOLYGLOT_USER_DIR);
		}
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("dir %s is exists"),OPOLYGLOT_USER_DIR);
	}
	if(!wxFileName::DirExists(OPOLYGLOT_USER_DATA))
	{
		OPOLYGLOT_WARNING(wxT("path %s is absent"),OPOLYGLOT_USER_DATA);
		if(wxDir::Make(OPOLYGLOT_USER_DATA))
		{
			OPOLYGLOT_MESSAGE(wxT("creating directory %s"),OPOLYGLOT_USER_DATA);
		} else
		{
			OPOLYGLOT_ERROR(wxT("creating dir %s"),OPOLYGLOT_USER_DATA);
		}
	}
	if(!wxFileName::FileExists(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_WARNING(wxT("not find file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		if(!wxCopyFile(OPOLYGLOT_GET_RES_XML_DATA_FILE,OPOLYGLOT_GET_XML_DATA_FILE))
		{
			OPOLYGLOT_ERROR(wxT("error copy file %s -> %s"),OPOLYGLOT_GET_RES_XML_DATA_FILE,OPOLYGLOT_GET_XML_DATA_FILE);
			return false;
		}

	}
	OPOLYGLOT_DEBUG(wxT("OPEN OPolyglot"));	
	
	taskBar= new OPolyglotTaskBar(this,_("Hide"));
	frame = new OPolyglot(this);
	SetTopWindow(frame);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&MainOPolyglot::OnSetup,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_ABOUT,&MainOPolyglot::OnAbout,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&MainOPolyglot::OnExitProgramm,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW,&MainOPolyglot::OnShow,this);
	return true;
}

MainOPolyglot::~MainOPolyglot()
{
	
	#line __LINE__ "src/MainOPolyglot.cpp|MainOPolyglot::~MainOPolyglot"
	OPOLYGLOT_MESSAGE();
	//delete frame;
}


void MainOPolyglot::OnSetup(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE();
	frameSetup = new OPolyglotSetup(this);
	frameSetup->Show();
	frame->SetShow(false);
	taskBar->SetLabel(wxEmptyString);
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&MainOPolyglot::OnSetup,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&MainOPolyglot::OnSetupFinish,this);
}

void MainOPolyglot::OnAbout(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot::OnAbout"));
	About *about = new  About(NULL);
	about->Show();
}

void MainOPolyglot::OnSetupFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE();
	delete frameSetup;
	frameSetup = NULL;
	frame->ScanLangs();
	frame->SetShow(true);
	taskBar->SetLabel(_("Hide"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&MainOPolyglot::OnSetupFinish,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&MainOPolyglot::OnSetup,this);
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT))
	{
		frame->SetWindowStyle(frame->GetWindowStyle()|wxSTAY_ON_TOP);
	} else
	{
		frame->SetWindowStyle(frame->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	}

}

void MainOPolyglot::OnShow(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("%p"),frameSetup);
	if(frame->IsShown())
	{
		frame->SetShow(false);
		taskBar->SetLabel(_("Show"));
	} else
	{
		frame->SetShow(true);
		taskBar->SetLabel(_("Hide"));
	}
}

void MainOPolyglot::OnHide(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("%p"),frameSetup);
}

void MainOPolyglot::OnExitProgramm(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE();
	if(!IS_NULLPTR(frameSetup))
	{
		frameSetup->Destroy();
	}
	delete frame;
	delete taskBar;


}
