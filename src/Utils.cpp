/*
 * Copyright 2026 Fenenko Oleksandr.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


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
			}
		}
	}
	return ret;
}

bool OPolyglotCheckForInstallFile(wxXmlNode *node)
{
	if(node == NULL)
	{
		wxLogError(wxT("'\t:%s:%d:%s node NULL"),__FILE__,__LINE__,__FUNCTION__);
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
				wxXmlNode *url = OPolyglotGetNodeFromId(doc,node->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_ID));
				if(url->GetName().IsSameAs(wxS("Url")))
				{
					wxString t = url->GetAttribute(wxS("file")).BeforeFirst(wxT('.'));
					if(!t.IsSameAs(wxT("full")))
					{
						model = t;
					}
				} else
				{
					OPOLYGLOT_ERROR_FOR_FUNC(wxS("error from node %s not Url"),node->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_URL));
				}
			}
		}
	} else
	{
		OPOLYGLOT_ERROR_FOR_FUNC(wxT("error node not \"Language\" \"%s\""),nodeLanguage->GetName());
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
				if(child->GetAttribute(wxS("id")).IsSameAs(nodeId->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_ID)))
				{
					flagInstalled = true;
				}
			}
		}
	}
	return flagInstalled;
}
