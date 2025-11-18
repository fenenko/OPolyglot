#pragma once

#include "GuiOPolyglot.h"
#include "OPolyglot.h"

class OPolyglotSetup : public GUIOPolyglotSetup
{
	public:
		OPolyglotSetup(OPolyglot *parent);	
		~OPolyglotSetup();
	protected:

		void OnSetupLanguages( wxCommandEvent& event ) wxOVERRIDE;
		void OnChangeLogLevel( wxCommandEvent& event ) wxOVERRIDE;
		void OnChangeStayOnTop( wxCommandEvent& event ) wxOVERRIDE; 
		void OnSelectMethodTranslation( wxCommandEvent& event ) wxOVERRIDE;
	private:
		OPolyglot *parent;
};

