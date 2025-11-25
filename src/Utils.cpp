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


wxXmlNode *OPolyglotGetNodeFromName(wxXmlDocument *doc,wxString name)
{
	wxXmlNode *findNode = NULL;
	for(wxXmlNode *node = doc->GetRoot()->GetChildren();node&&(findNode == NULL);node=node->GetNext())
	{
		if(node->GetName().IsSameAs(name))
		{
			findNode = node;
		}
	}
	return findNode;
}

wxXmlNode *OPolyglotGetNodeFromId(wxXmlDocument *doc,wxString id)
{
	wxXmlNode *findNode = NULL;
	for(wxXmlNode *node = doc->GetRoot()->GetChildren();node&&(findNode == NULL);node=node->GetNext())
	{
		if(id.IsSameAs(node->GetAttribute(wxT("id"))))
		{
			findNode = node;
		}
	}
	return findNode;
}


wxString OPolyglotGetTypeModelFromNode(wxXmlDocument *doc,wxXmlNode *nodeLanguage)
{
	wxString model = wxEmptyString;
	if(nodeLanguage->GetName().IsSameAs(wxS("Language")))
	{
		for(wxXmlNode *node = nodeLanguage->GetChildren(); node&&model.IsEmpty();node = node->GetNext()) 
		{
			if(node->GetName().IsSameAs(wxS("Id")))
			{
				wxXmlNode *url = OPolyglotGetNodeFromId(doc,node->GetNodeContent());
				if(url->GetName().IsSameAs(wxS("Url")))
				{
					wxString t = url->GetAttribute(wxS("file")).BeforeFirst(wxT('.'));
					if(!t.IsSameAs(wxT("full")))
					{
						model = t;
					}
				} else
				{
					OPOLYGLOT_ERROR(wxS("error from node %s not Url"),node->GetNodeContent());
				}
			}
		}
	} else
	{
		OPOLYGLOT_ERROR(wxT("error node not \"Language\" \"%s\""),nodeLanguage->GetName());
	}
	return model;
}


bool OPolyglotCheckThatLanguageInstalled(wxXmlDocument *doc,wxXmlNode *nodeLanguage)
{
	bool flagInstalled = true;
	if(!nodeLanguage->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_LANGUAGE))
	{
		return false;
	}
	for(wxXmlNode *nodeId=nodeLanguage->GetChildren();(flagInstalled)&&nodeId;nodeId = nodeId->GetNext())
	{
		flagInstalled = false;
		if(nodeId->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_ID))
		{
			for(wxXmlNode *child=OPolyglotGetNodeFromName(doc,OPOLYGLOT_NAME_NODE_INSTALLED)->GetChildren();child&&(!flagInstalled);child=child->GetNext())
			{
				if(child->GetAttribute(wxS("id")).IsSameAs(nodeId->GetNodeContent()))
				{
					flagInstalled = true;
				}
			}
		}
	}
	return flagInstalled;
}
