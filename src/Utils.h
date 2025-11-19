#pragma once
#include <wx/log.h>
#include <wx/string.h>

#define OPOLYGLOT_MESSAGE(msg,...) \
	wxLogMessage(wxT("\t\t%s:%d:%s\t\t" msg),__FILE__ ,__LINE__, __FUNCTION__ ,##__VA_ARGS__)


#define OPOLYGLOT_WARNING(msg,...) \
	wxLogWarning(wxT("\t%s:%d:%s\t\t" msg),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)


#define OPOLYGLOT_ERROR(msg,...) \
	wxLogError(wxT("\t%s:%d:%s\t\t" msg),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)


#define OPOLYGLOT_INFO(msg,...) \
	wxLogInfo(wxT("\t%s:%d:%s\t\t" msg),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)

#define OPOLYGLOT_DEBUG(msg,...) \
	wxLogDebug(wxT("\t%s:%d:%s\t\t" msg),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)



#define OPOLYGLOT_USER_DIR		wxStandardPaths::Get().GetUserLocalDataDir()


#define OPOLYGLOT_USER_DATA 	wxString::Format(wxT("%s/data"),wxStandardPaths::Get().GetUserLocalDataDir())
#if 0
#define OPOLYGLOT_DIR_OUT_FROM_NODE_XML(NODE_XML)	wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,NODE_XML->GetAttribute(wxT("dirOut")))

#define OPOLYGLOT_FILE_OUT_FROM_NODE_XML(NODE_XML)	wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,NODE_XML->GetAttribute(wxT("fileOut")))
#endif

#define OPOLYGLOT_FILE_TRAINEDDATA_FOR_NODE_XML(LANGUAGE_NODE_XML)	wxString::Format(wxT("%s/%s.traineddata"),OPOLYGLOT_USER_DATA,LANGUAGE_NODE_XML->GetAttribute(wxT("ocr")))

#define OPOLYGLOT_CONFIG_FILE_TRANSLATOR_FOR_NODE_XML(LANGUAGE_NODE_XML) wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,LANGUAGE_NODE_XML->GetAttribute(wxT("configfile")))

#define OPOLYGLOT_GET_DIR_TRANSLATOR_FOR_NODE_XML(LANGUAGE_NODE_XML)	wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,LANGUAGE_NODE_XML->GetAttribute(wxT("dir")))

#define OPOLYGLOT_CHECKING_INSTALLE_LANGUAGE_FROM_NODE_XML(LANGUAGE_NODE_XML)	\
	(wxFileName::FileExists(OPOLYGLOT_FILE_TRAINEDDATA_FOR_NODE_XML(LANGUAGE_NODE_XML)) \
	 &&wxFileName::FileExists(OPOLYGLOT_CONFIG_FILE_TRANSLATOR_FOR_NODE_XML(LANGUAGE_NODE_XML)))
#if 0
			(wxFileName::FileExists(wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,NODE_XML->GetAttribute(wxT("configfile")))) \
					&&wxFileName::FileExists(wxString::Format(wxT("%s/%s.traineddata"),OPOLYGLOT_USER_DATA,NODE_XML->GetAttribute(wxT("ocrfile")))))
#endif


#define OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(LANGUAGE_NODE_XML) \
				wxString::Format(wxT("%s -> %s (%s)") \
						,LANGUAGE_NODE_XML->GetAttribute(wxT("from")) \
						,LANGUAGE_NODE_XML->GetAttribute(wxT("to")) \
						,LANGUAGE_NODE_XML->GetAttribute(wxT("type")))

#define OPOLYGLOT_LABEL_LANGUAGEFROM_FROM_NODE_XML(LANGUAGE_NODE_XML) \
				wxString::Format(wxT("%s"),LANGUAGE_NODE_XML->GetAttribute(wxT("from")))

#if 0
				wxString::Format(wxT("%s (%s)") \
						,LANGUAGE_NODE_XML->GetAttribute(wxT("from")) \
						,LANGUAGE_NODE_XML->GetAttribute(wxT("type")))
#endif


#define OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(LANGUAGE_NODE_XML) \
				wxString::Format(wxT("%s"),LANGUAGE_NODE_XML->GetAttribute(wxT("to")))
#if 0
				wxString::Format(wxT("%s (%s)") \
						,LANGUAGE_NODE_XML->GetAttribute(wxT("to")) \
						,LANGUAGE_NODE_XML->GetAttribute(wxT("type")))
#endif

#define OPOLYGLOT_LABEL_LANGUAGE_FROM_STRING(TYPE_LANG,FROM_LANG,TO_LANG)	wxString::Format(wxT("%s: %s -> %s"),TYPE_LANG,FROM_LANG,TO_LANG)

#define OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE		wxT("./res/download.xml")

#define OPOLYGLOT_BOOL_TO_STRING(VALUE_BOOL)		VALUE_BOOL ? wxT("TRUE") : wxT("FALSE")

wxLogLevel OPolyglotGetLogLevel(wxString logLevel);


