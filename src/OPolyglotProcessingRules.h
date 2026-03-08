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
#include "OPolyglotType.h"
#include <wx/event.h>
#include <wx/string.h>
#include <wx/xml/xml.h>
#include <wx/dynarray.h>

class OPolyglotMultiline : public GUIOPolyglotMultilineText
{
	private:
	protected:
		virtual void OnOk(wxCommandEvent& event) wxOVERRIDE;
		virtual void OnCancel(wxCommandEvent& event) wxOVERRIDE;
		virtual void OnClose(wxCloseEvent& event) wxOVERRIDE;
	public:
		OPolyglotMultiline(wxWindow *parent,wxString string,bool readOnly);
		~OPolyglotMultiline();
		wxString GetValue();
};

class OPolyglotEditorRule : public GUIOPolyglotEditorRule
{
	private:
		long index;
		wxString testString;
		wxWindow *parent;
	protected:
		virtual void OnClose(wxCloseEvent& event) wxOVERRIDE;
		virtual void OnCancel(wxCommandEvent& event) wxOVERRIDE;
		virtual void OnTest(wxCommandEvent& event) wxOVERRIDE;
		virtual void OnSave(wxCommandEvent& event) wxOVERRIDE;
		void OnFinishTest(wxThreadEvent& event);
	public:
		OPolyglotEditorRule(wxWindow *parent,long index,wxString regEx,wxString replace,wxString comment);
		~OPolyglotEditorRule();
		wxString GetRegEx();
		wxString GetReplace();
		wxString GetComment();
		long 	GetItem();
};


class OPolyglotListProcessingRules: public GUIOPolyglotListRules
{
	private:
		wxXmlNode 	*nodePreprocessing = NULL;
		wxXmlDocument doc;
		wxEvtHandler *handler = NULL;
		wxString testString = wxEmptyString;
		OPolyglotEditorRule	*editor = NULL;
		long			itemSelect = -1;
		wxMenu			*menu = NULL;
		bool			flagChangeRules = false;
	protected:

		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnSelectItem( wxListEvent& event ) wxOVERRIDE;
		void OnAdd( wxCommandEvent& event) wxOVERRIDE;
		void OnTest( wxCommandEvent& event) wxOVERRIDE;
		void OnSave(wxCommandEvent& event) wxOVERRIDE;
		void BuildList();
		void OnFinishNewRule(wxThreadEvent& event);
		void OnFinishChangeRule(wxThreadEvent& event);
		void OnListRulesMenu(wxListEvent& event);
		void OnMenuEdit(wxCommandEvent& event);
		void OnMenuDelete(wxCommandEvent& event);
	public:
		OPolyglotListProcessingRules(wxEvtHandler *handler,wxString node);
		~OPolyglotListProcessingRules();
};

