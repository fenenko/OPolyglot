#include "OPolyglotTaskBar.h"
#include "OPolyglotEvent.h"
#include "Utils.h"
#include "../res/icon.xpm"
#include <wx/bmpbndl.h>
#include <wx/intl.h> 
#include <wx/menu.h>


enum{
	MENU_EXIT,
	MENU_SETUP_LANGUAGES,
	MENU_VIEW,
};


OPolyglotTaskBar::OPolyglotTaskBar(wxEvtHandler *handler,wxString label) : wxTaskBarIcon()
{
	this->handler = handler;
	if(!SetIcon(wxBitmapBundle(icon_xpm),_("offline translator OPolyglot")))
	{
		OPOLYGLOT_ERROR(wxT("SetIcon"));
	}
	labelMenu = label;
	if(!labelMenu.IsEmpty())
	{
		this->Bind(wxEVT_TASKBAR_LEFT_DOWN,&OPolyglotTaskBar::OnLeftDown,this);
	}
}


void OPolyglotTaskBar::OnMenuExit(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_PROGRAMM);
	wxQueueEvent(handler,event);
}


void OPolyglotTaskBar::OnSetupLanguage(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}


void OPolyglotTaskBar::OnView(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE(wxT(""));
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
}


void OPolyglotTaskBar::OnLeftDown(wxTaskBarIconEvent& event)
{
	OPOLYGLOT_MESSAGE();
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
}


wxMenu *OPolyglotTaskBar::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu();
	if(!labelMenu.IsEmpty())
	{
		menu->Append(MENU_VIEW,labelMenu);
		this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnView,this,MENU_VIEW);
	} else
	{
	}
	menu->Append(MENU_SETUP_LANGUAGES,_("Setup"),_("setup OPolyglot"));
	menu->Append(MENU_EXIT,_("E&xit"),_("exit in application OPolyglot"));
	
	this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnMenuExit,this,MENU_EXIT);
	this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnSetupLanguage,this,MENU_SETUP_LANGUAGES);
	return menu;
}

void OPolyglotTaskBar::SetLabel(wxString str)
{
	OPOLYGLOT_MESSAGE();
	labelMenu = str;
}

