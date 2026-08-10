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
#include <wx/dynarray.h>


class OPolyglotIdLine{
	private:
		wxString idText;
		int startLine;
		int endLine;
	public:
		OPolyglotIdLine(wxString &id,int start,int end);
		int GetStart();
		int GetEnd();
		wxString GetId();
};

WX_DECLARE_OBJARRAY(OPolyglotIdLine,OPolyglotArrayIdLine);


class OPolyglotViewTextTranslate : public GUIOPolyglotViewTextTranslate
{
	private:
		OPolyglotArrayIdLine ids;
	protected:
		wxString fileName;
		wxWindow *parent;
		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnCopy( wxCommandEvent& event ) wxOVERRIDE;
		void OnClear( wxCommandEvent& event ) wxOVERRIDE;
		void OnDoubleClickText(wxStyledTextEvent& event);
		void OnFinishEditTranslate(wxThreadEvent& event);
	public:
		OPolyglotViewTextTranslate(wxWindow *parent,wxString fileName);
		~OPolyglotViewTextTranslate();
		void LoadXML(int oldLine = -1);
		bool ViewTranslate();
};
