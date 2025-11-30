#include "MainOPolyglot.h"
#include "OPolyglotEvent.h"
#include "Utils.h"
#include "Config.h"
#include "Version.h"
#include <iostream>
#include <wx/dcscreen.h>
#include <wx/graphics.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/config.h>
wxIMPLEMENT_APP(MainOPolyglot);

bool MainOPolyglot::OnInit()
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	wxLog::SetLogLevel(OPolyglotGetLogLevel(config.Read(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT)));
	wxLog* logger = new wxLogStream(&(std::cout));
	wxLog::SetActiveTarget(logger);
	OPOLYGLOT_DEBUG(wxT("test log level DEBUG"));
	OPOLYGLOT_INFO(wxT("test log level INFO"));	
	OPOLYGLOT_MESSAGE(wxT("test log level MESSAGE"));
	OPOLYGLOT_WARNING(wxT("test log level WARNING"));
	OPOLYGLOT_ERROR(wxT("test log level ERROR"));
	OPOLYGLOT_ERROR(wxT("OPolyglot version git commit hash %s"),GIT_COMMIT_HASH); /* these messages such as error so that the software version is always displayed in the logs */
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
	
	frame = new OPolyglot(NULL);
	taskBar= new OPolyglotTaskBar(this,true);
	frame->Show(true);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&MainOPolyglot::OnSetup,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT_PROGRAMM,&MainOPolyglot::OnExitProgramm,this);
	return true;
}

MainOPolyglot::~MainOPolyglot()
{
	OPOLYGLOT_MESSAGE();
	//delete frame;
}


void MainOPolyglot::OnSetup(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE();
	frameSetup = new OPolyglotSetup(this);
	frameSetup->Show();
	frame->Show(false);
}

void MainOPolyglot::OnSetupFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE();
	delete frameSetup;
	frame->Show(true);
	OPOLYGLOT_DEBUG(wxT("%p"),frameSetup);
}

void MainOPolyglot::OnExitProgramm(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE();
	delete frame;
	delete taskBar;


}
