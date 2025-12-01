#include "OPolyglotThread.h"
#include "OPolyglotEvent.h"
#include "OPolyglotFunc.h"
#include "Utils.h"
#include "Config.h"
#include <tesseract/baseapi.h>
#include <wx/msgdlg.h>

OPolyglotThreadOCR::OPolyglotThreadOCR(wxWindow *handler,wxString dir,wxString lang,OPolyglotImage *image)
{
	OPOLYGLOT_MESSAGE(wxT("%s %s"),dir,lang);
	this->handler = handler;
	dirOCR = dir;
	langOCR = lang;
	imageForOCR = new OPolyglotImage(image);
	library = new wxDynamicLibrary(OPOLYGLOT_LIBRARY);
	if(IS_NULLPTR(library))
	{
		OPOLYGLOT_ERROR(wxT("Error load library %s"),OPOLYGLOT_LIBRARY);
		wxMessageDialog dialog(NULL
				,wxString::Format(wxS("%s %s"),_("error load library"),OPOLYGLOT_LIBRARY)
				,wxS("Error OPolyglot")
				,wxICON_ERROR|wxOK);
		dialog.ShowModal();
	}
	Run();
}

OPolyglotThreadOCR::~OPolyglotThreadOCR()
{
	OPOLYGLOT_MESSAGE();
	delete library;
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
	typedef wxString (*OCRFunc)(wxString,wxString,OPolyglotImage*);
	OCRFunc ocr = (OCRFunc)library->GetSymbol(wxT("OPolyglotDynamicOCR"));
	if(ocr == NULL)
	{
		OPOLYGLOT_ERROR(wxT("not find symbol OPolyglotDynamicOCR"));
		event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR);
		event->SetInt(-1);
		event->SetString(wxString::Format(wxT("%s"),_("not find symbol OPolyglotDynamicOCR")));
		wxQueueEvent(this->handler,event);
		return (wxThread::ExitCode)-1;
	}
	OPOLYGLOT_DEBUG(wxT("start ocr"));
	result = ocr(dirOCR,langOCR,imageForOCR);
	OPOLYGLOT_DEBUG(wxT("finish ocr"));
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR);
	event->SetString(result);
	wxQueueEvent(this->handler,event);
	library->Unload();
	delete imageForOCR;
	OPOLYGLOT_DEBUG(wxT("FINISH"));
	return (wxThread::ExitCode)0;
}

OPolyglotThreadTranslator::OPolyglotThreadTranslator(wxWindow *handler,wxArrayString *configs,wxString text)
{
	OPOLYGLOT_MESSAGE();
	this->handler = handler;
	configsYmlTranslator = configs;
	textOriginal = text;
	library = new wxDynamicLibrary(OPOLYGLOT_LIBRARY);
	if(IS_NULLPTR(library))
	{
		OPOLYGLOT_ERROR(wxT("Error load library %s"),OPOLYGLOT_LIBRARY);
		wxMessageDialog dialog(NULL
				,wxString::Format(wxS("%s %s"),_("error load library"),OPOLYGLOT_LIBRARY)
				,wxS("Error OPolyglot")
				,wxICON_ERROR|wxOK);
		dialog.ShowModal();
	}
	Run();
}


OPolyglotThreadTranslator::~OPolyglotThreadTranslator()
{
	OPOLYGLOT_MESSAGE();
	// the thread is being destroyed; make sure not to leave dangling pointers around
	delete library;
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
	typedef wxString (*TranslatorFunc)(wxString,wxString);
	TranslatorFunc translator = (TranslatorFunc)library->GetSymbol(wxS("OPolyglotDynamicTranslator"));
	if(IS_NULLPTR(translator))
	{
		OPOLYGLOT_ERROR(wxT("not find symbol OPolyglotDynamicTranslator in %s"),OPOLYGLOT_LIBRARY);
		event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR);
		event->SetInt(-1);
		event->SetString(wxString::Format(wxT("%s %s"),_("not find symbol OPolyglotDynamicTranslator in %s"),OPOLYGLOT_LIBRARY));
		wxQueueEvent(this->handler,event);
		return (wxThread::ExitCode)-1;
		
	}
	for(size_t i =0; i < configsYmlTranslator->GetCount();i+=1)
	{
		OPOLYGLOT_DEBUG(wxT("start translation %s"),configsYmlTranslator->Item(i));
		if(0 < result.Length())
		{
			result = translator(result,configsYmlTranslator->Item(i));
		}
	}

	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION);
	event->SetInt(0);
	event->SetString(result);
	OPOLYGLOT_INFO(wxT("FINISH"));
	wxQueueEvent(this->handler,event);
	library->Unload();
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
