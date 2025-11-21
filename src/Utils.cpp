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

bool OPolyglotCheckForInstallFile(wxXmlNode *node)
{
	if(node == NULL)
	{
		OPOLYGLOT_ERROR(wxT("node NULL"));
	}
	if(!node->GetName().IsSameAs(wxT("File")))
	{
		return false;
	}
#if 0
	OPOLYGLOT_DEBUG(wxT("%s"),node->GetAttribute(wxT("fileCheckForInstall")));
#endif
	return wxFileName::FileExists(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,node->GetAttribute(wxT("fileCheckForInstall"))));
}



wxXmlNode *OPolyglotGetNodeFile(wxXmlDocument *doc,wxString file_name)
{
	for(wxXmlNode *files = doc->GetRoot()->GetChildren();files;files = files->GetNext())
	{
		if(files->GetName().IsSameAs(wxT("Language")))
		{
			for(wxXmlNode *file = files->GetChildren();file; file = file->GetNext())
			{
				if(file->GetName().IsSameAs(wxT("File")))
				{
					if(file->GetNodeContent().IsSameAs(file_name))
					{
						return file;
					}
				}
			}
		}
	}
	OPOLYGLOT_WARNING(wxT("not find %s"),file_name);
	return NULL;
}
