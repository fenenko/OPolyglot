#include "OPolyglotType.h"
#include "Utils.h"
#include <wx/rawbmp.h>

OPolyglotImage::OPolyglotImage(wxBitmap bitmap)
{
	width = bitmap.GetWidth();
	height = bitmap.GetHeight();
	OPOLYGLOT_MESSAGE(wxT("%dx%d"),width,height);
	data = new wxMemoryBuffer(3*width*height);
	wxNativePixelData 	pixel(bitmap);
	wxNativePixelData::Iterator row(pixel);
	if(!row.IsOk())
	{
		OPOLYGLOT_ERROR(wxT("error row.IsOk()"));
	}
	for(int y =0; y < height;y++)
	{
		for(int x =0; x< width;x++)
		{
			row.MoveTo(pixel,x,y);
			data->AppendByte(row.Red());
			data->AppendByte(row.Green());
			data->AppendByte(row.Blue());
		}

	}
	
}

OPolyglotImage::~OPolyglotImage()
{
	OPOLYGLOT_MESSAGE();
	data->Clear();
	delete data;
}

void *OPolyglotImage::GetData()
{
	return data->GetData();
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
