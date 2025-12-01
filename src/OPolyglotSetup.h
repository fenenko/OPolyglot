#pragma once

#include "GuiOPolyglot.h"
#include "OPolyglotDownloadLanguage.h"
#include <wx/event.h>

class OPolyglotSetup : public GUIOPolyglotSetup
{
	public:
		OPolyglotSetup(wxEvtHandler *parent);	
		~OPolyglotSetup();
	protected:

		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnSetupLanguages( wxCommandEvent& event ) wxOVERRIDE;
		void OnChangeLogLevel( wxCommandEvent& event ) wxOVERRIDE;
		void OnChangeStayOnTop( wxCommandEvent& event ) wxOVERRIDE; 
		void OnSelectMethodTranslation( wxCommandEvent& event ) wxOVERRIDE;
		void OnSelectMethodOCR( wxCommandEvent& event ) wxOVERRIDE;
		void OnModeCreationText( wxCommandEvent& event ) wxOVERRIDE;
		void OnFinishSetupLanguage(wxThreadEvent& event);
		void OnEnablePreprocessing( wxCommandEvent& event ) wxOVERRIDE; 
		void OnEnablePostprocessing( wxCommandEvent& event ) wxOVERRIDE; 
		void OnRulesPreprocessing( wxCommandEvent& event ) wxOVERRIDE; 
		void OnRulesPostprocessing( wxCommandEvent& event ) wxOVERRIDE; 
	private:
		wxEvtHandler *handler;
		OPolyglotDownloadLanguage *download;
};

