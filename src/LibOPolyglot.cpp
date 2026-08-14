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


#include <malloc.h>
#include "translator/byte_array_util.h"
#include "translator/parser.h"
#include "translator/response.h"
#include "translator/response_options.h"
#include "translator/utils.h"
#include "common/version.h"
#include "marian.h"
#include "translator/service.h"
#include <wx/string.h>
#include <wx/thread.h>
#include <wx/msgqueue.h>
#include <wx/sstream.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <wx/xml/xml.h>
#include <wx/image.h>
#include <wx/thread.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/ffile.h>
#include "Utils.h"



static marian::bergamot::BlockingService *serviceTranslator = nullptr;
static marian::bergamot::ConfigParser<marian::bergamot::BlockingService> *configParser = nullptr;
static wxMutex 			mutexOCR;
static wxMutex			mutexTranslate;

wxString LibOPolyglotOCR(void *inputPixImage,wxString dirTesstdata,wxString langCode)
{
	wxMutexLocker lock(mutexOCR);
	OPOLYGLOT_MESSAGE(wxT("LibOPolyglotOCR PIX %s %s"),dirTesstdata,langCode);
	tesseract::TessBaseAPI *ocrEngine = new tesseract::TessBaseAPI();
	if(ocrEngine->Init(dirTesstdata.utf8_str().data(),langCode.utf8_str().data(),tesseract::OEM_LSTM_ONLY))
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotOCR not init ocrEngine"));
		return wxEmptyString;
	}
	ocrEngine->SetImage(static_cast<PIX *>(inputPixImage));
	ocrEngine->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	char *outText = ocrEngine->GetUTF8Text();
	wxString ret = wxString(outText,wxConvUTF8);
	delete[] outText;
	ocrEngine->End();
	delete ocrEngine;
	return ret;
}


void LibOPolyglotFree()
{
	wxMutexLocker lock(mutexTranslate);
	if(!IS_NULLPTR(serviceTranslator))
	{
		delete serviceTranslator;
		serviceTranslator = nullptr;
	}
	if(!IS_NULLPTR(configParser))
	{
		delete configParser;
		configParser = nullptr;
	}
}

wxString LibOPolyglotTranslator(wxString inputText,wxArrayString filesYml)
{
	using namespace marian::bergamot;
	wxMutexLocker lock(mutexTranslate);
	OPOLYGLOT_MESSAGE(wxT("LibOPolyglotTranslator %zu"),filesYml.GetCount());
	if(serviceTranslator == nullptr)
	{

		char *argv[] ={
			(char *)"OPolyglot",
			(char *)"--log-level",
			(char *)"err", /* trace,debug,info,warn,err(or),critical, off*/
			nullptr
		};
		OPOLYGLOT_DEBUG(wxT("LibOPolyglotTranslatorText start init BlockingService"));
		if(IS_NULLPTR(configParser))
		{
			configParser = new ConfigParser<BlockingService>("OPolyglot",false);
			configParser->parseArgs(3,argv);
		}
		try {
			serviceTranslator = new BlockingService(configParser->getConfig().serviceConfig);
		} catch (const std::exception& e) {
			OPOLYGLOT_ERROR(wxT("LibOPolyglotTranslatorText not init BlockingService %s "),wxString(e.what()));
			return wxEmptyString;
		}
	}
	std::vector<std::string> sources;
	std::vector<ResponseOptions> responseOpt;
	sources.push_back(inputText.utf8_str().data());
	responseOpt.emplace_back();
	ResponseOptions responseOptions;
	std::vector<Response> responses;
	if(filesYml.GetCount() == 1)
	{
		std::shared_ptr<TranslationModel> model = marian::New<TranslationModel>(parseOptionsFromFilePath(filesYml.Item(0).utf8_str().data()));
		responses = serviceTranslator->translateMultiple(model,std::move(sources),responseOpt);
		model.reset();
	} else
	{
		std::shared_ptr<TranslationModel> model = marian::New<TranslationModel>(parseOptionsFromFilePath(filesYml.Item(0).utf8_str().data()));// service.createCompatibleModel(options);
		std::shared_ptr<TranslationModel> modelSecond = marian::New<TranslationModel>(parseOptionsFromFilePath(filesYml.Item(1).utf8_str().data()));// service.createCompatibleModel(options);
		responses = serviceTranslator->pivotMultiple(model,modelSecond,std::move(sources),responseOpt);
		modelSecond.reset();
		model.reset();
	}
	
	wxString retVal = wxString(responses.front().target.text.c_str(),wxConvUTF8);
	responses.erase(responses.begin());
	OPOLYGLOT_DEBUG(wxT("LibOPolyglotTranslatorText FINISH"));
	return retVal;
}

wxXmlNode* LibOPolyglotOCRAndTranslate(void *inputPix,wxString& codeLanguageFrom,wxString& codeLanguageTo,bool onlyOCR)
{
	PIX *pixs = static_cast<PIX *>(inputPix);
	if(pixGetDepth(pixs) != 1)
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotOCRAndTranslate error input image is not binarize"));
		return NULL;
	}
	wxString langCode = codeLanguageFrom;
	wxString dirTraineddata = wxEmptyString;
	wxXmlNode *nodeResult = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Text"));
	nodeResult->AddAttribute(wxS("id"),GenerateUUIDv4());
	wxString idtiff = GenerateUUIDv4();
	wxString fileNameTiff = OPOLYGLOT_USER_DATA_IMG+wxFileName::GetPathSeparator()+idtiff+wxS(".tif");
	int result = pixWriteTiff(fileNameTiff.c_str(), pixs,IFF_TIFF_G4,"w");
	if(result == 0)
	{
		nodeResult->AddAttribute(wxS("idtiff"),idtiff);
	} else
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotOCRAndTranslate %d error saving file %s"),result,fileNameTiff);
		delete nodeResult;
		return NULL;
	}
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	if(config.Read(OPOLYGLOT_CONFIG_STRING_OCR_METHOD,OPOLYGLOT_CONFIG_STRING_OCR_METHOD_DEFAULT).IsSameAs(wxT("BEST")))
	{
		OPOLYGLOT_DEBUG(wxT("select BEST OCR %s"),OPOLYGLOT_GET_DIR_BEST_TRAINEDDATA);
		dirTraineddata = OPOLYGLOT_GET_DIR_BEST_TRAINEDDATA;
	} else
	{
		OPOLYGLOT_DEBUG(wxT("select FAST OCR %s"),OPOLYGLOT_GET_DIR_FAST_TRAINEDDATA);
		dirTraineddata = OPOLYGLOT_GET_DIR_FAST_TRAINEDDATA;
	}
	if(!wxFileName::FileExists(wxString::Format(wxT("%s/%s.traineddata"),dirTraineddata,langCode)))
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotOCRAndTranslate OCR config error not find :%s/%s.traineddata"),dirTraineddata,langCode);
		delete nodeResult;
		return NULL;
	}
	if(!config.Read(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR_DEFAULT).IsSameAs(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR_DEFAULT))
	{
		if(!OPolyglotGetCodeFromLanguage(OPolyglotGetOriginalLanguage(config.Read(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR))).IsSameAs(codeLanguageFrom))
		{
			langCode = langCode+"+"+OPolyglotGetCodeFromLanguage(OPolyglotGetOriginalLanguage(config.Read(OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR,OPOLYGLOT_CONFIG_STRING_ADDITIONAL_OCR_DEFAULT)));
		}
	}
	wxString res = LibOPolyglotOCR(pixs,dirTraineddata,langCode);
	nodeResult->AddAttribute(wxS("codeOCR"),langCode);
	if(res.IsEmpty())
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotOCRAndTranslate error LibOPolyglotOCR"));
		delete nodeResult;
		return NULL;
	}
	if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT))
	{
		res = OPolyglotPreProcessingText(res);
	}
	nodeResult->AddAttribute(wxS("original"),res);
	if(!onlyOCR)
	{
		wxArrayString configs = OPolyglotCreateConfigsFromBergamot(codeLanguageFrom,codeLanguageTo);
		wxString filesYml= configs.Item(0)+wxS("\n");
		if(2 == configs.GetCount())
		{
			filesYml += configs.Item(1);
		}
		res = LibOPolyglotTranslator(res,configs);
		if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT))
		{
			res = OPolyglotPostProcessingText(res);
		}
		if(res.IsEmpty())
		{
			OPOLYGLOT_ERROR(wxT("LibOPolyglotOCRAndTranslate error translating text"));
			delete nodeResult;
			return NULL;
		}
		nodeResult->AddAttribute(wxS("text"),res);
		nodeResult->AddAttribute(wxS("filesBergamot"),filesYml);
	} else
	{
		nodeResult->AddAttribute(wxS("onlyOCR"),wxS("true"));
	}
	OPOLYGLOT_DEBUG(wxT("LibOPolyglotOCRAndTranslate FINISH"));
	return nodeResult;
	
}
