#ifndef __MAIN_O_POLYGLOT__
#define __MAIN_O_POLYGLOT__
#include <wx/app.h>
#include "MainOPolyglot.h"
#include "OPolyglot.h"

class MainOPolyglot: public wxApp
{
	public:
		bool OnInit() wxOVERRIDE;
//		int 	OnExit();
	private:
		OPolyglot *frame;

};

wxDECLARE_APP(MainOPolyglot);

#endif


