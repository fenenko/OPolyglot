#include "Utils.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>


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


bool OPolyglotCheckForInstallLanguage(wxXmlNode *node)
{
	bool ret = true;
	if(node->GetName().IsSameAs(wxT("Language")))
	{
		for(wxXmlNode *child=node->GetChildren();child&&ret;child = child->GetNext())
		{
			if(child->GetName().IsSameAs(wxT("File")))
			{
				
				ret = wxFileName::FileExists(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,child->GetAttribute(wxT("fileCheckForInstall"))));
				OPOLYGLOT_DEBUG(wxT("%s/%s exist %s"),OPOLYGLOT_USER_DATA,child->GetAttribute(wxT("fileCheckForInstall")),OPOLYGLOT_BOOL_TO_STRING(ret));
			}
		}
	}
	return ret;
}
