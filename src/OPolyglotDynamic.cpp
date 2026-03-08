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


#include "OPolyglotType.h"
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
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

wxMessageQueue<wxString> resultText;

void callbackFinishTranslation(marian::bergamot::Response &&respo)
{
	resultText.Post(wxString(respo.target.text.c_str(),wxConvUTF8));
}


extern "C" {
void TestTest()
{
	std::cout << "test libOPolyglotTranslator" << std::endl;
}
}

extern "C"{
	wxString OPolyglotDynamicOCR(wxString dirTesstdata,wxString langCode,OPolyglotImage *image)
	{
		tesseract::TessBaseAPI ocrEngine;
		if((image == NULL)||(image == nullptr))
		{
			std::cerr << "OPolyglotDynamicOCR error image NULL pointer" << std::endl;
			std::cout << "OPolyglotDynamicOCR error image NULL pointer" << std::endl;
			return wxEmptyString;
		}
		int ret = ocrEngine.Init(dirTesstdata.utf8_str(),langCode.utf8_str());
		if(ret)
		{
			std::cerr << "OPolyglotDynamicOCR error Init( " << dirTesstdata << " , " << langCode << " )" << std::endl;
			std::cout << "OPolyglotDynamicOCR error Init( " << dirTesstdata << " , " << langCode << " )" << std::endl;
			return wxEmptyString;
		}
		ocrEngine.SetImage((const unsigned char *)image->GetData(),image->GetWidth(),image->GetHeight(),image->GetBytesPerPixel(),image->GetWidth()*image->GetBytesPerPixel());
		wxString result = wxString(ocrEngine.GetUTF8Text(),wxConvUTF8);
		return result;
	}
}

extern "C"{
wxString OPolyglotDynamicTranslator(wxString textForTranslate,wxString fileYml)
{
	using namespace marian::bergamot;
	char *argv[] = {
		(char *)"OPolyglot",
		(char*)"--log-level",
		(char *)"err", /* trace, debug, info, warn, err(or), critical, off */
		(char *)"--model-config-paths",
		(char *)fileYml.utf8_str().data()	,/*"/home/oleksandr/Projects/OPolyglot/config.yml",*/
		(char *)"--cpu-threads",
		(char *)"1",
		(char *)"--help",
		nullptr
	};
	resultText.Clear();
	ConfigParser<AsyncService> configParser("OPolyglot" , false);
	configParser.parseArgs(7, argv);
	auto &config = configParser.getConfig();
	AsyncService service(config.serviceConfig);
	auto options = parseOptionsFromFilePath(config.modelConfigPaths.front());
	std::shared_ptr<TranslationModel> model = service.createCompatibleModel(options);
	ResponseOptions responseOptions;
	service.translate(model, std::move(textForTranslate.utf8_string()),callbackFinishTranslation,responseOptions);
	wxString res;
	resultText.Receive(res);
	return res;
}
}
