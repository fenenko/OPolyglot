#pragma once
#include <wx/taskbar.h>
#include <wx/string.h>



class OPolyglotTaskBar : public wxTaskBarIcon
{
	public:
		OPolyglotTaskBar(wxEvtHandler *handler,wxString label);
		void OnView(wxCommandEvent& event);
		void OnMenuExit(wxCommandEvent &event);
		void OnMenuAbout(wxCommandEvent &event);
		void OnSetupLanguage(wxCommandEvent &event);
		void OnLeftDown(wxTaskBarIconEvent &event);
		virtual wxMenu *CreatePopupMenu() wxOVERRIDE;
		void SetLabel(wxString label);

	private:
		wxEvtHandler *handler;
		wxString labelMenu;
};
