#include "OPolyglotThread.h"
#include "OPolyglotEvent.h"
#include "OPolyglotFunc.h"
#include "Utils.h"
#include "Config.h"

OPolyglotThreadOCR::OPolyglotThreadOCR(wxWindow *handler,wxString dir,wxString lang,OPolyglotImage *image)
{
	OPOLYGLOT_MESSAGE(wxT("%s %s"),dir,lang);
	this->handler = handler;
	dirOCR = dir;
	langOCR = lang;
	imageForOCR = image;
}

OPolyglotThreadOCR::~OPolyglotThreadOCR()
{
	OPOLYGLOT_MESSAGE();
	handler = NULL;
}

void OPolyglotThreadOCR::OnExit()
{
	OPOLYGLOT_MESSAGE();
}

void OPolyglotThreadOCR::OnKill()
{
	OPOLYGLOT_WARNING();
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR);
	event->SetInt(-1);
	event->SetString(wxEmptyString);
	wxQueueEvent(this->handler,event);
}


wxThread::ExitCode OPolyglotThreadOCR::Entry()
{
	OPOLYGLOT_MESSAGE();
	wxThreadEvent *event = NULL;
	wxString result;
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_UPDATE_PROGRESS_MESSAGE);
	event->SetString(_("OCR..."));
	wxQueueEvent(this->handler,event);
	OPOLYGLOT_DEBUG(wxT("start ocr"));
	result = OPolyglotFuncOCR(dirOCR,langOCR,imageForOCR);
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR);
	event->SetString(result);
	wxQueueEvent(this->handler,event);
	return (wxThread::ExitCode)0;
}

OPolyglotThreadTranslator::OPolyglotThreadTranslator(wxWindow *handler,wxArrayString *configs,wxString text)
{
	OPOLYGLOT_MESSAGE();
	this->handler = handler;
	configsYmlTranslator = configs;
	textOriginal = text;
}


OPolyglotThreadTranslator::~OPolyglotThreadTranslator()
{
	OPOLYGLOT_MESSAGE();
	// the thread is being destroyed; make sure not to leave dangling pointers around
	handler = NULL;
	configsYmlTranslator = NULL;
}



wxThread::ExitCode OPolyglotThreadTranslator::Entry()
{
	wxThreadEvent *event = NULL;
	OPOLYGLOT_INFO(wxT("START"));
	wxString result = textOriginal;
	OPOLYGLOT_INFO(wxT("start translation"));
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_UPDATE_PROGRESS_MESSAGE);
	event->SetString(_("Translation..."));
	wxQueueEvent(this->handler,event);
	for(size_t i =0; i < configsYmlTranslator->GetCount();i+=1)
	{
		OPOLYGLOT_DEBUG(wxT("start translation %s"),configsYmlTranslator->Item(i));
		if(0 < result.Length())
		{
			result = OPolyglotFuncTranslate(result,configsYmlTranslator->Item(i));
		}
	}
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION);
	event->SetInt(0);
	event->SetString(result);
	OPOLYGLOT_INFO(wxT("FINISH"));
	wxQueueEvent(this->handler,event);
	return (wxThread::ExitCode)0;
}


void OPolyglotThreadTranslator::OnExit()
{
	OPOLYGLOT_MESSAGE();
}


void OPolyglotThreadTranslator::OnKill()
{
	OPOLYGLOT_WARNING();
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION);
	event->SetInt(-1);
	event->SetString(wxEmptyString);
	wxQueueEvent(this->handler,event);
}
