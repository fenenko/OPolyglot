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
#include <iostream>
#include <streambuf>
#include <string>
#include <wx/app.h>
#include <wx/intl.h>
#include "OPolyglotSettings.h"
#include "MainOPolyglot.h"
#include "OPolyglotTaskBar.h"
#include "OPolyglot.h"

class OPolyglotStreamBufTOwxLog : public std::streambuf {
public:
    enum LogType { LOG_INFO, LOG_ERROR };
    // Конструктор приймає тип логування (звичайне або помилки)
	OPolyglotStreamBufTOwxLog(LogType type);
protected:
    // Перевизначаємо метод обробки символів
    virtual int overflow(int v) wxOVERRIDE;
    // Для оптимізації виводу кількох символів одразу
    virtual std::streamsize xsputn(const char* s, std::streamsize n) wxOVERRIDE;

private:
    std::string m_buffer;
    LogType m_type;
};

class MainOPolyglot: public wxApp
{
	public:
		virtual bool OnInit() wxOVERRIDE;
		virtual int	OnExit() wxOVERRIDE;
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
		OPolyglotSettings *frameSetup = NULL;
		OPolyglot *frame = NULL;
		wxLocale locale;
#if OPOLYGLOT_DEBUG_ENABLED == 0
		std::streambuf* oldCoutBuf = nullptr;
	    std::streambuf* oldCerrBuf = nullptr;
		OPolyglotStreamBufTOwxLog* coutRedirect = nullptr;
		OPolyglotStreamBufTOwxLog* cerrRedirect = nullptr;
		wxFFile *logFile;
		wxLog *fileLogger;
#endif

};

wxDECLARE_APP(MainOPolyglot);

#endif


