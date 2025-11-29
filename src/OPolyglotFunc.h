#pragma once
#include "OPolyglotType.h"
#include <wx/string.h>



wxString OPolyglotFuncOCR(wxString dirTesstdata,wxString langCode,OPolyglotImage *image);
wxString OPolyglotFuncTranslate(wxString textForTranslate,wxString fileYml);
