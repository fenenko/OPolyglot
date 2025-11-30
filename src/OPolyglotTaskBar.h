#pragma once
#include <wx/taskbar.h>



class OPolyglotTaskBar : public wxTaskBarIcon
{
	public:
		OPolyglotTaskBar(wxEvtHandler *handler,bool flag);
		void OnView(wxCommandEvent& event);
		void OnMenuExit(wxCommandEvent &event);
		void OnSetupLanguage(wxCommandEvent &event);
		void OnLeftDown(wxTaskBarIconEvent &event);
		virtual wxMenu *CreatePopupMenu() wxOVERRIDE;
		void SetShow(bool flag);

	private:
		wxEvtHandler *handler;
		bool viewTranslator = false;	
};
