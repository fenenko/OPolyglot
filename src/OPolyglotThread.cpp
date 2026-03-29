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

wxString ErrorXml(wxString errorString)
{

	wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE, wxS("Error"));
	errorNode->AddAttribute(wxS("value"),errorString);
	wxString str = wxEmptyString;
	wxStringOutputStream sos(&str);
	wxXmlDocument docError;
	docError.SetRoot(errorNode);
	docError.Save(sos);
	return str;
}

OPolyglotThreadOCR::OPolyglotThreadOCR(wxWindow *handler,wxString dir,wxString lang,wxString xml)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadOCR %s %s "),dir,lang);
	this->handler = handler;
	dirOCR = dir;
	langOCR = lang;
	inputXml = xml;
	OPOLYGLOT_DEBUG(wxT("start load "));
	library = new wxDynamicLibrary(OPOLYGLOT_LIBRARY);
	if(IS_NULLPTR(library) || (!library->IsLoaded()))
	{
		OPOLYGLOT_ERROR(wxT("Error load library %s for OCR"),OPOLYGLOT_LIBRARY);
		wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT);
		event->SetString(ErrorXml(wxString::Format(wxS("error load library %s,for OCR"),OPOLYGLOT_LIBRARY)));
		wxQueueEvent(this->handler,event);
		return;
	}
	Run();
}

OPolyglotThreadOCR::~OPolyglotThreadOCR()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotThreadOCR"));
	delete library;
	handler = NULL;
}

void OPolyglotThreadOCR::OnExit()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadOCR::OnExit"));
}

void OPolyglotThreadOCR::OnKill()
{
	OPOLYGLOT_WARNING(wxT("OPolyglotThreadOCR::OnKill"));
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT);
	event->SetInt(-1);
	event->SetString(wxEmptyString);
	wxQueueEvent(this->handler,event);
}


wxThread::ExitCode OPolyglotThreadOCR::Entry()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadOCR::Entry"));
	wxThreadEvent *event = NULL;
	wxString result;
	typedef wxString (*OCRFunc)(wxString,wxString,wxString);
	OCRFunc ocr = (OCRFunc)library->GetSymbol(wxT("OPolyglotOCR"));
	if(ocr == NULL)
	{
		OPOLYGLOT_ERROR(wxT("not find symbol OPolyglotDynamicOCR"));
		event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT);
		event->SetString(ErrorXml(wxString::Format(wxS("not find symbol %s,for OCR"),wxS("OPolyglotDynamicOCR"))));
		wxQueueEvent(this->handler,event);
		return (wxThread::ExitCode)-1;
	}
	OPOLYGLOT_DEBUG(wxT("start ocr"));
	result = ocr(dirOCR,langOCR,inputXml);
	OPOLYGLOT_DEBUG(wxT("finish ocr"));
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT);
	event->SetString(result);
	wxQueueEvent(this->handler,event);
	library->Unload();
	OPOLYGLOT_DEBUG(wxT("FINISH"));
	return (wxThread::ExitCode)0;
}

OPolyglotThreadTranslator::OPolyglotThreadTranslator(wxWindow *handler,wxArrayString &configs,wxString text)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadTranslator(%ld)"),text.Length());
	this->handler = handler;
	configsYmlTranslator = configs;
	textOriginal = text;
	library = new wxDynamicLibrary(OPOLYGLOT_LIBRARY);
	if(IS_NULLPTR(library)||(!library->IsLoaded()))
	{
		OPOLYGLOT_ERROR(wxT("Error load library %s %p %s %s"),OPOLYGLOT_LIBRARY,library,OPOLYGLOT_BOOL_TO_STRING(library->IsLoaded()),wxSysErrorMsg(wxSysErrorCode()));
		wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT);
		event->SetString(ErrorXml(wxString::Format(wxS("Error load library %s,for translator"),OPOLYGLOT_LIBRARY)));
		wxQueueEvent(this->handler,event);
		return;
	}
	Run();
}


OPolyglotThreadTranslator::~OPolyglotThreadTranslator()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotThreadTranslator"));
	// the thread is being destroyed; make sure not to leave dangling pointers around
	delete library;
	handler = NULL;
}



wxThread::ExitCode OPolyglotThreadTranslator::Entry()
{
	wxThreadEvent *event = NULL;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadTranslator::Entry"));
	wxString result = textOriginal;
	wxString secondYml = wxEmptyString;
	if(configsYmlTranslator.GetCount() == 2)
	{
		secondYml = configsYmlTranslator.Item(1);
	}
	typedef wxString (*TranslatorFunc)(wxString,wxString,wxString);
	TranslatorFunc translator = (TranslatorFunc)library->GetSymbol(wxS("OPolyglotTranslator"));
	if(IS_NULLPTR(translator))
	{
		OPOLYGLOT_ERROR(wxT("not find symbol OPolyglotDynamicTranslator in %s"),OPOLYGLOT_LIBRARY);
		event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT);
		event->SetString(ErrorXml(wxString::Format(wxT("%s %s"),_("not find symbol OPolyglotDynamicTranslator in %s"),OPOLYGLOT_LIBRARY)));
		wxQueueEvent(this->handler,event);
		return (wxThread::ExitCode)-1;

	}
	result = translator(result,configsYmlTranslator.Item(0),secondYml);

	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT);
	event->SetString(result);
	wxQueueEvent(this->handler,event);
	library->Unload();
	return (wxThread::ExitCode)0;
}


void OPolyglotThreadTranslator::OnExit()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadTranslator::OnExit"));
}


void OPolyglotThreadTranslator::OnKill()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotThreadTranslator::OnKill"));
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT);
	event->SetInt(-1);
	event->SetString(wxEmptyString);
	wxQueueEvent(this->handler,event);
}
