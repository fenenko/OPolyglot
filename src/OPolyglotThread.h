#pragma once
#include "OPolyglotType.h"
#include "OPolyglotFunc.h"
#include <wx/thread.h>
#include <wx/arrstr.h>
#include <wx/window.h>

class OPolyglotThreadTranslator : public wxThread
{
	public:
		OPolyglotThreadTranslator(wxWindow *handler,wxArrayString *configsYml,wxString text);
		~OPolyglotThreadTranslator();
	protected:
		virtual ExitCode Entry() wxOVERRIDE;
		virtual void OnExit() wxOVERRIDE;
		virtual void OnKill() wxOVERRIDE;
	private:
		wxWindow *handler;
		wxArrayString *configsYmlTranslator;
		wxString textOriginal;
};

class OPolyglotThreadOCR : public wxThread
{
	public:
		OPolyglotThreadOCR(wxWindow *handler,wxString dirOCR,wxString langOCR,OPolyglotImage *image);
		~OPolyglotThreadOCR();
	protected:
		virtual ExitCode Entry() wxOVERRIDE;
		virtual void OnExit() wxOVERRIDE;
		virtual void OnKill() wxOVERRIDE;
	private:
		wxWindow *handler;
		wxString dirOCR;
		wxString langOCR;
		OPolyglotImage *imageForOCR;
		

};
