#pragma once
#include <wx/log.h>
#include <wx/string.h>
#include <wx/xml/xml.h>
#include "Config.h"


#define CLASS_NAME typeid(*this).name()


#define OPOLYGLOT_DEBUG_ENABLED 1 							/* debug log */

#define OPOLYGLOT_MESSAGE(msg,...) \
	wxLogMessage(wxT("\t\t%s:%d\t\t" msg),__FILE__ ,__LINE__,##__VA_ARGS__) ; wxLog::FlushActive()


#define OPOLYGLOT_WARNING(msg,...) \
	wxLogWarning(wxT("\t%s:%d\t\t" msg),__FILE__,__LINE__,##__VA_ARGS__) ; wxLog::FlushActive()


#define OPOLYGLOT_ERROR(msg,...) \
	wxLogError(wxT("\t%s:%d\t\t" msg),__FILE__,__LINE__,##__VA_ARGS__); \
	wxLog::FlushActive()

#define OPOLYGLOT_ERROR_FOR_FUNC(msg,...) \
	wxLogError(wxT("\t%s:%d\t\t" msg),__FILE__,__LINE__,##__VA_ARGS__)


#define OPOLYGLOT_INFO(msg,...) \
	wxLogInfo(wxT("\t\t%s:%d\t\t" msg),__FILE__,__LINE__,##__VA_ARGS__)


#if OPOLYGLOT_DEBUG_ENABLED  
#define OPOLYGLOT_DEBUG(msg,...) \
	wxLogDebug(wxT("\t%s:%d\t\t" msg),__FILE__,__LINE__,##__VA_ARGS__); \
	wxLog::FlushActive()
#else
    #define OPOLYGLOT_DEBUG(msg, ...) 
#endif


#define OPOLYGLOT_USER_DIR		wxStandardPaths::Get().GetUserLocalDataDir()


#define OPOLYGLOT_USER_DATA 	wxString::Format(wxT("%s/data"),OPOLYGLOT_USER_DIR)

#define OPOLYGLOT_LOG_FILENAME	wxString::Format(wxT("%s/log.txt"),OPOLYGLOT_USER_DATA)
#if 0
#define OPOLYGLOT_DIR_OUT_FROM_NODE_XML(NODE_XML)	wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,NODE_XML->GetAttribute(wxT("dirOut")))

#define OPOLYGLOT_FILE_OUT_FROM_NODE_XML(NODE_XML)	wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,NODE_XML->GetAttribute(wxT("fileOut")))
#endif

#define OPOLYGLOT_GET_DIR_BEST_TRAINEDDATA	wxString::Format(wxT("%s/tessdata/best"),OPOLYGLOT_USER_DATA)


#define OPOLYGLOT_GET_DIR_FAST_TRAINEDDATA		wxString::Format(wxT("%s/tessdata/fast"),OPOLYGLOT_USER_DATA)

#define OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(LANGUAGE_NODE_XML)	wxString::Format(wxT("%s/%s.traineddata"),OPOLYGLOT_GET_DIR_BEST_TRAINEDDATA,LANGUAGE_NODE_XML->GetAttribute(wxT("ocr")))


#define OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(LANGUAGE_NODE_XML)	wxString::Format(wxT("%s/%s.traineddata"),OPOLYGLOT_GET_DIR_FAST_TRAINEDDATA,LANGUAGE_NODE_XML->GetAttribute(wxT("ocr")))

#define OPOLYGLOT_CONFIG_FILE_TRANSLATOR_FOR_NODE_XML(LANGUAGE_NODE_XML) wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,LANGUAGE_NODE_XML->GetAttribute(wxT("configfile")))

#define OPOLYGLOT_GET_DIR_TRANSLATOR_FOR_NODE_XML(LANGUAGE_NODE_XML)	wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,LANGUAGE_NODE_XML->GetAttribute(wxT("dir")))


#if 0
#define OPOLYGLOT_CHECKING_INSTALLE_LANGUAGE_FROM_NODE_XML(LANGUAGE_NODE_XML)	\
	(wxFileName::FileExists(OPOLYGLOT_FILENAME_BEST_TRAINEDDATA_FRON_NODE_XML(LANGUAGE_NODE_XML)) \
	 &&wxFileName::FileExists(OPOLYGLOT_FILENAME_FAST_TRAINEDDATA_FRON_NODE_XML(LANGUAGE_NODE_XML)) \
	 &&wxFileName::FileExists(OPOLYGLOT_CONFIG_FILE_TRANSLATOR_FOR_NODE_XML(LANGUAGE_NODE_XML)))
#endif



#define OPOLYGLOT_LABEL_LANGUAGEFROM_FROM_NODE_XML(LANGUAGE_NODE_XML) \
				wxString::Format(wxT("%s"),LANGUAGE_NODE_XML->GetAttribute(wxT("from")))



#define OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(LANGUAGE_NODE_XML) \
				wxString::Format(wxT("%s"),LANGUAGE_NODE_XML->GetAttribute(wxT("to")))

#define OPOLYGLOT_LABEL_LANGUAGE_FROM_STRING(TYPE_LANG,FROM_LANG,TO_LANG)	wxString::Format(wxT("%s: %s -> %s"),TYPE_LANG,FROM_LANG,TO_LANG)

#define OPOLYGLOT_GET_XML_DATA_FILE				wxString::Format(wxT("%s/data.xml"),OPOLYGLOT_USER_DATA)
#ifdef __FLATPAK
#define OPOLYGLOT_GET_RES_XML_DATA_FILE			wxT("/app/share/download.xml")
#else
#define OPOLYGLOT_GET_RES_XML_DATA_FILE			wxT("./res/download.xml")
#endif

#ifdef __FLATPAK
#define OPOLYGLOT_LOCALE_DIR		wxT("/app/locale")
#else
#define OPOLYGLOT_LOCALE_DIR		wxT("./locale")
#endif

#ifdef __FLATPAK
#define OPOLYGLOT_ABOUT_FILE	wxT("/app/share/about.html")
#else
#define OPOLYGLOT_ABOUT_FILE	wxT("./res/about.html")
#endif

#define OPOLYGLOT_BOOL_TO_STRING(VALUE_BOOL)		VALUE_BOOL ? wxS("TRUE") : wxS("FALSE")

wxLogLevel OPolyglotGetLogLevel(wxString logLevel);

bool OPolyglotCheckForInstallLanguage(wxXmlNode *node);
bool OPolyglotCheckForInstallFile(wxXmlNode *node);


wxXmlNode *OPolyglotGetNodeFromId(wxXmlDocument *doc,wxString id);

wxXmlNode *OPolyglotGetNodeFromName(wxXmlDocument *doc,wxString name);

wxString OPolyglotGetTypeModelFromNode(wxXmlDocument *doc,wxXmlNode *nodeLanguage);

bool OPolyglotCheckThatLanguageInstalled(wxXmlDocument *doc,wxXmlNode *nodeLanguage);

#define OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(XML_DOCUMENT,LANGUAGE_NODE_XML) \
				wxString::Format(wxT("%s\t\t\t|%s -> %s | %s ") \
						,LANGUAGE_NODE_XML->GetAttribute(wxT("language"))\
						,LANGUAGE_NODE_XML->GetAttribute(wxT("from")) \
						,LANGUAGE_NODE_XML->GetAttribute(wxT("to")) \
						,OPolyglotGetTypeModelFromNode(XML_DOCUMENT,LANGUAGE_NODE_XML))

#define OPOLYGLOT_NAME_NODE_ID					wxS("Id")

#define OPOLYGLOT_NAME_NODE_ID_INSTALLED		wxS("IdInstalled")

#define OPOLYGLOT_NAME_NODE_INSTALLED			wxS("Installed")

#define OPOLYGLOT_NAME_NODE_LANGUAGE			wxS("Language")

#define OPOLYGLOT_ATTRIBUTE_NODE_CODE_FROM		wxS("codeFrom")
#define OPOLYGLOT_ATTRIBUTE_NODE_CODE_TO 		wxS("codeTo")

#define OPOLYGLOT_ATTRIBUTE_NODE_FROM			wxS("from")

#define OPOLYGLOT_ATTRIBUTE_NODE_TO				wxS("to")

#define OPOLYGLOT_ATTRIBUTE_NODE_ID				wxS("id")

#define OPOLYGLOT_ATTRIBUTE_NODE_URL			wxS("url")

#define OPOLYGLOT_NAME_NODE_PREPROCESSING		wxS("RulesPreProcessingText")

#define OPOLYGLOT_NAME_NODE_POSTPROCESSING		wxS("RulesPostProcessingText")

#define OPOLYGLOT_NAME_NODE_RULE				wxS("Rule")

#define OPOLYGLOT_ATTRIBUTE_NODE_REGULAR		wxS("regEx")

#define OPOLYGLOT_ATTRIBUTE_NODE_REPLACEMENT	wxS("replacement")

#define IS_NULLPTR(VALUE_PTR)		(((void *)VALUE_PTR == NULL)||((void *)VALUE_PTR == nullptr))
