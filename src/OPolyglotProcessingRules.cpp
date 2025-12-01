#include "OPolyglotPocessingRules.h"

OPolyglotPocessingRules::OPolyglotPocessingRules(wxEvtHandler *handler,wxString nodeName) : GUIOPolyglotListRules(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("%s")nodeName);
}

OPolyglotPocessingRules::~OPolyglotPocessingRules()
{
	OPOLYGLOT_MESSAGE();
}
