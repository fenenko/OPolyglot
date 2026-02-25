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
	MENU_ABOUT,
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
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTaskBar icon %s %s %s"),OPOLYGLOT_BOOL_TO_STRING(this->IsIconInstalled()),OPOLYGLOT_BOOL_TO_STRING(this->IsOk()),OPOLYGLOT_BOOL_TO_STRING(wxTaskBarIcon::IsAvailable()));
}


void OPolyglotTaskBar::OnMenuExit(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE(wxT("OnMenuExit"));
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT);
	wxQueueEvent(handler,event);
}


void OPolyglotTaskBar::OnSetupLanguage(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE(wxT("OnSetupLanguage"));
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}

void OPolyglotTaskBar::OnMenuAbout(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnMenuAbout"));
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_ABOUT));
}


void OPolyglotTaskBar::OnView(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE(wxT("OnView"));
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
}


void OPolyglotTaskBar::OnLeftDown(wxTaskBarIconEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnLeftDown"));
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
	//menu->Append(MENU_ABOUT,_("About"),_("about OPolyglot"));
	menu->Append(MENU_EXIT,_("E&xit"),_("exit in application OPolyglot"));
	
	this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnMenuExit,this,MENU_EXIT);
	this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnSetupLanguage,this,MENU_SETUP_LANGUAGES);
	//this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnMenuAbout,this,MENU_ABOUT);
	return menu;
}

void OPolyglotTaskBar::SetLabel(wxString str)
{
	OPOLYGLOT_MESSAGE(wxT("SetLabel(%s)"),str);
	labelMenu = str;
}

