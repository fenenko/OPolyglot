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


#include "OPolyglotProcessingRules.h"
#include "Utils.h"
#include "OPolyglotEvent.h"
#include "OPolyglotDialogError.h"
#ifdef __WXGTK__
#include "../res/icon.xpm"
#endif
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/regex.h>
#include <wx/msgdlg.h>
#include <wx/menu.h>

enum{
	MENU_EDIT,
	MENU_REMOVE,
};

OPolyglotMultiline::OPolyglotMultiline(wxWindow *parent,wxString editLine,bool readOnly) : GUIOPolyglotMultilineText(parent)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotMultiline"));
	if(readOnly)
	{
		this->ValueRO->SetValue(editLine);
		this->Cancel->Show(false);
		this->Value->Show(false);
		this->ValueRO->Show(true);
	}
	this->VBox->Layout();
	this->MainBox->Layout();
	this->Value->SetValue(editLine);
}

OPolyglotMultiline::~OPolyglotMultiline()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotMultiline"));
}

void OPolyglotMultiline::OnOk(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotMultiline::OnOk"));
	this->EndModal(wxID_OK);
}

void OPolyglotMultiline::OnCancel(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotMultiline::OnCancel"));
	this->EndModal(wxID_CANCEL);
}

void OPolyglotMultiline::OnClose(wxCloseEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotMultiline::OnClose"));
	this->EndModal(wxID_CANCEL);
}

wxString OPolyglotMultiline::GetValue()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotMultiline::GetValue"));
	return this->Value->GetValue();
}

OPolyglotEditorRule::OPolyglotEditorRule(wxWindow *parent,long index,wxString regEx,wxString replace,wxString comment) : GUIOPolyglotEditorRule(parent)
{
	int w,h;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditorRule"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
#else
	SetIcon(wxICON(icon));
#endif
	this->parent = parent;
	this->index = index;
	this->RegEx->SetValue(regEx);
	this->ReplacementRule->SetValue(replace);
	this->Comment->SetValue(comment);
	testString = wxEmptyString;
	if(this->index == -1)
	{
		this->Comment->SetValue(_("new regular expression rule for processing text"));
	}
	this->MainBox->Layout();
	this->MainBox->Fit(this);
	this->GetSize(&w,&h);
	this->SetSize(480,h);
	Show();

}

OPolyglotEditorRule::~OPolyglotEditorRule()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotEditorRule"));
}

void OPolyglotEditorRule::OnClose(wxCloseEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditorRule::OnClose"));
	wxThreadEvent *sendEvent = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP);
	sendEvent->SetInt(-1);
	wxQueueEvent(parent,sendEvent);
}

void OPolyglotEditorRule::OnSave(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditorRule::OnSave"));
	wxThreadEvent *sendEvent = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP);
	sendEvent->SetInt(0);
	wxQueueEvent(parent,sendEvent);
}

wxString OPolyglotEditorRule::GetRegEx()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditorRule::GetRegEx"));
	return this->RegEx->GetValue();
}

wxString OPolyglotEditorRule::GetReplace()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditorRule::GetReplace"));
	return this->ReplacementRule->GetValue();
}

wxString OPolyglotEditorRule::GetComment()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditorRule::GetComment"));
	return this->Comment->GetValue();
}

void OPolyglotEditorRule::OnCancel(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditorRule::OnCancel"));
	wxThreadEvent *sendEvent = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP);
	sendEvent->SetInt(-1);
	wxQueueEvent(parent,sendEvent);
}

void OPolyglotEditorRule::OnTest(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditorRule::OnTest"));
	OPolyglotMultiline *inputTest = new OPolyglotMultiline(this,testString,false);
	if(inputTest->ShowModal() ==wxID_CANCEL)
	{
		OPOLYGLOT_DEBUG(wxT("user cancel"));
		inputTest->Destroy();
		return;
	}
	testString = inputTest->GetValue();
	delete inputTest;
	wxRegEx regex(RegEx->GetValue());
	wxString result = wxString::Format(wxS("%s"),testString);
	wxString replace = ReplacementRule->GetValue();
	replace.Replace(wxS("\\n"),"\n");
	replace.Replace(wxS("\\r"),"\r");
	replace.Replace(wxS("\\t"),"\t");
	size_t count = regex.ReplaceAll(&result,replace);//wxString::Format(wxS("%s"),replace));
	OPOLYGLOT_MESSAGE(wxT("finish replace %zu"),count);
	OPolyglotMultiline *outputTest = new OPolyglotMultiline(this,result,true);
	outputTest->ShowModal();
	delete outputTest;
	//outputTest.Destroy();
}

long OPolyglotEditorRule::GetItem()
{
	return index;
}

void OPolyglotEditorRule::OnFinishTest(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotEditorRule::OnFinishTest"));
}


OPolyglotListProcessingRules::OPolyglotListProcessingRules(wxEvtHandler *handler,wxString nodeName) : GUIOPolyglotListRules(NULL)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules(%s)"),nodeName);
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
#else
	SetIcon(wxICON(icon));
#endif
	this->handler = handler;
	doc = new wxXmlDocument();
	if(!doc->Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotListProcessingRules error load data file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		OPolyglotDialogError msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE));
		wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
		return;
	}
	this->ListRules->InsertColumn(0,wxT("RegEx"));
	this->ListRules->InsertColumn(1,wxT("Replacement"));
	this->ListRules->InsertColumn(2,wxT("Comment"));
	nodePreprocessing = NULL;
	for(wxXmlNode *child = doc->GetRoot()->GetChildren();child&&(nodePreprocessing == NULL);child = child->GetNext())
	{
		if(child->GetName().IsSameAs(nodeName))
		{
			OPOLYGLOT_DEBUG(wxT("IsSameAs"));
			nodePreprocessing = child;
		}
	}
	BuildList();
	this->MainBox->Layout();
	Show();
}

void OPolyglotListProcessingRules::BuildList()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::BuildList"));
	this->ListRules->DeleteAllItems();
	for(wxXmlNode *rule = nodePreprocessing->GetChildren();rule;rule = rule->GetNext())
	{
		if(rule->GetName().IsSameAs(wxS("Rule")))
		{
			size_t index = this->ListRules->InsertItem(this->ListRules->GetItemCount(),rule->GetAttribute(wxS("regEx")));
			this->ListRules->SetItem(index,1,rule->GetAttribute(wxS("replaceRule")));
			this->ListRules->SetItem(index,2,rule->GetAttribute(wxS("comment")));
		}
	}
	this->ListRules->SetColumnWidth(0,-1);
	this->ListRules->SetColumnWidth(1,-1);
	this->ListRules->SetColumnWidth(2,-1);
}

OPolyglotListProcessingRules::~OPolyglotListProcessingRules()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglotListProcessingRules"));
	nodePreprocessing = NULL;	
	delete doc;
	if(!IS_NULLPTR(editor))
	{
		delete editor;
	}

}

void OPolyglotListProcessingRules::OnClose(wxCloseEvent& event)
{
	bool flagExit = true;
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::OnClose"));
	if(flagChangeRules)
	{
		wxMessageDialog msg(this
				,_("Your rules have changed, they have not been saved.\nDo you want to exit without saving?")
				,wxS("OPolyglot")
				,wxYES_NO);
		if(msg.ShowModal() == wxID_NO)
		{
			flagExit = false;
		}
	}
	if(flagExit)
	{
		wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
	}
}

void OPolyglotListProcessingRules::OnFinishNewRule(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::OnFinishNewRule"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotListProcessingRules::OnFinishNewRule,this);
	if(event.GetInt() == 0)
	{
		size_t index = this->ListRules->InsertItem(this->ListRules->GetItemCount(),editor->GetRegEx());
		this->ListRules->SetItem(index,1,editor->GetReplace());
		this->ListRules->SetItem(index,2,editor->GetComment());
		flagChangeRules = true;
		this->Save->Enable(true);
	}
	delete editor;
	editor = NULL;
	Show(true);
}

void OPolyglotListProcessingRules::OnFinishChangeRule(wxThreadEvent& event) 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::OnFinishChangeRule"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotListProcessingRules::OnFinishChangeRule,this);
	if(event.GetInt() == 0)
	{
		size_t index = editor->GetItem();
		this->ListRules->SetItem(index,0,editor->GetRegEx());
		this->ListRules->SetItem(index,1,editor->GetReplace());
		this->ListRules->SetItem(index,2,editor->GetComment());
		flagChangeRules = true;
		this->Save->Enable(true);
	}
	delete editor;
	editor = NULL;
	Show(true);

}

void OPolyglotListProcessingRules::OnSelectItem(wxListEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::OnSelectItem(%zu)"),event.GetIndex());
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotListProcessingRules::OnFinishChangeRule,this);
	editor = new OPolyglotEditorRule(this
			,event.GetIndex()
			,ListRules->GetItemText(event.GetIndex(),0)
			,ListRules->GetItemText(event.GetIndex(),1)
			,ListRules->GetItemText(event.GetIndex(),2));
	this->Show(false);
}

void OPolyglotListProcessingRules::OnAdd(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::OnAdd"));
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotListProcessingRules::OnFinishNewRule,this);
	editor = new OPolyglotEditorRule(this,-1,wxEmptyString,wxEmptyString,wxEmptyString);
	this->Show(false);
}

void OPolyglotListProcessingRules::OnTest(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::OnTest"));
	OPolyglotMultiline *inputTest = new OPolyglotMultiline(this,testString,false);
	if(inputTest->ShowModal() ==wxID_CANCEL)
	{
		OPOLYGLOT_DEBUG(wxT("user cancel"));
		inputTest->Destroy();
		return;
	}
	testString = inputTest->GetValue();
	delete inputTest;
	wxString result = wxString::Format(wxS("%s"),testString);
	for(int i = 0; i < this->ListRules->GetItemCount();i++)
	{
		wxRegEx regex(this->ListRules->GetItemText(i,0));
		wxString replace = this->ListRules->GetItemText(i,1);
		replace.Replace(wxS("\\n"),"\n");
		replace.Replace(wxS("\\r"),"\r");
		replace.Replace(wxS("\\t"),"\t");
		size_t count = regex.ReplaceAll(&result,replace);//wxString::Format(wxS("%s"),replace));
		OPOLYGLOT_MESSAGE(wxT("%d finish replace %zu"),i,count);
	}
	OPolyglotMultiline *outputTest = new OPolyglotMultiline(this,result,true);
	outputTest->ShowModal();
	delete outputTest;
	//outputTest.Destroy();
}

void OPolyglotListProcessingRules::OnListRulesMenu(wxListEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::OnListRulesMenu(%zu)"),event.GetIndex());
	menu = new wxMenu();
	itemSelect = event.GetIndex();
	this->Bind(wxEVT_MENU,&OPolyglotListProcessingRules::OnMenuEdit,this,MENU_EDIT);
	this->Bind(wxEVT_MENU,&OPolyglotListProcessingRules::OnMenuDelete,this,MENU_REMOVE);
	menu->Append(MENU_EDIT,_("Edit rule"));
	menu->Append(MENU_REMOVE,_("Remove rule"));
	PopupMenu(menu,event.GetPoint().x,event.GetPoint().y);
}


void OPolyglotListProcessingRules::OnMenuEdit(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::OnMenuEdit(%zu)"),itemSelect);
	this->Unbind(wxEVT_MENU,&OPolyglotListProcessingRules::OnMenuEdit,this,MENU_EDIT);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglotListProcessingRules::OnFinishNewRule,this);
	editor = new OPolyglotEditorRule(this
			,itemSelect
			,ListRules->GetItemText(itemSelect,0)
			,ListRules->GetItemText(itemSelect,1)
			,ListRules->GetItemText(itemSelect,2));
	this->Show(false);
}


void OPolyglotListProcessingRules::OnMenuDelete(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::OnMenuDelete"));
	this->Unbind(wxEVT_MENU,&OPolyglotListProcessingRules::OnMenuDelete,this,MENU_REMOVE);
	wxMessageDialog msg(this,_("Are you sure you want to remove this rule?"),wxS("OPolyglot"),wxYES_NO);
	if(msg.ShowModal() == wxID_YES)
	{
		OPOLYGLOT_DEBUG(wxT("delete rule %zu"),itemSelect);
		ListRules->DeleteItem(itemSelect);
		flagChangeRules = true;
		this->Save->Enable(true);
	}
	itemSelect = -1;
}

void OPolyglotListProcessingRules::OnSave(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotListProcessingRules::OnSave(%d,%d)"),ListRules->GetItemCount(),ListRules->GetColumnCount());
	for(wxXmlNode *rule = nodePreprocessing->GetChildren();rule;)
	{
		wxXmlNode *deleteNode = rule;
		rule = rule->GetNext();
		if(deleteNode->GetName().IsSameAs(wxS("Rule")))
		{
			if(!nodePreprocessing->RemoveChild(deleteNode))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglotListProcessingRules::OnSave failed to remove rule %s"),deleteNode->GetAttribute(wxS("regEx")));
				OPolyglotDialogError msg(this,wxString::Format(wxS("%s %s"),_("Failed to remove rule"),deleteNode->GetAttribute(wxS("regEx"))));
			}
			delete deleteNode;
		}
	}
	for(int i = 0; i < ListRules->GetItemCount();i++)
	{
		wxString regEx = ListRules->GetItemText(i,0);
		wxString replacement = ListRules->GetItemText(i,1);
		wxString comment = ListRules->GetItemText(i,2);
		wxXmlNode *newNode = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Rule"));
		newNode->AddAttribute(wxS("regEx"),regEx);
		newNode->AddAttribute(wxS("replaceRule"),replacement);
		newNode->AddAttribute(wxS("comment"),comment);
		nodePreprocessing->AddChild(newNode);
	}
	if(!doc->Save(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotListProcessingRules::OnSave error save file %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		OPolyglotDialogError msg(this,wxString::Format(wxS("%s %s"),_("Error save file")));
	}
	this->Save->Enable(false);
	flagChangeRules = false;

}

