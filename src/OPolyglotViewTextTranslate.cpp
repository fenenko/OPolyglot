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

#include "OPolyglotViewTextTranslate.h"
#include "OPolyglotEditTranslating.h"
#include "Utils.h"
#include "OPolyglotEvent.h"
#include <wx/clipbrd.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/arrimpl.cpp> 
#include <wx/dir.h>
#include <wx/xml/xml.h>
#if __WXGTK__
	#include "../res/icon.xpm"
#endif

WX_DEFINE_OBJARRAY(OPolyglotArrayIdLine);

enum{
	STYLE_TRANSLATE = 1,
	STYLE_NOT_TRANSLATE = 2
};

OPolyglotIdLine::OPolyglotIdLine(wxString &id,int start,int end)
{
	idText = id;
	startLine = start;
	endLine = end;
}


int OPolyglotIdLine::GetStart()
{
	return startLine;
}


int OPolyglotIdLine::GetEnd()
{
	return endLine;
}


wxString OPolyglotIdLine::GetId()
{
	return idText;
}


OPolyglotViewTextTranslate::OPolyglotViewTextTranslate(wxWindow *parent,wxString fileName)
	: GUIOPolyglotViewTextTranslate(parent)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	this->fileName = fileName;
	wxBitmap copyIcon = wxArtProvider::GetBitmap(wxART_COPY, wxART_BUTTON);
	wxBitmap clearIcon = wxArtProvider::GetBitmap(OPOLYGLOT_ART_CLEAR,wxART_BUTTON,copyIcon.GetSize());
	buttonCopy->SetBitmap(copyIcon);
	buttonClear->SetBitmap(clearIcon);
	this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	this->parent = parent;
	SetTitle(wxString::Format(wxT("OPolyglot %s"),_("translation text")));
	textTranslate->Clear();
	textTranslate->SetLexer(wxSTC_LEX_CONTAINER);
	textTranslate->AnnotationClearAll();
	textTranslate->StyleSetForeground(STYLE_TRANSLATE,wxColour(wxS("black")));
	textTranslate->StyleSetBold(STYLE_TRANSLATE,true);
	textTranslate->StyleSetForeground(STYLE_NOT_TRANSLATE,wxColour(wxS("gray")));
	textTranslate->StyleSetBold(STYLE_NOT_TRANSLATE,true);
	textTranslate->SetWrapMode( wxSTC_WRAP_WORD);
	textTranslate->SetLayoutCache(wxSTC_CACHE_DOCUMENT);
	textTranslate->SetEndAtLastLine(false);
	textTranslate->Bind(wxEVT_STC_DOUBLECLICK,&OPolyglotViewTextTranslate::OnDoubleClickText,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT,&OPolyglotViewTextTranslate::OnFinishEditTranslate,this);
	LoadXML();
	//wxPoint posParent = this->parent->GetPosition();
	//wxPoint pos = GetPosition();
	//pos.y = (posParent.y+25);
	//SetPosition(pos);
}


OPolyglotViewTextTranslate::~OPolyglotViewTextTranslate()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotViewTextTranslate"));
}


void OPolyglotViewTextTranslate::OnFinishEditTranslate(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::OnFinishEditTranslate"));
	this->Show(true);
	LoadXML(event.GetInt());
}


void OPolyglotViewTextTranslate::OnClose( wxCloseEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::OnClose"));
	Show(false);
}


void OPolyglotViewTextTranslate::OnCopy( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::OnCopy"));
	if(wxTheClipboard->Open())
	{
		    wxTheClipboard->SetData( new wxTextDataObject(textTranslate->GetText()) );
		    wxTheClipboard->Close();
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::OnCopy failed to open clipboard"));
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Failed to open clipboard")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
	}
}


void OPolyglotViewTextTranslate::OnClear(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::OnClear"));
	wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Are you sure you want to clear the translation text?")),wxT("OPolyglot"),wxYES_NO|wxNO_DEFAULT|wxICON_QUESTION);
	if(msg.ShowModal() == wxID_YES)
	{
		wxXmlDocument *doc = new wxXmlDocument();
		if(!doc->Load(fileName))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::OnClear error loading %s"),fileName);
			wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Error loading"),fileName),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			return;
		}
		for(;doc->GetRoot()->GetChildren();)
		{
			wxXmlNode *child = doc->GetRoot()->GetChildren();
			doc->GetRoot()->RemoveChild(child);
			delete child;
		}
		if(!doc->Save(fileName))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::OnClear Failed to save changes %s"),fileName);
			wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Failed to save changes"),fileName),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			return;
		}
		delete doc;
		if(!wxFileName::Rmdir(OPOLYGLOT_USER_DATA_IMG,wxPATH_RMDIR_RECURSIVE))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::OnClear Cannot be removed %s"),OPOLYGLOT_USER_DATA_IMG);
			wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Cannot be removed"),OPOLYGLOT_USER_DATA_IMG),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
		} else
		{
			if(!wxDir::Make(OPOLYGLOT_USER_DATA_IMG))	
			{
				OPOLYGLOT_ERROR(wxT("creating dir %s"),OPOLYGLOT_USER_DATA_IMG);
				wxSafeShowMessage("OPolyglot",wxString::Format(wxT("not created directory %s"),OPOLYGLOT_USER_DATA_IMG));
			}
		}

	}
	LoadXML();
}


void OPolyglotViewTextTranslate::OnDoubleClickText(wxStyledTextEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::OnDoubleClickText"));
	wxString findId = wxEmptyString;
	for(size_t i = 0; (i < ids.GetCount())&&findId.IsEmpty();++i)
	{
		if((ids.Item(i).GetStart() <= event.GetLine())&&(event.GetLine() < ids.Item(i).GetEnd()))
		{
			findId = ids.Item(i).GetId();
		}
	}
	if(findId.IsEmpty())
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotViewTextTranslate::OnDoubleClickText from line %d not find ID"),event.GetLine());
	} else
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglotViewTextTranslate::OnDoubleClickText %d %s"),event.GetLine(),findId);
		OPolyglotEditTranslating *edit=new OPolyglotEditTranslating(this,fileName,findId,event.GetLine()+1);
		edit->Show(true);
		this->Show(false);
	}
}


void OPolyglotViewTextTranslate::LoadXML(int oldLineCount )
{
	int countLines = 0;
	textTranslate->SetReadOnly(false);
	textTranslate->AnnotationClearAll();
	textTranslate->AnnotationSetVisible(wxSTC_ANNOTATION_STANDARD);
	if(oldLineCount == -1)
	{
		oldLineCount = textTranslate->GetLineCount();
		for(int i =0; i < textTranslate->GetLineCount();i++)
		{
			countLines += textTranslate->WrapCount(i);
		}
	} else
	{

		for(int  i = 0; i < oldLineCount-1;i++)
		{
			countLines += textTranslate->WrapCount(i);
		}
	}
	OPOLYGLOT_MESSAGE(wxT("OPolyglotViewTextTranslate::LoadXML(%d) ids %zu"),oldLineCount,ids.GetCount());
	OPOLYGLOT_DEBUG(wxT("OPolyglotViewTextTranslate::LoadXML old document %d visible lines %d"),textTranslate->GetLineCount(),countLines);
#if 0
	wxRect rect = this->parent->GetRect();
	wxPoint pos = GetPosition();
	pos.y = (rect.GetY()+rect.GetHeight()+5);
	SetPosition(pos);
#endif
	textTranslate->Clear();
	textTranslate->ClearAll();
	wxXmlDocument doc;
	if(!doc.Load(fileName))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::LoadXML not load %s"),fileName);
		return;
	}
	if(!doc.GetRoot()->GetName().IsSameAs(wxT("Texts")))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotViewTextTranslate::LoadXML not valid root %s not \"Texts\n"),doc.GetRoot()->GetName());
		return;
	}
	OPOLYGLOT_DEBUG(wxT("OPolyglotViewTextTranslate::LoadXML start GetLineCount(%d)"),textTranslate->GetLineCount());
	ids.Clear();
	int s = textTranslate->GetLineCount();
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().IsSameAs(wxT("Text")))
		{
			int start= textTranslate->GetTextLength();
			int startLine = textTranslate->GetLineCount() -s;
			if(child->GetAttribute(wxS("onlyOCR")).IsEmpty())
			{
				textTranslate->AppendText(child->GetAttribute(wxT("text")));
				int end = textTranslate->GetTextLength();
				textTranslate->StartStyling(start);
				textTranslate->SetStyling(end-start,STYLE_TRANSLATE);
			} else
			{
				textTranslate->AppendText(child->GetAttribute(wxT("original")));
				int end = textTranslate->GetTextLength();
				textTranslate->StartStyling(start);
				textTranslate->SetStyling(end-start,STYLE_TRANSLATE);
			}
			int endLine = textTranslate->GetLineCount() - s;
			if(!child->GetAttribute(wxS("id")).IsEmpty())
			{
				OPOLYGLOT_DEBUG(wxT("OPolyglotViewTextTranslate::LoadXML %s(%d,%d)"),child->GetAttribute(wxS("id")),startLine,endLine);
				wxString id = child->GetAttribute(wxS("id"));
				ids.Add(OPolyglotIdLine(id,startLine,endLine));
			} else
			{
				OPOLYGLOT_DEBUG(wxT("OPolyglotViewTextTranslate::LoadXML WARNING not find id"));
			}
		}
	}
	if((countLines != 0)&&(oldLineCount < textTranslate->GetLineCount())&&(0 <= (oldLineCount-2)))
	{
		textTranslate->AnnotationSetText(oldLineCount-2,wxS("----------------------------------------------------"));
		textTranslate->AnnotationSetStyle(oldLineCount-2,STYLE_NOT_TRANSLATE);
		OPOLYGLOT_DEBUG(wxT("OPolyglotViewTextTranslate::LoadXML oldLineCount-2 %d"),oldLineCount);

	} else
	{
		countLines = 0;
	}
	if(textTranslate->GetLineCount() != 0)
	{
		buttonCopy->Enable(true);
		buttonClear->Enable(true);
	}
	textTranslate->Update();
	textTranslate->SetFirstVisibleLine(countLines);
	textTranslate->SetReadOnly(true);

}


bool OPolyglotViewTextTranslate::ViewTranslate()
{
	LoadXML();
	Show(true);
	this->Raise();
	return true;
}
