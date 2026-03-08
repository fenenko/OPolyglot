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
#include "Utils.h"
#include <wx/rawbmp.h>

OPolyglotRuleOfReplacement::OPolyglotRuleOfReplacement(wxString regex,wxString replacement)
{
	OPOLYGLOT_DEBUG();
	reg = regex;
	replace = replacement;
}

OPolyglotRuleOfReplacement::~OPolyglotRuleOfReplacement()
{
	OPOLYGLOT_DEBUG();
}

wxString OPolyglotRuleOfReplacement::GetRegEx()
{
	return reg;
}

wxString OPolyglotRuleOfReplacement::GetReplacement()
{
	return replace;
}


OPolyglotImage::OPolyglotImage()
{
	OPOLYGLOT_DEBUG();
	width = -1;
	height = -1;
	data = NULL;
}

OPolyglotImage::OPolyglotImage(OPolyglotImage *image)
{
	OPOLYGLOT_DEBUG();
	width = image->GetWidth();
	height = image->GetHeight();
	data = new unsigned char[width*height*3];
	memcpy(data,image->data,width*height*3);
}

bool OPolyglotImage::SetData(wxBitmap bitmap)
{
	OPOLYGLOT_DEBUG();
	width = bitmap.GetWidth();
	height = bitmap.GetHeight();
	data = new unsigned char[width*height*3];
	OPOLYGLOT_MESSAGE(wxT("%d %dx%d"),width*height*3,width,height);
	wxNativePixelData 	pixel(bitmap);
	wxNativePixelData::Iterator row(pixel);
	if(!row.IsOk())
	{
		OPOLYGLOT_ERROR(wxT("error row.IsOk()"));
		return false;
	}
	for(int y =0; y < height;y++)
	{
		for(int x =0; x< width;x++)
		{
			row.MoveTo(pixel,x,y);
			data[(y*width*3)+x*3] = (unsigned char)(row.Red());
			data[(y*width*3)+x*3+1] =(unsigned char)(row.Green());
			data[(y*width*3)+x*3+2] = (unsigned char)(row.Blue());

		}
	}
	return true;
}

size_t OPolyglotImage::GetDataLength()
{
	return width*height*3;
}

OPolyglotImage::~OPolyglotImage()
{
	OPOLYGLOT_DEBUG();
	//data->~wxMemoryBuffer();
	if(IS_NULLPTR(data))
	{
		delete data;
	}
}

void *OPolyglotImage::GetData()
{
	return data;
}

int OPolyglotImage::GetWidth()
{
	return width;
}

int OPolyglotImage::GetHeight()
{
	return height;
}

int OPolyglotImage::GetBytesPerPixel()
{
	return 3;
}
