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
#include "Utils.h"
#include "Config.h"
#include <wx/dcscreen.h>
#include <wx/dcmemory.h>
#include <wx/display.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/sstream.h>
#include <wx/font.h>
#include <wx/msgdlg.h>
#include <wx/arrimpl.cpp>

WX_DEFINE_OBJARRAY(OPolyglotArrayRect);

enum{
	TIMER_ID,
};
enum{
	ID_KEY_ESCAPE=wxID_ANY,
	ID_MENU_ONLYOCR,
	ID_MENU_DELETE,
};

static wxMutex mutexDraw;

void DrawCaption(wxDC &dc,int width)
{
	wxRect rect(0,0,width,70);
	wxColour col(115,183,43);
	dc.SetBrush(wxBrush(col));
	wxFont font;
	int fontSize = 64;
	font.SetFamily(wxFONTFAMILY_MODERN);
	wxSize size;
	do{
		font.SetPointSize(fontSize);
		dc.SetFont(font);
		size = dc.GetTextExtent(wxString::Format(wxS("OPolyglot %s"),_("Screen Translator")));
		fontSize--;
	}while((rect.width <= size.GetWidth())||(rect.height/2 < size.GetHeight()));
	int x,y;
	x = (rect.width-size.GetWidth())/2+rect.x;
	y = rect.y;
	OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame text in %d %d %dx%d"),x,y,rect.width,rect.height);
	dc.DrawRectangle(rect);
	dc.SetTextForeground(wxColour(21,16,157));
	dc.DrawText(wxString::Format(wxS("OPolyglot %s"),_("Screen Translator")),x,y);
	font.SetPointSize(fontSize/2);
	dc.SetFont(font);
	size = dc.GetTextExtent(wxString::Format(wxS("%s 'F1'"),_("for reference, press the key")));
	x = (rect.width-size.GetWidth())/2+rect.x;
	y = rect.y+rect.height-size.GetHeight()-5;
	dc.DrawText(wxString::Format(wxS("%s 'F1'"),_("for reference, press the key")),x,y);
}

OPolyglotFullscreenFrame::OPolyglotFullscreenFrame(wxWindow *parent,wxString fileName) : GUIFullscreen(parent)
{
	nodeScreenshot = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("ScreenshotFile"));
	nodeScreenshot->AddAttribute(wxS("fileName"),fileName);
	Panel = new wxPanel(this);
	Panel->SetBackgroundStyle(wxBG_STYLE_PAINT);
	Panel->Bind(wxEVT_PAINT, &OPolyglotFullscreenFrame::OnPaint, this);
	Bind(wxEVT_CHAR_HOOK, &OPolyglotFullscreenFrame::OnCharHook, this);
	Panel->Bind(wxEVT_LEFT_DOWN, &OPolyglotFullscreenFrame::OnMouseLeftDown, this);
	Panel->Bind(wxEVT_LEFT_UP, &OPolyglotFullscreenFrame::OnMouseLeftUp, this);
	Panel->Bind(wxEVT_RIGHT_DOWN, &OPolyglotFullscreenFrame::OnMouseRightDown ,this);
	Panel->Bind(wxEVT_RIGHT_UP,&OPolyglotFullscreenFrame::OnMouseRightUp, this);
	Panel->Bind(wxEVT_MOTION, &OPolyglotFullscreenFrame::OnMouseMotion, this);
	Bind(wxEVT_MENU, &OPolyglotFullscreenFrame::OnItemDelete, this,ID_MENU_DELETE);
	Bind(wxEVT_MENU, &OPolyglotFullscreenFrame::OnItemOnlyOCR, this,ID_MENU_ONLYOCR);
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
	DrawCaption(dc,bitmapFile.GetWidth());
	dc.SelectObject(bitmapDC);
	dc.DrawBitmap(bitmapFile,0,0);
	dc.SelectObject(wxNullBitmap);
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame(%dx%d) %s"),bitmapDC.GetWidth(),bitmapDC.GetHeight(),OPOLYGLOT_BOOL_TO_STRING(bitmapDC.IsOk()));
	this->parent = parent;
	wxDisplay dis(this);
	this->SetSize(bitmapDC.GetWidth(),bitmapDC.GetHeight());
	this->Show(true);
	this->ShowFullScreen(true,wxFULLSCREEN_ALL);
	this->SetFocus();
	Panel->Refresh();
}


OPolyglotFullscreenFrame::~OPolyglotFullscreenFrame()
{
	wxMutexLocker lock(mutex);
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotFullscreenFrame"));
}

void OPolyglotFullscreenFrame::OnItemDelete(wxCommandEvent& event)
{
	wxMutexLocker lock(mutex);
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnItemDelete"));
	boxs.RemoveAt(selectBoxMenu);
	boxsOption.RemoveAt(selectBoxMenu);
	selectBoxMenu = -1;
	Refresh();
}

void OPolyglotFullscreenFrame::OnItemOnlyOCR(wxCommandEvent& event)
{
	wxMutexLocker lock(mutex);
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnItemOnlyOCR %s"),OPOLYGLOT_BOOL_TO_STRING(event.IsChecked()));
	if(event.IsChecked())
	{
		boxsOption.Item(selectBoxMenu) = 1;
	} else
	{
		boxsOption.Item(selectBoxMenu) = 0;
	}
	selectBoxMenu = -1;
	Refresh();
}

void OPolyglotFullscreenFrame::OnMouseRightDown(wxMouseEvent& event)
{
    selectBoxMenu = -1;	
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnMouseRightDown"));
	int x = event.GetX();
	int y = event.GetY();
	for(size_t i = 0; (i < boxs.GetCount())&&(selectBoxMenu == static_cast<size_t>(-1));i++)
	{
		if(((boxs.Item(i).GetX() <= x)&&(boxs.Item(i).GetY() <= y))
				&&((x <= (boxs.Item(i).GetX()+boxs.Item(i).GetWidth()))
					&&(y <= (boxs.Item(i).GetY()+boxs.Item(i).GetHeight()))))
		{
			selectBoxMenu = i;
		}
	}
}

void OPolyglotFullscreenFrame::OnMouseRightUp(wxMouseEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnMouseRightUp %zu"),selectBoxMenu);
	if(selectBoxMenu != static_cast<size_t>(-1))
	{
		wxMenu contextMenu;
		wxMenuItem* itemOnlyOCR = contextMenu.AppendCheckItem(ID_MENU_ONLYOCR,wxString::Format(wxS("%s"),_("Only OCR")));
		itemOnlyOCR->Check((boxsOption.Item(selectBoxMenu) == 1));
		contextMenu.Append(ID_MENU_DELETE,wxString::Format(wxS("%s"),_("Delete")),wxEmptyString);
		PopupMenu(&contextMenu);
	}
}

void OPolyglotFullscreenFrame::OnCharHook(wxKeyEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OnCharHook"));
	OPOLYGLOT_DEBUG(wxT("OnCharHook"));
	if(event.GetKeyCode() == WXK_ESCAPE)
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnCharHook(WXK_ESCAPE)"));
		OPOLYGLOT_DEBUG(wxT("OnCharHook(WXK_ESCAPE)"));
		wxThreadEvent *ev = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_OCR_START);
		ev->SetString(wxEmptyString);
		wxQueueEvent(parent,ev);
		this->Destroy();
		return;
	}
	if(event.GetKeyCode() == WXK_F1)
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnCharHook(WXK_F1)"));
		wxMessageDialog msg(this
				,wxString::Format(wxT("%s\n%s\n%s\n%s")
					,_("\"Esc\": Exit screen translation.")
					,_("\"Enter\": Start translating selected areas.")
					,_("To select an area: Click and drag the Left Mouse Button (LMB).")
					,_("Right-click the selected region for options (Only OCR, Delete)."))
				,wxT("OPolyglot"),wxICON_INFORMATION|wxOK);
		msg.ShowModal();
		return;
	}
	if(event.GetKeyCode() == WXK_RETURN)
	{
		for(size_t i = 0; i < boxs.GetCount();i++)
		{
			wxXmlNode *rect = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxS("TextRegion"));
			rect->AddAttribute(wxS("x"),wxString::Format(wxT("%d"),boxs.Item(i).GetX()));
			rect->AddAttribute(wxS("w"),wxString::Format(wxT("%d"),boxs.Item(i).GetWidth()));
			rect->AddAttribute(wxS("y"),wxString::Format(wxT("%d"),boxs.Item(i).GetY()));
			rect->AddAttribute(wxS("h"),wxString::Format(wxT("%d"),boxs.Item(i).GetHeight()));
			if(boxsOption.Item(i) == 1)
			{
				rect->AddAttribute(wxS("onlyOCR"),wxS("true"));
			}
			rect->AddAttribute(wxS("id"),GenerateUUIDv4());
			nodeScreenshot->AddChild(rect);
		}
		wxString s = wxEmptyString;
		wxStringOutputStream so(&s);
		wxXmlDocument doc;
		doc.SetRoot(nodeScreenshot);
		doc.Save(so);
		OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnCharHook(WXK_RETURN)"));
		wxThreadEvent *ev = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_OCR_START);
		ev->SetString(wxString(s));
		wxQueueEvent(parent,ev);
		this->Destroy();
		return;
	}
}

void OPolyglotFullscreenFrame::OnMouseLeftUp( wxMouseEvent& event ) 
{
	size_t i = boxs.GetCount()-1;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnMouseLeftUp"));
	startX = -1;
	startY = -1;
	if((boxs.Item(i).GetWidth() < 6)||(boxs.Item(i).GetHeight() < 6))
	{
		if(0 < boxs.GetCount())
		{
			boxs.RemoveAt(i);
			boxsOption.RemoveAt(i);
		}
	}
	Refresh();
}

void OPolyglotFullscreenFrame::OnMouseMotion( wxMouseEvent& event)
{
	wxMutexLocker lock(mutex);
	int x = event.GetX();
	int y = event.GetY();
	if(event.LeftIsDown())
	{
		if(selectBoxResize == static_cast<size_t>(-1))
		{
			size_t  i = boxs.GetCount()-1;
			if(startX <= x)
			{
				boxs.Item(i).SetWidth(x-startX );
			} else
			{
				boxs.Item(i).SetX(x);
				boxs.Item(i).SetWidth(startX-x);
			}
			if(startY <= y)
			{
				boxs.Item(i).SetHeight(y-startY);
			} else
			{
				boxs.Item(i).SetY(y);
				boxs.Item(i).SetHeight(startY-y);
			}
			OPOLYGLOT_DEBUG(wxT("OPolyglotFullscreenFrame::OnMouseMotion %d %d %dx%d"),boxs.Item(i).GetX(),boxs.Item(i).GetY(),boxs.Item(i).GetWidth(),boxs.Item(i).GetHeight());
		} else
		{
			switch(selectLineResize)
			{
				case 1:
					boxs.Item(selectBoxResize).SetX(x);
					boxs.Item(selectBoxResize).SetWidth(endX-x);
					break;
				case 2:
					boxs.Item(selectBoxResize).SetY(y);
					boxs.Item(selectBoxResize).SetHeight(endY-y);
					break;
				case 3:
					boxs.Item(selectBoxResize).SetWidth(x-startX);
					break;
				case 4:
					boxs.Item(selectBoxResize).SetHeight(y-startY);
					break;
			}
		}
		Refresh();
	} else
	{
		selectBoxResize = -1;
		selectLineResize = 0;
		for(size_t i = 0; (i < boxs.GetCount())&&(selectBoxResize == static_cast<size_t>(-1));i++)
		{
			if((boxs.Item(i).GetX()==x)
					&&((boxs.Item(i).GetY() <= y)
						&&(y <= (boxs.Item(i).GetY()+boxs.Item(i).GetHeight()))))
			{
				selectBoxResize = i;
				this->SetCursor(wxCURSOR_SIZEWE);
				selectLineResize = 1;
			}
			if(((boxs.Item(i).GetX()+boxs.Item(i).GetWidth())==x)
					&&((boxs.Item(i).GetY() <= y)
						&&(y <= (boxs.Item(i).GetY()+boxs.Item(i).GetHeight()))))
			{
				selectBoxResize = i;
				this->SetCursor(wxCURSOR_SIZEWE);
				selectLineResize = 3;
			}
			if((boxs.Item(i).GetY()==y)
					&&((boxs.Item(i).GetX()<=x)
						&&(x <= (boxs.Item(i).GetX()+boxs.Item(i).GetWidth()))))
			{
				selectBoxResize = i;
				this->SetCursor(wxCURSOR_SIZENS);
				selectLineResize = 2;
			}
			if(((boxs.Item(i).GetY()+boxs.Item(i).GetHeight())==y)
					&&((boxs.Item(i).GetX()<=x)
						&&(x <= (boxs.Item(i).GetX()+boxs.Item(i).GetWidth()))))
			{
				selectBoxResize = i;
				this->SetCursor(wxCURSOR_SIZENS);
				selectLineResize = 4;
			}


		}
		if(selectBoxResize == static_cast<size_t>(-1))
		{
			this->SetCursor(wxCURSOR_ARROW);
		}
	}
}


void OPolyglotFullscreenFrame::OnMouseLeftDown( wxMouseEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame::OnMouseLeftDown"));
	if(selectBoxMenu != static_cast<size_t>(-1))
	{
		selectBoxMenu = -1;
	}
	if(selectBoxResize == static_cast<size_t>(-1))
	{
		startX = event.GetX();
		startY = event.GetY();
		boxsOption.Add(0);
		boxs.Add(wxRect(event.GetX(),event.GetY(),0,0));
	} else
	{
		startX = boxs.Item(selectBoxResize).GetX();
		startY = boxs.Item(selectBoxResize).GetY();
		endX = boxs.Item(selectBoxResize).GetX()+boxs.Item(selectBoxResize).GetWidth();
		endY = boxs.Item(selectBoxResize).GetY()+boxs.Item(selectBoxResize).GetHeight();
	}
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
	for(size_t i = 0 ; i < boxs.GetCount();i++)
	{
		int fontSize = 16;
		if(boxs.Item(i).GetHeight()/2 < fontSize)
		{
			fontSize = boxs.Item(i).GetHeight()/2;
		}
		wxColour col;
		dc.GetPixel(boxs.Item(i).GetX(),boxs.Item(i).GetY(),&col);
		col.Set(~col.GetRed(),~col.GetGreen(),~col.GetBlue());
		wxPen pen(col,2,wxPENSTYLE_SHORT_DASH );
		wxFont font;
		font.SetFamily(wxFONTFAMILY_MODERN);
		font.SetPointSize(fontSize);
		dc.SetPen(pen);
		dc.SetFont(font);
		dc.SetTextForeground(col);
		if(boxsOption.Item(i) == 0)
		{
 			dc.DrawText(wxString::Format(wxS("%d"),static_cast<int>(i+1)),boxs.Item(i).GetX(),boxs.Item(i).GetY());
		} else
		{
 			dc.DrawText(wxString::Format(wxS("%d %s"),static_cast<int>(i+1),_("Only OCR")),boxs.Item(i).GetX(),boxs.Item(i).GetY());
		}
		dc.DrawLine(boxs.Item(i).GetX()
				,boxs.Item(i).GetY()
				,boxs.Item(i).GetX()+boxs.Item(i).GetWidth()
				,boxs.Item(i).GetY());
		dc.DrawLine(boxs.Item(i).GetX()+boxs.Item(i).GetWidth()
				,boxs.Item(i).GetY()
				,boxs.Item(i).GetX()+boxs.Item(i).GetWidth()
				,boxs.Item(i).GetY()+boxs.Item(i).GetHeight());
		dc.DrawLine(boxs.Item(i).GetX()+boxs.Item(i).GetWidth()
				,boxs.Item(i).GetY()+boxs.Item(i).GetHeight()
				,boxs.Item(i).GetX()
				,boxs.Item(i).GetY()+boxs.Item(i).GetHeight());
		dc.DrawLine(boxs.Item(i).GetX()
				,boxs.Item(i).GetY()+boxs.Item(i).GetHeight()
				,boxs.Item(i).GetX()
				,boxs.Item(i).GetY());
	}
	time.Pause();
}
