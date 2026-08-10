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

#include "OPolyglotEditTranslating.h"
#include "OPolyglotEvent.h"
#include "Utils.h"
#include "LibOPolyglot.h"
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/dcmemory.h>
#include <wx/dcbuffer.h>
#include <wx/tokenzr.h>
#if __WXGTK__
	#include "../res/icon.xpm"
#endif


OPolyglotEditTranslating::OPolyglotEditTranslating(wxWindow* parent,wxString fileName,wxString &id,int oldLine) : GUIOpolyglotEditTranslating(parent) , wxThreadHelper()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditTranslating::OPolyglotEditTranslating %s"),id);
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	oldLineCount = oldLine;
	idText = id;
	this->SetTitle(wxString::Format(wxS("OPolyglot %s %s"),_("edit"),idText));
	wxPoint pos = parent->GetPosition();
	SetPosition(pos);
	imageView->SetBackgroundStyle(wxBG_STYLE_PAINT);
	imageView->Bind(wxEVT_PAINT,&OPolyglotEditTranslating::OnPaint,this);
	Bind(wxEVT_SIZE,&OPolyglotEditTranslating::OnSize,this);
	handler = parent;
	wxXmlDocument *doc = new wxXmlDocument();
	this->fileName = fileName;
	if(!doc->Load(fileName))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating::OPolyglotEditTranslating not load %s"),fileName);
	} else
	{
		if(doc->GetRoot()->GetName().IsSameAs(wxT("Texts")))
		{
			OPOLYGLOT_DEBUG(wxT("OPolyglotEditTranslating find ROOT child is pressed %s"),OPOLYGLOT_BOOL_TO_STRING(!IS_NULLPTR(doc->GetRoot()->GetChildren()) ));
			for(wxXmlNode *child = doc->GetRoot()->GetChildren();child;child = child->GetNext())
			{
				if(child->GetName().IsSameAs(wxS("Text")))
				{
					if((!child->GetAttribute(wxS("id")).IsEmpty())
							&&(child->GetAttribute(wxS("id")).IsSameAs(idText)))
					{
						if(child->GetAttribute(wxS("onlyOCR")).IsEmpty())
						{
							textOCR->SetValue(child->GetAttribute(wxS("original")));

							textTranslate->SetValue(child->GetAttribute(wxS("text")));
							textOCR->Bind( wxEVT_COMMAND_TEXT_UPDATED, &OPolyglotEditTranslating::OnTextOCR , this );
							onlyOCR = false;
							filesBergamot = child->GetAttribute(wxS("filesBergamot"));
						} else
						{
							textOCR->SetValue(_("Only OCR"));
							textTranslate->SetValue(child->GetAttribute(wxS("original")));
							textOCR->Enable(false);
							onlyOCR = true;
							filesBergamot = wxEmptyString;
						}
						oldOCR = textOCR->GetValue();
						oldText = textTranslate->GetValue();
						Translate->Enable(false);
						Save->Enable(false);
						bitmap = wxNullBitmap;
						if(!child->GetAttribute(wxS("idtiff")).IsEmpty())
						{
							wxString fileImage = OPOLYGLOT_USER_DATA_IMG+wxFileName::GetPathSeparator()+child->GetAttribute(wxS("idtiff"))+wxS(".tif");
							OPOLYGLOT_DEBUG(wxT("OPolyglotEditTranslating start load file %s"),fileImage);
							wxBitmap bitmapTiff;
							int x = 0;
							int y = 0;
							int w = 0;
							int h = 0;
							if(bitmapTiff.LoadFile(fileImage,wxBITMAP_TYPE_TIFF))
							{
								if((child->GetAttribute(wxS("x")).IsEmpty())
										||(child->GetAttribute(wxS("y")).IsEmpty())
										||(child->GetAttribute(wxS("w")).IsEmpty())
										||(child->GetAttribute(wxS("h")).IsEmpty()))
								{
									x = 0;
									y = 0;
									w = bitmapTiff.GetWidth();
									h = bitmapTiff.GetHeight();
								} else
								{
									if((!child->GetAttribute(wxS("x")).ToInt(&x))
											||(!child->GetAttribute(wxS("y")).ToInt(&y))
											||(!child->GetAttribute(wxS("w")).ToInt(&w))
											||(!child->GetAttribute(wxS("h")).ToInt(&h)))
									{
										x = 0;
										y = 0;
										w = bitmapTiff.GetWidth();
										h = bitmapTiff.GetHeight();
									}
								}
								wxMemoryDC dc(bitmapTiff);
								bitmap = wxBitmap(w,h);
								wxMemoryDC dcB(bitmap);
								dcB.Blit(0,0,w,h,&dc,x,y);
								dcB.SelectObject(wxNullBitmap);
								OPOLYGLOT_DEBUG(wxT("OPolyglotEditTranslating finish create bitmap %s"),OPOLYGLOT_BOOL_TO_STRING(bitmap.IsOk()));
							} else
							{
								OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating not loaded %s"),fileImage);
							}
						} else
						{
							OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating not finded attribute idtiff"));
						}
					}
				}
			}
		} else
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating::OPolyglotEditTranslating not valid root \"%s\" not \"Texts\""),doc->GetRoot()->GetName());
			
		}
	}
	delete doc;
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH,&OPolyglotEditTranslating::OnFinishTranslating,this);
	imageView->Refresh();
}


OPolyglotEditTranslating::~OPolyglotEditTranslating()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditTranslating::~OPolyglotEditTranslating"));
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT);
	event->SetInt(oldLineCount);
	wxQueueEvent(handler,event);
}

wxThread::ExitCode OPolyglotEditTranslating::Entry()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditTranslating::Entry"));
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	wxString text = textOCR->GetValue();
	wxArrayString configs = wxStringTokenize(filesBergamot,wxT("\n"),wxTOKEN_RET_EMPTY);
	if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_PREPROCESSING_DEFAULT))
	{
		text = OPolyglotPreProcessingText(text);
	}
	text = LibOPolyglotTranslator(text,configs);
	if(text.IsEmpty())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating::Entry error translating"));
		return (wxThread::ExitCode)0;
	}
	if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING,OPOLYGLOT_CONFIG_BOOL_ENABLED_POSTPROCESSING_DEFAULT))
	{
		text = OPolyglotPostProcessingText(text);
	}
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH);
	event->SetString(text);
	wxQueueEvent(this,event);
	return (wxThread::ExitCode)0;
}

void OPolyglotEditTranslating::OnSize(wxSizeEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditTranslating::OnSize"));
	imageView->Refresh();
	event.Skip();
}


void OPolyglotEditTranslating::OnHScroll(wxScrollEvent& event)
{
	startViewX = hScroll->GetThumbPosition();
	imageView->Refresh();
}

void OPolyglotEditTranslating::OnVScroll(wxScrollEvent& event)
{
	startViewY = vScroll->GetThumbPosition();
	imageView->Refresh();
}


void OPolyglotEditTranslating::OnTextOCR( wxCommandEvent& event ) 
{
	if(!oldOCR.IsSameAs(textOCR->GetValue()))
	{
		Translate->Enable(true);
	} else
	{
		Translate->Enable(false);
	}
}


void OPolyglotEditTranslating::OnTextTranslate(wxCommandEvent& event)
{
	if(!oldText.IsSameAs(textTranslate->GetValue()))
	{
		Save->Enable(true);
	} else
	{
		Save->Enable(false);
	}
}


void OPolyglotEditTranslating::OnClose(wxCloseEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditTranslating::OnClose"));
	if(Save->IsEnabled())
	{
		wxMessageDialog msg(this,_("You have unsaved changes. Are you sure you want to exit?"),wxS("OPolyglot"),wxICON_QUESTION|wxYES_NO|wxNO_DEFAULT);
		if(msg.ShowModal() == wxID_YES)
		{
			Destroy();
		}

	} else
	{
		Destroy();
	}
}


void OPolyglotEditTranslating::OnSave(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditTranslating::OnSave"));
	wxXmlDocument doc;
	if(doc.Load(fileName))
	{
		if(doc.GetRoot()->GetName().IsSameAs(wxS("Texts")))
		{
			for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
			{
				if(child->GetName().IsSameAs(wxS("Text")))
				{
					if((!child->GetAttribute(wxS("id")).IsEmpty())
							&&(child->GetAttribute(wxS("id")).IsSameAs(idText)))
					{
						if(child->GetAttribute(wxS("onlyOCR")).IsEmpty())
						{
							child->DeleteAttribute(wxS("text"));
							child->AddAttribute(wxS("text"),textTranslate->GetValue());
							oldText = textTranslate->GetValue();
						} else
						{
							child->DeleteAttribute(wxS("original"));
							child->AddAttribute(wxS("original"),textTranslate->GetValue());
							oldText = textTranslate->GetValue();
						}
						Save->Enable(false);
						if(!doc.Save(fileName))
						{
							OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating::OnSave error saving file %s"),fileName);
							wxMessageDialog msg(this,wxString::Format(wxS("%s:%s"),_("Error saving file"),fileName),wxS("OPolyglot"),wxICON_ERROR|wxOK);
							msg.ShowModal();
							return;
						}
					}
				}
			}
		} else
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating::OnSave not valid root \"%s\" not \"Texts\""),doc.GetRoot()->GetName());
		}
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating::OnSave not load %s"),fileName);
	}
}


void OPolyglotEditTranslating::OnSaveAndTranslating( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditTranslating::OnTranslate"));
	progress = new OPolyglotDialogProgress(this,_("Translating..."));
	progress->Show();
	oldOCR = textOCR->GetValue();
	textOCR->Enable(false);
	textTranslate->Enable(false);
	Translate->Enable(false);
	wxXmlDocument doc;
	if(doc.Load(fileName))
	{
		if(doc.GetRoot()->GetName().IsSameAs(wxS("Texts")))
		{
			for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
			{
				if(child->GetName().IsSameAs(wxS("Text")))
				{
					if((!child->GetAttribute(wxS("id")).IsEmpty())
							&&(child->GetAttribute(wxS("id")).IsSameAs(idText)))
					{
						child->DeleteAttribute(wxS("original"));
						child->AddAttribute(wxS("original"),textOCR->GetValue());
						oldOCR = textOCR->GetValue();
						Save->Enable(false);
						if(!doc.Save(fileName))
						{
							OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating::OnSaveAndTranslating error saving file %s"),fileName);
							wxMessageDialog msg(this,wxString::Format(wxS("%s:%s"),_("Error saving file"),fileName),wxS("OPolyglot"),wxICON_ERROR|wxOK);
							msg.ShowModal();
							return;
						}
					}
				}
			}
		} else
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating::OnSaveAndTranslating not valid root \"%s\" not \"Texts\""),doc.GetRoot()->GetName());
		}
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating::OnSaveAndTranslating not load %s"),fileName);
	}
	if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
    {
        OPOLYGLOT_ERROR("OPolyglotEditTranslating::OnSaveAndTranslating Could not create the worker thread!");
        return;
    }
    if (GetThread()->Run() != wxTHREAD_NO_ERROR)
    {
        OPOLYGLOT_ERROR("OPolyglotEditTranslating::OnSaveAndTranslating Could not run the worker thread!");
        return;
    }
	
}


void OPolyglotEditTranslating::OnPaint(wxPaintEvent& event)
{
	if(bitmap.IsOk())
	{
		int x=0,y=0;
		int width,height;
		imageView->GetSize(&width,&height);
		if(bitmap.GetWidth() < width)
			{
				hScroll->Show(false);
				hBox1_2->Layout();
				vBox1->Layout();
				x = (width-bitmap.GetWidth())/2;
				startViewX = 0;
			} else
			{
				hScroll->Show(true);
				hBox1_2->Layout();
				vBox1->Layout();
				imageView->GetSize(&width,&height);
					hScroll->SetScrollbar(startViewX,width,bitmap.GetWidth(),width,true);
					x = 0;
			}
			if(bitmap.GetHeight() < height)
			{
				vScroll->Show(false);
				hBox1_1->Layout();
				y = (height-bitmap.GetHeight())/2;
				startViewY = 0;
			} else
			{
				vScroll->Show(true);
				hBox1_1->Layout();
				imageView->GetSize(&width,&height);
					vScroll->SetScrollbar(startViewY,height,bitmap.GetHeight(),height,true);
					y = 0;
			}
		wxMemoryDC dcBitmap(bitmap);
		if(bitmap.GetWidth() < width)
		{
			width = bitmap.GetWidth();
		}
		if(bitmap.GetHeight() < height)
		{
			height = bitmap.GetHeight();
		}
		wxBitmap bDst(width,height);
		wxMemoryDC dcDst(bDst);
		dcDst.Blit(0,0,width,height,&dcBitmap,startViewX,startViewY);
		dcDst.SelectObject(wxNullBitmap);
		wxAutoBufferedPaintDC dc(imageView);
		dc.Clear();
		dc.DrawBitmap(bDst,x,y);
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotEditTranslating::OnPaint not load screenshot "));
	}

}

void OPolyglotEditTranslating::OnFinishTranslating(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditTranslating::OnFinishTranslating"));
	progress->Finish();
	textOCR->Enable(true);
	textTranslate->Enable(true);
	textTranslate->SetValue(event.GetString());
	if(!oldText.IsSameAs(event.GetString()))
	{
		Save->Enable(true);
	}
	oldText = textTranslate->GetValue();
}
