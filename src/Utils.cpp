#include "Utils.h"


wxLogLevel OPolyglotGetLogLevel(wxString logLevel)
{
	if(logLevel.IsSameAs(wxT("INFO")))
	{
		return wxLogLevelValues::wxLOG_Info;
	}
	if(logLevel.IsSameAs(wxT("DEBUG")))
	{
		return wxLogLevelValues::wxLOG_Debug;
	}
	if(logLevel.IsSameAs(wxT("MESSAGE")))
	{
		return wxLogLevelValues::wxLOG_Message;
	}
	if(logLevel.IsSameAs(wxT("WARNING")))
	{
		return wxLogLevelValues::wxLOG_Warning;
	}
	if(logLevel.IsSameAs(wxT("ERROR")))
	{
		return wxLogLevelValues::wxLOG_Error;
	}
	return wxLogLevelValues::wxLOG_Max;
}
