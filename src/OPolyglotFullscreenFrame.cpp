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


#include "OPolyglotFullscreenFrame.h"
#include "OPolyglotEvent.h"
#include "OPolyglotType.h"
#include "Utils.h"
#include "Config.h"
#include <wx/dcscreen.h>
#include <wx/dcmemory.h>
//#include <wx/dcclient.h>
#include <wx/display.h>
//#include <wx/accel.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/sstream.h>
#include <wx/font.h>

enum{
	TIMER_ID,
};
enum{
	ID_KEY_ESCAPE=1001,
};

OPolyglotFullscreenFrame::OPolyglotFullscreenFrame(wxWindow *parent,wxString fileName) : GUIFullscreen(parent)
{
	nodeScreenshot = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("ScreenshotFile"));
	nodeScreenshot->AddAttribute(wxS("fileName"),fileName);
	OPOLYGLOT_DEBUG(wxT("node content %s"),nodeScreenshot->GetNodeContent());
	Panel = new wxPanel(this);
	Panel->SetBackgroundStyle(wxBG_STYLE_PAINT);
	Panel->Bind(wxEVT_PAINT, &OPolyglotFullscreenFrame::OnPaint, this);
	//dc.GetSize(&w,&h);
	if(fileName.Contains(wxS("bmp")))
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame bitmap type BMP"));
		if(!bitmapFile.LoadFile(fileName,wxBITMAP_TYPE_BMP))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotFullscreenFrame not load screenshot %s"),fileName);
		}
	}
	if(fileName.Contains(wxS("png")))
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame bitmap type PNG"));
		if(!bitmapFile.LoadFile(fileName,wxBITMAP_TYPE_PNG))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotFullscreenFrame not load screenshot %s"),fileName);
		}
	}

	bitmapDC = wxBitmap(bitmapFile.GetWidth(),bitmapFile.GetHeight(),bitmapFile.GetDepth());
	wxMemoryDC dc(bitmapFile);
	wxColour col(115,183,43);
	dc.SetBrush(wxBrush(col));
	wxFont font;
	int fontSize = 64;
	font.SetFamily(wxFONTFAMILY_MODERN);
	wxSize size;
	do{
		font.SetPointSize(fontSize);
		dc.SetFont(font);
		size = dc.GetTextExtent(wxString::Format(wxS("OPolyglot %s"),_("captured screen")));
		fontSize--;
	}while((parent->GetRect().width <= size.GetWidth())||(parent->GetRect().height/2 < size.GetHeight()));
	int x,y;
	x = (parent->GetRect().width-size.GetWidth())/2+parent->GetRect().x;
	y = parent->GetRect().y;
	OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame text in %d %d"),x,y);
	dc.DrawRectangle(parent->GetRect());
	dc.SetTextForeground(wxColour(21,16,157));
	dc.DrawText(wxString::Format(wxS("OPolyglot %s"),_("captured screen")),x,y);
	font.SetPointSize(fontSize/2);
	dc.SetFont(font);
	size = dc.GetTextExtent(wxString::Format(wxS("%s 'F1'"),_("for reference, press the key")));
	x = (parent->GetRect().width-size.GetWidth())/2+parent->GetRect().x;
	y = parent->GetRect().y+parent->GetRect().height-size.GetHeight()-5;
	dc.DrawText(wxString::Format(wxS("%s 'F1'"),_("for reference, press the key")),x,y);
	


	dc.SelectObject(bitmapDC);
	dc.DrawBitmap(bitmapFile,0,0);
	dc.SelectObject(wxNullBitmap);
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame(%dx%d) %s"),bitmapDC.GetWidth(),bitmapDC.GetHeight(),OPOLYGLOT_BOOL_TO_STRING(bitmapDC.IsOk()));
	this->parent = parent;
	startX = -1;
	startY = -1;
	endX = -1;
	endY = -1;
	wxDisplay dis(this);
	OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame display %d %d %dx%d"),dis.GetGeometry().GetX(),dis.GetGeometry().GetY(),dis.GetGeometry().GetWidth(),dis.GetGeometry().GetHeight());
	this->SetSize(bitmapDC.GetWidth(),bitmapDC.GetHeight());
	this->Show(true);
	this->ShowFullScreen(true,wxFULLSCREEN_ALL);
	this->SetFocus();
	Bind(wxEVT_CHAR_HOOK, &OPolyglotFullscreenFrame::OnCharHook, this);
	Panel->Bind(wxEVT_LEFT_DOWN, &OPolyglotFullscreenFrame::OnMouseLeftDown, this);
	Panel->Bind(wxEVT_LEFT_UP, &OPolyglotFullscreenFrame::OnMouseLeftUp, this);
	Panel->Bind(wxEVT_MOTION, &OPolyglotFullscreenFrame::OnMouseMotion, this);
	Panel->Refresh();
}


OPolyglotFullscreenFrame::~OPolyglotFullscreenFrame()
{
	wxMutexLocker lock(mutex);
	//timer->Stop();
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotFullscreenFrame"));
}


void OPolyglotFullscreenFrame::OnCharHook(wxKeyEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnCharHook"));
	OPOLYGLOT_DEBUG(wxT("OnCharHook"));
	if(event.GetKeyCode() == WXK_ESCAPE)
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnCharHook(WXK_ESCAPE)"));
		OPOLYGLOT_DEBUG(wxT("OnCharHook(WXK_ESCAPE)"));
		this->Destroy();
		return;
	}
	if(event.GetKeyCode() == WXK_RETURN)
	{
		wxString s = wxEmptyString;
		wxStringOutputStream so(&s);
		wxXmlDocument doc;
		doc.SetRoot(nodeScreenshot);
		doc.Save(so);
		OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnCharHook(WXK_RETURN)"));
		OPOLYGLOT_DEBUG(wxT("node content %s"),s);
		wxThreadEvent *ev = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_OCR_START);
		ev->SetString(wxString(s));
		wxQueueEvent(parent,ev);
		this->Destroy();
		return;
	}
}

void OPolyglotFullscreenFrame::OnMouseLeftUp( wxMouseEvent& event ) 
{

	endX = event.GetX();
	endY = event.GetY();
	int x1,x2,y1,y2;
	if(startX < endX)
	{
		x1 = startX;
		x2 = endX;
	} else
	{
		x1 = endX;
		x2 = startX;
	}
	if(startY < endY)
	{
		y1 = startY;
		y2 = endY;
	} else
	{
		y1 = endY;
		y2 = startY;
	}
	int fontSize = 16;
	if((y2-y1)/2 < fontSize)
	{
		fontSize = (y2-y1)/2;
	}
	countRectOCR+=1;
	wxMemoryDC memDC(bitmapDC);
	wxColour col;
	memDC.GetPixel(startX,startY,&col);
	col.Set(~col.GetRed(),~col.GetGreen(),~col.GetBlue());
	wxPen pen(col,2,wxPENSTYLE_SHORT_DASH );
	wxFont font;
	font.SetFamily(wxFONTFAMILY_MODERN);
	font.SetPointSize(fontSize);
	memDC.SetPen(pen);
	memDC.SetFont(font);
	memDC.SetTextForeground(col);
	memDC.DrawText(wxString::Format(wxS("%d"),countRectOCR),x1,y1);
	memDC.DrawLine(x1,y1,x2,y1);
	memDC.DrawLine(x2,y1,x2,y2);
	memDC.DrawLine(x1,y2,x2,y2);
	memDC.DrawLine(x1,y2,x1,y1);
	memDC.SelectObject(wxNullBitmap);
	OPOLYGLOT_MESSAGE(wxT("OPolylotFullscreenFrame::OnMouseLeftUp new RectOCR %d %d %dx%d"),x1,y1,x2-x1,y2-y1);
	wxXmlNode *rect = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("TextRegion"));
	rect->AddAttribute(wxS("x"),wxString::Format(wxT("%d"),x1));
	rect->AddAttribute(wxS("w"),wxString::Format(wxT("%d"),x2-x1));
	rect->AddAttribute(wxS("y"),wxString::Format(wxT("%d"),y1));
	rect->AddAttribute(wxS("h"),wxString::Format(wxT("%d"),y2-y1));
	nodeScreenshot->AddChild(rect);
	startX = -1;
	startY = -1;
	endX = -1;
	endY = -1;
	Refresh();
}

void OPolyglotFullscreenFrame::OnMouseMotion( wxMouseEvent& event)
{
	if(event.LeftIsDown())
	{
		endX = event.GetX();
		endY = event.GetY();
	}
	//OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame::OnMouseMotion %dx%d %dx%d"),startX,startY,endX,endY);
	Refresh();
}


void OPolyglotFullscreenFrame::OnMouseLeftDown( wxMouseEvent& event ) 
{
	startX = event.GetX();
	startY = event.GetY();
	endX = startX;
	endY = startY;
	Refresh();
}


void OPolyglotFullscreenFrame::OnPaint(wxPaintEvent& event)
{
	wxStopWatch time;
	time.Start();
	wxMutexLocker lock(mutex);
	wxAutoBufferedPaintDC dc(this->Panel);
	dc.Clear();
	dc.DrawBitmap(bitmapDC,0,0);
	if(startX != -1)
	{
		int x1,y1,x2,y2;
		if(startX < endX)
		{
			x1 = startX;
			x2 = endX;
		} else
		{
			x1 = endX;
			x2 = startX;
		}
		if(startY < endY)
		{
			y1 = startY;
			y2 = endY;
		} else
		{
			y1 = endY;
			y2 = startY;
		}
		wxColour col;
		dc.GetPixel(startX,startY,&col);
		col.Set(~col.GetRed(),~col.GetGreen(),~col.GetBlue());
		wxPen pen(col,2,wxPENSTYLE_SHORT_DASH );
		dc.SetPen(pen);
		dc.DrawLine(x1,y1,x2,y1);
		dc.DrawLine(x2,y1,x2,y2);
		dc.DrawLine(x1,y2,x2,y2);
		dc.DrawLine(x1,y2,x1,y1);
	}
	time.Pause();
}
