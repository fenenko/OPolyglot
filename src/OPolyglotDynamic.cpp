#include "OPolyglotDynamic.h"
#include "translator/byte_array_util.h"
#include "translator/parser.h"
#include "translator/response.h"
#include "translator/response_options.h"
#include "translator/utils.h"
#include "common/version.h"
#include "marian.h"
#include "translator/service.h"
#include <wx/string.h>



extern "C" {
void TestTest()
{
	std::cout << "test libOPolyglotTranslator" << std::endl;
}
}
extern "C"{
wxString OPolyglotTranslate(wxString textForTranslate,wxString fileYml)
{
	using namespace marian::bergamot;
	// OPOLYGLOT_DEBUG(wxT(TRANSLATOR_H_VERSION));
	// OPOLYGLOT_MESSAGE(wxT("fileYml %s"),fileYml);
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
	service.translate(model, std::move(textForTranslate.utf8_string()),callback,responseOptions);
	Response response = future.get();
	wxString res = wxString::FromUTF8(response.getTranslatedText());
	return res;
}
}
