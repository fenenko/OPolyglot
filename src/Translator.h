#pragma once
#include <wx/string.h>
#include <memory>
#define TRANSLATOR_H_VERSION	"$Id: Translator.h,v 1.8 2025/11/17 08:57:16 oleksandr Exp oleksandr $"

#if 0
namespace marian{
	class Options;
	namespace bergamot{
		class Service;
		class AsyncService;
		class TranslationModel;
	}
}
#endif


class Translator
{
	public:
#if 0
		Translator(wxString fileYml);
		~Translator();
		wxString translate(wxString source);
#endif
		static wxString translate(wxString textSource,wxString fileYml);
#if 0
	private:
		char *argv_config[7];
		//marian::bergamot::ConfigParser conf;
		std::shared_ptr<marian::Options> options;
		marian::bergamot::AsyncService *service;
		std::shared_ptr<marian::bergamot::TranslationModel> model;
#endif
};

