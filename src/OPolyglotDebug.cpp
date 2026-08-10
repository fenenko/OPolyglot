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

#include "Utils.h"
#include "OPolyglotDebug.h"
#include <wx/rawbmp.h>
#include <wx/dcbuffer.h>
#include <leptonica/allheaders.h>

OPolyglotDebugViewImage::OPolyglotDebugViewImage(wxWindow *parent,void *pixImage)
	: GUIOPolyglotDebugViewImage(parent)
{
	int w,h;
	PIX *pix = static_cast<PIX *>(pixImage);
	pixGetDimensions(pix,&w,&h,NULL);
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDebugViewImage %dx%d %d"),w,h,pixGetWpl(pix));
	bitmap = wxBitmap(w,h);
	wxNativePixelData data(bitmap);
	wxNativePixelData::Iterator p(data);
	p.Offset(data,0,0);
	if(pixGetDepth(pix) == 8)
	{
		for(int iy = 0; iy < h;iy++)
		{
			wxNativePixelData::Iterator rowStart = p;
			l_uint32 *line;
			line = pixGetData(pix)+iy*pixGetWpl(pix);
			for(int ix = 0; ix < w;ix++,++p)
			{
				l_uint8 gray_val = GET_DATA_BYTE(line, ix);
				p.Red() = gray_val;
				p.Green() = gray_val;
				p.Blue() = gray_val;
			}	
			p =rowStart;
			p.OffsetY(data,1);
		}
	} else
	{
		if(pixGetDepth(pix) == 1)
		{
			for(int iy = 0; iy < h;iy++)
			{
				wxNativePixelData::Iterator rowStart = p;
				l_uint32 *line;
				line = pixGetData(pix)+iy*pixGetWpl(pix);
				for(int ix = 0; ix < w;ix++,++p)
				{
					if(GET_DATA_BIT(line, ix) == 0)
					{
						p.Red() = 0xff;
						p.Green() = 0xff;
						p.Blue() = 0xff;
					} else
					{
						p.Red() = 0;
						p.Green() = 0;
						p.Blue() = 0;
					}
				}	
				p =rowStart;
				p.OffsetY(data,1);
			}
		}
	}

	this->handler = parent;

	imageView->SetBackgroundStyle(wxBG_STYLE_PAINT);
	imageView->Bind(wxEVT_PAINT,&OPolyglotDebugViewImage::OnPaint,this);
	this->SendSizeEvent();
}

OPolyglotDebugViewImage::~OPolyglotDebugViewImage()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotDebugViewImage"));
}

void OPolyglotDebugViewImage::OnSize(wxSizeEvent& event)
{
	int range = bitmap.GetWidth();
	int thumbSize = imageView->GetSize().GetWidth();
	OPOLYGLOT_DEBUG(wxT("-------- %d %d"),thumbSize,range);
	this->Layout();
	if(thumbSize < range)
	{
		hScroll->SetScrollbar(0,thumbSize,range,thumbSize,true);
		hScroll->Show(true); 
	} else
	{
		hScroll->Show(false);
		hScroll->SetScrollbar(0,10,100,10,true);
	}
	this->Layout();
	if(imageView->GetSize().GetHeight() < bitmap.GetHeight())
	{
		vScroll->SetScrollbar(0,
				imageView->GetSize().GetHeight(),
				bitmap.GetHeight(),
				imageView->GetSize().GetHeight(),
				true);
		vScroll->Show(true);
	} else
	{
		vScroll->Show(false);
		vScroll->SetScrollbar(0,10,100,10,true);
	}
	this->Layout();
	imageView->Refresh();
}


void OPolyglotDebugViewImage::OnVScroll(wxScrollEvent& event)
{
	imageView->Refresh();
}

void OPolyglotDebugViewImage::OnHScroll(wxScrollEvent& event)
{
	imageView->Refresh();
}

void OPolyglotDebugViewImage::OnNext(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDebugViewImage::OnNext"));
	EndModal(wxID_EXIT);
}

void OPolyglotDebugViewImage::OnPaint(wxPaintEvent& paint)
{
	wxBitmap viewBitmap(imageView->GetSize().GetWidth(),imageView->GetSize().GetHeight());
	wxMemoryDC dcViewBitmap(viewBitmap);
	wxMemoryDC dcBitmap(bitmap);
	dcViewBitmap.Blit(0,0,viewBitmap.GetWidth(),viewBitmap.GetHeight(),&dcBitmap,hScroll->GetThumbPosition(),vScroll->GetThumbPosition());
	dcViewBitmap.SelectObject(wxNullBitmap);
	dcBitmap.SelectObject(wxNullBitmap);
	wxAutoBufferedPaintDC dc(imageView);
	dc.DrawBitmap(viewBitmap,0,0);
}



