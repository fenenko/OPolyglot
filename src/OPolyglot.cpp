#include <wx/log.h>
#include <wx/dcscreen.h>
#include "OPolyglot.h"
#include "Translator.h"
#include "Utils.h"
#include "Config.h"
#include <wx/clipbrd.h>
#include <wx/display.h>
#include "../res/icon.xpm"
#include <wx/panel.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
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
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

enum{
};

wxDEFINE_EVENT(wxEVT_COMMAND_OPOLYGLOT_SELECT_AREA,	wxThreadEvent);
wxDEFINE_EVENT(wxEVT_COMMAND_OPOLYGLOT_EXIT_PROGRAMM,	wxThreadEvent);
wxDEFINE_EVENT(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION,	wxThreadEvent);
wxDEFINE_EVENT(wxEVT_COMMAND_OPOLYGLOT_SETUP_LANGUAGES,	wxThreadEvent);
wxDEFINE_EVENT(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES,	wxThreadEvent);

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
	wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP_LANGUAGES));
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
	//wxQueueEvent(this->parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_VIEW));
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

FullscreenFrame::FullscreenFrame(wxWindow *parent) : GUIFullscreen(parent)
{
	OPOLYGLOT_MESSAGE();
	this->Show(false);
	wxScreenDC dc;
	int w,h;
	dc.GetSize(&w,&h);
	bitmap = wxBitmap(w,h);
	wxMemoryDC memDC;
	memDC.SelectObject(bitmap);
	memDC.Blit(0,0,w,h,&dc,0,0);
	memDC.SelectObject(wxNullBitmap);
	this->parent = parent;
	wxMouseState state = wxGetMouseState();
	startX = state.GetX();
	startY = state.GetY();
	oldX = startX;
	oldY = startY;
	this->Bind(wxEVT_TIMER,&FullscreenFrame::OnTimeMouseState,this);
	timer = new wxTimer();
	timer->SetOwner(this,TIMER_ID);
	timer->Start(TIMEOUT_CHECK_MOUSE_STATE);
	//this->SetCursor(wxICON(magic_xpm));
	Bind(wxEVT_PAINT, &FullscreenFrame::OnPaint, this);
	//this->Show(true);
	//this->Refresh();
	wxDisplay dis(this);
	wxRect r1 = dis.GetGeometry();
	timePressedLeft = 0;
	//action.MouseMove(startX,startY);
	OPOLYGLOT_DEBUG(wxT("mouse %d %d display %d %d %dx%d"),startX,startY,r1.GetX(),r1.GetY(),r1.GetWidth(),r1.GetHeight());
	//this->SetFocus();
	//action.MouseDown(wxMOUSE_BTN_LEFT);
	//wxMilliSleep(50);
	//action.MouseUp(wxMOUSE_BTN_LEFT);
	//action.MouseDown(wxMOUSE_BTN_LEFT);
	timer->Start(TIMEOUT_FULLSCREAN_CHECK_MOUSE_STATE);
}
FullscreenFrame::~FullscreenFrame()
{
	timer->Stop();
	OPOLYGLOT_MESSAGE();
}

void FullscreenFrame::OnMouseLeftUp( wxMouseEvent& event ) 
{
	OPOLYGLOT_MESSAGE(wxT("FullscreenFrame"));
}

void FullscreenFrame::OnTimeMouseState(wxTimerEvent &event)
{
	timer->Stop();
	wxMouseState state = wxGetMouseState();
	if(!state.LeftIsDown())
	{
		int x,y,w,h;
		if(timePressedLeft  < TIME_PRESSED_LEFT )
		{

			OPOLYGLOT_MESSAGE(wxT("time pressed left is small %d"),timePressedLeft);
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SELECT_AREA);
			event->SetString(wxEmptyString);
			wxQueueEvent(this->parent,event);
			this->Destroy();
			return;
		}
		if(state.GetX() < startX)
		{
			x = startX;
			startX = state.GetX();
		} else
		{
			x = state.GetX();
		}
		if(state.GetY() < startY)
		{
			y = startY;
			startY = state.GetY();
		} else
		{
			y = state.GetY();
		}
		w = x - startX;
		h = y - startY;
		if((32 < (w))&&(16 < (h)))
		{
			OPOLYGLOT_MESSAGE(wxT("%d %d %dx%d"),startX,startY,w,h);
			wxBitmap bitmapArea(w,h);
			wxMemoryDC screenDC;
			screenDC.SelectObject(bitmap);
			wxMemoryDC memDC;
			memDC.SelectObject(bitmapArea);
			if(!memDC.StretchBlit(0,0,w,h,&screenDC,startX,startY,w,h))
			{
				OPOLYGLOT_ERROR(wxT("memDC.StretchBlit(0,0,%d,%d,&dc,%d,%d,%d,%d)"),w,h,startX,startY,w,h);
			} else
			{
				OPOLYGLOT_DEBUG(wxT("select area %d %d %dx%d"),startX,startY,w,h);
				memDC.SelectObject(wxNullBitmap);
				wxString str = wxFileName::GetTempDir();
				str.Append(wxT("/area.png"));
				bitmapArea.SaveFile(str,wxBITMAP_TYPE_PNG);
				wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SELECT_AREA);
				event->SetString(str);
				wxQueueEvent(this->parent,event);
				this->Destroy();
				return;

			}

		} else
		{
			OPOLYGLOT_MESSAGE(wxT("is small select AREA %dx%d"),w,h);
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SELECT_AREA);
			event->SetString(wxEmptyString);
			wxQueueEvent(this->parent,event);
			this->Destroy();
			return;
		}
	} else
	{
		if((startX != state.GetX())||(startY != state.GetY()))
		{
			this->Show(true);
			this->ShowFullScreen(true);
			this->SetFocus();
			this->Refresh();
		}
		timePressedLeft++;
		if((oldX != state.GetX())||(oldY != state.GetY()))
		{
			this->Refresh(true);
		}
		oldX = state.GetX();
		oldY = state.GetY();
	}
	timer->Start(TIMEOUT_FULLSCREAN_CHECK_MOUSE_STATE);
}
void FullscreenFrame::OnPaint(wxPaintEvent& event)
{
	wxColour col;
	wxMouseState state = wxGetMouseState();
	int x1,y1,x2,y2;
	wxDisplay dis(this);
	if(startX < state.GetX())
	{
		x1 = startX;
		x2 = state.GetX();
	} else
	{
		x1 = state.GetX();
		x2 = startX;
	}
	if(startY < state.GetX())
	{
		y1 = startY;
		y2 = state.GetY();
	} else
	{
		y1 = state.GetY();
		y2 = startY;
	}
#if 0
	OPOLYGLOT_DEBUG(wxT("mouse %d %d %dx%d"),x1,y1,x2-x1,y2-y1);
#endif
	wxPaintDC dc(this);
	dc.DrawBitmap(bitmap,0,0);
	dc.GetPixel(startX,startY,&col);
	col.Set(~col.GetRed(),~col.GetGreen(),~col.GetBlue());
	wxPen pen(col,2,wxPENSTYLE_SHORT_DASH );
	dc.SetPen(pen);
	dc.DrawLine(x1,y1,x2,y1);
	dc.DrawLine(x2,y1,x2,y2);
	dc.DrawLine(x1,y2,x2,y2);
	dc.DrawLine(x1,y2,x1,y1);
}



OPolyglot::OPolyglot(wxFrame *frame) 
	: GuiOPolyglot(frame) , wxThreadHelper()
{
	SetIcon(wxICON(icon));
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
		this->SetWindowStyle(this->GetWindowStyle() ^ wxSTAY_ON_TOP);
	}
#if 0
	config->Write(wxT("test"),wxT("TEST"));
	OPOLYGLOT_MESSAGE(wxT("config %s"),config->GetPath());
	delete config;
#endif

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
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SELECT_AREA,&OPolyglot::OnSelectArea,this);
	this->Bind(wxEVT_RIGHT_DOWN,&OPolyglot::OnRightClick,this);	
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP_LANGUAGES,&OPolyglot::OnSetupLanguages,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT_PROGRAMM,&OPolyglot::OnExitProgramm,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION,&OPolyglot::OnExitThreadTranslation,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES,&OPolyglot::OnFinishSetupLanguages,this);

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
	if( (0 == this->LanguageFrom->GetCount())||(0 == this->LanguageTo->GetCount()))
	{
		OPolyglotDownloadLanguage *frameDownload = new OPolyglotDownloadLanguage(this);
		frameDownload->Show();
		this->ScanLangs();
	}
	this->MainVBox->Layout();
	this->Layout();
	this->Refresh();
	this->Update();
}



OPolyglot::~OPolyglot()
{
	OPOLYGLOT_MESSAGE();
}

wxThread::ExitCode OPolyglot::Entry()
{
	long timeOCR = 0;
	wxString result = wxEmptyString;
	OPOLYGLOT_MESSAGE(wxT("start"));
	wxStopWatch sw;
	if(!filenameImageAreaForOCR.IsEmpty())
	{
		int ret;
		wxString langCode = this->GetLangCodeForOCR();
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
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION);
			event->SetInt(-1);
			event->SetString(wxString::Format(wxT("%s :%s/%s.traineddata"),_("Error file missing:"),dirTraineddata,langCode));
			wxQueueEvent(this, event);
			return (wxThread::ExitCode)-1;
		}


		OPOLYGLOT_DEBUG(wxT("start ocr %s"),filenameImageAreaForOCR);
		mutexProgressThreadTranslation.Lock();
		messageProgressThreadTranslation = wxT("OCR...");
		mutexProgressThreadTranslation.Unlock();
		tesseract::TessBaseAPI ocr;
		OPOLYGLOT_DEBUG(wxT("ocr.Init(%s,%s)"),dirTraineddata,langCode);
		ret = ocr.Init(dirTraineddata,langCode);
		if(ret)
		{
			OPOLYGLOT_ERROR(wxT("tesseract init %d"),ret);
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION);
			event->SetInt(-1);
			event->SetString(wxString::Format(wxT("%s %d"),_("error tesseract init"),ret));
			wxQueueEvent(this,event);
			return (wxThread::ExitCode)-1;
		}
		Pix *image = pixRead(filenameImageAreaForOCR.utf8_str());
		ocr.SetImage(image);
		result = wxString(ocr.GetUTF8Text(),wxConvUTF8);

		/*
		 *after OCR many chars '\n'  what breaks translating, this code replace '\n' on ' '
		 */
		for(size_t i =1; i < result.Length();i++)
		{
			if((wxT('\n') == result.GetChar(i))
					&&((wxT('.') != result.GetChar(i-1))
						||(wxT(',') != result.GetChar(i-1))
						||(wxT(':') != result.GetChar(i-1))
						||(wxT(';') != result.GetChar(i-1))))
			{
#if 0
				OPOLYGLOT_DEBUG(wxT("is %ld \\n"),i);
#endif
				result.SetChar(i,wxT(' '));
			}
		}
		this->textOriginal->SetValue(result);
		timeOCR = sw.Time();
		OPOLYGLOT_MESSAGE(wxT("ocr finish %ld"),timeOCR);
	} else
	{
		result = this->textOriginal->GetValue();
	}
	OPOLYGLOT_DEBUG(wxT("start translation"));
	mutexProgressThreadTranslation.Lock();
	messageProgressThreadTranslation = _("Translation...");
	mutexProgressThreadTranslation.Unlock();

	for( size_t i = 0;i < configTranslatorFileYml.GetCount();i+=1)
	{
		OPOLYGLOT_DEBUG(wxT("%ld %ld"),i,configTranslatorFileYml.GetCount());
		OPOLYGLOT_DEBUG(wxT("start translation %s"),configTranslatorFileYml.Item(i));
		result = Translator::translate(result,wxString::Format(wxT("%s/%s"),OPOLYGLOT_USER_DATA,configTranslatorFileYml.Item(i)));
	}
	
	wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION);
	event->SetInt(0);
	event->SetString(result);
	long ms = sw.Time();
	wxString resTime;
	if(ms < 10000)
	{
		resTime  =wxString::Format(wxT("time %ld MS"),ms);
	} else
	{
		resTime = wxString::Format(wxT("time %.2f S"),((double)ms)/1000.0);
	}
	OPOLYGLOT_MESSAGE(wxT("finish %s"),resTime);
	wxQueueEvent(this, event);
	return (wxThread::ExitCode) 0;
}

void OPolyglot::OnExitThreadTranslation(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT(""));
	timerProgressOcrTranslation->Stop();
	progressThreadTranslation->Destroy();
	if(event.GetInt())
	{
		OPOLYGLOT_ERROR(wxT("error thread %s"),event.GetString());
		wxMessageDialog msg(this,wxString::Format(wxT("%s"),event.GetString()),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
	}
	this->textTranslation->SetValue(event.GetString());
	if(this->IsShown())
	{
		if(this->EnableAutoTranslate->IsChecked())
		{
			timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
		} 
		if(this->OCRTranslate->IsChecked())
		{
			timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
		}
	}
	this->buttonShowTranslate->SetValue(true);
	wxPostEvent(this->buttonShowTranslate,wxCommandEvent(wxEVT_TOGGLEBUTTON));
}

#if 0
void OPolyglot::OnSelectLanguage(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("%s"),this->ChoiseTranslateDirect->GetStringSelection());
}
#endif

void OPolyglot::OnShowTranslate(wxCommandEvent &event)
{
	if(this->buttonShowTranslate->GetValue())
	{
		int width,height;
		if(!this->translatePanel->IsShown())
		{
			this->MainVBox->Layout();
			MainVBox->Fit(this);
			this->Refresh();
			this->GetSize(&width,&height);
			OPOLYGLOT_MESSAGE(wxT("show"));
			this->translatePanel->Show(true);
			if(height < 120)
			{
				this->SetSize(width,480);
			}
			this->buttonShowTranslate->SetLabel(_("hide the translation"));
		}
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("hide"));
		this->translatePanel->Show(false);
		this->buttonShowTranslate->SetLabel(_("show translation"));
		this->MainVBox->Layout();
		MainVBox->Fit(this);
		this->Refresh();
	}
}

void OPolyglot::OnShowOriginal(wxCommandEvent &event)
{
	if(this->buttonShowOriginal->GetValue())
	{
		OPOLYGLOT_MESSAGE(wxT("show"));
		this->textOriginal->Show(true);
		this->buttonShowOriginal->SetLabel(_("hide the text of the original"));
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("hide"));
		this->textOriginal->Show(false);
		this->buttonShowOriginal->SetLabel(_("show the text of the original"));
	}
	this->MainVBox->Layout();
	//MainVBox->Fit(this);
	//this->Refresh();
}

/*
   void OPolyglot::OnExit()
   {
   OPOLYGLOT_MESSAGE();
   }*/

void OPolyglot::ScanLangs()
{
	wxXmlDocument doc;
	this->ScanLanguageFrom();
	this->ScanLanguageTo();
	this->CreateTranslatorConfig();
}

wxString OPolyglot::GetLangCodeForOCR()
{
	OPOLYGLOT_MESSAGE(wxT("%s"),this->LanguageFrom->GetStringSelection());
	wxString ret;
	wxXmlDocument doc;
	if(!doc.Load(OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE))
	{
		OPOLYGLOT_ERROR(wxT("load file download language %s"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return wxEmptyString;
	} else
	{
		OPOLYGLOT_DEBUG(wxT("load %s"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE);
	}
	/*
	 * search for occ settings for selected language
	 */
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(OPOLYGLOT_CHECKING_INSTALLE_LANGUAGE_FROM_NODE_XML(child)
				&&this->LanguageFrom->GetStringSelection().IsSameAs(OPOLYGLOT_LABEL_LANGUAGEFROM_FROM_NODE_XML(child)))
		{
			ret = child->GetAttribute(wxT("ocr"));
		}
	}
	OPOLYGLOT_DEBUG(wxT("select ocr %s"),ret);
	return ret;
}

void OPolyglot::ScanLanguageFrom()
{
	OPOLYGLOT_MESSAGE();
	wxString select = this->LanguageFrom->GetStringSelection();
	wxString selectLanguageTo = wxEmptyString; /* this->LanguageTo->GetStringSelection().Before(' ');*/
	this->LanguageFrom->Clear();
	wxXmlDocument doc;
	if(!doc.Load(OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE))
	{
		OPOLYGLOT_ERROR(wxT("load file download language %s"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	} else
	{
		OPOLYGLOT_DEBUG(wxT("load %s"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE);
	}
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().Cmp(wxT("Language")) == 0)
		{
			if(OPOLYGLOT_CHECKING_INSTALLE_LANGUAGE_FROM_NODE_XML(child))
			{
				if((!selectLanguageTo.IsSameAs(child->GetAttribute("from")))&&(this->LanguageFrom->FindString(OPOLYGLOT_LABEL_LANGUAGEFROM_FROM_NODE_XML(child)) == wxNOT_FOUND))
				{
					OPOLYGLOT_DEBUG(wxT("add LanguageFrom %s"),OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(child));
					this->LanguageFrom->Append(OPOLYGLOT_LABEL_LANGUAGEFROM_FROM_NODE_XML(child));
				}
			}

		}
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

	OPOLYGLOT_DEBUG(wxT("LanguageFrom %s"),this->LanguageFrom->GetStringSelection());
}

void OPolyglot::ScanLanguageTo()
{
	wxXmlDocument doc;
	OPOLYGLOT_MESSAGE();
	wxString selectLanguageFrom = this->LanguageFrom->GetStringSelection().Before(' ');
	this->LanguageTo->Clear();
	if(!doc.Load(OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE))
	{
		OPOLYGLOT_ERROR(wxT("error load file download language %s"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	} else
	{
		OPOLYGLOT_DEBUG(wxT("load %s"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE);
	}
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		if(child->GetName().Cmp(wxT("Language")) == 0)
		{
			if(OPOLYGLOT_CHECKING_INSTALLE_LANGUAGE_FROM_NODE_XML(child))
			{
				if((!selectLanguageFrom.IsSameAs(child->GetAttribute("to")))
						&&(this->LanguageTo->FindString(OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(child)) == wxNOT_FOUND)
						&&((this->LanguageFrom->GetStringSelection().IsSameAs(OPOLYGLOT_LABEL_LANGUAGEFROM_FROM_NODE_XML(child)))))
				{
				
					OPOLYGLOT_DEBUG(wxT("add LanguageTo %s"),OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(child));
					this->LanguageTo->Append(OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(child));
				}
			}
		}
	}
	/*
	 *start finding the possibility of cross translation through English
	 * LanguageFrom to English -> English to LanguageTo
	 * example polish -> ukrainian
	 * polish ->  english -> english -> ukrainian
	 */
	for(wxXmlNode *childFrom = doc.GetRoot()->GetChildren();(childFrom);childFrom=childFrom->GetNext())
	{
		/*
		 *
		 * find from laguage to english
		 * this is necessary for the future when direct translations between languages are possible
		 */
		if(OPOLYGLOT_CHECKING_INSTALLE_LANGUAGE_FROM_NODE_XML(childFrom)
				&&OPOLYGLOT_LABEL_LANGUAGEFROM_FROM_NODE_XML(childFrom).IsSameAs(this->LanguageFrom->GetStringSelection())
				&&(childFrom->GetAttribute(wxT("to")).IsSameAs(wxT("English"))))
		{
			OPOLYGLOT_DEBUG(wxT("find translate %s to English"),OPOLYGLOT_LABEL_LANGUAGE_FROM_NODE_XML(childFrom));
			for(wxXmlNode *childTo = doc.GetRoot()->GetChildren();childTo;childTo = childTo->GetNext())
			{
				if(OPOLYGLOT_CHECKING_INSTALLE_LANGUAGE_FROM_NODE_XML(childTo) /* checking that the language is established */
						&& (!this->LanguageFrom->GetStringSelection().Before(wxT(' ')).IsSameAs(childTo->GetAttribute(wxT("to")))) /*checking so that there is no translation into itself ukrainina->english->ukrainiain*/
						&& childTo->GetAttribute("from").IsSameAs(wxT("English")))
				{
					if((this->LanguageTo->FindString(OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(childTo)) == wxNOT_FOUND)
							&&(!this->LanguageFrom->GetStringSelection().IsSameAs(OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(childTo)))) /* checking the selected identical language with this->LanguageFrom */
					{
						OPOLYGLOT_DEBUG(wxT("add cross English LanguageTo %s"),OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(childTo));
						this->LanguageTo->Append(OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(childTo));
					}

				}

			}

		}
	}


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
	//this->ScanLanguageFrom();
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,this->LanguageTo->GetStringSelection());
	CreateTranslatorConfig();
}


void OPolyglot::OnFinishSetupLanguages(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE();
	this->ScanLangs();
}

void OPolyglot::OnSetupLanguages(wxThreadEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	//frameDownloadsLanguage->Show();
	OPolyglotSetup *setup = new OPolyglotSetup(this);
	setup->Show();
#if 0
	OPolyglotDownloadLanguage *frameDownload = new OPolyglotDownloadLanguage(this);
	frameDownload->Show();
#endif
}

void OPolyglot::OnExitProgramm(wxThreadEvent& WXUNUSED(event))
{
	OPOLYGLOT_MESSAGE();
	this->Show(false);
#if 0
	dialogTranslation->Show(false);
#endif
	delete taskBar;
	this->Destroy();
}


void OPolyglot::OnRightClick(wxMouseEvent &event)
{
	OPOLYGLOT_MESSAGE();
}

void OPolyglot::OnSelectArea(wxThreadEvent &event)
{
	OPOLYGLOT_DEBUG("");
	if(event.GetString().IsEmpty())
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
	OPOLYGLOT_MESSAGE(wxT("%s"),event.GetString());
	coordStartX = -1;
	coordStartY = -1;
	filenameImageAreaForOCR = event.GetString();
	//fullscreen->Destroy();
	StartThreadTranslation();
	//dialogTranslation->StartOCRAndTranslation(event.GetString());
}
#if 0
void OPolyglot::OnTranslateFinish(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE();
	if(this->EnableAutoTranslate->IsChecked())
	{
		timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
	} 
	if(this->OCRTranslate->IsChecked())
	{
		timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
	}
}
#endif
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
	if(this->EnableAutoTranslate->IsChecked())
	{
		OPOLYGLOT_MESSAGE("run timer");
		timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
	} else
	{
		OPOLYGLOT_MESSAGE("stop timer");
		timerClipboardChecking->Stop();
	}
}

void OPolyglot::SetVisible(bool flag)
{
	if(flag)
	{

		OPOLYGLOT_MESSAGE(wxT("show"));
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
		if(GetThread() != nullptr)
		{
			OPOLYGLOT_DEBUG("Thread is running");
			progressThreadTranslation->Show(true);
		}
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("hide"));
		this->Show(false);
		flagShow = false;
		timerClipboardChecking->Stop();
		timerMouseState->Stop();
		if(GetThread() != nullptr)
		{
			OPOLYGLOT_DEBUG("Thread is running");
			progressThreadTranslation->Show(false);
		}
	}
}

void OPolyglot::OnClose( wxCloseEvent& event ) { 
	OPOLYGLOT_MESSAGE();
	this->SetVisible(false);
}
#if 0
void OPolyglot::SetVisible()
{
	OPOLYGLOT_MESSAGE();
#if 0
	if(viewDialogTranslator)
	{
		this->dialogTranslation->Show(true);
	}
#endif
	this->Show(true);
	if(this->EnableAutoTranslate->IsChecked())
	{
		timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
	} 
	if(this->OCRTranslate->IsChecked())
	{
		timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
	}
}

void OPolyglot::Hide()
{
	OPOLYGLOT_MESSAGE();
	timerClipboardChecking->Stop();
	timerMouseState->Stop();
	this->Show(false);
}
#endif

void OPolyglot::OnTimerProgressOCRTranslation(wxTimerEvent &event)
{
	//OPOLYGLOT_DEBUG();
	wxMutexLocker lock(mutexProgressThreadTranslation);
	if(!progressThreadTranslation->Pulse(messageProgressThreadTranslation))
	{
		OPOLYGLOT_WARNING(wxT("cancelled user"));
		timerProgressOcrTranslation->Stop();
		progressThreadTranslation->Destroy();
		GetThread()->Kill();
		if(this->IsShown())
		{
			if(this->EnableAutoTranslate->IsChecked())
			{
				timerClipboardChecking->Start(TIMEOUT_CLIPBOARD_CHECKING);
			} 
			if(this->OCRTranslate->IsChecked())
			{
				timerMouseState->Start(TIMEOUT_CHECK_MOUSE_STATE);
			}
		}
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
				//timerClipboardChecking->Stop();
				//timerMouseState->Stop();
				
				lastClipboardText = data.GetText();
				this->textOriginal->SetValue(lastClipboardText);
				timerClipboardChecking->Stop();
				timerMouseState->Stop();
				filenameImageAreaForOCR = wxEmptyString;
				StartTranslation();

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
	OPOLYGLOT_MESSAGE();
	filenameImageAreaForOCR = wxEmptyString; /* disable OCR */
	StartTranslation();
}

#if 0
void OPolyglot::OnKill()
{
	OPOLYGLOT_WARNING();
}
#endif

void OPolyglot::StartThreadTranslation()
{
	OPOLYGLOT_MESSAGE();
	if(CreateThread(wxTHREAD_DETACHED) != wxTHREAD_NO_ERROR)
	{
		OPOLYGLOT_ERROR(wxT("Failed to create translator thread"));
		wxMessageDialog msg(this,wxString::Format(wxT("%s\n%s"),_("ERROR"),_("Failed to create translator thread")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	if(GetThread()->Run() != wxTHREAD_NO_ERROR)
	{
		OPOLYGLOT_ERROR(wxT("Failed to start translator thread"));
		wxMessageDialog msg(this,wxString::Format(wxT("%s/%s"),_("Failed to start translator thread")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	messageProgressThreadTranslation = wxEmptyString;
	progressThreadTranslation = new wxProgressDialog(wxT("OPolyglot"),_T("Translating..."),100,this,wxPD_APP_MODAL|wxPD_CAN_ABORT);
	progressThreadTranslation->Show();
	timerProgressOcrTranslation->Start(200);
}

void OPolyglot::CreateTranslatorConfig()
{
	bool flagFinish = false;
	wxConfig *config = new wxConfig(OPOLYGLOT_CONFIG_ARGUMENT);
	wxString configFileBestMemory = wxEmptyString; 	/* model "best-memory" */
	wxString configFileBest = wxEmptyString; /* model "best" */
	wxString configFileFast = wxEmptyString; /* model "tiny */
	wxString configFileToEnglishBestMemory 	=wxEmptyString; /* model "best-memory" */
	wxString configFileToEnglishBest		=wxEmptyString; /* model "best" */
	wxString configFileToEnglishFast		=wxEmptyString; /* model "fast" */
	wxString configFileFromEnglishBestMemory	=wxEmptyString; 
	wxString configFileFromEnglishBest			=wxEmptyString;
	wxString configFileFromEnglishFast			=wxEmptyString;
	OPOLYGLOT_MESSAGE(wxT("%s| %s -> %s")
			,config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT)
			,this->LanguageFrom->GetStringSelection()
			,this->LanguageTo->GetStringSelection());
	/*
	 * start create array for translating
	 */
	configTranslatorFileYml.Clear();
	if(this->LanguageFrom->GetStringSelection().IsEmpty()
			||this->LanguageTo->GetStringSelection().IsEmpty())
	{
		OPOLYGLOT_WARNING(wxT("not installed language"));
		return;
	}
	wxXmlDocument doc;
	if(!doc.Load(OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE))
	{
		OPOLYGLOT_ERROR(wxT("error load file download language %s"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s :%s"),_("Error load file"),OPOLYGLOT_GET_FILE_DOWNLOAD_LANGUAGE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return;
	}
	/*
	 * Begin search config file for  translation 
	 */
	for(wxXmlNode *child = doc.GetRoot()->GetChildren();child&&(!flagFinish);child=child->GetNext())
	{
		if(child->GetName().IsSameAs(wxT("Language"))&&OPOLYGLOT_CHECKING_INSTALLE_LANGUAGE_FROM_NODE_XML(child))
		{
			/* search direct translation example Polish -> English */
			if(this->LanguageFrom->GetStringSelection().IsSameAs(OPOLYGLOT_LABEL_LANGUAGEFROM_FROM_NODE_XML(child)) 
					&&(this->LanguageTo->GetStringSelection().IsSameAs(OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(child))))
			{
				
				wxString typeLanguage = child->GetAttribute(wxT("type"));
				OPOLYGLOT_DEBUG(wxT("find language %s %s %s"),typeLanguage,child->GetAttribute(wxT("from")),child->GetAttribute(wxT("to")));
				if(typeLanguage.IsSameAs(wxT("base")))
				{
					configFileBest = child->GetAttribute(wxT("configfile"));
					OPOLYGLOT_DEBUG(wxT("configFileBest"));
				}
				if(typeLanguage.IsSameAs(wxT("base-memory")))
				{
					configFileBestMemory = child->GetAttribute(wxT("configfile"));
					OPOLYGLOT_DEBUG(wxT("configFileBestMemory"));
				}
				if(typeLanguage.IsSameAs(wxT("tiny")))
				{
					configFileFast = child->GetAttribute(wxT("configfile"));
					OPOLYGLOT_DEBUG(wxT("configFileFast"));
				}
			}
			/* search cross translation example Polish -> Ukrainian (Polish -> English -> Ukrainian) */
			/* search translation to Polish -> English */
			if( (this->LanguageFrom->GetStringSelection().IsSameAs(OPOLYGLOT_LABEL_LANGUAGEFROM_FROM_NODE_XML(child)))
					&&(OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(child).IsSameAs(wxT("English"))))
			{
				wxString typeTranslation = child->GetAttribute(wxT("type"));
				OPOLYGLOT_DEBUG(wxT("find language %s %s %s"),typeTranslation,child->GetAttribute(wxT("from")),child->GetAttribute(wxT("to")));
				if(typeTranslation.IsSameAs(wxT("base")))
				{
					configFileToEnglishBest = child->GetAttribute(wxT("configfile"));
					OPOLYGLOT_DEBUG(wxT("configFileToEnglishBest"));
				}
				if(typeTranslation.IsSameAs(wxT("base-memory")))
				{
					configFileToEnglishBestMemory = child->GetAttribute(wxT("configfile"));
					OPOLYGLOT_DEBUG(wxT("configFileToEnglishBestMemory"));
				}
				if(typeTranslation.IsSameAs(wxT("tiny")))
				{
					configFileToEnglishFast = child->GetAttribute(wxT("configfile"));
					OPOLYGLOT_DEBUG(wxT("configFileToEnglishFast"));
				}
			}
			/* search translationto  English -> Ukrainian */
			if((this->LanguageTo->GetStringSelection().IsSameAs(OPOLYGLOT_LABEL_LANGUAGETO_FROM_NODE_XML(child)))
					&&(OPOLYGLOT_LABEL_LANGUAGEFROM_FROM_NODE_XML(child).IsSameAs(wxT("English"))))
			{
				wxString typeTranslation = child->GetAttribute(wxT("type"));
				OPOLYGLOT_DEBUG(wxT("find language %s %s %s"),typeTranslation,child->GetAttribute(wxT("from")),child->GetAttribute(wxT("to")));
				if(typeTranslation.IsSameAs(wxT("base")))
				{
					configFileFromEnglishBest = child->GetAttribute(wxT("configfile"));
					OPOLYGLOT_DEBUG(wxT("configFileFromEnglishBest"));
				}
				if(typeTranslation.IsSameAs(wxT("base-memory")))
				{
					configFileFromEnglishBestMemory = child->GetAttribute(wxT("configfile"));
					OPOLYGLOT_DEBUG(wxT("configFileFromEnglishBestMemory"));
				}
				if(typeTranslation.IsSameAs(wxT("tiny")))
				{
					configFileFromEnglishFast = child->GetAttribute(wxT("configfile"));
					OPOLYGLOT_DEBUG(wxT("configFileFromEnglishFast"));
				}
			}
		}
	}
	if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST")))
	{
		/*
		 * is performed if the translation method is selected "BEST"
		 */
		OPOLYGLOT_DEBUG(wxT("build config direct BEST %s -> %s")
				,this->LanguageFrom->GetStringSelection()
				,this->LanguageTo->GetStringSelection());
		if(!configFileBest.IsEmpty())
		{
			configTranslatorFileYml.Add(configFileBest);
		}
		if((!configFileBestMemory.IsEmpty())&&(configTranslatorFileYml.GetCount() == 0))
		{
			configTranslatorFileYml.Add(configFileBestMemory);
		} 
		if((!configFileFast.IsEmpty())&&(configTranslatorFileYml.GetCount() == 0))
		{
			configTranslatorFileYml.Add(configFileFast);
		}

	} else
	{
		/*
		 * is performed if the translation method is selected "FAST"
		 */
		OPOLYGLOT_DEBUG(wxT("build config direct FAST %s -> %s")
				,this->LanguageFrom->GetStringSelection()
				,this->LanguageTo->GetStringSelection());
		if(!configFileFast.IsEmpty())
		{
			configTranslatorFileYml.Add(configFileFast);
		}
		if((!configFileBestMemory.IsEmpty())&&(configTranslatorFileYml.GetCount() == 0))
		{
			configTranslatorFileYml.Add(configFileBestMemory);
		}
		if((!configFileBest.IsEmpty())&&(configTranslatorFileYml.GetCount() == 0))
		{
			configTranslatorFileYml.Add(configFileBest);
		}
	}
	/*
	 * build cross translator
	 */
	if(configTranslatorFileYml.GetCount() == 0)
	{
		if(config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST")))
		{
			/*
			 * is performed if the translation method is selected "BEST"
			 */
			OPOLYGLOT_DEBUG(wxT("build config cross BEST %s -> %s")
					,this->LanguageFrom->GetStringSelection()
					,this->LanguageTo->GetStringSelection());
			if(!configFileToEnglishBest.IsEmpty())
			{
				configTranslatorFileYml.Add(configFileToEnglishBest);
			}
			if((!configFileToEnglishBestMemory.IsEmpty())&&(configTranslatorFileYml.GetCount() == 0))
			{
				configTranslatorFileYml.Add(configFileToEnglishBestMemory);
			} 
			if((!configFileToEnglishFast.IsEmpty())&&(configTranslatorFileYml.GetCount() == 0))
			{
				configTranslatorFileYml.Add(configFileToEnglishFast);
			}
			if(!configFileFromEnglishBest.IsEmpty())
			{
				configTranslatorFileYml.Add(configFileFromEnglishBest);
			}
			if((!configFileFromEnglishBestMemory.IsEmpty())&&(configTranslatorFileYml.GetCount() <= 1))
			{
				configTranslatorFileYml.Add(configFileFromEnglishBestMemory);
			} 
			if((!configFileFromEnglishFast.IsEmpty())&&(configTranslatorFileYml.GetCount() <= 1))
			{
				configTranslatorFileYml.Add(configFileFromEnglishFast);
			}

		} else
		{
			/*
			 * is performed if the translation method is selected "FAST"
			 */
			OPOLYGLOT_DEBUG(wxT("build config cross FAST %s -> %s")
					,this->LanguageFrom->GetStringSelection()
					,this->LanguageTo->GetStringSelection());
			if(!configFileToEnglishFast.IsEmpty())
			{
				configTranslatorFileYml.Add(configFileToEnglishFast);
			}
			if((!configFileToEnglishBestMemory.IsEmpty())&&(configTranslatorFileYml.GetCount() == 0))
			{
				configTranslatorFileYml.Add(configFileToEnglishBestMemory);
			}
			if((!configFileToEnglishBest.IsEmpty())&&(configTranslatorFileYml.GetCount() == 0))
			{
				configTranslatorFileYml.Add(configFileToEnglishBest);
			}
			if(!configFileFromEnglishFast.IsEmpty())
			{
				configTranslatorFileYml.Add(configFileFromEnglishFast);
			}
			if((!configFileFromEnglishBestMemory.IsEmpty())&&(configTranslatorFileYml.GetCount() <= 1))
			{
				configTranslatorFileYml.Add(configFileFromEnglishBestMemory);
			}
			if((!configFileFromEnglishBest.IsEmpty())&&(configTranslatorFileYml.GetCount() <= 1))
			{
				configTranslatorFileYml.Add(configFileFromEnglishBest);
			}
		}

		if(configTranslatorFileYml.GetCount() < 2)
		{
			OPOLYGLOT_ERROR(wxT("Error in cross-translation: English to %s translator not found"),this->LanguageFrom->GetStringSelection());
			wxMessageDialog msg(this,wxString::Format(wxT("%s %s %s")
						,_("Error in cross-translation: English to")
						,this->LanguageFrom->GetStringSelection()
						,_("translator not found")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			return;
		}
	} /*if(configTranslatorFileYml.GetCount() == 0) */
	// if(config.Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT).IsSameAs(_("BEST")))
	OPOLYGLOT_DEBUG(wxT("%s| %s -> %s\nfiles yml:")
			,config->Read(OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD,OPOLYGLOT_CONFIG_STRING_TRANSLATION_METHOD_DEFAULT)
			,this->LanguageFrom->GetStringSelection()
			,this->LanguageTo->GetStringSelection());
	for(size_t i =0; i < configTranslatorFileYml.GetCount();i++)
	{
		OPOLYGLOT_DEBUG(wxT("%ld: %s"),i,configTranslatorFileYml.Item(i));
	}
	delete config;

}

void OPolyglot::StartTranslation()
{
	OPOLYGLOT_MESSAGE();
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
#if 1
	if(mouseState.LeftIsDown())
	{

		OPOLYGLOT_MESSAGE();
		if((coordStartX == -1)&&(coordStartY == -1))
		{
			coordStartX = mouseState.GetX();
			coordStartY = mouseState.GetY();
		}
		fullscreen = new FullscreenFrame(this);
		timerMouseState->Stop();
	}
#endif
	event.Skip();
#if 0
	if(mouseState.LeftIsDown()&&(coordStartX == -1)&&(coordStartY == -1))
	{
		coordStartX = mouseState.GetX();
		coordStartY = mouseState.GetY();
	}
	if((!mouseState.LeftIsDown())&&(coordStartX != -1)&&(coordStartY != -1))
	{
		int x,y,w,h;
		x = mouseState.GetX();
		y = mouseState.GetY();
		w = abs(x-coordStartX);
		h = abs(y-coordStartY);
		if(x < coordStartX)
		{
			coordStartX = x;
		}
		if(y < coordStartY)
		{
			coordStartY = y;
		}
		if((32 < w) &&(32 < h))
		{
			int width,height;
			timerMouseState->Stop();
			timerClipboardChecking->Stop();
			wxScreenDC dc;
			dc.GetSize(&width,&height);
			wxBitmap bitmap(w,h);
			wxMemoryDC memDC;
			memDC.SelectObject(bitmap);
			if(!memDC.StretchBlit(0,0,w,h,&dc,coordStartX,coordStartY,w,h))
			{
				OPOLYGLOT_ERROR();
			} else
			{
				//wxString str = wxString(wxT("%s/area.png"),wxFileName::GetTempDir());
				wxString str = wxFileName::GetTempDir();

				str.Append(wxT("/area.png"));
				OPOLYGLOT_MESSAGE(wxT("%s %d  %d %dx%d"),str,coordStartX,coordStartY,w,h);
				bitmap.SaveFile(str,wxBITMAP_TYPE_PNG );
				coordStartX = -1;
				coordStartY = -1;
				dialogTranslation->StartOCRAndTranslation(str);

			}
		} else
		{
			OPOLYGLOT_MESSAGE("NOT SELECT AREA");
		}
		coordStartX = -1;
		coordStartY = -1;
	}
#endif


}

