#include "OPolyglotAbout.h"
#include "Utils.h"
#include "../res/icon.xpm"
#include <tomcrypt.h>
#include <tesseract/version.h>
#include "Version.h"
#include "OPolyglotVersion.h"

wxString value = wxT(
" Copyright 2026 Fenenko Oleksandr.\n"
" Licensed under the Apache License, Version 2.0 (the \"License\");\n"
" you may not use this file except in compliance with the License.\n"
" You may obtain a copy of the License at\n"
" \thttp://www.apache.org/licenses/LICENSE-2.0\n"
" Unless required by applicable law or agreed to in writing, software\n"
" distributed under the License is distributed on an \"AS IS\" BASIS,\n"
" WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
" See the License for the specific language governing permissions and\n"
" limitations under the License.\n"
);

About::About(wxWindow* parent) : GUIAbout(parent)
{
	wxVersionInfo version = wxGetLibraryVersionInfo ();
	OPOLYGLOT_MESSAGE();
	SetIcon(wxICON(icon));
	licenseOpolyglot->SetValue(value);
	labelOpolyglot->SetLabel(wxString::Format(wxT("OPolyglot version %s %d git hash %s"),OPOLYGLOT_VERSION_NAME,OPOLYGLOT_VERSION_NAME,GIT_COMMIT_HASH));
	listLibraries->AppendText(wxString::Format(wxT("  wxWidgets:\t%d.%d.%d\n"),version.GetMajor(),version.GetMinor(),version.GetMicro()));
	listLibraries->AppendText(wxString::Format(wxT("   Bergamot:\tv0.6.0\n")));
	listLibraries->AppendText(wxString::Format(wxT("  tesseract:\t%s\n"),TESSERACT_VERSION_STR));
	listLibraries->AppendText(wxString::Format(wxT("libtomcrypt:\t%s\n"),SCRYPT));
}

About::~About()
{
	OPOLYGLOT_MESSAGE();
}
