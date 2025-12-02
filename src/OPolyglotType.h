#pragma once
#include <wx/buffer.h>
#include <wx/bitmap.h>
#include <wx/string.h>
#include <wx/xml/xml.h>
#include <wx/dynarray.h>


class OPolyglotRuleOfReplacement
{
	public:
		OPolyglotRuleOfReplacement(wxString regex,wxString replacement);
		~OPolyglotRuleOfReplacement();
		wxString GetRegEx();
		wxString GetReplacement();
	private:
		wxString reg;
		wxString replace;
};

WX_DEFINE_ARRAY_PTR(OPolyglotRuleOfReplacement *,OPolyglotRulesReplacement);
WX_DEFINE_ARRAY_PTR(wxXmlNode *,ArrayXmlNode);

class OPolyglotImage{
	public:
		OPolyglotImage();
		OPolyglotImage(OPolyglotImage *image);
		~OPolyglotImage();
		bool	SetData(wxBitmap bitmap);
		void 	*GetData();
		int 	GetWidth();
		int		GetHeight();
		int 	GetBytesPerPixel();
		size_t  GetDataLength();
	private:
		unsigned char *data;
		int width;
		int height;
		int bytesOnPixel;
};

