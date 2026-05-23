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
#include <wx/sstream.h>
#include <cwchar>
#include <random>

wxString GenerateUUIDv4() 
{
    std::random_device rd;
    // Використовуємо mt19937 для якісної генерації псевдовипадкових чисел
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<int> hex_dist(0, 15);
    std::uniform_int_distribution<int> variant_dist(8, 11); // Діапазон для 8, 9, A, B

    wxString uuid;
    // Резервуємо пам'ять, щоб уникнути зайвих алокацій
    uuid.Alloc(36); 

    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            uuid += "-";
        } else if (i == 14) {
            uuid += "4"; // Обов'язковий маркер UUID версії 4
        } else if (i == 19) {
            // Обов'язковий маркер варіанту (може бути лише 8, 9, a або b)
            uuid += wxString::Format("%x", variant_dist(gen)); 
        } else {
            // Будь-який інший hex-символ
            uuid += wxString::Format("%x", hex_dist(gen));
        }
    }

    return uuid;
}


int wxCMPFUNC_CONV CompareLocaleNoCase(const wxString& first, const wxString& second)
{
    wxString f = first.Lower();
    wxString s = second.Lower();
    return std::wcscoll(f.wc_str(), s.wc_str());
}

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
					languageFrom.Add(childLanguage->GetAttribute(OPOLYGLOT_XML_ATTRIBUTE_FROM));
				}
			}
		}
	}
	languageFrom.Sort();
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


wxString OPolyglotGetErrorXml(wxString errorString)
{
	wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE, wxS("Error"));
	errorNode->AddAttribute(wxS("value"),errorString);
	wxString str = wxEmptyString;
	wxStringOutputStream sos(&str);
	wxXmlDocument docError;
	docError.SetRoot(errorNode);
	docError.Save(sos);
	return str;
}

#define OPOLYGLOT_LIST_ORIGINAL_NAME_LANGUAGES \
		wxT("Albanian"),wxT("Arabic"),wxT("Azerbaijani"),wxT("Belarusian"),wxT("Bulgarian"),wxT("Chinese"), \
		wxT("Croatian"),wxT("Czech"),wxT("Danish"),wxT("Dutch"),wxT("English"),wxT("Estonian"),wxT("Finnish"),wxT("French"),wxT("Georgian"), \
		wxT("German"),wxT("Greek"),wxT("Hebrew"),wxT("Hungarian"),wxT("Icelandic"),wxT("Indonesian"),wxT("Italian"),wxT("Japanese"), \
		wxT("Korean"),wxT("Latvian"),wxT("Lithuanian"),wxT("Norwegian Bokmål"),wxT("Norwegian Nynorsk"),wxT("Norwegian"),wxT("Persian"),wxT("Polish"), \
		wxT("Portuguese"),wxT("Romanian"),wxT("Russian"),wxT("Serbian"),wxT("Slovak"),wxT("Slovenian"),wxT("Spanish"),wxT("Swedish"),wxT("Turkish"), \
		wxT("Ukrainian"),wxT("Vietnamese")

#define OPOLIGLOT_LIST_TRANSLATED_NAME_LANGUAGES \
		_("Albanian"),_("Arabic"),_("Azerbaijani"),_("Belarusian"),_("Bulgarian"),_("Chinese"), \
		_("Croatian"),_("Czech"),_("Danish"),_("Dutch"),_("English"),_("Estonian"),_("Finnish"),_("French"),_("Georgian"), \
		_("German"),_("Greek"),_("Hebrew"),_("Hungarian"),_("Icelandic"),_("Indonesian"),_("Italian"),_("Japanese"), \
		_("Korean"),_("Latvian"),_("Lithuanian"),_("Norwegian Bokmål"),_("Norwegian Nynorsk"),_("Norwegian"),_("Persian"),_("Polish"), \
		_("Portuguese"),_("Romanian"),_("Russian"),_("Serbian"),_("Slovak"),_("Slovenian"),_("Spanish"),_("Swedish"),_("Turkish"), \
		_("Ukrainian"),_("Vietnamese") 
	

wxArrayString OPolyglotGetTranslatedLanguages(wxArrayString input)
{
	wxArrayString retValue;
	wxArrayString original = { OPOLYGLOT_LIST_ORIGINAL_NAME_LANGUAGES };
	wxArrayString translated  = { OPOLIGLOT_LIST_TRANSLATED_NAME_LANGUAGES };
	for(size_t i =0; i < input.GetCount();i++)
	{
		if(original.Index(input.Item(i)) != wxNOT_FOUND)
		{
			retValue.Add(translated.Item(original.Index(input.Item(i))));
		} else
		{
			retValue.Add(input.Item(i));
		}
	}
	retValue.Sort();
	return retValue;
}

wxString OPolyglotGetOriginalLanguage(wxString input)
{
	wxString retValue = wxEmptyString;
	wxArrayString original = { OPOLYGLOT_LIST_ORIGINAL_NAME_LANGUAGES };
	wxArrayString translated  = { OPOLIGLOT_LIST_TRANSLATED_NAME_LANGUAGES };
	if(translated.Index(input) != wxNOT_FOUND)
	{
		retValue = original.Item(translated.Index(input));
	} else
	{
		retValue = input;
	}
	return retValue;
}


wxString OPolyglotGetTranslateLanguage(wxString input)
{
	wxString retValue = wxEmptyString;
	wxArrayString original = { OPOLYGLOT_LIST_ORIGINAL_NAME_LANGUAGES };
	wxArrayString translated  = { OPOLIGLOT_LIST_TRANSLATED_NAME_LANGUAGES };
	if(original.Index(input) != wxNOT_FOUND)
	{
		retValue = translated.Item(original.Index(input));
	} else
	{
		retValue = input;
	}
	if(retValue.IsEmpty())
	{
		retValue = input;
	}
	return retValue;
}
