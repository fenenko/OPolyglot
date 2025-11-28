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
static tesseract::TessBaseAPI *ocrEngine= NULL;
static Pix	*image = NULL;

extern "C"{
	wxString OPolyglotDynamicOCRInit(wxString dirTesstdata,wxString langCode,wxString fileNameImage)
	{
		ocrEngine = new tesseract::TessBaseAPI();
		if(ocrEngine == nullptr)
		{
			ocrEngine = NULL;
			return wxS("OPolyglotDynamicOCRInit error new");
		}
		int ret = ocrEngine->Init(dirTesstdata.utf8_str(),langCode.utf8_str());
		if(ret)
		{
			delete ocrEngine;
			ocrEngine = NULL;
			return wxString::Format(wxT("OPolyglotDynamicOCRInit error ocrEngine->Init %d"),ret);
		}
		image = pixRead(fileNameImage.utf8_str());
		if(image == nullptr)
		{
			delete ocrEngine;
			ocrEngine = NULL;
			image = NULL;
			return wxString::Format(wxS("OPolyglotDynamicOCRInit error pixRead %s"),fileNameImage);
		}
		return wxEmptyString;
	}
	wxString OPolyglotDynamicOCR()
	{
		ocrEngine->SetImage(image);
		wxString result = wxString(ocrEngine->GetUTF8Text(),wxConvUTF8);
		return result;

	}

	void OPolyglotDynamicOCRDestroy()
	{
		if(ocrEngine != NULL)
		{
			ocrEngine->~TessBaseAPI();
		}
		if(image != NULL)
		{
			pixDestroy(&image);
		}
	}
}

extern "C"{
wxString OPolyglotTranslate(wxString textForTranslate,wxString fileYml)
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
