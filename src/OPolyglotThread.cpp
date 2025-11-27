#include "OPolyglotThread.h"
#include "OPolyglot.h"
#include "Utils.h"

OPolyglotThreadOCR::OPolyglotThreadOCR(wxWindow *handler,wxString dir,wxString lang,wxString fileForOCR)
{
	OPOLYGLOT_MESSAGE(wxT("%s %s"),dir,lang);
	this->handler = handler;
	dirOCR = dir;
	langOCR = lang;
	filenameImageAreaForOCR = fileForOCR;
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
	wxString libName = wxS("libopolyglot-ocr-translator");
	wxThreadEvent *event = NULL;
	wxArrayString listLoadLibs;
	wxDynamicLibrary library;
	wxDynamicLibraryDetailsArray libs = wxDynamicLibrary::ListLoaded();
	wxString result;
	for(size_t i =0; i < libs.GetCount();i++)
	{
		listLoadLibs.Add(libs.Item(i).GetName());
	}
	if(!library.Load(libName))
	{
		OPOLYGLOT_ERROR(wxT("not loaded %s"),libName);
		event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR);
		event->SetInt(-1);
		event->SetString(wxString::Format(wxT("%s %s"),_("error load shared library "),libName));
		wxQueueEvent(this->handler,event);
		return (wxThread::ExitCode)-1;
	}
	libs = wxDynamicLibrary::ListLoaded();
	for(size_t i =0;i < libs.GetCount();i++)
	{
		if(listLoadLibs.Index(libs.Item(i).GetName()) == wxNOT_FOUND)
		{
			OPOLYGLOT_DEBUG(wxT("%ld\t:%s"),i,libs.Item(i).GetName());
		}
	}
	OPOLYGLOT_INFO(wxT("start ocr"));
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_UPDATE_PROGRESS_MESSAGE);
	event->SetString(_("OCR..."));
	wxQueueEvent(this->handler,event);
	typedef wxString (*OCRFunc)(wxString,wxString,wxString);
	OCRFunc ocr = (OCRFunc)library.GetSymbol(wxT("OPolyglotOCR"));
	if(ocr == NULL)
	{
		OPOLYGLOT_ERROR(wxT("not find symbol OPolyglotOCR"));
		event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR);
		event->SetInt(-1);
		event->SetString(wxString::Format(wxT("%s"),_("not find symbol OPolyglotOCR")));
		wxQueueEvent(this->handler,event);
		return (wxThread::ExitCode)-1;
	}
	result = ocr(filenameImageAreaForOCR,dirOCR,langOCR);
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
	wxString libName=wxS("libopolyglot-ocr-translator");
	wxThreadEvent *event = NULL;
	wxString result = textOriginal;
	OPOLYGLOT_INFO(wxT("START"));
#if 1
	wxArrayString listLoadLibs;
	wxDynamicLibraryDetailsArray libs = wxDynamicLibrary::ListLoaded();
	for(size_t i =0; i < libs.GetCount();i++)
	{
		listLoadLibs.Add(libs.Item(i).GetName());
	}
#endif
	wxDynamicLibrary library;
	if(!library.Load(libName))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotThreadTranslator not loaded %s"),libName);
		event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION);
		event->SetInt(-1);
		event->SetString(wxString::Format(wxT("%s %s"),_("error load shared library "),libName));
		wxQueueEvent(this->handler,event);
		return (wxThread::ExitCode)-1;
	}
#if 1
	libs = wxDynamicLibrary::ListLoaded();
	for(size_t i =0;i < libs.GetCount();i++)
	{
		if(listLoadLibs.Index(libs.Item(i).GetName()) == wxNOT_FOUND)
		{
			OPOLYGLOT_DEBUG(wxT("%ld\t:%s"),i,libs.Item(i).GetName());
		}
	}
#endif
	OPOLYGLOT_INFO(wxT("start translation"));
	typedef wxString (*TranslatorFunc)(wxString,wxString);
	TranslatorFunc translate= (TranslatorFunc)library.GetSymbol(wxT("OPolyglotTranslate"));
	if(translate == NULL)
	{
		OPOLYGLOT_ERROR(wxT("not find symbol OPolyglotTranslate"));
		event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION);
		event->SetInt(-1);
		event->SetString(wxString::Format(wxT("%s"),_("not find symbol OPolyglotTranslate")));
		wxQueueEvent(this->handler,event);
		return (wxThread::ExitCode)-1;
	}
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_UPDATE_PROGRESS_MESSAGE);
	event->SetString(_("Translation..."));
	wxQueueEvent(this->handler,event);
	for(size_t i =0; i < configsYmlTranslator->GetCount();i+=1)
	{
		OPOLYGLOT_DEBUG(wxT("start translation %s"),configsYmlTranslator->Item(i));
		if(0 < result.Length())
		{
			result = translate(result,wxString::Format(wxT("%s"),configsYmlTranslator->Item(i)));
		}
	}
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION);
	event->SetInt(0);
	event->SetString(result);
	library.Unload();
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
