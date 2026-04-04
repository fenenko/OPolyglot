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
#include "Utils.h"



static marian::bergamot::BlockingService *serviceTranslator = nullptr;
static wxMutex 			mutexOCR;
static wxMutex			mutexTranslate;

wxString LibOPolyglotOCR(wxString inputXml,wxString dirTesstdata,wxString langCode)
{
	wxMutexLocker lock(mutexOCR);
	OPOLYGLOT_MESSAGE(wxT("LibOPolyglotOCR %s %s"),dirTesstdata,langCode);
	tesseract::TessBaseAPI *ocrEngine = new tesseract::TessBaseAPI();
	wxString lang = langCode;
	int ret = ocrEngine->Init(dirTesstdata.utf8_str().data(),langCode.utf8_str().data(),tesseract::OEM_LSTM_ONLY );
	if(ret)
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotOCR not init ocrEngine"));
		wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE, wxS("Error"));
		errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::LibOPolyglotOCR\ninit ocrEngine for lang %s return %d"),langCode,ret));
		wxString str = wxEmptyString;
		wxStringOutputStream sos(&str);
		wxXmlDocument docError;
		docError.SetRoot(errorNode);
		docError.Save(sos);
		delete ocrEngine;
		return str;
	}
	wxStringInputStream sis(inputXml);
	wxXmlDocument	*inputDoc = new wxXmlDocument(sis);
	wxXmlNode 		*rootNode = inputDoc->GetRoot();
	if(!rootNode->GetName().IsSameAs(wxT("ScreenshotFile")))
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotOCR not valid root node "),rootNode->GetName());
		wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE, wxS("Error"));
		errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::LibOPolyglotOCR\nroot node not \"ScreenshotFile\" %s"),rootNode->GetName()));
		wxString str = wxEmptyString;
		wxStringOutputStream sos(&str);
		wxXmlDocument docError;
		docError.SetRoot(errorNode);
		docError.Save(sos);
		ocrEngine->End();
		delete ocrEngine;
		delete inputDoc;
		return str;
	}
	wxString fileName = rootNode->GetAttribute(wxS("fileName"));
	wxImage *image = new wxImage();
	if(fileName.Contains(wxS("png")))
	{
		if(!image->LoadFile(fileName,wxBITMAP_TYPE_PNG))
		{
			OPOLYGLOT_ERROR(wxT("LibOPolyglotOCR not load file %s"),fileName);
			wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Error"));
			errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::LibOPolyglotOCR\nnot loading %s"),fileName));
			wxString str = wxEmptyString;
			wxStringOutputStream sos(&str);
			wxXmlDocument docError;
			docError.SetRoot(errorNode);
			docError.Save(sos);
			ocrEngine->End();
			delete ocrEngine;
			delete inputDoc;
			return str;
		}
	}
	if(fileName.Contains(wxS("bmp")))
	{
		if(!image->LoadFile(fileName,wxBITMAP_TYPE_BMP))
		{
			OPOLYGLOT_ERROR(wxT("LibOPolyglotOCR not load file %s"),fileName);
			wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Error"));
			errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::LibOPolyglotOCR\nnot loading %s"),fileName));
			wxString str = wxEmptyString;
			wxStringOutputStream sos(&str);
			wxXmlDocument docError;
			docError.SetRoot(errorNode);
			docError.Save(sos);
			ocrEngine->End();
			delete ocrEngine;
			delete inputDoc;
			return str;
		}
	}
	if(!image->IsOk())
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotOCR is not ok image %s"),fileName);	
		wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Error"));
		errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::LibOPolyglotOCR\nimage failed %s"),fileName));
		wxString str = wxEmptyString;
		wxStringOutputStream sos(&str);
		wxXmlDocument docError;
		docError.SetRoot(errorNode);
		docError.Save(sos);
		ocrEngine->End();
		delete ocrEngine;
		delete inputDoc;
		return str;
	}
	ocrEngine->SetImage((const unsigned char *)image->GetData(),image->GetWidth(),image->GetHeight(),3,image->GetWidth()*3);
	wxXmlNode	*outNode = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Texts"));
	outNode->AddAttribute(wxS("fileName"),fileName);
	for(wxXmlNode *child = inputDoc->GetRoot()->GetChildren();child;child = child->GetNext())
	{
		int x,y,w,h;
		if(child->GetName().IsSameAs(wxT("TextRegion")))
		{
			if(child->GetAttribute(wxS("x")).ToInt(&x)
					&&child->GetAttribute(wxS("y")).ToInt(&y)
					&&child->GetAttribute(wxS("w")).ToInt(&w)
					&&child->GetAttribute(wxS("h")).ToInt(&h))
			{
				ocrEngine->SetRectangle(x,y,w,h);
				char *outText = ocrEngine->GetUTF8Text();
				wxXmlNode *textNode = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Text"));
				textNode->AddAttribute(wxS("original"),wxString(outText,wxConvUTF8));
				textNode->AddAttribute(wxS("codeOCR"),lang);
				if(!child->GetAttribute(wxS("onlyOCR")).IsEmpty())
				{
					textNode->AddAttribute(wxS("onlyOCR"),wxS("true"));
				}
				outNode->AddChild(textNode);
				delete[] outText;
			}
		}
	}
	delete inputDoc;
	ocrEngine->End();
	delete ocrEngine;
	delete image;
	wxString outStr = wxEmptyString;
	wxStringOutputStream sos(&outStr);
	wxXmlDocument outputDoc;
	outputDoc.SetRoot(outNode);
	outputDoc.Save(sos);
	OPOLYGLOT_DEBUG("LibOPolyglotOCR FINISH");
	return outStr;

}


wxString LibOPolyglotTranslator(wxString inputXMl,wxString fileYml,wxString fileYmlSecond)
{
	using namespace marian::bergamot;
	wxMutexLocker lock(mutexTranslate);
	OPOLYGLOT_MESSAGE(wxT("LibOPolyglotTranslator %s %s"),fileYml,fileYmlSecond);
	if(serviceTranslator == nullptr)
	{

		char *argv[] ={
			(char *)"OPolyglot",
			(char *)"--log-level",
			(char *)"err", /* trace,debug,info,warn,err(or),critical, off*/
			nullptr
		};
		OPOLYGLOT_DEBUG(wxT("LibOPolyglotTranslator start init BlockingService"));
		ConfigParser<BlockingService> *configParser = new ConfigParser<BlockingService>("OPolyglot",false);
		configParser->parseArgs(3,argv);
		try {
			serviceTranslator = new BlockingService(configParser->getConfig().serviceConfig);
		} catch (const std::exception& e) {
			OPOLYGLOT_ERROR(wxT("LibOPolyglotTranslator not init BlockingService %s "),wxString(e.what()));
			delete configParser;
			wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE, wxS("Error"));
			errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("libopolyglot::LibOPolyglotTranslator not init BlockingService %s"),wxString(e.what())));
			wxString str = wxEmptyString;
			wxStringOutputStream sos(&str);
			wxXmlDocument docError;
			docError.SetRoot(errorNode);
			docError.Save(sos);
			return str;
		}
	}
	wxStringInputStream sis(inputXMl);
	wxXmlDocument doc(sis);
	if(!doc.IsOk())
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotTranslator not valid xml document"));
		wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE, wxS("Error"));
		errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::LibOPolyglotTranslator\n input value not valid xml document")));
		wxString str = wxEmptyString;
		wxStringOutputStream sos(&str);
		wxXmlDocument docError;
		docError.SetRoot(errorNode);
		docError.Save(sos);
		return str;
	}
	if(!doc.GetRoot()->GetName().IsSameAs(wxS("Texts")))
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotTranslator not valid root xml node %s"),doc.GetRoot()->GetName());
		wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE, wxS("Error"));
		errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::LibOPolyglotTranslator\nnot valid root xml node \"Texts\" != \"%s\""),doc.GetRoot()->GetName()));
		wxString str = wxEmptyString;
		wxStringOutputStream sos(&str);
		wxXmlDocument docError;
		docError.SetRoot(errorNode);
		docError.Save(sos);
		return str;
	}
	//wxArrayInt notTranslateItem;
	std::vector<std::string> sources;
	std::vector<ResponseOptions> responseOpt;
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Text")))
		{
			if(child->GetAttribute(wxS("onlyOCR")).IsEmpty())
			{
				sources.push_back(child->GetAttribute(wxS("original")).utf8_str().data());
				responseOpt.push_back(*(new ResponseOptions()));
			} 
		}
	}
	ResponseOptions responseOptions;
	std::vector<Response> responses;
	if(fileYmlSecond.IsEmpty())
	{
		std::shared_ptr<TranslationModel> model = marian::New<TranslationModel>(parseOptionsFromFilePath(fileYml.utf8_str().data()));// service.createCompatibleModel(options);
		responses = serviceTranslator->translateMultiple(model,std::move(sources),responseOpt);
		model.reset();
	} else
	{
		std::shared_ptr<TranslationModel> model = marian::New<TranslationModel>(parseOptionsFromFilePath(fileYml.utf8_str().data()));// service.createCompatibleModel(options);
		std::shared_ptr<TranslationModel> modelSecond = marian::New<TranslationModel>(parseOptionsFromFilePath(fileYmlSecond.utf8_str().data()));// service.createCompatibleModel(options);
		responses = serviceTranslator->pivotMultiple(model,modelSecond,std::move(sources),responseOpt);
		model.reset();
		modelSecond.reset();

	}
	size_t i = 0;
	wxXmlNode *rootNode = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Texts"));
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext(),i++)
	{
		if(child->GetName().IsSameAs(wxS("Text")))
		{
			wxXmlNode *childNew = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Text"));
			if(child->GetAttribute(wxS("onlyOCR")).IsEmpty())
			{
				childNew->AddAttribute(wxS("text"),wxString(responses.front().target.text.c_str(),wxConvUTF8));
				childNew->AddAttribute(wxS("filesBergamot"),wxString::Format(wxS("%s %s"),fileYml,fileYmlSecond));
				responses.erase(responses.begin());
			} else
			{
				childNew->AddAttribute(wxS("text"),child->GetAttribute(wxS("original")));
				childNew->AddAttribute(wxS("onlyOCR"),wxS("true"));
			}
			childNew->AddAttribute(wxS("original"),child->GetAttribute(wxS("original")));
			childNew->AddAttribute(wxS("codeOCR"),child->GetAttribute(wxS("codeOCR")));
			rootNode->AddChild(childNew);
		}
	}
	wxString outStr = wxEmptyString;
	wxStringOutputStream sos(&outStr);
	wxXmlDocument outputDoc;
	outputDoc.SetRoot(rootNode);
	outputDoc.Save(sos);
	//responseOpt.clear();
	//model.reset();
	//delete rootNode;
	OPOLYGLOT_DEBUG(wxT("LibOPolyglotTranslator FINISH"));
	return outStr;
}
