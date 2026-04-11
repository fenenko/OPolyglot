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
#include <wx/imagpng.h>
#include "LibOPolyglot.h"
#ifdef __WXGTK__
#include "../res/icons_clear.xpm"
#include "../res/icon_rechange.xpm"
#endif




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
#ifdef __WXGTK__
			if(size.GetWidth() == 16)
			{
				return wxBitmap(icon_clear_16_xpm);
			}
			wxBitmap bitmap(icon_clear_64_xpm);
			wxImage img = bitmap.ConvertToImage();
			img.Rescale(size.GetWidth(),size.GetHeight(),wxIMAGE_QUALITY_HIGH );
			return wxBitmap(img);
#endif
#ifdef __WXMSW__
			wxBitmap res = wxBITMAP_PNG(OPOLYGLOT_CLEAR);//, wxBITMAP_TYPE_BMP_RESOURCE);
			return res;
#endif
		}
		if(id == OPOLYGLOT_ART_RECHANGE)
		{
#ifdef __WXGTK__
			if((size.GetWidth() != 16)||(size.GetHeight() !=16))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotArtProvide::CreateBitmap size %dx%d != 16x16"),size.GetWidth(),size.GetHeight());
			}
			return wxBitmap(icon_rechange_xpm);
#endif
#ifdef __WXMSW__
			wxBitmap res = wxBITMAP_PNG(OPOLYGLOT_REFRESH);//,wxBITMAP_TYPE_BMP_RESOURCE);
			return res;
#endif
		}
	}
	return wxNullBitmap;
}

OPolyglotStreamBufTOwxLog::OPolyglotStreamBufTOwxLog(LogType v) : std::streambuf()
{
	m_type = v;
}

int OPolyglotStreamBufTOwxLog::overflow(int v)
{
	if (v == '\n') {
		// Коли зустрічаємо кінець рядка, виводимо буфер
		if (m_type == LOG_ERROR) {
			wxLogError("%s\n", m_buffer.c_str());
		} else {
			wxLogMessage("%s\n", m_buffer.c_str());
		}
		m_buffer.clear(); // Очищаємо буфер для наступного рядка
	} else if (v != traits_type::eof()) {
		// Додаємо символ до буфера
		m_buffer += traits_type::to_char_type(v);
	}
	return v;
}

std::streamsize OPolyglotStreamBufTOwxLog::xsputn(const char* s,std::streamsize n)
{
	for (std::streamsize i = 0; i < n; ++i) {
		overflow(s[i]);
	}
	return n;
}

wxIMPLEMENT_APP(MainOPolyglot);


bool MainOPolyglot::OnInit()
{
	wxInitAllImageHandlers();
	//wxImage::AddHandler(new wxPNGHandler);
	//wxImage::AddHandler(new wxTIFFHandler);
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	wxLog::SetLogLevel(OPolyglotGetLogLevel(config.Read(OPOLYGLOT_CONFIG_STRING_LOG_LEVEL,OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT)));
	wxLog* logger = new wxLogStream(&(std::cout));
	wxLog::SetActiveTarget(logger);
	if(!wxFileName::DirExists(OPOLYGLOT_USER_DIR))
	{
		if(wxDir::Make(OPOLYGLOT_USER_DIR))
		{
		} else
		{
			OPOLYGLOT_ERROR(wxT("creating dir %s"),OPOLYGLOT_USER_DIR);
			wxSafeShowMessage("OPolyglot",wxString::Format(wxT("not created directory %s"),OPOLYGLOT_USER_DIR));
			return false;
		}
	} else
	{
	}
	if(!wxFileName::DirExists(OPOLYGLOT_USER_DATA))
	{
		if(wxDir::Make(OPOLYGLOT_USER_DATA))
		{
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
	oldCoutBuf = std::cout.rdbuf();
	oldCerrBuf = std::cerr.rdbuf();
	coutRedirect = new OPolyglotStreamBufTOwxLog(OPolyglotStreamBufTOwxLog::LOG_INFO);
	cerrRedirect = new OPolyglotStreamBufTOwxLog(OPolyglotStreamBufTOwxLog::LOG_ERROR);
	std::cout.rdbuf(coutRedirect);
	std::cerr.rdbuf(cerrRedirect);
#endif
	wxDateTime now = wxDateTime::Now();
	OPOLYGLOT_ERROR(wxT("-------START OPOLYGLOT %s-----------"),now.Format("%c", wxDateTime::CET));
	OPOLYGLOT_ERROR(wxT("%s %d\t%s"),OPOLYGLOT_VERSION_NAME,OPOLYGLOT_VERSION_MINOR,GIT_COMMIT_HASH);
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
	OPOLYGLOT_MESSAGE(wxT("test log level MESSAGE"));
	OPOLYGLOT_WARNING(wxT("test log level WARNING"));
	OPOLYGLOT_ERROR(wxT("test log level ERROR"));
	OPOLYGLOT_MESSAGE(wxT("config dir %s"),OPOLYGLOT_USER_DIR);
	OPOLYGLOT_MESSAGE(wxT("download xml %s"),wxGetenv("DOWNLOAD_XML"));
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
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot::OnSetup"));
	frameSetup = new OPolyglotSettings(this);
	frameSetup->Show();
	frame->SetShow(false);
	taskBar->SetLabel(wxEmptyString);
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&MainOPolyglot::OnSetup,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&MainOPolyglot::OnSetupFinish,this);
}

void MainOPolyglot::OnAbout(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot::OnAbout"));
	OPolyglotAbout *about = new  OPolyglotAbout(NULL);
	about->Show();
}

void MainOPolyglot::OnSetupFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot::OnSetupFinish"));
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
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot::OnShow(%p)"),frameSetup);
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
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot::OnHide(%p)"),frameSetup);
}

void MainOPolyglot::OnExitProgramm(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot::OnExitProgramm"));
	if(!IS_NULLPTR(frameSetup))
	{
		frameSetup->Destroy();
	}
	frame->Destroy();
	LibOPolyglotFree();
	OPOLYGLOT_DEBUG(wxT("MainOPolyglot::OnExitProgramm %s"),OPOLYGLOT_BOOL_TO_STRING(frame == nullptr));
	delete taskBar;
}

int MainOPolyglot::OnExit()
{
	OPOLYGLOT_MESSAGE(wxT("MainOPolyglot::OnExit"));
	if(coutRedirect)
	{
		std::cout.rdbuf(oldCoutBuf);
		delete coutRedirect;
	}
	if(cerrRedirect)
	{
		std::cerr.rdbuf(oldCerrBuf);
		delete cerrRedirect;
	}
	return wxApp::OnExit();
}
