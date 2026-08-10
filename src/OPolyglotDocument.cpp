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

#include <wx/log.h>
#include <cstdint>
#include <cstring>
#include <wx/dcbuffer.h>
#include <wx/rawbmp.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>
#include <leptonica/allheaders.h>
#include "OPolyglotDocument.h"
#include "OPolyglotEvent.h"
#include "OPolyglotDebug.h"
#include "Utils.h"
#include "Config.h"
#include "LibOPolyglot.h"
#ifndef __WXMSW__
#include "../res/icon.xpm"
#endif

#define PAGE_SPACING		0

static wxMutex mutexPaint;

OPolyglotDocument::OPolyglotDocument(wxEvtHandler *handler,wxString file,wxString languageFrom,wxString languageTo) 
	: GUIOPolyglotDocumentView(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDocument(%s , %s->%s)"),file,languageFrom,languageTo);
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	this->handler = handler;
	fileName = file;
	ScanLanguageFrom();
	if(0 == this->LanguageFrom->GetCount())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDocument LanguageFrom(0)"));
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Failed LanguageFrom == 0")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		this->Destroy();
	}
	if(!LanguageFrom->SetStringSelection(languageFrom))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDocument LanguageFrom->SetStringSelection(%s)"),languageFrom);
		wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Failed LanguageFrom->SetStringSelection"),languageFrom),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		this->Destroy();
	}
	ScanLanguageTo();
	if(0 == this->LanguageTo->GetCount())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDocument LanguageTo(0)"));
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Failed LanguageTo == 0")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		this->Destroy();
	}
	if(!LanguageTo->SetStringSelection(languageTo))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDocument LanguageTo->SetStringSelection(%s)"),languageTo);
		wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Failed LanguageTo->SetStringSelection"),languageTo),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		this->Destroy();
	}
	FPDF_InitLibrary();
	doc = FPDF_LoadDocument(file,nullptr);
	if(!doc)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDocument error load %s"),file);
		wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Error loading PDF file"),file),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		doc = nullptr;
		this->Destroy();
		return;
	}
	OPOLYGLOT_DEBUG(wxT("OPolyglotDocument %s count page %d"),file,FPDF_GetPageCount(doc));
	allCountPage->SetLabel(wxString::Format(wxT("%d"),FPDF_GetPageCount(doc)));
	currentPage->SetRange(1,FPDF_GetPageCount(doc));
	currentPage->SetValue(1);
	pageDocumentZoomMul = 1;
	pageDocumentZoomDiv = 1;
	documentView->SetBackgroundStyle(wxBG_STYLE_PAINT);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH, &OPolyglotDocument::OnFinishThreadTranslator,this);
	documentView->Bind( wxEVT_LEFT_DOWN,  &OPolyglotDocument::OnLeftDown , this );
	documentView->Bind( wxEVT_LEFT_UP,  &OPolyglotDocument::OnStartTranslate, this );
	documentView->Bind( wxEVT_MOTION, &OPolyglotDocument::OnMotion , this );
	documentView->Bind( wxEVT_MOUSEWHEEL,  &OPolyglotDocument::OnMouseWheel , this );
	documentView->Bind(wxEVT_PAINT,&OPolyglotDocument::OnPaint,this);
	this->SendSizeEvent();
	viewTextTranslate = new OPolyglotViewTextTranslate(this,OPOLYGLOT_GET_XML_FILE_TRANSLATE);

}

void OPolyglotDocument::OnRenderPage( wxSizeEvent& event ) 
{
	//this->Unbind( wxEVT_SIZE, &OPolyglotDocument::OnSize ,this );
	int pageNumber = currentPage->GetValue()-1;
	FPDF_PAGE page = FPDF_LoadPage(doc,pageNumber);
	if(!page)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDocument::OnRenderPage failed to load page %d"),pageNumber);
		wxMessageDialog msg(this,wxString::Format(wxT("%s %d"),_("Failed to load page"),pageNumber),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		this->Destroy();
		return;
	}
	int width = FPDF_GetPageWidth(page)*documentView->GetDPI().GetWidth()*pageDocumentZoomMul/(72*pageDocumentZoomDiv);
	int height = FPDF_GetPageHeight(page)*documentView->GetDPI().GetHeight()*pageDocumentZoomMul/(72*pageDocumentZoomDiv);
	hScroll->Show(documentView->GetSize().GetWidth() < width);
	hScroll->SetScrollbar(0,documentView->GetSize().GetWidth(),width,documentView->GetSize().GetWidth(),true);
	this->Layout();
	vScroll->Show(documentView->GetSize().GetHeight() < height);
	vScroll->SetScrollbar(0,documentView->GetSize().GetHeight(),height,documentView->GetSize().GetHeight(),true);
	this->Layout();
	startX = -1;
	startY = -1;
	FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(width,height,FPDFBitmap_BGR,NULL,0);
	FPDFBitmap_FillRect(bitmap,0,0,width,height,0xffffffff);
	FPDF_RenderPageBitmap(bitmap,page,0,0,width,height,0,FPDF_ANNOT | FPDF_LCD_TEXT);
	uint8_t *buffer = static_cast<uint8_t *>(FPDFBitmap_GetBuffer(bitmap));
	wxImage image(width,height,true);
	uint8_t *dataImage = image.GetData();
	for(int iy = 0; iy < height;iy++)
	{
		for(int ix = 0; ix < width;ix++)
		{
				
			int idSrc = iy*FPDFBitmap_GetStride(bitmap)+ix*3;
			int idDst = (iy*width+ix)*3;
			dataImage[idDst] = buffer[idSrc+2];		//R
			dataImage[idDst+1] = buffer[idSrc+1];	//G
			dataImage[idDst+2] = buffer[idSrc];		//B
		}
	}
	pageDocument = wxBitmap(image);
	FPDFBitmap_Destroy(bitmap);
	FPDF_ClosePage(page);
	documentView->Refresh();
	event.Skip();
}

OPolyglotDocument::~OPolyglotDocument()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotDocument"));
	if(!IS_NULLPTR(viewTextTranslate))
	{
		viewTextTranslate->Destroy();
	}
	if(!IS_NULLPTR(doc))
	{
		FPDF_CloseDocument(doc);
	}
	FPDF_DestroyLibrary();
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}

void OPolyglotDocument::OnMouseWheel(wxMouseEvent& event)
{
	vScroll->SetThumbPosition(vScroll->GetThumbPosition()+-1*event.GetWheelRotation()/10);
	documentView->Refresh();
}

void OPolyglotDocument::OnVScroll(wxScrollEvent& event)
{
	OPOLYGLOT_DEBUG(wxT("OPolyglotDocument::OnVScroll %d"),vScroll->GetThumbPosition());
	documentView->Refresh();
}


void OPolyglotDocument::OnLeftDown( wxMouseEvent& event )
{
	OPOLYGLOT_DEBUG(wxT("OPolyglotDocument::OnLeftDown %d %d"),event.GetX(),event.GetY());
	this->SetCursor(wxCURSOR_SIZING );
	startX = event.GetX();
	startY = event.GetY();
}

wxThread::ExitCode OPolyglotDocument::Entry()
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	wxXmlDocument *xmlTranslate = new wxXmlDocument();
	if(!xmlTranslate->Load(OPOLYGLOT_GET_XML_FILE_TRANSLATE))
	{
		OPOLYGLOT_WARNING(wxT("OPolyglotDocument::Entry not load %s"),OPOLYGLOT_GET_XML_FILE_TRANSLATE);
		wxXmlNode *root = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Texts"));
		xmlTranslate->SetRoot(root);
	}
	FPDF_PAGE page = FPDF_LoadPage(doc,pageNumber);
	if(!page)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDocument::Entry Failed to load page %d"),pageNumber);
		wxMessageDialog msg(this,wxString::Format(wxT("%s %d"),_("Failed to load page"),pageNumber),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		this->Destroy();
		return (wxThread::ExitCode)0;
	}
	FPDF_BITMAP bitmap300 = FPDFBitmap_CreateEx(
			FPDF_GetPageWidth(page)*300/72,
			FPDF_GetPageHeight(page)*300/72,
			FPDFBitmap_Gray,NULL,0);
	FPDFBitmap_FillRect(bitmap300,0,0,
			FPDF_GetPageWidth(page)*300/72,
			FPDF_GetPageHeight(page)*300/72,
			0xffffffff);
	FPDF_RenderPageBitmap(bitmap300,page,0,0,FPDF_GetPageWidth(page)*300/72,FPDF_GetPageHeight(page)*300/72,0,FPDF_GRAYSCALE|FPDF_PRINTING);
	uint8_t *data300 = static_cast<uint8_t *>(FPDFBitmap_GetBuffer(bitmap300));
	OPOLYGLOT_DEBUG(wxT("OPolyglotDocument::Entry pixCreate(%dx%d)"),pageWidth,pageHeight);
	PIX *pix = pixCreate(pageWidth,pageHeight,8);
	PIX *pixFinish=NULL;
	for(int iy = 0; iy < pageHeight;iy++)
	{
		int idy300 = (iy+pageStartY)*FPDFBitmap_GetStride(bitmap300);
		l_uint32* dst_row = (l_uint32 *)pixGetData(pix) + iy*pixGetWpl(pix);
		for(int ix = 0; ix < pageWidth;ix++)
		{
			SET_DATA_BYTE(dst_row,ix,data300[idy300+ix+pageStartX]);
		}
	}
	FPDFBitmap_Destroy(bitmap300);
	FPDF_ClosePage(page);
	l_int32 tresh,fg_val,bg_val;
	if(InvertColorOCR->GetValue())
	{
		pixInvert(pix,pix);
	}
	if(pixSplitDistributionFgBg(pix,0.1f,1,&tresh,&fg_val,&bg_val,NULL) ==0 )
	{
	}
	OPOLYGLOT_DEBUG(wxT("pixSplitDistributionFgBg tresh %d,fg_val %d,bg_val %d"),tresh,fg_val,bg_val);
	pixFinish = pixSauvolaOnContrastNorm(pix,
			static_cast<int>(config.ReadLong(OPOLYGLOT_CONFIG_INT_SAUVOLA_MINDIFF,OPOLYGLOT_CONFIG_INT_SAUVOLA_MINDIFF_DEFAULT)),NULL,NULL);
	if(!pixFinish)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDocument::Entry error binarization image start pixThresholdToBinary"));
		pixFinish = pixThresholdToBinary(pix, tresh);
	}
	pixDestroy(&pix);
	wxString codeLanguageFrom = OPolyglotGetCodeFromLanguage(OPolyglotGetOriginalLanguage(this->LanguageFrom->GetStringSelection()));
	wxString codeLanguageTo = OPolyglotGetCodeFromLanguage(OPolyglotGetOriginalLanguage(LanguageTo->GetStringSelection()));
	wxXmlNode *textNode = LibOPolyglotOCRAndTranslate(pixFinish,codeLanguageFrom,codeLanguageTo,OnlyOCR->GetValue());
	pixDestroy(&pixFinish);
	xmlTranslate->GetRoot()->AddChild(textNode);
	
	if(!xmlTranslate->Save(OPOLYGLOT_GET_XML_FILE_TRANSLATE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotDocument::Entry Failed to save changes %s"),OPOLYGLOT_GET_XML_FILE_TRANSLATE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s %d"),_("Failed to save changes"),pageNumber),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
	}
	delete xmlTranslate;
	wxQueueEvent(this,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
	return (wxThread::ExitCode)0;
}

void OPolyglotDocument::OnFinishThreadTranslator(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDocument::OnFinishThreadTranslator"));
	documentView->DeletePendingEvents();
	wxYield();
	documentView->Bind( wxEVT_LEFT_DOWN,  &OPolyglotDocument::OnLeftDown , this );
	documentView->Bind( wxEVT_LEFT_UP,  &OPolyglotDocument::OnStartTranslate, this );
	documentView->Bind( wxEVT_MOTION, &OPolyglotDocument::OnMotion , this );
	documentView->Bind( wxEVT_MOUSEWHEEL,  &OPolyglotDocument::OnMouseWheel , this );
	currentPage->Enable(true);
	vScroll->Enable(true);
	hScroll->Enable(true);
	progress->Finish();
	viewTextTranslate->ViewTranslate();
}


void OPolyglotDocument::OnShowTranslation( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDocument::OnShowTranslation"));
	viewTextTranslate->Show(true);
	viewTextTranslate->Raise();
}

void OPolyglotDocument::OnStartTranslate( wxMouseEvent& event )
{
	OPOLYGLOT_DEBUG(wxT("OPolyglotDocument::OnStartTranslate start %d %d, event %d %d"),startX,startY,event.GetX(),event.GetY());
	if(startX == -1)
	{
		return;
	}
	if(event.GetX() <= startX)
	{
		if((startX - event.GetX()) < 50)
		{
			this->SetCursor(wxCURSOR_ARROW);
			startX = -1;
			startY = -1;
			documentView->Refresh();
			return;
		}
	} else
	{
		if((event.GetX()-startX) < 50)
		{
			this->SetCursor(wxCURSOR_ARROW);
			startX = -1;
			startY = -1;
			documentView->Refresh();
			return;
		}
	}
	if(event.GetY() <= startY)
	{
		if((startY - event.GetY())<10)
		{
			this->SetCursor(wxCURSOR_ARROW);
			startX = -1;
			startY = -1;
			documentView->Refresh();
			return;
		}
	} else
	{
		if((event.GetY() - startY)<10)
		{
			this->SetCursor(wxCURSOR_ARROW);
			startX = -1;
			startY = -1;
			documentView->Refresh();
			return;
		}
	}
	documentView->Unbind( wxEVT_LEFT_DOWN,  &OPolyglotDocument::OnLeftDown , this );
	documentView->Unbind( wxEVT_LEFT_UP,  &OPolyglotDocument::OnStartTranslate , this );
	documentView->Unbind( wxEVT_MOTION, &OPolyglotDocument::OnMotion , this );
	documentView->Unbind( wxEVT_MOUSEWHEEL,  &OPolyglotDocument::OnMouseWheel , this );
	currentPage->Enable(false);
	vScroll->Enable(false);
	hScroll->Enable(false);
	pageNumber = currentPage->GetValue() - 1;
	OPOLYGLOT_DEBUG(wxT("OPolyglotDocument::OnStartTranslate %d %d %d"),pageNumber,event.GetX(),event.GetY());
	this->SetCursor(wxCURSOR_ARROW);
	int x1,x2,y1,y2;
	if(startX < event.GetX())
	{
		x1 =startX;
		x2 =event.GetX();
	} else
	{
		x1 = event.GetX();
		x2 = startX;
	}
	if(startY < event.GetY())
	{
		y1 = startY;
		y2 = event.GetY();
	} else
	{
		y1 = event.GetY();
		y2 = startY;
	}
	x1 += hScroll->GetThumbPosition();
	x2 += hScroll->GetThumbPosition();
	y1 += vScroll->GetThumbPosition();
	y2 += vScroll->GetThumbPosition();

	pageStartX = x1*300*pageDocumentZoomDiv/(documentView->GetDPI().GetWidth()*pageDocumentZoomMul);
	pageWidth = x2*300*pageDocumentZoomDiv/((documentView->GetDPI().GetWidth() )*pageDocumentZoomMul) - pageStartX;
	pageStartY = y1*300*pageDocumentZoomDiv/(documentView->GetDPI().GetHeight()*pageDocumentZoomMul);
	pageHeight = y2*300*pageDocumentZoomDiv/((documentView->GetDPI().GetHeight() )*pageDocumentZoomMul) - pageStartY;
	startX = -1;
	startY = -1;
	documentView->Refresh();

	progress = new OPolyglotDialogProgress(this,_("Translating..."));
	progress->Show();
	if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
    {
        OPOLYGLOT_ERROR("OPolyglotDocument::OnStartTranslate Could not create the worker thread!");
		wxMessageDialog msg(this,_("Could not create the worker thread!"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
		msg.ShowModal();
        return;
    }
 
    if (GetThread()->Run() != wxTHREAD_NO_ERROR)
    {
        OPOLYGLOT_ERROR("OPolyglotDocument::OnStartTranslate Could not run the worker thread!");
		wxMessageDialog msg(this,_("Could not run the worker thread!"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
		msg.ShowModal();
        return;
    }
}


void OPolyglotDocument::OnMotion( wxMouseEvent& event )
{
	if(startX != -1)
	{
		documentView->Refresh();
	}
}


void OPolyglotDocument::OnHScroll(wxScrollEvent& event)
{
	OPOLYGLOT_DEBUG(wxT("OPolyglotDocument::OnHScroll %d"),hScroll->GetThumbPosition());
	documentView->Refresh();
}

void OPolyglotDocument::OnSetCurrentPage( wxCommandEvent& event )
{
	OPOLYGLOT_DEBUG(wxT("OPolyglotDocument::OnSetCurrentPage %d"),currentPage->GetValue());
	this->SendSizeEvent();

}

void OPolyglotDocument::OnPaint(wxPaintEvent& event)
{
	wxMutexLocker lock(mutexPaint);
	wxAutoBufferedPaintDC dc(documentView);
	dc.SetBackground(wxColour(128,128,128));
	dc.Clear();
	int w,h;
	w = pageDocument.GetWidth();
	h = pageDocument.GetHeight();
	if(documentView->GetSize().GetWidth() < w)
	{
		w = documentView->GetSize().GetWidth();
	}
	if(documentView->GetSize().GetHeight() < h)
	{
		h = documentView->GetSize().GetHeight();
	}
	wxMemoryDC memDC(pageDocument);
	dc.Blit(0,0,w,h,&memDC,hScroll->GetThumbPosition(),vScroll->GetThumbPosition(),wxCOPY,false,-1,-1);
	if(startX != -1)
	{
		wxColour col;
		//dc.GetPixel(startX,startY,&col);
		//col.Set(~col.GetRed(),~col.GetGreen(),~col.GetBlue());
		col.Set(118,184,42);
		wxPen pen(col,2,wxPENSTYLE_SHORT_DASH);
		dc.SetPen(pen);
		wxPoint screenPos = documentView->ScreenToClient(wxGetMousePosition());
		int x1,x2,y1,y2;
		if(screenPos.x < startX)
		{
			x1 = screenPos.x;
			x2 = startX;
		} else
		{
			x1 = startX;
			x2 = screenPos.x;
		}
		if(screenPos.y < startY)
		{
			y1 = screenPos.y;
			y2 = startY;
		} else
		{
			y1 = startY;
			y2 = screenPos.y;
		}
		dc.DrawLine(x1,y1,x2,y1);
		dc.DrawLine(x2,y1,x2,y2);
		dc.DrawLine(x2,y2,x1,y2);
		dc.DrawLine(x1,y2,x1,y1);
	}
}


void OPolyglotDocument::OnDocumentZoom(wxCommandEvent& event) 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDocument::OnDocumentZoom %s"),DocumentZoom->GetStringSelection());
	if(DocumentZoom->GetStringSelection().IsSameAs(wxS("100%")))
	{
		pageDocumentZoomMul = 1;
		pageDocumentZoomDiv = 1;
	} else
	{
		if(DocumentZoom->GetStringSelection().IsSameAs(wxS("75%")))
		{
			pageDocumentZoomMul = 3;
			pageDocumentZoomDiv = 4;
		} else
		{
			if(DocumentZoom->GetStringSelection().IsSameAs(wxS("50%")))
			{
				pageDocumentZoomMul = 1;
				pageDocumentZoomDiv = 2;
			} else
			{
				if(DocumentZoom->GetStringSelection().IsSameAs(wxS("150%")))
				{
					pageDocumentZoomMul = 3;
					pageDocumentZoomDiv = 2;
				} else
				{
					if(DocumentZoom->GetStringSelection().IsSameAs(wxS("200%")))
					{
						pageDocumentZoomMul = 2;
						pageDocumentZoomDiv = 1;
					}
				}
			}

		}

	}
	this->SendSizeEvent();
}

void OPolyglotDocument::ScanLanguageFrom()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDocument::ScanLanguageFrom"));
	this->LanguageFrom->Clear();
	wxArrayString languages = OPolyglotGetTranslatedLanguages(OPolyglotGetInstalledLanguagesFrom());
	languages.Sort(CompareLocaleNoCase);
	LanguageFrom->Append(languages);
	if(0 < LanguageFrom->GetCount())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		if(!LanguageFrom->SetStringSelection(OPolyglotGetTranslateLanguage(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT))))
		{
			LanguageFrom->SetSelection(0);
		}
		config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPolyglotGetOriginalLanguage(this->LanguageFrom->GetStringSelection()));
	}
}

void OPolyglotDocument::ScanLanguageTo()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDocument::ScanLanguageTo"));
	LanguageTo->Clear();
	wxArrayString languages = OPolyglotGetTranslatedLanguages(
		OPolyglotGetInstalledLanguagesTo(
				OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection())));
	languages.Sort(CompareLocaleNoCase);
	LanguageTo->Append(languages);
	if(0 < LanguageTo->GetCount())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		if(!LanguageTo->SetStringSelection(OPolyglotGetTranslateLanguage(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT))))
		{
			LanguageTo->SetSelection(0);
		}
		config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPolyglotGetOriginalLanguage(this->LanguageTo->GetStringSelection()));
	}
}

void OPolyglotDocument::OnSelectLanguageFrom( wxCommandEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDocument::OnSelectLanguageFrom"));
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPolyglotGetOriginalLanguage(this->LanguageFrom->GetStringSelection()));
	this->ScanLanguageTo();
}

void OPolyglotDocument::OnSelectLanguageTo( wxCommandEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotDocument::OnSelectLanguageTo"));
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPolyglotGetOriginalLanguage(this->LanguageTo->GetStringSelection()));
}

