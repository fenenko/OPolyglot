#pragma once
#include <wx/buffer.h>
#include <wx/bitmap.h>

class OPolyglotImage{
	public:
		OPolyglotImage(wxBitmap bitmap);
		~OPolyglotImage();
		void 	*GetData();
		int 	GetWidth();
		int		GetHeight();
		int 	GetBytesPerPixel();
	private:
		wxMemoryBuffer *data;
		int width;
		int height;
		int bytesOnPixel;
};

