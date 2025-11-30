#pragma once
#include <wx/taskbar.h>



class OPolyglotTaskBar : public wxTaskBarIcon
{
	public:
		OPolyglotTaskBar(wxEvtHandler *handler);
		void OnView(wxCommandEvent& event);
		void OnMenuExit(wxCommandEvent &event);
		void OnSetupLanguage(wxCommandEvent &event);
		void OnLeftDown(wxTaskBarIconEvent &event);
		virtual wxMenu *CreatePopupMenu() wxOVERRIDE;

	private:
		wxEvtHandler *handler;
		bool viewTranslator = false;	
};
