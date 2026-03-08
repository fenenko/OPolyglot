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

