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


#pragma once

#include "GuiOPolyglot.h"
#include "OPolyglotDownload.h"
#include "OPolyglotProcessingRules.h"
#include <wx/event.h>
#include <wx/arrstr.h>


class OPolyglotViewLog : public GUIViewLog
{
	public:
		OPolyglotViewLog(wxFrame *parent);
		~OPolyglotViewLog();
	protected:
	private:
};

class OPolyglotSettings : public GUIOPolyglotSettings
{
	public:
		OPolyglotSettings(wxEvtHandler *parent);	
		~OPolyglotSettings();
	protected:
		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnChangeLogLevel( wxCommandEvent& event ) wxOVERRIDE;
		void OnChangeStayOnTop( wxCommandEvent& event ) wxOVERRIDE; 
		void OnSelectMethodTranslation( wxCommandEvent& event ) wxOVERRIDE;
		void OnSelectMethodOCR( wxCommandEvent& event ) wxOVERRIDE;
		void OnEnablePreprocessing( wxCommandEvent& event ) wxOVERRIDE; 
		void OnEnablePostprocessing( wxCommandEvent& event ) wxOVERRIDE; 
		void OnRulesPreprocessing( wxCommandEvent& event ) wxOVERRIDE; 
		void OnRulesPostprocessing( wxCommandEvent& event ) wxOVERRIDE; 
		void OnAdditionalLanguage(wxCommandEvent& event) wxOVERRIDE;
		void OnViewLog( wxCommandEvent& event ) wxOVERRIDE; 
		void OnRulesPreprocessingFinish(wxThreadEvent& event);
		void OnRulesPostprocessingFinish(wxThreadEvent& event);
		void OnSelectInterfaceLanguage( wxCommandEvent& event ) wxOVERRIDE;
		void ScanLangs();
		void OnLanguagesDownloadAll(wxCommandEvent& event);
		void OnLanguagesRemoveAll(wxCommandEvent& event);
		void OnLanguageDownload(wxCommandEvent& event);
		void OnLanguageRemove(wxCommandEvent& event);
		void OnDownloadFinish(wxThreadEvent& event);
		void OnSauvolaEnabled( wxCommandEvent& event ) wxOVERRIDE;
		void OnSauvolaMindiff(wxSpinEvent& event) wxOVERRIDE;

	private:
		wxEvtHandler *handler = NULL;
		OPolyglotListProcessingRules *listRules = NULL;
		OPolyglotViewLog *view = NULL;
		wxString	  currentSystemLang;
		wxXmlDocument xmlLanguages;
		OPolyglotInstallLanguages *download = nullptr;
};

