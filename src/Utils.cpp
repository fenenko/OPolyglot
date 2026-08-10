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
#include "OPolyglotEvent.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/config.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <cwchar>
#include <random>
#if __WXGTK__
	#include "../res/icon.xpm"
#endif


enum{
	TIMER_ID,
	TIMER_MOUSE_ID
};

wxString ConvertMdToHtml(const wxString& markdown) {
    wxString html = markdown;
    wxRegEx re;

    // 1. Image: ![alt](url)
    // It is important to process references to avoid conflicts
	re.Compile(wxT("!\\[([^\\]]+)\\]\\(([^\\)]+)\\)"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("<img src=\"\\2\" alt=\"\\1\" />"));

    // 2. Link: [text](url)
    re.Compile(wxT("\\[([^\\]]+)\\]\\(([^\\)]+)\\)"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("<a href=\"\\2\">\\1</a>"));

    // 3. Headings: H6 to H1
	// Use wxRE_NEWLINE so that the ^ symbol marks the beginning of a line, not the entire text
	re.Compile(wxT("^###### (.*?)$"), wxRE_ADVANCED | wxRE_NEWLINE); 
    re.ReplaceAll(&html, wxT("<h6>\\1</h6>"));
    
    re.Compile(wxT("^##### (.*?)$"), wxRE_ADVANCED | wxRE_NEWLINE);  
    re.ReplaceAll(&html, wxT("<h5>\\1</h5>"));
    
    re.Compile(wxT("^#### (.*?)$"), wxRE_ADVANCED | wxRE_NEWLINE);   
    re.ReplaceAll(&html, wxT("<h4>\\1</h4>"));
    
    re.Compile(wxT("^### (.*?)$"), wxRE_ADVANCED | wxRE_NEWLINE);    
    re.ReplaceAll(&html, wxT("<h3>\\1</h3>"));
    
    re.Compile(wxT("^## (.*?)$"), wxRE_ADVANCED | wxRE_NEWLINE);     
    re.ReplaceAll(&html, wxT("<h2>\\1</h2>"));
    
    re.Compile(wxT("^# (.*?)$"), wxRE_ADVANCED | wxRE_NEWLINE);      
    re.ReplaceAll(&html, wxT("<h1>\\1</h1>"));

	// 4. PRIMARY PROCESSING OF LISTS
	// 4.1. Nested lists (bulleted and numbered with indentation)
    re.Compile(wxT("^[ \\t]+[\\*\\-] (.*?)$"), wxRE_ADVANCED | wxRE_NEWLINE);
    re.ReplaceAll(&html, wxT("<ul_nested><li>\\1</li></ul_nested>"));
    re.Compile(wxT("^[ \\t]+[0-9]+\\. (.*?)$"), wxRE_ADVANCED | wxRE_NEWLINE);
    re.ReplaceAll(&html, wxT("<ol_nested><li>\\1</li></ol_nested>"));

    // 4.2. Basic lists (without indentation)
    re.Compile(wxT("^[\\*\\-] (.*?)$"), wxRE_ADVANCED | wxRE_NEWLINE);
    re.ReplaceAll(&html, wxT("<ul><li>\\1</li></ul>"));
    re.Compile(wxT("^[0-9]+\\. (.*?)$"), wxRE_ADVANCED | wxRE_NEWLINE);
    re.ReplaceAll(&html, wxT("<ol><li>\\1</li></ol>"));

    // 5. GLUING ADJACENT ELEMENTS OF THE SAME LEVEL
    re.Compile(wxT("<\\/ul_nested>\\n<ul_nested>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("\n"));
    re.Compile(wxT("<\\/ol_nested>\\n<ol_nested>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("\n"));
    
    re.Compile(wxT("<\\/ul>\\n<ul>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("\n"));
    re.Compile(wxT("<\\/ol>\\n<ol>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("\n"));

    // 6. BUILDING CORRECT SEMANTIC NESTING (DOM)
	// Step A: Open the parent <li> to receive the nested list
	// Remove the closing </li></ul> tags of the parent before opening the <ul_nested>
    re.Compile(wxT("<\\/li><\\/ul>\\n<ul_nested>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("\n<ul>\n"));
    re.Compile(wxT("<\\/li><\\/ol>\\n<ul_nested>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("\n<ul>\n"));
    re.Compile(wxT("<\\/li><\\/ul>\\n<ol_nested>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("\n<ol>\n"));
    re.Compile(wxT("<\\/li><\\/ol>\\n<ol_nested>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("\n<ol>\n"));

    // Step B: Return from the nested list back to the parent
	// Join the end of the nested list to the beginning of the next parent element
    re.Compile(wxT("<\\/ul_nested>\\n<ul>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("</ul>\n</li>\n"));
    re.Compile(wxT("<\\/ul_nested>\\n<ol>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("</ul>\n</li>\n"));
    re.Compile(wxT("<\\/ol_nested>\\n<ul>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("</ol>\n</li>\n"));
    re.Compile(wxT("<\\/ol_nested>\\n<ol>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("</ol>\n</li>\n"));

    // Step B: Final closing of remaining nested lists at the end of the block
    re.Compile(wxT("<\\/ul_nested>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("</ul>\n</li>\n</ul>"));
    re.Compile(wxT("<\\/ol_nested>"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("</ol>\n</li>\n</ol>"));

	// 7. Inline code: `code`
	// Execute before bold and italic to avoid conflicts
    re.Compile(wxT("`(.*?)`"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("<code>\\1</code>"));

    // 8. Bold text: **text**
	re.Compile(wxT("\\*\\*(.*?)\\*\\*"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("<strong>\\1</strong>"));

    // 9. Italics: *text*
	re.Compile(wxT("\\*(.*?)\\*"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("<em>\\1</em>"));

    // 10. Line breaks (optional: replace empty lines with <br> or <p>)
	// For easy visualization, replace regular line breaks with <br>
	re.Compile(wxT("([^>])\\n"), wxRE_ADVANCED);
    re.ReplaceAll(&html, wxT("\\1<br>\n"));

	wxString finalHtml = wxString::Format(
        wxT("<!DOCTYPE html>\n")
        wxT("<html lang=\"en\">\n")
        wxT("<head>\n")
        wxT("    <meta charset=\"UTF-8\">\n")
        wxT("    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n")
        wxT("    <title>%s</title>\n") 
        wxT("</head>\n")
        wxT("<body>\n")
        wxT("%s\n") 
        wxT("</body>\n")
        wxT("</html>"),
        wxS("README"), html
    );

    return finalHtml;
}

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
	codeFrom = wxEmptyString;
	codeTo = wxEmptyString;
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
	if(codeFrom.IsEmpty())
	{
		codeFrom = languageFrom;
	}
	if(codeTo.IsEmpty())
	{
		codeTo = languageTo;
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
		wxT("Ukrainian"),wxT("Vietnamese"),wxEmptyString

#define OPOLYGLOT_LIST_TRANSLATED_NAME_LANGUAGES \
		_("Albanian"),_("Arabic"),_("Azerbaijani"),_("Belarusian"),_("Bulgarian"),_("Chinese"), \
		_("Croatian"),_("Czech"),_("Danish"),_("Dutch"),_("English"),_("Estonian"),_("Finnish"),_("French"),_("Georgian"), \
		_("German"),_("Greek"),_("Hebrew"),_("Hungarian"),_("Icelandic"),_("Indonesian"),_("Italian"),_("Japanese"), \
		_("Korean"),_("Latvian"),_("Lithuanian"),_("Norwegian Bokmål"),_("Norwegian Nynorsk"),_("Norwegian"),_("Persian"),_("Polish"), \
		_("Portuguese"),_("Romanian"),_("Russian"),_("Serbian"),_("Slovak"),_("Slovenian"),_("Spanish"),_("Swedish"),_("Turkish"), \
		_("Ukrainian"),_("Vietnamese"),wxEmptyString
	

wxArrayString OPolyglotGetTranslatedLanguages(wxArrayString input)
{
	wxArrayString retValue;
	
	wxString original[] = { OPOLYGLOT_LIST_ORIGINAL_NAME_LANGUAGES };
	wxString translated[]  = { OPOLYGLOT_LIST_TRANSLATED_NAME_LANGUAGES };
	for(size_t i =0; i < input.GetCount();i++)
	{
		char flag = -1;
		for(size_t j  = 0; (!original[j].IsEmpty())&&(flag != 0);j++)
		{
			if(original[j].IsSameAs(input.Item(i)))
			{
				retValue.Add(translated[j]);
				flag = 0;
			}

		}
		if(flag != 0)
		{
			retValue.Add(input.Item(i));
		}
	}
	retValue.Sort();
	return retValue;
}

wxString OPolyglotGetOriginalLanguage(wxString input)
{
	wxString original[] = { OPOLYGLOT_LIST_ORIGINAL_NAME_LANGUAGES };
	wxString translated[]  = { OPOLYGLOT_LIST_TRANSLATED_NAME_LANGUAGES };
	OPOLYGLOT_DEBUG(wxT("OPolyglotGetOriginalLanguage %s"),input);
	for(size_t i = 0; !original[i].IsEmpty();i++)
	{
		if(translated[i].IsSameAs(input))
		{
			return original[i];
		} else
		{
			//OPOLYGLOT_DEBUG(wxT("%s"),translated[i]);
		}

	}
	OPOLYGLOT_ERROR(wxT("OPolyglotGetOriginalLanguage Not found %s"),input);
	return input;
}


wxString OPolyglotGetTranslateLanguage(wxString input)
{
	wxString original[] = { OPOLYGLOT_LIST_ORIGINAL_NAME_LANGUAGES };
	wxString translated[]  = { OPOLYGLOT_LIST_TRANSLATED_NAME_LANGUAGES };
	for(size_t i = 0; !original[i].IsEmpty();i++)
	{
		if(original[i].IsSameAs(input))
		{
			return translated[i];
		}
	}
	return input;
}

wxString OPolyglotPostProcessingText(wxString& textInput)
{
	wxArrayString postProcessingRegex;
	wxArrayString postProcessingReplace;
	wxXmlDocument docRegex;
	if(!docRegex.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotPostProcessingText not load %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		return wxEmptyString;
	}
	for(wxXmlNode *node = docRegex.GetRoot()->GetChildren();node;node = node->GetNext())
	{
		if(node->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_POSTPROCESSING))
		{
			for(wxXmlNode *rule = node->GetChildren();rule;rule = rule->GetNext())
			{
				if(rule->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_RULE))
				{
					wxString value;
					if(rule->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_REGULAR,&value))
					{
						postProcessingRegex.Add(value);
					} else
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotPostProcessingText %s attribute node regular is empty"),rule->GetName());
						return wxEmptyString;
					}
					if(rule->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_REPLACEMENT,&value))
					{
						postProcessingReplace.Add(value);
					} else
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotPostProcessingText %s attribute node replacement is empty"),rule->GetName());
						return wxEmptyString;
					}
				}
			}
		}
	}
	wxRegEx regex;
	wxString retVal = textInput.Clone();
	for(size_t i =0; i < postProcessingRegex.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("OnExitThreadTranslation %zu %s %s"),i+1,postProcessingRegex.Item(i),postProcessingReplace.Item(i));
		if(!regex.Compile(postProcessingRegex.Item(i)))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotPostProcessingText error compile rule %s"),postProcessingRegex.Item(i));
			return wxEmptyString;
		}
		wxString replace = postProcessingReplace.Item(i);
		replace.Replace(wxS("\\a"),"\a");
		replace.Replace(wxS("\\b"),"\b");
		replace.Replace(wxS("\\n"),"\n");
		replace.Replace(wxS("\\r"),"\r");
		replace.Replace(wxS("\\t"),"\t");
		replace.Replace(wxS("\\v"),"\v");
		replace.Replace(wxS("\\f"),"\f");
		(void)regex.ReplaceAll(&retVal,replace);
	}
	return retVal;
}

wxString OPolyglotPreProcessingText(wxString& textInput)
{
	wxXmlDocument doc;
	wxArrayString preProcessingRegex;
	wxArrayString preProcessingReplace;
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotPreProcessingText not load %s for read preProcessing rules"),OPOLYGLOT_GET_XML_DATA_FILE);
		return wxEmptyString;
	}
	for(wxXmlNode *node = doc.GetRoot()->GetChildren();node;node = node->GetNext())
	{
		if(node->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_PREPROCESSING))
		{
			for(wxXmlNode *rule = node->GetChildren();rule;rule = rule->GetNext())
			{
				if(rule->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_RULE))
				{
					wxString value;
					if(rule->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_REGULAR,&value))
					{
						preProcessingRegex.Add(value);
					} else
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotPreProcessingText %s attribute node regular is empty"),rule->GetName());
						return wxEmptyString;
					}
					if(rule->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_REPLACEMENT,&value))
					{
						preProcessingReplace.Add(value);
					} else
					{
						OPOLYGLOT_ERROR(wxT("OPolyglotPreProcessingText %s attribute node replacement is empty"),rule->GetName());
						return wxEmptyString;
					}
				}
			}
		}	
	}
	wxString retVal = textInput.Clone();
	wxRegEx regex;
	for(size_t i =0; (i < preProcessingRegex.GetCount());i++)
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotPreProcessingText rule %zu %s %s"),i+1,preProcessingRegex.Item(i),preProcessingReplace.Item(i));
		if(!regex.Compile(preProcessingRegex.Item(i)))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotPreProcessingText error compile rule %s"),preProcessingRegex.Item(i));
			return wxEmptyString;
		}
		wxString replace = preProcessingReplace.Item(i);
		replace.Replace(wxS("\\a"),"\a");
		replace.Replace(wxS("\\b"),"\b");
		replace.Replace(wxS("\\n"),"\n");
		replace.Replace(wxS("\\r"),"\r");
		replace.Replace(wxS("\\t"),"\t");
		replace.Replace(wxS("\\v"),"\v");
		replace.Replace(wxS("\\f"),"\f");
		int count = regex.ReplaceAll(&retVal,replace);
		OPOLYGLOT_MESSAGE(wxT("OPolyglotPreProcessingText pre processing replace %zu %d"),i,count);
	}
	return retVal;
}


OPolyglotDialogProgress::OPolyglotDialogProgress(wxWindow *parent,wxString label) : GUIOPolyglotDialogProgress(NULL)
{
	int w,h;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDialogProgress"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	this->SetTitle(wxT("OPolyglot"));
	this->parent = parent;
	timerUpdate.SetOwner(this,TIMER_ID);
	this->Bind(wxEVT_TIMER,&OPolyglotDialogProgress::OnUpdateProgress,this);
	timerUpdate.Start(200);
	ProgressLabel->SetLabel(label);
	this->vBox->Fit(this);
	this->vBox->Layout();
	this->GetSize(&w,&h);
	this->SetSize(480,h);
	this->Raise();
}


OPolyglotDialogProgress::~OPolyglotDialogProgress()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDialogProgress::~OPolyglotDialogProgress"));
}


void OPolyglotDialogProgress::OnCancel(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDialogProgress::OnCancel"));
	wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER));
}


void OPolyglotDialogProgress::OnUpdateProgress(wxTimerEvent &event)
{
	Progress->Pulse();
}


void OPolyglotDialogProgress::Finish()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotProgress::Finish"));
	this->Destroy();
}
