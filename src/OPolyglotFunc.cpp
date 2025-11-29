#include "OPolyglotFunc.h"
#include "OPolyglotType.h"
#include "Utils.h"
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

wxMessageQueue<wxString> resultText;

void callbackFinishTranslation(marian::bergamot::Response &&respo)
{
	resultText.Post(wxString(respo.target.text.c_str(),wxConvUTF8));
}


OPolyglotOCR::OPolyglotOCR(wxString dir,wxString lang,OPolyglotImage *image)
{
	OPOLYGLOT_MESSAGE();
	dirTessData = dir;
	langCode = lang;
	imageForOCR = image;
	ocrEngine = (void *)(new tesseract::TessBaseAPI());
	int ret = ((tesseract::TessBaseAPI *)ocrEngine)->Init(dirTessData.utf8_str(),langCode.utf8_str());
	if(ret)
	{
		OPOLYGLOT_ERROR(wxT("init error %d"),ret);
	}
	((tesseract::TessBaseAPI *)ocrEngine)->SetImage((const unsigned char *)imageForOCR->GetData()
		,imageForOCR->GetWidth()
		,imageForOCR->GetHeight()
		,imageForOCR->GetBytesPerPixel()
		,imageForOCR->GetBytesPerPixel()*3);
}
OPolyglotOCR::~OPolyglotOCR()
{
	OPOLYGLOT_MESSAGE();
	
	((tesseract::TessBaseAPI *)ocrEngine)->~TessBaseAPI();
	//imageForOCR->~OPolyglotImage();
}

wxString OPolyglotOCR::ocr()
{
	return wxString(((tesseract::TessBaseAPI *)ocrEngine)->GetUTF8Text(),wxConvUTF8);
}


wxString OPolyglotFuncOCR(wxString dirTesstdata,wxString langCode,OPolyglotImage *image)
{
	std::cout << "OPolyglotDynamicOCR start " << std::endl;
	tesseract::TessBaseAPI ocrEngine;
	if((image == NULL)||(image == nullptr))
	{
		std::cerr << "OPolyglotDynamicOCR error image NULL pointer" << std::endl;
		std::cout << "OPolyglotDynamicOCR error image NULL pointer" << std::endl;
		return wxEmptyString;
	}
	std::cout << "OPolyglotDynamicOCR start init " << std::endl;
	int ret = ocrEngine.Init(dirTesstdata.utf8_str(),langCode.utf8_str());
	if(ret)
	{
		return wxEmptyString;
	}
	std::cout << "OPolyglotDynamicOCR set image " << std::endl;
	ocrEngine.SetImage((const unsigned char *)image->GetData(),image->GetWidth(),image->GetHeight(),image->GetBytesPerPixel(),image->GetWidth()*image->GetBytesPerPixel());
	std::cout << "OPolyglotDynamicOCR start ocr " << std::endl;
	wxString result = wxString(ocrEngine.GetUTF8Text(),wxConvUTF8);
	std::cout << "OPolyglotDynamicOCR finish"  << std::endl;
	//ocrEngine.~TessBaseAPI();
	//image->~OPolyglotImage();
	return result;
}

wxString OPolyglotFuncTranslate(wxString textForTranslate,wxString fileYml)
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
