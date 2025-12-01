#pragma once

#include "GuiOPolyglot.h"
#include <wx/event.h>

class OPolyglotListProcessingRules: public GUIOPolyglotListRules
{
	public:
		OPolyglotListProcessingRules(wxEvtHandler *handler,wxString node);
		~OPolyglotListProcessingRules();
		
};

