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


#include "OPolyglotThread.h"
#include "OPolyglotEvent.h"
#include "Utils.h"
#include "Config.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/filename.h>
#include <wx/config.h>
#include "LibOPolyglot.h"


OPolyglotThreadOCR::OPolyglotThreadOCR(wxWindow *handler,wxString dir,wxString lang,wxString xml)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadOCR %s %s "),dir,lang);
	this->handler = handler;
	dirOCR = dir;
	langOCR = lang;
	inputXml = xml;
	Run();
}

OPolyglotThreadOCR::~OPolyglotThreadOCR()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotThreadOCR"));
	handler = NULL;
}

void OPolyglotThreadOCR::OnExit()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadOCR::OnExit"));
}

void OPolyglotThreadOCR::OnKill()
{
	OPOLYGLOT_WARNING(wxT("OPolyglotThreadOCR::OnKill"));
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH);
	event->SetInt(-1);
	event->SetString(wxEmptyString);
	wxQueueEvent(this->handler,event);
}


wxThread::ExitCode OPolyglotThreadOCR::Entry()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadOCR::Entry"));
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH);
	event->SetString(LibOPolyglotOCR(inputXml,dirOCR,langOCR
				,config->ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_SAUVOLA,OPOLYGLOT_CONFIG_BOOL_ENABLED_SAUVOLA_DEFAULT)
				,static_cast<int>(config->ReadLong(OPOLYGLOT_CONFIG_INT_SAUVOLA_WHSIZE,OPOLYGLOT_CONFIG_INT_SAUVOLA_WHSIZE_DEFAULT))
				,static_cast<float>(config->ReadDouble(OPOLYGLOT_CONFIG_DOUBLE_SAUVOLA_FACTOR,OPOLYGLOT_CONFIG_DOUBLE_SAUVOLA_FACTOR_DEFAULT))));
	delete config;
	wxQueueEvent(this->handler,event);
	OPOLYGLOT_DEBUG(wxT("OPolyglotThreadOCR::Entry FINISH"));
	return (wxThread::ExitCode)0;
}

OPolyglotThreadTranslator::OPolyglotThreadTranslator(wxWindow *handler,wxArrayString &configs,wxString &inputXML)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadTranslator"));
	this->handler = handler;
	configsYmlTranslator = configs;
	textOriginal = inputXML;
}


OPolyglotThreadTranslator::~OPolyglotThreadTranslator()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotThreadTranslator"));
	// the thread is being destroyed; make sure not to leave dangling pointers around
	handler = NULL;
}



wxThread::ExitCode OPolyglotThreadTranslator::Entry()
{
	wxThreadEvent *event = NULL;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadTranslator::Entry"));
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH);
	wxString configYml = configsYmlTranslator.Item(0);
	wxString configYmlSecond = wxEmptyString;
	if(configsYmlTranslator.GetCount() == 2)
	{
		configYmlSecond = configsYmlTranslator.Item(1);
	}
	event->SetString(LibOPolyglotTranslator(textOriginal,configYml,configYmlSecond));
	wxQueueEvent(this->handler,event);
	return (wxThread::ExitCode)0;
}


void OPolyglotThreadTranslator::OnExit()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadTranslator::OnExit"));
}


void OPolyglotThreadTranslator::OnKill()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadTranslator::OnKill"));
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH);
	event->SetInt(-1);
	event->SetString(wxEmptyString);
	wxQueueEvent(this->handler,event);
}
