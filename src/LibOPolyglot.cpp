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

wxString LibOPolyglotOCR(wxString inputXml,wxString dirTesstdata,wxString langCode,bool enableSauvola,int sauvolaWhsize,float sauvolaFactor)
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
	PIX* rawImg = NULL;
	const unsigned char *rgb_buffer = image->GetData();
	rawImg = pixCreate(image->GetWidth(),image->GetHeight(),32);
	if(IS_NULLPTR(rawImg))
	{
		OPOLYGLOT_ERROR(wxT("LibOPolyglotOCR failed pixCreate "));	
		wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Error"));
		errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::LibOPolyglotOCR\nfailed pixCreate")));
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
	l_uint32* data = pixGetData(rawImg);
	int wpl = pixGetWpl(rawImg);
	for(int y = 0; y < image->GetHeight();y++)
	{
		l_uint32* line = data+y*wpl;
		for(int x = 0; x < image->GetWidth();x++)
		{
			int offset = y*3*image->GetWidth()+x*3;
			unsigned char r = rgb_buffer[offset];
			unsigned char g = rgb_buffer[offset+1];
			unsigned char b = rgb_buffer[offset+2];
			l_uint32 pixel_val;
			composeRGBPixel(r,g,b , &pixel_val);
			line[x] = pixel_val;
		}
	}
	delete image;
	wxXmlNode	*outNode = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Texts"));
	wxString id = wxS("false");
	if(enableSauvola)
	{
		PIX* pix_gray = pixConvertRGBToLuminance(rawImg);
		pixDestroy(&rawImg);
		float max_val;
		int max_loc;
		rawImg = pix_gray;
		NUMA* hist = pixGetGrayHistogram(rawImg,4);
		numaGetMax(hist,&max_val,&max_loc);

		OPOLYGLOT_DEBUG("LibOPolyglot::LibOPolyglotOCR forgeround color %d %f",max_loc,max_val);
		numaDestroy(&hist);
		if(max_loc < 100)
		{
			pixInvert(rawImg, rawImg);
		}
		PIX *pix_binar;
		int result = pixSauvolaBinarize(rawImg, sauvolaWhsize, sauvolaFactor, 1, nullptr, nullptr, nullptr, &pix_binar);
		OPOLYGLOT_MESSAGE(wxT("LibOPolyglot::LibOPolyglotOCR result Sauvola(%d,%g)  %d"),sauvolaWhsize,sauvolaFactor,result);
		id = GenerateUUIDv4();
		outNode->AddAttribute(wxS("idtiff"),id);
		wxString fileName = OPOLYGLOT_USER_DATA_IMG+wxFileName::GetPathSeparator()+id+wxS(".tif");
		//const char *file = fileName.fn_str();
		result = pixWriteTiff(fileName.fn_str(), pix_binar,IFF_TIFF_G4,"w");
		OPOLYGLOT_DEBUG(wxT("LibOPolyglot::LibOPolyglotOCR result save %s %d "),fileName.fn_str(),result);
		pixDestroy(&rawImg);
		rawImg = pix_binar;
	}
	ocrEngine->SetImage(rawImg);
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
				textNode->AddAttribute(wxS("idtiff"),id);
				textNode->AddAttribute(wxS("x"),child->GetAttribute(wxS("x")));
				textNode->AddAttribute(wxS("y"),child->GetAttribute(wxS("y")));
				textNode->AddAttribute(wxS("w"),child->GetAttribute(wxS("w")));
				textNode->AddAttribute(wxS("h"),child->GetAttribute(wxS("h")));
				if(!child->GetAttribute(wxS("onlyOCR")).IsEmpty())
				{
					textNode->AddAttribute(wxS("onlyOCR"),wxS("true"));
				}
				if(!child->GetAttribute(wxS("id")).IsEmpty())
				{
					OPOLYGLOT_DEBUG(wxT("LibOPolyglotOCR id %s"),child->GetAttribute(wxS("id")));
					textNode->AddAttribute(wxS("id"),child->GetAttribute(wxS("id")));
				}
				outNode->AddChild(textNode);
				delete[] outText;
			}
		}
	}
	delete inputDoc;
	ocrEngine->End();
	delete ocrEngine;
	pixDestroy(&rawImg);
	wxString outStr = wxEmptyString;
	wxStringOutputStream sos(&outStr);
	wxXmlDocument outputDoc;
	outputDoc.SetRoot(outNode);
	outputDoc.Save(sos);
	OPOLYGLOT_DEBUG("LibOPolyglotOCR FINISH");
	return outStr;

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
		if(IS_NULLPTR(configParser))
		{
			configParser = new ConfigParser<BlockingService>("OPolyglot",false);
			configParser->parseArgs(3,argv);
		}
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
	std::vector<std::string> sources;
	std::vector<ResponseOptions> responseOpt;
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxS("Text")))
		{
			if(child->GetAttribute(wxS("onlyOCR")).IsEmpty())
			{
				sources.push_back(child->GetAttribute(wxS("original")).utf8_str().data());
				responseOpt.emplace_back();
			} 
		}
	}
	ResponseOptions responseOptions;
	std::vector<Response> responses;
	if(fileYmlSecond.IsEmpty())
	{
		std::shared_ptr<TranslationModel> model = marian::New<TranslationModel>(parseOptionsFromFilePath(fileYml.utf8_str().data()));
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
	if(!doc.GetRoot()->GetAttribute(wxS("idtiff")).IsEmpty())
	{
		rootNode->AddAttribute(wxS("idtiff"),doc.GetRoot()->GetAttribute(wxS("idtiff")));
	}
	
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
			}
			for(wxXmlAttribute *attrs = child->GetAttributes();attrs;attrs=attrs->GetNext())
			{
				childNew->AddAttribute(attrs->GetName(),attrs->GetValue());
			}
			OPOLYGLOT_DEBUG(wxT("LibOPolyglotTranslator id %s"),childNew->GetAttribute(wxS("id")));
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
