#include "Translator.h"
#include "translator/byte_array_util.h"
#include "translator/parser.h"
#include "translator/response.h"
#include "translator/response_options.h"
#include "translator/utils.h"
#include "common/version.h"
#include "marian.h"
#include "Utils.h"
#include "translator/service.h"


#if 0

Translator::Translator(wxString fileYml)
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
	OPOLYGLOT_MESSAGE(wxT("%s"),fileYml);
	ConfigParser<AsyncService> configParser("OPolyglot",false);
	configParser.parseArgs(7, argv);
	service = new AsyncService(configParser.getConfig().serviceConfig);
	options = parseOptionsFromFilePath(configParser.getConfig().modelConfigPaths.front());
	model = (service)->createCompatibleModel(options);
}

Translator::~Translator()
{
	OPOLYGLOT_MESSAGE();
	delete service;
}
#endif

/*
 
static wxString Translator::translate(wxString *textSource,wxString *fileYml)
*/

wxString Translator::translate(wxString textSource,wxString fileYml)
{
	using namespace marian::bergamot;
	OPOLYGLOT_DEBUG(wxT(TRANSLATOR_H_VERSION));
	OPOLYGLOT_DEBUG(wxT("$Id: Translator.cpp,v 1.9 2025/11/17 08:57:16 oleksandr Exp oleksandr $"));
	OPOLYGLOT_MESSAGE(wxT("fileYml %s"),fileYml);
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
	ConfigParser<AsyncService> configParser("OPolyglot" , false);
	configParser.parseArgs(7, argv);
	auto &config = configParser.getConfig();
	AsyncService service(config.serviceConfig);
	auto options = parseOptionsFromFilePath(config.modelConfigPaths.front());
	std::shared_ptr<TranslationModel> model = service.createCompatibleModel(options);
	ResponseOptions responseOptions;
	std::promise<Response> promise;
	std::future<Response> future = promise.get_future();
	auto callback = [&promise](Response &&response){
		promise.set_value(std::move(response));
	};
	service.translate(model, std::move(textSource.utf8_string()),callback,responseOptions);
	Response response = future.get();
	wxString res = wxString::FromUTF8(response.getTranslatedText());
	return res;
}

#if 0
wxString Translator::translate(wxString source)
{
	marian::bergamot::ResponseOptions responseOptions;
	std::promise<marian::bergamot::Response> promise;
	std::future<marian::bergamot::Response> future = promise.get_future();
	auto callback = [&promise](marian::bergamot::Response &&response){
		promise.set_value(std::move(response));
	};
	service->translate(model,std::move(source.utf8_string()),callback,responseOptions);
	marian::bergamot::Response response = future.get();
	wxString str = wxString::FromUTF8(response.getTranslatedText());
	return str;
}
#endif
