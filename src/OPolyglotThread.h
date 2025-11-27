#pragma once
#include "OPolyglot.h"
#include <wx/thread.h>
#include <wx/dynlib.h>
class OPolyglotThreadTranslator : public wxThread
{
	public:
		OPolyglotThreadTranslator(wxWindow *handler,wxString dirOCR,wxString landOCR,wxArrayString *configsYml,wxString textOriginal,wxString fileForOcr);
		~OPolyglotThreadTranslator();
	protected:
		virtual ExitCode Entry() wxOVERRIDE;
		virtual void OnExit() wxOVERRIDE;
		virtual void OnKill() wxOVERRIDE;
	private:
		wxWindow *handler;
		wxString dirOCR;
		wxString langOCR;
		wxArrayString *configsYmlTranslator;
		wxString textOriginal;
		wxString filenameImageAreaForOCR;
		wxDynamicLibrary 		*library;
};
