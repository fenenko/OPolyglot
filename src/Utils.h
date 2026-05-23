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


#pragma once
#include <wx/artprov.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/xml/xml.h>
#include "Config.h"


#define CLASS_NAME typeid(*this).name()



#define OPOLYGLOT_MESSAGE(msg,...) \
	wxLogMessage(wxT("\t\t%s:%d\t\t" msg),__FILE__ ,__LINE__,##__VA_ARGS__) ; wxLog::FlushActive()


#define OPOLYGLOT_WARNING(msg,...) \
	wxLogWarning(wxT("\t%s:%d\t\t" msg),__FILE__,__LINE__,##__VA_ARGS__) ; wxLog::FlushActive()


#define OPOLYGLOT_ERROR(msg,...) \
	wxLogError(wxT("\t%s:%d\t\t" msg),__FILE__,__LINE__,##__VA_ARGS__); \
	wxLog::FlushActive()



#define OPOLYGLOT_INFO(msg,...) \
	wxLogInfo(wxT("\t\t%s:%d\t\t" msg),__FILE__,__LINE__,##__VA_ARGS__)


#if OPOLYGLOT_DEBUG_ENABLED  
#define OPOLYGLOT_DEBUG(msg,...) \
	wxLogDebug(wxT("\t%s:%d\t\t" msg),__FILE__,__LINE__,##__VA_ARGS__); \
	wxLog::FlushActive()
#else
#define OPOLYGLOT_DEBUG(msg,...) \
	/* msg */
#endif

#define OPOLYGLOT_BOOL_TO_STRING(VALUE_BOOL)		VALUE_BOOL ? wxS("TRUE") : wxS("FALSE")


#define OPOLYGLOT_ART_CLEAR		wxART_MAKE_ART_ID(OPOLYGLOT_ART_CLEAR)
#define OPOLYGLOT_ART_RECHANGE	wxART_MAKE_ART_ID(OPOLYGLOT_ART_RECHANGE)

wxString GenerateUUIDv4(); 

int wxCMPFUNC_CONV CompareLocaleNoCase(const wxString& first, const wxString& second);

wxLogLevel OPolyglotGetLogLevel(wxString logLevel);

wxArrayString OPolyglotGetInstalledLanguagesFrom();

wxArrayString OPolyglotGetInstalledLanguagesTo(wxString languageFrom);

wxArrayString OPolyglotCreateConfigsFromBergamot(wxString languageFrom,wxString languageTo);

wxArrayString OPolyglotGetTranslatedLanguages(wxArrayString input);

wxString OPolyglotGetOriginalLanguage(wxString input);

wxString OPolyglotGetTranslateLanguage(wxString input);

wxString OPolyglotGetCodeFromLanguage(wxString language);

wxString OPolyglotGetErrorXml(wxString errorString);

#define OPOLYGLOT_XML_NODE_ID					wxS("Id")

#define OPOLYGLOT_XML_NODE_ID_INSTALLED		wxS("IdInstalled")

#define OPOLYGLOT_XML_NODE_INSTALLED			wxS("Installed")

#define OPOLYGLOT_XML_NODE_LANGUAGE			wxS("Language")

#define OPOLYGLOT_XML_ATTRIBUTE_CODE_FROM		wxS("codeFrom")
#define OPOLYGLOT_XML_ATTRIBUTE_CODE_TO 		wxS("codeTo")

#define OPOLYGLOT_XML_ATTRIBUTE_FROM			wxS("from")

#define OPOLYGLOT_XML_ATTRIBUTE_TO				wxS("to")

#define OPOLYGLOT_XML_ATTRIBUTE_ID				wxS("id")

#define OPOLYGLOT_ATTRIBUTE_NODE_URL			wxS("url")

#define OPOLYGLOT_NAME_NODE_PREPROCESSING		wxS("RulesPreProcessing")

#define OPOLYGLOT_NAME_NODE_POSTPROCESSING		wxS("RulesPostProcessing")

#define OPOLYGLOT_NAME_NODE_RULE				wxS("Rule")

#define OPOLYGLOT_ATTRIBUTE_NODE_REGULAR		wxS("regEx")

#define OPOLYGLOT_ATTRIBUTE_NODE_REPLACEMENT	wxS("replaceRule")

#define IS_NULLPTR(VALUE_PTR)		(((void *)VALUE_PTR == NULL)||((void *)VALUE_PTR == nullptr))
