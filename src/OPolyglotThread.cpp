#include "OPolyglotThread.h"
#include "OPolyglotEvent.h"
#include "Utils.h"
#include "Config.h"

OPolyglotThreadOCR::OPolyglotThreadOCR(wxWindow *handler,wxString dir,wxString lang,OPolyglotImage *image)
{
	OPOLYGLOT_MESSAGE(wxT("%s %s"),dir,lang);
	this->handler = handler;
	dirOCR = dir;
	langOCR = lang;
	imageForOCR= image;
	library = new wxDynamicLibrary(OPOLYGLOT_LIBRARY);
	if(library == NULL)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotThreadTranslator not loaded %s"),OPOLYGLOT_LIBRARY);
		
		wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION);
		event->SetInt(-1);
		event->SetString(wxString::Format(wxT("%s %s"),_("error load shared library "),OPOLYGLOT_LIBRARY));
		wxQueueEvent(this->handler,event);
		return;
	}
	OPOLYGLOT_DEBUG(wxT("loaded %s"),OPOLYGLOT_BOOL_TO_STRING(library->IsLoaded()));
	this->Run();
}

OPolyglotThreadOCR::~OPolyglotThreadOCR()
{
	OPOLYGLOT_MESSAGE();
	if(library->IsLoaded())
	{
#if 0
		typedef void (*OCRDestroy)();
		OCRDestroy ocrDestroy = (OCRDestroy)library->GetSymbol(wxS("OPolyglotDynamicOCRDestroy"));
		if(ocrDestroy == NULL)
		{
			OPOLYGLOT_ERROR(wxT("not find symbol OPolyglotDynamicOCRDestroy"));
		} else
		{
			ocrDestroy();
		}
#endif
		library->Unload();
	}
	delete library;
	library = NULL;
	handler = NULL;
}

void OPolyglotThreadOCR::OnExit()
{
	OPOLYGLOT_MESSAGE();
#if 0
	if(library->IsLoaded())
	{
		typedef void (*OCRDestroy)();
		OCRDestroy ocrDestroy = (OCRDestroy)library->GetSymbol(wxS("OPolyglotDynamicOCRDestroy"));
		if(ocrDestroy == NULL)
		{
			OPOLYGLOT_ERROR(wxT("not find symbol OPolyglotDynamicOCRDestroy"));
		} else
		{
			ocrDestroy();
		}
		library->Unload();
	}
#endif
}

void OPolyglotThreadOCR::OnKill()
{
	OPOLYGLOT_WARNING();
	//imageForOCR->~OPolyglotImage();
	imageForOCR = NULL;
#if 0
	if(library->IsLoaded())
	{
		typedef void (*OCRDestroy)();
#if 1
		OCRDestroy ocrDestroy = (OCRDestroy)library->GetSymbol(wxS("OPolyglotDynamicOCRDestroy"));
		if(ocrDestroy == NULL)
		{
			OPOLYGLOT_ERROR(wxT("not find symbol OPolyglotDynamicOCRDestroy"));
		} else
		{
			ocrDestroy();
		}
		library->Unload();
#endif
	}
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR);
	event->SetInt(-1);
	event->SetString(wxEmptyString);
	wxQueueEvent(this->handler,event);
#endif
}


wxThread::ExitCode OPolyglotThreadOCR::Entry()
{
	OPOLYGLOT_MESSAGE();
	wxThreadEvent *event = NULL;
	wxString result;
	event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_UPDATE_PROGRESS_MESSAGE);
	event->SetString(_("OCR..."));
	wxQueueEvent(this->handler,event);
#if 0
	typedef wxString (*OCRInit)(wxString,wxString,OPolyglotImage*);
	OCRInit ocrInit = (OCRInit)library->GetSymbol(wxS("OPolyglotDynamicOCRInit"));
	if(ocrInit == nullptr)
	{
		OPOLYGLOT_ERROR(wxT("not find symbol OPolyglotDynamicOCRInit"));
		event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR);
		event->SetInt(-1);
		event->SetString(wxString::Format(wxT("%s"),_("not find symbol OPolyglotDynamicOCRInit")));
		wxQueueEvent(this->handler,event);
		return (wxThread::ExitCode)-1;
	}
	wxString resStr = ocrInit(dirOCR,langOCR,imageForOCR);
	if(!resStr.IsEmpty())
	{
		OPOLYGLOT_ERROR(wxT("error ocrInit %s"),resStr);
		event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR);
		event->SetInt(-1);
		event->SetString(wxString::Format(wxT("%s"),_("not find symbol OPolyglotDynamicOCRInit")));
		wxQueueEvent(this->handler,event);
		return (wxThread::ExitCode)-1;
	}
#endif
#if 1
	OPOLYGLOT_INFO(wxT("start ocr"));
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
	result = ocr(dirOCR,langOCR,imageForOCR);
	//imageForOCR->~OPolyglotImage();
	imageForOCR = NULL;
#endif
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
