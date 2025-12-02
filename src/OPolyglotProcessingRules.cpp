#include "OPolyglotProcessingRules.h"
#include "Utils.h"
#include "OPolyglotEvent.h"
#include "../res/icon.xpm"
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>

OPolyglotListProcessingRules::OPolyglotListProcessingRules(wxEvtHandler *handler,wxString nodeName) : GUIOPolyglotListRules(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("%s"),nodeName);
	SetIcon(wxICON(icon));
	this->handler = handler;
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("error load data file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
		return;
	}
	this->ListRules->AppendColumn(wxT("RegEx"));
	this->ListRules->AppendColumn(wxT("Replacement"));
	this->ListRules->AppendColumn(wxT("Comment"));
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(nodeName))
		{
			OPOLYGLOT_DEBUG(wxT("IsSameAs"));
			for(wxXmlNode *rule = child->GetChildren();rule;rule = rule->GetNext())
			{
				rules.Add(rule);
			}
		}
	}
	BuildList();
	this->MainBox->Layout();
	Show();
}

void OPolyglotListProcessingRules::BuildList()
{
	OPOLYGLOT_MESSAGE();
	OPOLYGLOT_DEBUG(wxT("%ld"),rules.GetCount());
	for(size_t i = 0; i < rules.GetCount();i++)
	{
		size_t index = this->ListRules->InsertItem(0,rules.Item(i)->GetAttribute(wxS("regEx")));
		this->ListRules->SetItem(index,1,rules.Item(i)->GetAttribute(wxS("replacement")));
		this->ListRules->SetItem(index,2,rules.Item(i)->GetAttribute(wxS("comment")));
	}
}

OPolyglotListProcessingRules::~OPolyglotListProcessingRules()
{
	OPOLYGLOT_MESSAGE();
}

void OPolyglotListProcessingRules::OnClose(wxCloseEvent& event)
{
	OPOLYGLOT_MESSAGE();
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}

void OPolyglotListProcessingRules::OnSelectItem(wxListEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("%ld"),event.GetIndex());
}
