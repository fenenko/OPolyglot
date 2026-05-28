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


#include "OPolyglotAbout.h"
#include "Utils.h"
#ifndef __WXMSW__
#include "../res/icon.xpm"
#endif
#include "Version.h"
#include <wx/textfile.h>

OPolyglotAbout::OPolyglotAbout(wxWindow* parent) : GUIAbout(parent)
{
	this->SetTitle(wxString::Format(wxT("%s OPolyglot"),_("About")));
	wxVersionInfo version = wxGetLibraryVersionInfo ();
	OPOLYGLOT_MESSAGE(wxT("OPolyglotAbout"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	readmeOpolyglot->Bind(wxEVT_HTML_LINK_CLICKED,&OPolyglotAbout::OnLinkClicked,this);	
	labelOpolyglot->SetLabel(wxString::Format(wxT("OPolyglot %s %s %s"),_("version"),OPOLYGLOT_VERSION,wxEmptyString));
	licensesOpolyglot->LoadFile(OPOLYGLOT_LICENSES_FILE);
	wxString md = wxEmptyString;
	wxTextFile file(OPOLYGLOT_README);
	if(file.Open())
	{
		for(wxString str = file.GetFirstLine();!file.Eof();str = file.GetNextLine())
		{
			md = md + str + wxS("\n");
		}
		wxString html = ConvertMdToHtml(md);
		OPOLYGLOT_DEBUG(wxT("OPolyglotAbout HTML\n%s"),html);
		if(!readmeOpolyglot->SetPage(html))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotAbout couldn`t load HTML Readme"));
		}
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotAbout couldn`t open %s"),OPOLYGLOT_README);
	}
}

OPolyglotAbout::~OPolyglotAbout()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotAbout"));
}


void OPolyglotAbout::OnLinkClicked(wxHtmlLinkEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotAbout::OnLinkClicked %s"),event.GetLinkInfo().GetHref());
	wxString href = event.GetLinkInfo().GetHref();
    if (href.StartsWith(wxT("http://")) || href.StartsWith(wxT("https://"))) {
        wxLaunchDefaultBrowser(href);
        
    } else {
        event.Skip();
    }
}
