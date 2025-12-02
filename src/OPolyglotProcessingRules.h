#pragma once
#include "GuiOPolyglot.h"
#include "OPolyglotType.h"
#include <wx/event.h>
#include <wx/string.h>
#include <wx/xml/xml.h>
#include <wx/dynarray.h>

class OPolyglotEditorRule : public GUIOPolyglotEditorRule
{
	private:
		long index;
		wxString regEx;
		wxString replacement;
		wxString comment;
	protected:
	public:
		OPolyglotEditorRule(wxWindow *parent,long index,wxString regEx,wxString replace,wxString comment);
		~OPolyglotEditorRule();

};


class OPolyglotListProcessingRules: public GUIOPolyglotListRules
{
	private:
		ArrayXmlNode rules;
		wxXmlDocument doc;
		wxEvtHandler *handler;
	protected:

		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnSelectItem( wxListEvent& event ) wxOVERRIDE;
		void BuildList();
	public:
		OPolyglotListProcessingRules(wxEvtHandler *handler,wxString node);
		~OPolyglotListProcessingRules();
		
};

