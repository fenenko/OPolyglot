#include "OPolyglotAbout.h"
#include "Utils.h"
#include "../res/icon.xpm"

wxString value = wxT("<html><body><h1>Polyglot 0.1.0</h1>"
"</p>"
" Copyright 2026 Fenenko Oleksandr.<br>"
" Licensed under the Apache License, Version 2.0 (the \"License\");<br>"
" you may not use this file except in compliance with the License.<br>"
" You may obtain a copy of the License at<br>"
"     http://www.apache.org/licenses/LICENSE-2.0<br>"
" Unless required by applicable law or agreed to in writing, software<br>"
" distributed under the License is distributed on an \"AS IS\" BASIS,<br>"
" WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>"
" See the License for the specific language governing permissions and<br>"
" limitations under the License.<br>"
"</p>"
 "<p><h2>This software uses the following libraries</h2></p></body></html>");

About::About(wxWindow* parent) : GUIAbout(parent)
{
	OPOLYGLOT_MESSAGE();
	SetIcon(wxICON(icon));
	Info->SetPage(value);
}

About::~About()
{
	OPOLYGLOT_MESSAGE();
}
