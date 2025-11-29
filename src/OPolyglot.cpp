#include <wx/log.h>
#include "OPolyglot.h"
#include "OPolyglotEvent.h"
#include "Utils.h"
#include "Config.h"
#include <wx/clipbrd.h>
#include "../res/icon.xpm"
#include "../res/icon_copy.xpm"
#include <wx/panel.h>
#include <wx/rawbmp.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotSetup.h"
#include <wx/arrimpl.cpp> 
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/config.h>
#include <wx/display.h>
#include <wx/regex.h>

enum{
};


#if 0
/* XPM */

static const char *magic_xpm[] = {
/* columns rows colors chars-per-pixel */
"24 24 3 1 ",
"  c None",
". c black",
", c white",
/* pixels */
"                        ",
"        ,           ,   ",
"       ,.,        ,..,  ",
"      ,...,      ,...., ",
"      ,...,      ,...., ",
"       ,.,        ,..,  ",
"        ,    ..    ,,   ",
"            ....        ",
"           ......       ",
"          ...,,...      ",
"         .....,...      ",
"        ...,.....       ",
"       ...,,,...        ",
"      ...,,,...    ,,,  ",
"     ...,,,...    ,..,  ",
"    ...,,,...    ,...., ",
"   ...,,,...      ,..,  ",
"  ...,,,...        ,,   ",
"  ...,,...              ",
"   ......               ",
"    ....                ",
"     ..                 ",
"                        ",
"                        ",
};
#endif
enum{
	TIMER_ID,
	TIMER_MOUSE_ID,
	TIMER_PROGRESS_OCR_TRANSLATION_ID,
};

enum{
	MENU_EXIT,
	MENU_SETUP_LANGUAGES,
	MENU_VIEW,
};

OPolyglotTaskBar::OPolyglotTaskBar(wxWindow *parent) : wxTaskBarIcon()
{
	this->parent = parent;
	this->Bind(wxEVT_TASKBAR_LEFT_DOWN,&OPolyglotTaskBar::OnLeftDown,this);
}
void OPolyglotTaskBar::OnMenuExit(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_PROGRAMM);
	wxQueueEvent(this->parent,event);
}

void OPolyglotTaskBar::OnSetupLanguage(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}

void OPolyglotTaskBar::OnView(wxCommandEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	if(this->parent->IsShown())
	{
		((OPolyglot *)(this->parent))->SetVisible(false);
	} else
	{
		((OPolyglot *)(this->parent))->SetVisible(true);
	}
}

void OPolyglotTaskBar::OnLeftDown(wxTaskBarIconEvent& event)
{
	OPOLYGLOT_MESSAGE();
	if(this->parent->IsShown())
	{
		((OPolyglot *)(this->parent))->SetVisible(false);
	} else
	{
		((OPolyglot *)(this->parent))->SetVisible(true);
	}
}

wxMenu *OPolyglotTaskBar::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu();
	if(this->parent->IsShown())
	{
		menu->Append(MENU_VIEW,_("Hide"));
	} else
	{
		menu->Append(MENU_VIEW,_("Show"));
	}
	menu->Append(MENU_SETUP_LANGUAGES,_("Setup"),_("setup OPolyglot"));
	menu->Append(MENU_EXIT,_("E&xit"),_("exit in application OPolyglot"));
	
	this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnView,this,MENU_VIEW);
	this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnMenuExit,this,MENU_EXIT);
	this->Bind(wxEVT_MENU,&OPolyglotTaskBar::OnSetupLanguage,this,MENU_SETUP_LANGUAGES);
	return menu;
}




OPolyglot::OPolyglot(wxFrame *frame) 
	: GuiOPolyglot(frame)  
{
	SetIcon(wxICON(icon));
	this->ButtonCopyTranslate->SetBitmap(wxICON(icon_copy));
	this->ButtonCopyTranslate->SetToolTip(_("Copies the translation text to the clipboard."));
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxSize s = this->GetSize();
	viewDialogTranslator = false;
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT))
	{
		this->SetWindowStyle(this->GetWindowStyle()|wxSTAY_ON_TOP);
	} else
	{
		this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	}

	wxPoint pos((geom.width - s.GetWidth())/2,64);
	this->SetPosition(pos);
	timerClipboardChecking = new wxTimer(this,TIMER_ID);
	timerMouseState = new wxTimer(this,TIMER_MOUSE_ID);
	timerProgressOcrTranslation = new wxTimer(this,TIMER_PROGRESS_OCR_TRANSLATION_ID);
	taskBar = new OPolyglotTaskBar(this);
	if(!taskBar->SetIcon(wxBitmapBundle(icon_xpm),_("offline translator OPolyglot")))
	{
		OPOLYGLOT_ERROR();
	}
	this->Bind(wxEVT_TIMER,wxTimerEventHandler(OPolyglot::OnTimeCheckClipboard),this,TIMER_ID);
	this->Bind(wxEVT_TIMER,wxTimerEventHandler(OPolyglot::OnTimeCheckMouseState),this,TIMER_MOUSE_ID);
	this->Bind(wxEVT_TIMER,wxTimerEventHandler(OPolyglot::OnTimerProgressOCRTranslation),this,TIMER_PROGRESS_OCR_TRANSLATION_ID);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SEND_IMAGE,&OPolyglot::OnReceivImage,this);
	this->Bind(wxEVT_RIGHT_DOWN,&OPolyglot::OnRightClick,this);	
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglot::OnSetupLanguages,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT_PROGRAMM,&OPolyglot::OnExitProgramm,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION,&OPolyglot::OnExitThreadTranslation,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP,&OPolyglot::OnFinishSetupLanguages,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_OCR,&OPolyglot::OnExitThreadOCR,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_UPDATE_PROGRESS_MESSAGE,&OPolyglot::OnUpdateProgressMessage,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_HIDE,&OPolyglot::OnHide,this);

	if(this->EnableAutoTranslate->IsChecked())
	{
		timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
	}
	if (wxTheClipboard->Open()&&wxTheClipboard->IsSupported(wxDF_TEXT))
	{
		wxTextDataObject data;
		wxTheClipboard->GetData( data );
		lastClipboardText =  data.GetText();
	}  else
	{
		lastClipboardText = wxEmptyString;
	}
	wxTheClipboard->Close();
	mouseLeftButtonPressed = false;
	coordStartX = -1;
	coordStartY = -1;
	this->ScanLangs();
	this->MainVBox->Layout();
	this->Layout();
	this->Refresh();
	this->Update();
	imageForOCR = NULL;
	if( (0 == this->LanguageFrom->GetCount())||(0 == this->LanguageTo->GetCount()))
	{
		OPolyglotDownloadLanguage *frameDownload = new OPolyglotDownloadLanguage(this);
		this->SetVisible(false);
		frameDownload->Show();
	}
	wxArrayString test;
	test.Add(wxS("English|eng"));
	test.Add(wxS("Ukrainian|ukr"));
	test.Add(wxS("Russian|rus"));
	OPOLYGLOT_ERROR(wxT("test %d"),test.Index(wxS("Russian|*")));

}



OPolyglot::~OPolyglot()
{
	OPOLYGLOT_MESSAGE();
	//timerClipboardChecking->~wxTimer();
	delete timerClipboardChecking;
	//timerMouseState->~wxTimer();
	delete timerMouseState;
	delete timerProgressOcrTranslation;
}


void OPolyglot::FinishThread()
{

	timerProgressOcrTranslation->Stop();
	progressThreadTranslation->Destroy();
	if(this->IsShown())
	{
		if(this->EnableAutoTranslate->IsChecked())
		{
			if (wxTheClipboard->Open())
			{
				// This data objects are held by the clipboard,
				// so do not delete them in the app.
				wxTheClipboard->SetData( new wxTextDataObject(this->textTranslation->GetValue()) );
				lastClipboardText = this->textTranslation->GetValue();
				wxTheClipboard->Close();
			} else
			{
				OPOLYGLOT_ERROR(wxT("error open clipboard"));
			}
			timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
		} 
		if(this->OCRTranslate->IsChecked())
		{
			timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
		}
	}
	this->Enable(true);
	this->ButtonCopyTranslate->Enable(true);
	/* imitate pressed on buttonShowTranslate */
	this->buttonShowTranslate->SetValue(true);
	wxPostEvent(this->buttonShowTranslate,wxCommandEvent(wxEVT_TOGGLEBUTTON));
}


void OPolyglot::OnExitThreadTranslation(wxThreadEvent &event)
{
	
	OPOLYGLOT_MESSAGE(wxT("%d"),event.GetInt());
	threadTranslator = NULL;
	if((event.GetInt()!=0)&&(!event.GetString().IsEmpty()))
	{
		OPOLYGLOT_ERROR(wxT("error thread translation%s"),event.GetString());
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),event.GetString()),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
	}
	if(!event.GetString().IsEmpty())
	{
		this->textTranslation->Clear();
		this->textTranslation->AppendText(event.GetString());
	}
	FinishThread();
}


void OPolyglot::OnShowTranslate(wxCommandEvent &event)
{
	OPOLYGLOT_INFO(wxT("Show(%s)"),OPOLYGLOT_BOOL_TO_STRING(this->buttonShowTranslate->GetValue()));
	if(this->buttonShowTranslate->GetValue())
	{
		int width,height;
		if(!this->translatePanel->IsShown())
		{
			this->MainVBox->Layout();
			MainVBox->Fit(this);
			this->Refresh();
			this->GetSize(&width,&height);
			this->translatePanel->Show(true);
			if(height < 120)
			{
				this->SetSize(width,480);
			}
			this->buttonShowTranslate->SetLabel(_("hide the translation"));
		}
	} else
	{
		this->translatePanel->Show(false);
		this->buttonShowTranslate->SetLabel(_("show translation"));
		this->MainVBox->Layout();
		MainVBox->Fit(this);
		this->Refresh();
	}
}


void OPolyglot::OnCopyTextTranslate( wxCommandEvent& event ) 
{
	bool flagTimerClipboard = timerClipboardChecking->IsRunning();
	OPOLYGLOT_MESSAGE();
	timerClipboardChecking->Stop();
	if (wxTheClipboard->Open())
	{
    	// This data objects are held by the clipboard,
	    // so do not delete them in the app.
    	wxTheClipboard->SetData( new wxTextDataObject(this->textTranslation->GetValue()) );
		lastClipboardText = this->textTranslation->GetValue();
	    wxTheClipboard->Close();
	} else
	{
		OPOLYGLOT_ERROR(wxT("error open clipboard"));
	}
	if(flagTimerClipboard)
	{
		timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
	}

}

void OPolyglot::AddOrSetOriginalText(wxString text)
{
	OPOLYGLOT_MESSAGE(wxT("text length %ld"),text.Length());
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	bool flag = config->ReadBool(OPOLYGLOT_CONFIG_BOOL_METHOT_CREATION_TEXT,OPOLYGLOT_CONFIG_BOOL_METHOT_CREATION_TEXT);
	delete config;
	// Регулярний вираз для знаходження переносу рядка між двома маленькими буквами в Unicode
    wxRegEx regex("([\\p{L}])[\n\r]([\\p{L}])");
    // Заміна переносу рядка на пробіл
    wxString result = text;
	size_t res = regex.ReplaceAll(&result, wxS("\\1 \\2"));
	OPOLYGLOT_DEBUG(wxT("count Replace %ld"),res);
	if(flag)
	{
		this->textOriginal->Clear();
	}
	this->textOriginal->AppendText(result);
	if(flag)
	{
		this->textOriginal->AppendText(wxS(" "));
	}
}

void OPolyglot::OnExitThreadOCR(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("%d"),event.GetInt());
	threadOCR = NULL;
	if((event.GetInt()!=0)&&(!event.GetString().IsEmpty()))
	{
		OPOLYGLOT_ERROR(wxT("error thread ocr %s"),event.GetString());
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),event.GetString()),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		FinishThread();
		//imageForOCR->~OPolyglotImage();
		imageForOCR = NULL;
		return;
	}
	if(!event.GetString().IsEmpty())
	{
		AddOrSetOriginalText(event.GetString());
		//imageForOCR->~OPolyglotImage();
		imageForOCR = NULL;
		threadTranslator = new OPolyglotThreadTranslator(this,&configTranslatorFileYml,this->textOriginal->GetValue());
		threadTranslator->Run();
	} else
	{
		OPOLYGLOT_DEBUG(wxT("FinishThread"));
		FinishThread();	
		//imageForOCR->~OPolyglotImage();
		imageForOCR = NULL;
	}
}

void OPolyglot::OnShowOriginal(wxCommandEvent &event)
{
	OPOLYGLOT_INFO(wxT("Show(%s)"),OPOLYGLOT_BOOL_TO_STRING(this->buttonShowOriginal->GetValue()));
	if(this->buttonShowOriginal->GetValue())
	{
		this->textOriginal->Show(true);
		this->buttonShowOriginal->SetLabel(_("hide the text of the original"));
	} else
	{
		this->textOriginal->Show(false);
		this->buttonShowOriginal->SetLabel(_("show the text of the original"));
	}
	this->MainVBox->Layout();
}

void OPolyglot::OnHide(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE();
	this->SetVisible(false);
}


void OPolyglot::ScanLangs()
{
	OPOLYGLOT_INFO();
	wxXmlDocument doc;
	if(!doc.Load(OPOLYGLOT_GET_XML_DATA_FILE))
	{
		OPOLYGLOT_ERROR(wxT("load file download language %s"),OPOLYGLOT_GET_XML_DATA_FILE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_XML_DATA_FILE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	installCodeTranslator.Clear();
	installLanguageFrom.Clear();
	installLanguageTo.Clear();
	for(wxXmlNode *language=doc.GetRoot()->GetChildren();language;language = language->GetNext())
	{
		if(language->GetName().IsSameAs(OPOLYGLOT_NAME_NODE_LANGUAGE))
		{
			if(OPolyglotCheckThatLanguageInstalled(&doc,language))
			{
				wxString code = language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_CODE_FROM)+language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_CODE_TO);
				wxString valueFrom = wxString::Format(wxS("%s|%s"),language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_FROM),language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_CODE_FROM));
				wxString valueTo = wxString::Format(wxS("%s|%s"),language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_TO),language->GetAttribute(OPOLYGLOT_ATTRIBUTE_NODE_CODE_TO));
				if(installCodeTranslator.Index(code) == wxNOT_FOUND)
				{
					installCodeTranslator.Add(code);
				}
				if(installLanguageFrom.Index(valueFrom) == wxNOT_FOUND)
				{
					installLanguageFrom.Add(valueFrom);
				}
				if(installLanguageTo.Index(valueTo) == wxNOT_FOUND)
				{
					installLanguageTo.Add(valueTo);
				}
			}
		}
	}
	installLanguageFrom.Sort(false);
	installLanguageTo.Sort(false);
	OPOLYGLOT_DEBUG(wxT("installed languageFrom %ld"),installLanguageFrom.GetCount());
	for(size_t i =0; i < installLanguageFrom.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("%ld : %s %s %s")
				,i
				,installLanguageFrom.Item(i)
				,installLanguageFrom.Item(i).SubString(0,installLanguageFrom.Item(i).Length()-5)
				,installLanguageFrom.Item(i).SubString(
					installLanguageFrom.Item(i).Length()-3
					,installLanguageFrom.Item(i).Length()-1));
	}
	OPOLYGLOT_DEBUG(wxT("installed languageTo %ld"),installLanguageTo.GetCount());
	for(size_t i =0; i < installLanguageTo.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("%ld : %s"),i,installLanguageTo.Item(i));
	}
	OPOLYGLOT_DEBUG(wxT("installed translator %ld"),installCodeTranslator.GetCount());
	for(size_t i =0; i < installCodeTranslator.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("%ld : %s"),i,installCodeTranslator.Item(i));
	}

	this->ScanLanguageFrom();
	this->ScanLanguageTo();
	this->CreateTranslatorConfig();
}


void OPolyglot::ScanLanguageFrom()
{
#define GET_CODE_FROM	installLanguageFrom.Item(i).SubString(installLanguageFrom.Item(i).Length()-3,installLanguageFrom.Item(i).Length()-1)
#define GET_NAME_FROM 	installLanguageFrom.Item(i).SubString(0,installLanguageFrom.Item(i).Length()-5)
	OPOLYGLOT_MESSAGE();
	this->LanguageFrom->Clear();
	for(size_t i = 0; i < installLanguageFrom.GetCount();i++)
	{
		this->LanguageFrom->Append(GET_NAME_FROM);
	}
	if(0 < this->LanguageFrom->GetCount())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		int find = this->LanguageFrom->FindString(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT));
		if(find != wxNOT_FOUND)
		{
			this->LanguageFrom->SetSelection(find);
		} else
		{
			this->LanguageFrom->SetSelection(0);
		}
		config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,this->LanguageFrom->GetStringSelection());
	}
	OPOLYGLOT_DEBUG(wxT("LanguageFrom %s %d"),this->LanguageFrom->GetStringSelection(),this->LanguageFrom->GetSelection());
}

void OPolyglot::ScanLanguageTo()
{
#define GET_CODE_TO	installLanguageTo.Item(i).SubString(installLanguageTo.Item(i).Length()-3,installLanguageTo.Item(i).Length()-1)
#define GET_NAME_TO	installLanguageTo.Item(i).SubString(0,installLanguageTo.Item(i).Length()-5)
	wxXmlDocument doc;
	OPOLYGLOT_MESSAGE();
	wxString selectCodeLanguageFrom = wxEmptyString;
	this->LanguageTo->Clear();
	if(0 <= this->LanguageFrom->GetSelection() )
	{
		for(size_t i =0; (i < installLanguageFrom.GetCount())&&selectCodeLanguageFrom.IsEmpty();i++)
		{
			if(this->LanguageFrom->GetStringSelection().IsSameAs(GET_NAME_FROM))
			{
				selectCodeLanguageFrom =GET_CODE_FROM;
			}
		}
	}
	this->LanguageTo->Clear();
	for(size_t i = 0; i < installLanguageTo.GetCount();i++)
	{
		if(!selectCodeLanguageFrom.IsSameAs(GET_CODE_TO))
		{
			//if(codeLanguageTo.Index(GET_CODE_TO) == wxNOT_FOUND)
			if(this->LanguageTo->GetStrings().Index(GET_NAME_TO) == wxNOT_FOUND)
			{
				this->LanguageTo->Append(GET_NAME_TO);
			}

		}
	}
	/* start find available cross translate exmple POLISH -> UKRAINIAN : POLISH -> ENGLISH, ENGLISH -> UKRAINIAN */
	wxString codeToEng = selectCodeLanguageFrom+wxS("eng");
	if(installCodeTranslator.Index(codeToEng) != wxNOT_FOUND)
	{
		for(size_t i = 0; i < installLanguageTo.GetCount();i++)
		{
			wxString codeFromEng = wxS("eng")+GET_CODE_TO;
			if((installCodeTranslator.Index(codeFromEng) != wxNOT_FOUND)&&(!selectCodeLanguageFrom.IsSameAs(GET_CODE_TO)))
			{
				//if(codeLanguageTo.Index(GET_CODE_TO) == wxNOT_FOUND)
				if(this->LanguageTo->GetStrings().Index(GET_NAME_TO) == wxNOT_FOUND)
				{
					this->LanguageTo->Append(GET_NAME_TO);
				}
			}
		}
	}

	OPOLYGLOT_DEBUG(wxT("select this->LanguageTo"));
	if(0 < this->LanguageTo->GetCount())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		int find = this->LanguageTo->FindString(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT));
		if(find != wxNOT_FOUND)
		{
			this->LanguageTo->SetSelection(find);
		} else
		{
			this->LanguageTo->SetSelection(0);
		}
		config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,this->LanguageTo->GetStringSelection());
	}
}

void OPolyglot::OnUpdateProgressMessage(wxThreadEvent& event)
{
	OPOLYGLOT_DEBUG();
	wxMutexLocker lock(mutexProgressThreadTranslation);
	messageProgressThreadTranslation = event.GetString();
}


void OPolyglot::OnSelectLanguageFrom( wxCommandEvent& event )
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE();
	this->ScanLanguageTo();
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,this->LanguageFrom->GetStringSelection());
	CreateTranslatorConfig();
}


void OPolyglot::OnSelectLanguageTo( wxCommandEvent& event )
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE();
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,this->LanguageTo->GetStringSelection());
	CreateTranslatorConfig();
}


void OPolyglot::OnFinishSetupLanguages(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE();
	this->SetVisible(true);
	this->ScanLangs();
}

void OPolyglot::OnSetupLanguages(wxThreadEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	OPolyglotSetup *setup = new OPolyglotSetup(this);
	setup->Show();
}

void OPolyglot::OnExitProgramm(wxThreadEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	delete taskBar;
	this->Destroy();
}


void OPolyglot::OnRightClick(wxMouseEvent &event)
{
	OPOLYGLOT_MESSAGE();
}

void OPolyglot::OnReceivImage(wxThreadEvent &event)
{

	if(event.GetInt() == 0)
	{
		OPOLYGLOT_MESSAGE(wxT("non select area"));
		if((this->IsShown()))
		{
			if(this->OCRTranslate->IsChecked())
			{
				timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
			}
			if(this->EnableAutoTranslate->IsChecked())
			{
				timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
			} 
		}
		return;
	}
	coordStartX = -1;
	coordStartY = -1;
	imageForOCR = event.GetPayload<OPolyglotImage *>();
	StartThreadTranslation();
}
void OPolyglot::OnOCRTranslate( wxCommandEvent& event )
{
	if(this->OCRTranslate->IsChecked())
	{
		OPOLYGLOT_MESSAGE(wxT("checked"));
		timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("not checked"));
		timerMouseState->Stop();
	}

}

void OPolyglot::OnEnableClipboard( wxCommandEvent& event ) {
	OPOLYGLOT_MESSAGE(wxT("%s"),OPOLYGLOT_BOOL_TO_STRING(this->EnableAutoTranslate->IsChecked()));
	if(this->EnableAutoTranslate->IsChecked())
	{
		timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
	} else
	{
		timerClipboardChecking->Stop();
	}
}

void OPolyglot::SetVisible(bool flag)
{
	OPOLYGLOT_MESSAGE(wxT("%s"),OPOLYGLOT_BOOL_TO_STRING(flag));
	if(flag)
	{

		this->Show(true);
		flagShow = true;
		if(this->EnableAutoTranslate->IsChecked())
		{
			timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
		} 
		if(this->OCRTranslate->IsChecked())
		{
			timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
		}
		if((threadTranslator != NULL) )
		{
			OPOLYGLOT_DEBUG(wxT("Thread is running"));
			if(threadTranslator->IsRunning())
			{
				progressThreadTranslation->Show(true);
			}
		}
		if((threadOCR != NULL) )
		{
			OPOLYGLOT_DEBUG(wxT("Thread is running"));
			if(threadOCR->IsRunning())
			{
				progressThreadTranslation->Show(true);
			}
		}
	} else
	{
		this->Show(false);
		flagShow = false;
		timerClipboardChecking->Stop();
		timerMouseState->Stop();
		if((threadTranslator != NULL))
		{
			OPOLYGLOT_DEBUG(wxT("Thread is running"));
			if(threadTranslator->IsRunning())
			{
				progressThreadTranslation->Show(false);
			}
		}
		if((threadOCR != NULL))
		{
			OPOLYGLOT_DEBUG(wxT("Thread is running"));
			if(threadOCR->IsRunning())
			{
				progressThreadTranslation->Show(false);
			}
		}
	}
}

void OPolyglot::OnClose( wxCloseEvent& event ) { 
	OPOLYGLOT_MESSAGE();
	this->SetVisible(false);
}

void OPolyglot::OnTimerProgressOCRTranslation(wxTimerEvent &event)
{
	wxMutexLocker lock(mutexProgressThreadTranslation);
	if(!progressThreadTranslation->Pulse(messageProgressThreadTranslation))
	{
		OPOLYGLOT_WARNING(wxT("cancelled user"));
		timerProgressOcrTranslation->Stop();
		if(threadTranslator != NULL)
		{
			if(threadTranslator->IsRunning())
			{
				threadTranslator->Kill();
			}
			threadTranslator = NULL;
		}
		if(threadOCR != NULL)
		{
			if(threadOCR->IsRunning())
			{
				threadOCR->Kill();
			}
			threadOCR = NULL;
		}
		FinishThread();
	}
}

void OPolyglot::OnTimeCheckClipboard(wxTimerEvent &event)
{
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );

			if(!lastClipboardText.IsSameAs(data.GetText()))
			{
				
				lastClipboardText = data.GetText();
				AddOrSetOriginalText(lastClipboardText);
				timerClipboardChecking->Stop();
				timerMouseState->Stop();
				imageForOCR = nullptr;
				StartThreadTranslation();

			}
		}
		wxTheClipboard->Close();
	} else
	{
		OPOLYGLOT_ERROR(wxT("error open clipboards"));
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Error open clipboards")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
	}
}


void OPolyglot::OnStartTranslate(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("%s -> %s"),this->LanguageFrom->GetStringSelection(),this->LanguageTo->GetStringSelection());
	StartThreadTranslation();
}


void OPolyglot::StartThreadTranslation()
{
	OPOLYGLOT_MESSAGE(wxT("ocr %p %s"),imageForOCR,OPOLYGLOT_BOOL_TO_STRING(!IS_NULLPTR(imageForOCR)));
	if(configTranslatorFileYml.GetCount() == 0)
	{
		OPOLYGLOT_ERROR(wxT("error config files translator %ld"),configTranslatorFileYml.GetCount());
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Error not find config translator")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}

	if(!IS_NULLPTR(imageForOCR))
	{
		wxString langCode = wxEmptyString; //codeLanguageFrom.Item(this->LanguageFrom->GetSelection());
		for(size_t i =0; (i < installLanguageFrom.GetCount())&&langCode.IsEmpty();i++)
		{
			if(this->LanguageFrom->GetStringSelection().IsSameAs(GET_NAME_FROM))
			{
				langCode = GET_CODE_FROM;
			}
		}
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		wxString dirTraineddata = wxEmptyString;
		if(config.Read(OPOLYGLOT_CONFIG_STRING_OCR_METHOD,OPOLYGLOT_CONFIG_STRING_OCR_METHOD_DEFAULT).IsSameAs(wxT("BEST")))
		{
			OPOLYGLOT_DEBUG(wxT("select BEST OCR %s"),OPOLYGLOT_GET_DIR_BEST_TRAINEDDATA);
			dirTraineddata = OPOLYGLOT_GET_DIR_BEST_TRAINEDDATA;
		} else
		{
			OPOLYGLOT_DEBUG(wxT("select FAST OCR %s"),OPOLYGLOT_GET_DIR_FAST_TRAINEDDATA);
			dirTraineddata = OPOLYGLOT_GET_DIR_FAST_TRAINEDDATA;
		}
		if(!wxFileName::FileExists(wxString::Format(wxT("%s/%s.traineddata"),dirTraineddata,langCode)))
		{
			OPOLYGLOT_ERROR(wxT("OCR config error not find :%s/%s.traineddata"),dirTraineddata,langCode);
			return ;
		}
		threadOCR = new OPolyglotThreadOCR(this,dirTraineddata,langCode,imageForOCR);
		OPOLYGLOT_DEBUG(wxT("start threadOCR"));
		threadOCR->Run();
		threadTranslator = NULL;
	} else
	{
		threadTranslator = new OPolyglotThreadTranslator(this,&configTranslatorFileYml,wxString(this->textOriginal->GetValue()));
		threadTranslator->Run();
		threadOCR = NULL;
	}
	messageProgressThreadTranslation = wxEmptyString;
	progressThreadTranslation = new wxProgressDialog(wxT("OPolyglot"),_T("OCR and Translating..."),100,this,wxPD_APP_MODAL|wxPD_CAN_ABORT);
	progressThreadTranslation->Show();
	timerProgressOcrTranslation->Start(200);
	this->Enable(false);
}

void OPolyglot::CreateTranslatorConfig()
{
	OPOLYGLOT_MESSAGE(wxT("%s -> %s"),this->LanguageFrom->GetStringSelection(),this->LanguageTo->GetStringSelection());
	configTranslatorFileYml.Clear();
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	if((0 <= this->LanguageFrom->GetSelection() )||(0 <= this->LanguageTo->GetSelection() ))
	{
		wxString code = wxEmptyString;//codeLanguageFrom.Item(this->LanguageFrom->GetSelection())+codeLanguageTo.Item(this->LanguageTo->GetSelection());
		for(size_t i = 0; (i < installLanguageFrom.GetCount())&&code.IsEmpty();i++)
		{
			if(this->LanguageFrom->GetStringSelection().IsSameAs(GET_NAME_FROM))
			{
				code = GET_CODE_FROM;
			}
		}
		for(size_t i =0; (i < installLanguageTo.GetCount())&&(code.Length() == 3);i++)
		{
			if(this->LanguageTo->GetStringSelection().IsSameAs(GET_NAME_TO))
			{
				code = code + GET_CODE_TO;
			}
		}
		if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST")))
		{
			OPOLYGLOT_DEBUG(wxT("find translation for BEST method : %s"),code);
			if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,code)))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
			if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,code))&&(configTranslatorFileYml.GetCount()==0))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
			if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,code))&&(configTranslatorFileYml.GetCount()==0))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
		} else /*if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST"))) */
		{
			OPOLYGLOT_DEBUG(wxT("find translation for FAST method : %s"),code);
			if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,code)))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
			if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,code))&&(configTranslatorFileYml.GetCount()==0))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
			if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,code))&&(configTranslatorFileYml.GetCount()==0))
			{
				configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,code));
			}
		} /*  else if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST"))) */
		if(0 == configTranslatorFileYml.GetCount())
		{
			/* start find cross English translation */
			wxString codeToEng = wxEmptyString;//codeLanguageFrom.Item(this->LanguageFrom->GetSelection())+wxS("eng");
			for(size_t i = 0; (i < installLanguageFrom.GetCount())&&codeToEng.IsEmpty();i++)
			{
				if(this->LanguageFrom->GetStringSelection().IsSameAs(GET_NAME_FROM))
				{
					codeToEng = GET_CODE_FROM+wxS("eng");
				}
			}
			wxString codeFromEng = wxEmptyString;//wxS("eng")+codeLanguageTo.Item(this->LanguageTo->GetSelection());
			for(size_t i =0;(i < installLanguageTo.GetCount())&&codeFromEng.IsEmpty();i++)
			{
				if(this->LanguageTo->GetStringSelection().IsSameAs(GET_NAME_TO))
				{
					codeFromEng = wxS("eng")+GET_CODE_TO;
				}
			}
			if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST")))
			{
				OPOLYGLOT_DEBUG(wxT("start find cross translation for BEST method : %s -> %s"),codeToEng,codeFromEng);
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng)))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng))&&(configTranslatorFileYml.GetCount()==0))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng))&&(configTranslatorFileYml.GetCount()==0))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount()==1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount()==1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount()==1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(configTranslatorFileYml.GetCount() < 2)
				{
					OPOLYGLOT_ERROR(wxT("not find for BEST full method translation"));
					configTranslatorFileYml.Clear();
				}
			} else /* if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST"))) */
			{
				OPOLYGLOT_DEBUG(wxT("start find cross translation for FAST method : %s -> %s"),codeToEng,codeFromEng);
				if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng)))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng))&&(configTranslatorFileYml.GetCount()==0))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng))&&(configTranslatorFileYml.GetCount()==0))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeToEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount() == 1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/tiny.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount()==1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base-memory.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(wxFileName::FileExists(wxString::Format(wxT("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng))&&(configTranslatorFileYml.GetCount()==1))
				{
					configTranslatorFileYml.Add(wxString::Format(wxS("%s/base.%s/config.yml"),OPOLYGLOT_USER_DATA,codeFromEng));
				}
				if(configTranslatorFileYml.GetCount() < 2)
				{
					OPOLYGLOT_ERROR(wxT("not find for FAST full method translation"));
					configTranslatorFileYml.Clear();
				}
			} /* else if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST"))) */
		} /*  if(0 == configTranslatorFileYml.GetCount()) */
	} /* if((0 <= this->LanguageFrom->GetSelection() )||(0 <= this->LanguageTo->GetSelection() )) */
	OPOLYGLOT_DEBUG(wxT("Select config file : %ld"),configTranslatorFileYml.GetCount());
	for(size_t i = 0; i < configTranslatorFileYml.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("%ld : %s"),i,configTranslatorFileYml.Item(i));
	}
	delete config;
}

void OPolyglot::StartTranslation()
{
	OPOLYGLOT_MESSAGE(wxT("Translator configuration %s -> %s"),this->LanguageFrom->GetStringSelection(),this->LanguageTo->GetStringSelection());
	for(size_t i =0; i < configTranslatorFileYml.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("\t%ld : %s"),i,configTranslatorFileYml.Item(i));
	}
	StartThreadTranslation();
}



void OPolyglot::OnTimeCheckMouseState(wxTimerEvent &event)
{
	wxMouseState mouseState = wxGetMouseState();
	if(mouseState.LeftIsDown())
	{

		OPOLYGLOT_MESSAGE(wxT("OPolyglotFullscreenFrame"));
		if((coordStartX == -1)&&(coordStartY == -1))
		{
			coordStartX = mouseState.GetX();
			coordStartY = mouseState.GetY();
		}
		imageForOCR = new OPolyglotImage();
		fullscreen = new OPolyglotFullscreenFrame(this,imageForOCR);
		timerMouseState->Stop();
	}
	event.Skip();

}

