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


extern "C" {
void TestTest()
{
	std::cout << "test libOPolyglotTranslator" << std::endl;
}
}

extern "C"{

	wxString OPolyglotOCR(wxString dirTesstdata,wxString langCode,wxString inputXml)
	{
		std::cerr << "OPolyglotOCR" << std::endl;
		std::cerr << dirTesstdata.utf8_str() << " " << langCode.utf8_str() << std::endl;
		tesseract::TessBaseAPI *ocrEngine = new tesseract::TessBaseAPI();
		wxString lang = langCode;
		int ret = ocrEngine->Init(dirTesstdata.utf8_str(),langCode.utf8_str(),tesseract::OEM_LSTM_ONLY );
		//int ret = ocrEngine->Init("/home/ofenenko/.opolyglot/data/tessdata/lstm/",langCode.utf8_str(),tesseract::OEM_TESSERACT_LSTM_COMBINED);
		if(ret)
		{
			wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE, wxS("Error"));
			errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::OPolyglotOCR\ninit ocrEngine for lang %s return %d"),langCode,ret));
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
			wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE, wxS("Error"));
			errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::OPolyglotOCR\nroot node not \"ScreenshotFile\" %s"),rootNode->GetName()));
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
				wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Error"));
				errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::OPolyglotOCR\nnot loading %s"),fileName));
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
				wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Error"));
				errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::OPolyglotOCR\nnot loading %s"),fileName));
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
			wxXmlNode *errorNode =new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("Error"));
			errorNode->AddAttribute(wxS("value"),wxString::Format(wxT("error libopolyglot::OPolyglotOCR\nimage failed %s"),fileName));
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
					delete outText;
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
		return outStr;

	}
}

extern "C"{
	wxString OPolyglotTranslator(wxString inputXMl,wxString fileYml,wxString fileYmlSecond)
	{
		wxStringInputStream sis(inputXMl);
		wxXmlDocument doc(sis);
		using namespace marian::bergamot;
		char *argv[] ={
			(char *)"OPolyglot",
			(char *)"--log-level",
			(char *)"err", /* trace,debug,info,warn,err(or),critical, off*/
			nullptr
		};
		wxArrayInt notTranslateItem;
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
					notTranslateItem.Add(-1);
				} else
				{
					notTranslateItem.Add(0);
				}
			} else
			{
				notTranslateItem.Add(0);
			}
		}
		ConfigParser<BlockingService> configParser("OPolyglot",false);
		configParser.parseArgs(3,argv);
		auto &config = configParser.getConfig();
		BlockingService service(config.serviceConfig);
		std::cout << "libopolyglot::OPolyglotTranslator " << fileYml.utf8_str().data() << std::endl;
		auto options = parseOptionsFromFilePath(fileYml.utf8_str().data());
		std::shared_ptr<TranslationModel> model = marian::New<TranslationModel>(options);// service.createCompatibleModel(options);
		ResponseOptions responseOptions;
		std::vector<Response> responses;
		if(fileYmlSecond.IsEmpty())
		{
			std::cout << "libopolyglot::OPolyglotTranslator translate" << std::endl;
			responses = service.translateMultiple(model,std::move(sources),responseOpt);
		} else
		{
			std::cout << "libopolyglot::OPolyglotTranslator pivot" << std::endl;
			auto optionsSecond = parseOptionsFromFilePath(fileYmlSecond.utf8_str().data());
			std::shared_ptr<TranslationModel> modelSecond = marian::New<TranslationModel>(optionsSecond);// service.createCompatibleModel(options);
			responses = service.pivotMultiple(model,modelSecond,std::move(sources),responseOpt);

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
		std::cout << "libopolyglot::OPolyglotTranslator finish " << std::endl;
		return outStr;
	}
}
