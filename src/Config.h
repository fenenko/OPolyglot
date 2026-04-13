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
#include <wx/language.h>


#define OPOLYGLOT_DEBUG_ENABLED 0 							/* debug log enable 1 disable 0 default 0*/

#define OPOLYGLOT_LIBRARY									wxS("libopolyglot")
#define OPOLYGLOT_TIMEOUT_DOWNLOAD							30000
#define OPOLYGLOT_CERT_FILE_PATH							wxS("./cacert.pem")

#ifdef __WXGTK__
#define OPOLYGLOT_CONFIG_ARGUMENT							wxT("opolyglot"),wxT("Oleksandr Fenenko"),wxT(".opolyglot/config")
#endif
#ifdef __WXMSW__
#define OPOLYGLOT_CONFIG_ARGUMENT							wxT("opolyglot"),wxT("Oleksandr Fenenko"),wxT("OPolyglot")
#endif
#define OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP					wxT("StayOnTop")
#define OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT			true
#define OPOLYGLOT_CONFIG_STRING_LOG_LEVEL					wxT("LogLevel")
#if OPOLYGLOT_DEBUG_ENABLED
	#define OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT			wxT("MESSAGE")
#else
	#define OPOLYGLOT_CONFIG_STRING_LOG_LEVEL_DEFAULT			wxT("ERROR")
#endif
#define OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM				wxT("LanguageFrom")
#define OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT			wxT("English")
#define OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO					wxT("LanguageTo")
#define OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD			wxT("TranslationMethod")
#define OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT	wxT("BEST")
#define OPOLYGLOT_CONFIG_STRING_OCR_METHOD					wxT("OCRMethod")
#define OPOLYGLOT_CONFIG_STRING_OCR_METHOD_DEFAULT			wxT("BEST")
#define OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE			wxT("LanguageInterface")
#define OPOLYGLOT_CONFIG_STRING_LANGUAGE_INTERFACE_DEFAULT	wxLANGUAGE_DEFAULT
#define OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING			wxS("EnablePreprocessing")
#define OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT	true
#define OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING			wxS("EnablePostprocessing")
#define OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT	false
#define OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR				wxT("AdditionalOCR")
#define OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR_DEFAULT		wxT("NONE")



