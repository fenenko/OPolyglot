#ifndef __MAIN_O_POLYGLOT__
#define __MAIN_O_POLYGLOT__
#include <wx/app.h>
#include "OPolyglotSetup.h"
#include "MainOPolyglot.h"
#include "OPolyglotTaskBar.h"
#include "OPolyglot.h"

class MainOPolyglot: public wxApp
{
	public:
		bool OnInit() wxOVERRIDE;
		~MainOPolyglot();
//		int 	OnExit();
	protected:
		void OnSetup(wxThreadEvent& event);
		void OnSetupFinish(wxThreadEvent& event);
		void OnAbout(wxThreadEvent& event);
		void OnExitProgramm(wxThreadEvent& event);
		void OnShow(wxThreadEvent& event);
		void OnHide(wxThreadEvent& event);
	private:
		OPolyglotTaskBar	*taskBar = NULL;
		OPolyglotSetup *frameSetup = NULL;
		OPolyglot *frame = NULL;

};

wxDECLARE_APP(MainOPolyglot);

#endif


