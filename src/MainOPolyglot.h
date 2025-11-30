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
		void OnExitProgramm(wxThreadEvent& event);
	private:
		OPolyglotTaskBar	*taskBar;
		OPolyglotSetup *frameSetup;
		OPolyglot *frame;

};

wxDECLARE_APP(MainOPolyglot);

#endif


