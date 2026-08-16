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

#include "OPolyglotDialogError.h"
#include "MainOPolyglot.h"
#include "Utils.h"
#include "wx/uri.h"
#include <wx/filedlg.h>
#include <wx/textfile.h>
#if __WXGTK__
	#include "../res/error.xpm"
#endif

OPolyglotDialogError::OPolyglotDialogError(wxWindow *parent,wxString message)
	: GUIOPolyglotDialogError(parent)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDialogError"));
#ifdef __WXMSW__
	bitmap->SetIcon(wxIcon("OPOLYGLOT_ICON_ERROR"));
#else 
	bitmap->SetIcon(wxICON(error));
#endif
	labelInfo->SetLabel(message);
	buttonSubmit->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotDialogError::OnSubmit,this);
	buttonClose->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&OPolyglotDialogError::OnClose,this);
	this->ShowModal();	

}

OPolyglotDialogError::~OPolyglotDialogError()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotDialogError"));	
}
void OPolyglotDialogError::OnSubmit(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDialogError::OnSubmit"));
	wxString lastError;
	wxString log = MainOPolyglot::GetLastLog(lastError);
	wxFileDialog saveDialog(this,_("Save OPolyglot log"),wxGetHomeDir(),wxS("log.txt"),wxS("*.txt"),wxFD_SAVE);
	if(saveDialog.ShowModal() == wxID_CANCEL)
	{
		EndModal(wxID_EXIT);
		return;
	}
	wxTextFile textFile(saveDialog.GetPath());
	textFile.Create();
	textFile.AddLine(log);
	textFile.Write();
	textFile.Close();
	wxString title = wxS("Automated bug report OPolyglot ")+GenerateUUIDv4();
	wxURI uriTitle(title);
	wxString body = wxString::Format(wxS("%s\n%s\n%s\n%s\n%s"),
			wxS("An error in the log file"),
			lastError,
			wxS("Error"),
		labelInfo->GetLabel(),
		_("Please attach the previously saved log file to this message."));
	wxURI uriBody(body);
	wxString url = wxString::Format(wxS("https://github.com/fenenko/OPolyglot/issues/new?title=%s&body=%s&labels=bug"),uriTitle.BuildURI(),uriBody.BuildURI());
	wxLaunchDefaultBrowser(url);
	EndModal(wxID_EXIT);

}

void OPolyglotDialogError::OnClose(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDialogError::OnClose"));
	EndModal(wxID_EXIT);
}

