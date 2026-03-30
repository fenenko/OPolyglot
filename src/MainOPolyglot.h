/*
 * Copyright 2026 Fenenko Oleksandr.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MAIN_O_POLYGLOT__
#define __MAIN_O_POLYGLOT__
#include <wx/app.h>
#include <wx/intl.h>
#include "OPolyglotSetup.h"
#include "MainOPolyglot.h"
#include "OPolyglotTaskBar.h"
#include "OPolyglot.h"

class MainOPolyglot: public wxApp
{
	public:
		bool OnInit() wxOVERRIDE;
		static wxString	LibraryOPolyglotOCR(wxString inputXML);
		static wxString LibraryOPolyglotTranslate(wxString &inputXML,wxArrayString &configsYml);
		static wxString LibraryOPolyglotOCR(wxString &inputXML,wxString dirOCR,wxString langOCR);
		~MainOPolyglot();
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
		static wxDynamicLibrary *libOPolyglot;
		static wxMutex 			mutexOCR;
		static wxMutex			mutexTranslate;
		wxLocale locale;

};

wxDECLARE_APP(MainOPolyglot);

#endif


