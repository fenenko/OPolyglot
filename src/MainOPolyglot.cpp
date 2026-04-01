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


#include "MainOPolyglot.h"
#include "OPolyglotEvent.h"
#include "OPolyglotAbout.h"
#include "Utils.h"
#include "Config.h"
#include "Version.h"
#include "OPolyglotVersion.h"
#include <iostream>
#include <wx/dcscreen.h>
#include <wx/graphics.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/config.h>
#include <wx/translation.h>
#include <wx/uilocale.h>
#include <wx/ffile.h>
#include "GuiOPolyglot.h"
#include <iostream>
#include <fstream>
#include "../res/icons_clear.xpm"
#include "../res/icon_rechange.xpm"

wxDynamicLibrary* MainOPolyglot::libOPolyglot = nullptr; 
wxMutex MainOPolyglot::mutexOCR;
wxMutex MainOPolyglot::mutexTranslate;

class OPolyglotArtProvider : public wxArtProvider
{
	protected:
		virtual wxBitmap CreateBitmap(const wxArtID& id,const wxArtClient& client,const wxSize& size) wxOVERRIDE;
};

wxBitmap OPolyglotArtProvider::CreateBitmap(const wxArtID& id,const wxArtClient& client,const wxSize& size)
{
	OPOLYGLOT_DEBUG(wxT("OPolyglotArtProvide::CreateBitmap %dx%d"),size.GetWidth(),size.GetHeight());
	if(client == wxART_BUTTON)
	{
		if(id == OPOLYGLOT_ART_CLEAR)
		{
			OPOLYGLOT_DEBUG(wxT("OPolyglotArtProvide::CreateBitmap icon_clear"));
			if(size.GetWidth() == 16)
			{
				return wxBitmap(icon_clear_16_xpm);
			}
			wxBitmap bitmap(icon_clear_64_xpm);
			wxImage img = bitmap.ConvertToImage();
			img.Rescale(size.GetWidth(),size.GetHeight(),wxIMAGE_QUALITY_HIGH );
			return wxBitmap(img);
		}
		if(id == OPOLYGLOT_ART_RECHANGE)
		{
			if((size.GetWidth() != 16)||(size.GetHeight() != 16))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotArtProvide::CreateBitmap size %dx%d != 16x16"),size.GetWidth(),size.GetHeight());
			}
			return wxBitmap(icon_rechange_xpm);
		}
	}
	return wxNullBitmap;
}


wxIMPLEMENT_APP(MainOPolyglot);

wxString MainOPolyglot::LibraryOPolyglotTranslate(wxString &inputXML,wxArrayString &configsYml)
{
	wxMutexLocker(MainOPolyglot::mutexTranslate);
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot::LibraryOPolyglotTranslate"));
	typedef wxString (*TranslatorFunc)(wxString,wxString,wxString);
	TranslatorFunc translator = (TranslatorFunc)libOPolyglot->GetSymbol(wxS("OPolyglotTranslator"));
	if(IS_NULLPTR(translator))
	{
		OPOLYGLOT_ERROR(wxT("MainOPolyglot::LibraryOPolyglotOCR not find symbol \"OPolyglotTranslator\" in library \"%s\""),OPOLYGLOT_LIBRARY);
		return OPolyglotGetErrorXml(wxT("MainOPolyglot::LibraryOPolyglotOCR not finding symbol \"OPolyglotTranslator\""));
	}
	wxString secondYml = wxEmptyString;
	if(configsYml.GetCount() == 2)
	{
		secondYml = configsYml.Item(1);
	}
	return translator(inputXML,configsYml.Item(0),secondYml);
}

wxString MainOPolyglot::LibraryOPolyglotOCR(wxString &inputXML,wxString dirOCR,wxString langOCR)
{
	wxMutexLocker(MainOPolyglot::mutexOCR);
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot::LibraryOPolyglotOCR"));
	typedef wxString (*OCRFunc)(wxString,wxString,wxString);
	OCRFunc ocr = (OCRFunc)libOPolyglot->GetSymbol(wxT("OPolyglotOCR"));
	if(IS_NULLPTR(ocr))
	{
		OPOLYGLOT_ERROR(wxT("MainOPolyglot::LibraryOPolyglotOCR not find symbol \"OPolyglotOCR\" in library \"%s\""),OPOLYGLOT_LIBRARY);
		return OPolyglotGetErrorXml(wxT("MainOPolyglot::LibraryOPolyglotOCR not finding symbol \"OPolyglotOCR\""));
	}
	return ocr(dirOCR,langOCR,inputXML);
}

bool MainOPolyglot::OnInit()
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	wxLog::SetLogLevel(OPolyglotGetLogLevel(config.Read(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT)));
	wxLog* logger = new wxLogStream(&(std::cout));
	wxLog::SetActiveTarget(logger);
	OPOLYGLOT_DEBUG(wxT("start "));
	if(!wxFileName::DirExists(OPOLYGLOT_USER_DIR))
	{
		OPOLYGLOT_WARNING(wxT("path %s is absent"),OPOLYGLOT_USER_DIR);
		if(wxDir::Make(OPOLYGLOT_USER_DIR))
		{
			OPOLYGLOT_MESSAGE(wxT("creating directory %s"),OPOLYGLOT_USER_DIR);
		} else
		{
			OPOLYGLOT_ERROR(wxT("creating dir %s"),OPOLYGLOT_USER_DIR);
			wxSafeShowMessage("OPolyglot",wxString::Format(wxT("not created directory %s"),OPOLYGLOT_USER_DIR));
			return false;
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
			wxSafeShowMessage("OPolyglot",wxString::Format(wxT("not created directory %s"),OPOLYGLOT_USER_DATA));
			return false;
		}
	}
#if OPOLYGLOT_DEBUG_ENABLED
	wxLog::SetLogLevel(wxLOG_Debug);
#else
	wxLog::SetLogLevel(OPolyglotGetLogLevel(config.Read(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT)));
#endif
#if OPOLYGLOT_DEBUG_ENABLED == 0
	wxFFile *logFile = new wxFFile(OPOLYGLOT_LOG_FILENAME,"a");
	wxLog* fileLogger = new wxLogStderr(logFile->fp());
	wxLog::SetActiveTarget(fileLogger);
#endif
	wxDateTime now = wxDateTime::Now();
	OPOLYGLOT_ERROR(wxT("-------START OPOLYGLOT %s-----------"),now.Format("%c", wxDateTime::CET));
	OPOLYGLOT_MESSAGE(wxT("OnInit"));
	OPOLYGLOT_INFO(wxT("test log level INFO"));	
	OPOLYGLOT_MESSAGE(wxT("test log level MESSAGE"));
	OPOLYGLOT_WARNING(wxT("test log level WARNING"));
	OPOLYGLOT_ERROR(wxT("test log level ERROR"));
	OPOLYGLOT_ERROR(wxT("version: %s %d"),OPOLYGLOT_VERSION_NAME,OPOLYGLOT_VERSION_MINOR);
	OPOLYGLOT_ERROR(wxT("git commit hash %s"),GIT_COMMIT_HASH); /* these messages such as error so that the software version is always displayed in the logs */
	OPOLYGLOT_ERROR(wxT("%s"),wxGetOsDescription());
#ifdef __WXGTK__
	OPOLYGLOT_ERROR(wxT("%s %s")
			,wxGetLinuxDistributionInfo().Description
			,wxGetLinuxDistributionInfo().CodeName);
#endif
#if __SNAP
	OPOLYGLOT_ERROR(wxT("SNAP"));
#endif
	OPOLYGLOT_ERROR("------------------------------------------------");
	OPOLYGLOT_MESSAGE(wxT("config dir %s"),OPOLYGLOT_USER_DIR);
	OPOLYGLOT_MESSAGE(wxT("download xml %s"),wxGetenv("DOWNLOAD_XML"));
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxTIFFHandler);
	//wxImage::AddHandler(new wxJPEGHandler);
	wxFileTranslationsLoader::AddCatalogLookupPathPrefix(OPOLYGLOT_LOCALE_DIR);
	if(!locale.Init(config.ReadLong(OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE,OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE_DEFAULT)))
	{
		OPOLYGLOT_WARNING(wxT("MainOPolyglot init language "));
	}
	if(!locale.AddCatalog("opolyglot"))
	{
		OPOLYGLOT_ERROR(wxT("MainOPolyglot language %s not find catalog locales"),wxUILocale::GetLanguageName(wxLANGUAGE_DEFAULT));
		wxSafeShowMessage("OPolyglot",wxString::Format(wxT("not find catalog locales")));
		return false;
	}
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot language %s %d"),wxUILocale::GetLanguageName(wxUILocale::GetSystemLanguage()),wxLANGUAGE_DEFAULT);
	if(!wxFileName::FileExists(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_WARNING(wxT("not find file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		if(!wxCopyFile(OPOLYGLOT_GET_RES_XML_DATA_FILE,OPOLYGLOT_GET_XML_DATA_FILE))
		{
			OPOLYGLOT_ERROR(wxT("error coping file %s -> %s"),OPOLYGLOT_GET_RES_XML_DATA_FILE,OPOLYGLOT_GET_XML_DATA_FILE);
			wxSafeShowMessage("OPolyglot",wxString::Format(wxT("error coping file \"%s -> %s\""),OPOLYGLOT_GET_RES_XML_DATA_FILE,OPOLYGLOT_GET_XML_DATA_FILE));
			return false;
		}

	}
	libOPolyglot = new wxDynamicLibrary(OPOLYGLOT_LIBRARY);
	if((IS_NULLPTR(libOPolyglot))||(!libOPolyglot->IsLoaded()))
	{
		OPOLYGLOT_ERROR(wxT("MainOPolyglot not loaded library %s"),OPOLYGLOT_LIBRARY);
		wxSafeShowMessage("OPolyglot",wxString::Format(wxT("not loaded library %s"),OPOLYGLOT_LIBRARY));
		return false;
	}
	wxArtProvider::Push(new OPolyglotArtProvider());
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
	//OPOLYGLOT_MESSAGE(wxT("~MainOPolyglot"));
}


void MainOPolyglot::OnSetup(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnSetup"));
	frameSetup = new OPolyglotSetup(this);
	frameSetup->Show();
	frame->SetShow(false);
	taskBar->SetLabel(wxEmptyString);
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&MainOPolyglot::OnSetup,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&MainOPolyglot::OnSetupFinish,this);
}

void MainOPolyglot::OnAbout(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnAbout"));
	About *about = new  About(NULL);
	about->Show();
	libOPolyglot->Unload();
	delete libOPolyglot;
}

void MainOPolyglot::OnSetupFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnSetupFinish"));
	delete frameSetup;
	frameSetup = NULL;
	frame->ScanLanguageFrom();
	frame->ScanLanguageTo();
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
	OPOLYGLOT_MESSAGE(wxT("OnShow(%p)"),frameSetup);
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
	OPOLYGLOT_MESSAGE(wxT("OnHide(%p)"),frameSetup);
}

void MainOPolyglot::OnExitProgramm(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnExitProgramm"));
	if(!IS_NULLPTR(frameSetup))
	{
		frameSetup->Destroy();
	}
	frame->~wxFrame();
	OPOLYGLOT_DEBUG(wxT("MainOPolyglot::OnExitProgramm %s"),OPOLYGLOT_BOOL_TO_STRING(frame == nullptr));
	delete taskBar;
}
