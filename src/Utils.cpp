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
#include <wx/config.h>


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
				wxXmlNode *url = OPolyglotGetNodeFromId(doc,node->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_ID));
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
	if(!nodeLanguage->GetName().IsSameAs(OPOLYGLOT_XML_NODE_LANGUAGE))
	{
		return false;
	}
	for(wxXmlNode *nodeId=nodeLanguage->GetChildren();(flagInstalled)&&nodeId;nodeId = nodeId->GetNext())
	{
		flagInstalled = false;
		if(nodeId->GetName().IsSameAs(OPOLYGLOT_XML_NODE_ID))
		{
			for(wxXmlNode *child=OPolyglotGetNodeFromName(doc,OPOLYGLOT_XML_NODE_INSTALLED)->GetChildren();child&&(!flagInstalled);child=child->GetNext())
			{
				if(child->GetAttribute(wxS("id")).IsSameAs(nodeId->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_ID)))
				{
					flagInstalled = true;
				}
			}
		}
	}
	return flagInstalled;
}

wxArrayString	OPolyglotGetInstalledLanguagesFrom()
{
	wxArrayString installedFiles;
	wxXmlDocument doc;
	wxArrayString languageFrom;
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotGetInstalledLanguagesFrom don`t load %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		return languageFrom;
	}
	for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(OPOLYGLOT_XML_NODE_INSTALLED))
		{
			for(wxXmlNode *childInstalled = child->GetChildren();childInstalled;childInstalled = childInstalled->GetNext())
			{
				if(childInstalled->GetName().IsSameAs(OPOLYGLOT_XML_NODE_ID_INSTALLED))
				{
					installedFiles.Add(childInstalled->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_ID));
				}
			}
		}
	}
	OPOLYGLOT_DEBUG(wxT("OPolyglotGetInstalledLanguagesFrom count ids installed %ld"),installedFiles.GetCount());
	for(wxXmlNode *childLanguage=doc.GetRoot()->GetChildren();childLanguage;childLanguage=childLanguage->GetNext())
	{
		if(childLanguage->GetName().IsSameAs(OPOLYGLOT_XML_NODE_LANGUAGE))
		{
			bool flagInstalled = true;
			for(wxXmlNode *childId=childLanguage->GetChildren();childId&&flagInstalled;childId=childId->GetNext())
			{
				if(childId->GetName().IsSameAs(OPOLYGLOT_XML_NODE_ID))
				{
					flagInstalled = installedFiles.Index(childId->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_ID)) != wxNOT_FOUND;
				}
			}
			if(flagInstalled)
			{
				if(languageFrom.Index(childLanguage->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_FROM)) == wxNOT_FOUND)
				{
					OPOLYGLOT_DEBUG(wxT("ADD"));
					languageFrom.Add(childLanguage->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_FROM));
				}
			}
		}
	}
	languageFrom.Sort();
	OPOLYGLOT_DEBUG(wxT("%ld"),languageFrom.GetCount());
	return languageFrom;
}

wxArrayString OPolyglotGetInstalledLanguagesTo(wxString languageFrom)
{
	wxArrayString installedFiles;
	wxXmlDocument doc;
	wxArrayString languageTo;
	wxArrayString languageFromTo;
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotGetInstalledLanguagesFrom don`t load %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		return languageTo;
	}
	for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(OPOLYGLOT_XML_NODE_INSTALLED))
		{
			for(wxXmlNode *childInstalled = child->GetChildren();childInstalled;childInstalled = childInstalled->GetNext())
			{
				if(childInstalled->GetName().IsSameAs(OPOLYGLOT_XML_NODE_ID_INSTALLED))
				{
					installedFiles.Add(childInstalled->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_ID));
				}
			}
		}
		if(child->GetName().IsSameAs(OPOLYGLOT_XML_NODE_LANGUAGE)&&child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_FROM).IsSameAs(languageFrom))
		{
			if(languageFromTo.Index(child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_TO)) == wxNOT_FOUND)
			{
				languageFromTo.Add(child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_TO));
			}
		}
	}
	OPOLYGLOT_DEBUG(wxT("OPolyglotGetInstalledLanguagesTo count ids installed %ld languageFromTo %ld"),installedFiles.GetCount(),languageFromTo.GetCount());
	for(wxXmlNode *childLanguage=doc.GetRoot()->GetChildren();childLanguage;childLanguage=childLanguage->GetNext())
	{
		if(childLanguage->GetName().IsSameAs(OPOLYGLOT_XML_NODE_LANGUAGE))
		{
			bool flagInstalled = true;
			for(wxXmlNode *childId=childLanguage->GetChildren();childId&&flagInstalled;childId=childId->GetNext())
			{
				if(childId->GetName().IsSameAs(OPOLYGLOT_XML_NODE_ID))
				{
					flagInstalled = installedFiles.Index(childId->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_ID)) != wxNOT_FOUND;
				}
			}
			if(flagInstalled)
			{
				if((languageFromTo.Index(childLanguage->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_FROM)) != wxNOT_FOUND)||(languageFrom.IsSameAs(childLanguage->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_FROM))))
				{
					if((languageTo.Index(childLanguage->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_TO)) == wxNOT_FOUND)&&(!languageFrom.IsSameAs(childLanguage->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_TO))))
					{
						languageTo.Add(childLanguage->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_TO));
					}
				}
			}
		}
	}
	return languageTo;
}

wxString OPolyglotGetCodeFromLanguage(wxString language)
{
	wxString code =wxEmptyString;
	wxXmlDocument doc;
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotGetCodeFromLanguage not read file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		return code;
	}
	for(wxXmlNode *child=doc.GetRoot()->GetChildren();child&&(code.IsEmpty());child=child->GetNext())
	{
		if(child->GetName().IsSameAs(OPOLYGLOT_XML_NODE_LANGUAGE))
		{
			if(child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_FROM).IsSameAs(language))
			{
				code= child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_CODE_FROM);
			}
			if(child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_TO).IsSameAs(language))
			{
				code= child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_CODE_TO);
			}
			
		}
	}
	return code;
}

wxArrayString OPolyglotCreateConfigsFromBergamot(wxString languageFrom,wxString languageTo)
{
	wxString codeFrom,codeTo;
	wxXmlDocument doc;
	wxArrayString configs;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotCreateConfigsFromBergamot %s -> %s"),languageFrom,languageTo);
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotCreateConfigsFromBergamot not read file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		return configs;
	}
	for(wxXmlNode *child=doc.GetRoot()->GetChildren();child;child=child->GetNext())
	{
		if(child->GetName().IsSameAs(OPOLYGLOT_XML_NODE_LANGUAGE))
		{
			if(child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_FROM).IsSameAs(languageFrom))
			{
				codeFrom = child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_CODE_FROM);
			}
			if(child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_TO).IsSameAs(languageTo))
			{
				codeTo = child->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_CODE_TO);
			}
			
		}
	}
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	bool flagBest = config.Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(wxT("BEST"));
	if(flagBest)
	{
		wxString fileName = wxString::Format(wxT("%s/base.%s%s/config.yml"),OPOLYGLOT_USER_DATA,codeFrom,codeTo);
		if(wxFileName::FileExists(fileName))
		{
			configs.Add(fileName);
		}
		fileName = wxString::Format(wxT("%s/base-memory.%s%s/config.yml"),OPOLYGLOT_USER_DATA,codeFrom,codeTo);
		if((configs.GetCount() == 0)&&wxFileName::FileExists(fileName))
		{
			configs.Add(fileName);
		}
		fileName = wxString::Format(wxT("%s/tiny.%s%s/config.yml"),OPOLYGLOT_USER_DATA,codeFrom,codeTo);
		if((configs.GetCount() == 0)&&wxFileName::FileExists(fileName))
		{
			configs.Add(fileName);
		}
	} else
	{
		wxString fileName = wxString::Format(wxT("%s/tiny.%s%s/config.yml"),OPOLYGLOT_USER_DATA,codeFrom,codeTo);
		if(wxFileName::FileExists(fileName))
		{
			configs.Add(fileName);
		}
		fileName = wxString::Format(wxT("%s/base-memory.%s%s/config.yml"),OPOLYGLOT_USER_DATA,codeFrom,codeTo);
		if((configs.GetCount() == 0)&&wxFileName::FileExists(fileName))
		{
			configs.Add(fileName);
		}
		fileName = wxString::Format(wxT("%s/base.%s%s/config.yml"),OPOLYGLOT_USER_DATA,codeFrom,codeTo);
		if((configs.GetCount() == 0)&&wxFileName::FileExists(fileName))
		{
			configs.Add(fileName);
		}
	}
	if(0 == configs.GetCount())
	{
		wxString codeToEng = codeFrom+wxS("eng");
		wxString codeFromEng = wxS("eng")+codeTo;
		if(flagBest)
		{
			wxString fileName = wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng);
			if(wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
			fileName = wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng);
			if((configs.GetCount() == 0)&&wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
			fileName = wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng);
			if((configs.GetCount() == 0)&&wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
			fileName = wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng);
			if((configs.GetCount() == 1)&&wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
			fileName = wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng);
			if((configs.GetCount() == 1)&&wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
			fileName = wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFrom,codeFromEng);
			if((configs.GetCount() == 1)&&wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
		} else
		{
			wxString fileName = wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng);
			if(wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
			fileName = wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng);
			if((configs.GetCount() == 0)&&wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
			fileName = wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng);
			if((configs.GetCount() == 0)&&wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
			fileName = wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng);
			if((configs.GetCount() == 1)&&wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
			fileName = wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng);
			if((configs.GetCount() == 1)&&wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
			fileName = wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFrom,codeFromEng);
			if((configs.GetCount() == 1)&&wxFileName::FileExists(fileName))
			{
				configs.Add(fileName);
			}
		}
	}
	if(configs.GetCount() == 0)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotCreateConfigsFromBergamot not created config for %s -> %s"),codeFrom,codeTo);
	}
	return configs;
}


