#pragma once

#include "GuiOPolyglot.h"
#include "OPolyglotDownloadLanguage.h"

class OPolyglotSetup : public GUIOPolyglotSetup
{
	public:
		OPolyglotSetup(wxWindow *parent);	
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
	private:
		wxWindow *handler;
		OPolyglotDownloadLanguage *download;
};

