#pragma once
#include "OPolyglotType.h"
#include <wx/string.h>

class OPolyglotOCR
{
	public:
		OPolyglotOCR(wxString dir,wxString lang,OPolyglotImage *image);
		~OPolyglotOCR();
		wxString ocr();
	private:
		wxString dirTessData;
		wxString langCode;
		OPolyglotImage *imageForOCR;
		void			*ocrEngine=NULL;

};

wxString OPolyglotFuncOCR(wxString dirTesstdata,wxString langCode,OPolyglotImage *image);
wxString OPolyglotFuncTranslate(wxString textForTranslate,wxString fileYml);
