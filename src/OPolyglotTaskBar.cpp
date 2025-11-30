#include "OPolyglotTaskBar.h"


enum{
	MENU_EXIT,
	MENU_SETUP_LANGUAGES,
	MENU_VIEW,
};


OPolyglotTaskBar::OPolyglotTaskBar(wxWindow *parent) : wxTaskBarIcon()
{
	this->parent = parent;
	if(!SetIcon(wxBitmapBundle(icon_xpm),_("offline translator OPolyglot")))
	{
		OPOLYGLOT_ERROR(wxT("SetIcon"));
	}
	this->Bind(wxEVT_TASKBAR_LEFT_DOWN,&OPolyglotTaskBar::OnLeftDown,this);
}


void OPolyglotTaskBar::OnMenuExit(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_PROGRAMM);
	wxQueueEvent(this->parent,event);
}


void OPolyglotTaskBar::OnSetupLanguage(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}


void OPolyglotTaskBar::OnView(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	if(this->parent->IsShown())
	{
		((OPolyglot *)(this->parent))->SetVisible(false);
	} else
	{
		((OPolyglot *)(this->parent))->SetVisible(true);
	}
}


void OPolyglotTaskBar::OnLeftDown(wxTaskBarIconEvent& event)
{
	OPOLYGLOT_MESSAGE();
	if(this->parent->IsShown())
	{
		((OPolyglot *)(this->parent))->SetVisible(false);
	} else
	{
		((OPolyglot *)(this->parent))->SetVisible(true);
	}
}


wxMenu *OPolyglotTaskBar::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu();
	if(this->parent->IsShown())
	{
		menu->Append(MENU_VIEW,_("Hide"));
	} else
	{
		menu->Append(MENU_VIEW,_("Show"));
	}
	menu->Append(MENU_SETUP_LANGUAGES,_("Setup"),_("setup OPolyglot"));
	menu->Append(MENU_EXIT,_("E&xit"),_("exit in application OPolyglot"));
	
	this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnView,this,MENU_VIEW);
	this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnMenuExit,this,MENU_EXIT);
	this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnSetupLanguage,this,MENU_SETUP_LANGUAGES);
	return menu;
}
