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
#include "OPolyglotVersion.h"

About::About(wxWindow* parent) : GUIAbout(parent)
{
	this->SetTitle(wxString::Format(wxT("%s OPolyglot"),_("About")));
	wxVersionInfo version = wxGetLibraryVersionInfo ();
	OPOLYGLOT_MESSAGE(wxT("About"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
#else
	SetIcon(wxICON(icon));
#endif
	labelOpolyglot->SetLabel(wxString::Format(wxT("OPolyglot %s %s %d-%s"),_("version"),OPOLYGLOT_VERSION_NAME,OPOLYGLOT_VERSION_MINOR,GIT_COMMIT_HASH));
	licensesOpolyglot->LoadFile(OPOLYGLOT_LICENSES_FILE);
}

About::~About()
{
	OPOLYGLOT_MESSAGE(wxT("~About"));
}
