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
#include "OPolyglot.h"
#include "OPolyglotEvent.h"
#include "Utils.h"
#include "Config.h"
#include <wx/clipbrd.h>
#include <wx/panel.h>
#include <wx/rawbmp.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotSettings.h"
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/config.h>
#include <wx/display.h>
#include <wx/regex.h>
#include <wx/dcmemory.h>
#include <wx/uri.h>
#include <wx/sstream.h>
#include <wx/arrimpl.cpp>
#include <wx/dcbuffer.h>
#include <wx/filedlg.h>
#include "LibOPolyglot.h"
#include <leptonica/allheaders.h>

#if __WXGTK__
	#include "../res/icon.xpm"
	#include <libportal/portal.h>
	#include <libportal-gtk3/portal-gtk3.h>
#endif



#if __WXGTK__
	static wxMutex 		mutex;
	static XdpPortal	*portal;
	static wxWindow		*parent;
	static int			countRun;
static void portal_screenshot_ready(GObject *source_object,GAsyncResult *res, gpointer user_data)
{
	wxMutexLocker lock(mutex);
	OPOLYGLOT_DEBUG(wxT("OPolyglot::portal_screenshot_ready"));
	GError	*error = NULL;
	XdpPortal *portal = XDP_PORTAL(source_object);
	char *uri = xdp_portal_take_screenshot_finish(portal,res,&error);
	if(error)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot::portal_screenshot_ready %s %s"),error->message,OPOLYGLOT_BOOL_TO_STRING(uri != NULL));
		OPOLYGLOT_DEBUG(wxT("ERROR OPolyglot::portal_screenshot_ready %s %s"),error->message,OPOLYGLOT_BOOL_TO_STRING(uri != NULL));
		g_error_free(error);
		return;
	}
	OPOLYGLOT_DEBUG(wxT("OPolyglot::portal_screenshot_ready %s "),wxString::FromUTF8(uri));
	if(0 < countRun)
	{
		wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SCREENSHOT_FINISH);
		event->SetInt(countRun);
		event->SetString(wxString::FromUTF8(uri));
		wxQueueEvent(parent,event);
	}
	countRun+=1;
	g_free(uri);
}
static void PortalInit()
{
	wxMutexLocker lock(mutex);
	countRun = 0;
	portal = xdp_portal_new();
}

static void PortalTakeScreenshot(wxWindow *w)
{
	wxMutexLocker lock(mutex);
	parent = w;
	xdp_portal_take_screenshot(portal
			,xdp_parent_new_gtk(GTK_WINDOW(w->GetHandle()))
			,XDP_SCREENSHOT_FLAG_NONE
			,NULL
			,portal_screenshot_ready
			,NULL);
}

#endif




OPolyglot::OPolyglot(wxEvtHandler *handler) 
	: GuiOPolyglot(NULL) , wxThreadHelper() 
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot"));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	panelMain->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif

	this->handler = handler;
	wxDisplay display(this);
	wxRect geom = display.GetGeometry();
	wxSize s = this->GetSize();
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
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SCREENSHOT_FINISH,&OPolyglot::OnScreenshot,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_OCR_START,&OPolyglot::OnStartThreadTranslator,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH,&OPolyglot::OnFinishThreadTranslator,this);
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_CLOSE_TRANSLATOR,&OPolyglot::OnCloseTranslator,this);
	this->ScanLanguageFrom();
	this->ScanLanguageTo();
	frameDownload = nullptr;
	if( (0 == this->LanguageFrom->GetCount())||(0 == this->LanguageTo->GetCount()))
	{
		frameDownload = new OPolyglotDownloadLanguage(this);
		this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglot::OnFinishSetupLanguages,this);
		wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
		frameDownload->Show();
	}
	MainVBox->Fit(this);
	MainVBox->Layout();
	this->Show();
	wxScreenDC dc;
	int w,h;
	dc.GetSize(&w,&h);
	if((0 < w)&&(0 < h))
	{
		wxBitmap bitmap(w,h);
		wxMemoryDC memDC;
		memDC.SelectObject(bitmap);
		memDC.Blit(0,0,w,h,&dc,0,0);
		memDC.SelectObject(wxNullBitmap);
		wxNativePixelData pix(bitmap);
		wxNativePixelData::Iterator p(pix);
		if(!p.IsOk())
		{
			OPOLYGLOT_ERROR(wxT("OPolyglot wxNativePixelData"));
			wxMessageDialog msg(this,wxS("OPolyglot wxNativePixelData"),wxS("OPolyglot"),wxICON_ERROR|wxOK);
			msg.ShowModal();
			return;
		}
		p.MoveTo(pix,0,0);
		for(int iy =0; (iy < h)&&(flagCreateScreenshotOnlyPortal);iy++)
		{
			wxNativePixelData::Iterator r = p;
			for(int ix = 0;(ix < w)&&(flagCreateScreenshotOnlyPortal);ix++,r++)
			{
				if((r.Red()!=wxBLACK->GetRed())
						||(r.Green()!=wxBLACK->GetGreen())
						||(r.Blue()!=wxBLACK->GetBlue()))
				{
					flagCreateScreenshotOnlyPortal = false;
				}

			}
			p.OffsetY(pix,1);

		}
	}
	if(((w == 0)&&(h == 0))||(flagCreateScreenshotOnlyPortal))
	{
#if __WXGTK__
		OPOLYGLOT_MESSAGE(wxT("OPolyglot use libportal to capture the screen"));
		PortalInit();
		PortalTakeScreenshot(this);
#else
		OPOLYGLOT_ERROR(wxT("OPolyglot not supported screen capture"));
#endif
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglot available screen capture"));
	}

	viewTextTranslate = new OPolyglotViewTextTranslate(this,OPOLYGLOT_GET_XML_FILE_TRANSLATE);
}



OPolyglot::~OPolyglot()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglot"));
	if(!IS_NULLPTR(frameDownload ))
	{
		delete frameDownload;
	}
	delete LanguageFrom;
}


void OPolyglot::OnDocumentTranslator( wxCommandEvent& event )
{
	wxFileDialog
		openFileDialog(this, _("Open pdf file"), wxGetHomeDir(), "",
				"*.pdf", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
	{
		OPOLYGLOT_WARNING(wxT("OPolyglot::OnDocumentTranslator cancelled by user"));
		return;     
	}
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnDocumentTranslator(%s)"),openFileDialog.GetPath());
	OPolyglotDocument *document = new OPolyglotDocument(this
			,openFileDialog.GetPath()
			,LanguageFrom->GetStringSelection()
			,LanguageTo->GetStringSelection());
	this->Bind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglot::OnCloseDocumentTranslator,this);
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
	document->Show();
}

void OPolyglot::OnMenuSetup( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnMenuSetup"));
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SETUP));
}

void OPolyglot::OnMenuAbout( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OnMenuAbout"));
	wxQueueEvent(handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_ABOUT));
}

void OPolyglot::OnCaptureScreen(wxCommandEvent& event)
{
	this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
	this->Lower();
	this->Update();
	{
		int w,h;
		wxSafeYield();
		wxScreenDC dc;
		dc.GetSize(&w,&h);
		if(((0 < w)&&(0 < h))&&(!flagCreateScreenshotOnlyPortal))
		{
			OPOLYGLOT_MESSAGE(wxT("OnCaptureScreen(%dx%d)"),w,h);
			wxBitmap bitmap(w,h);
			wxMemoryDC memDC;
			memDC.SelectObject(bitmap);
			memDC.Blit(0,0,w,h,&dc,0,0);
			memDC.SelectObject(wxNullBitmap);
			wxString fileName = wxString::Format(wxS("%s%s%s"),wxFileName::GetTempDir(),wxString(wxFileName::GetPathSeparator()),wxT("screen.png"));
			OPOLYGLOT_DEBUG(wxT("OPolyglot::OnCaptureScreen screenshot %s"),fileName);
			if(!bitmap.SaveFile(fileName,wxBITMAP_TYPE_PNG))
			{
				OPOLYGLOT_ERROR(wxT("OPolyglot::OnCaptureScreen not save screenshot %s"),fileName);
				wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("error saving screenshot"),fileName),wxT("OPolyglot"),wxOK|wxICON_ERROR);
				msg.ShowModal();
				return;
			}
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SCREENSHOT_FINISH);
			event->SetInt(-1);
			event->SetString(fileName);
			wxQueueEvent(this,event);
		} else
		{
#if __WXGTK__
			OPOLYGLOT_MESSAGE(wxT("OnCaptureScreen using libportal"));
			PortalTakeScreenshot(this);
#else
			OPOLYGLOT_ERROR(wxT("OPolyglot::OnCaptureScreen error creating screenshot"));
			wxMessageDialog msg(this,wxString::Format(wxT("%s %dx%d"),_("Error creating screenshot"),w,h),wxT("OPolyglot"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			return;
#endif
		}
	}
}

void OPolyglot::ScanLanguageFrom()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::ScanLanguageFrom"));
	this->LanguageFrom->Clear();
	wxArrayString languages = OPolyglotGetTranslatedLanguages(OPolyglotGetInstalledLanguagesFrom());
	languages.Sort(CompareLocaleNoCase);
	this->LanguageFrom->Append(languages);
	if(0 < this->LanguageFrom->GetCount())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		if(!LanguageFrom->SetStringSelection(OPolyglotGetTranslateLanguage(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT))))
		{
			LanguageFrom->SetSelection(0);
		}
		config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPolyglotGetOriginalLanguage(this->LanguageFrom->GetStringSelection()));
		buttonShowTranslator->Enable(true);
		buttonCaptureScreen->Enable(true);
	} else
	{
		buttonShowTranslator->Enable(false);
		buttonCaptureScreen->Enable(false);
	}
	OPOLYGLOT_DEBUG(wxT("LanguageFrom %s %d"),this->LanguageFrom->GetStringSelection(),this->LanguageFrom->GetSelection());
}

void OPolyglot::ScanLanguageTo()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::ScanLanguageTo"));
	this->LanguageTo->Clear();
	wxArrayString languages = OPolyglotGetTranslatedLanguages(
			OPolyglotGetInstalledLanguagesTo(
				OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection())));
	languages.Sort(CompareLocaleNoCase);
	this->LanguageTo->Append(languages);
	if(0 < this->LanguageTo->GetCount())
	{
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		if(!LanguageTo->SetStringSelection(OPolyglotGetTranslateLanguage(config.Read(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPOLYGLOT_CONFIG_STRING_LANGUAGE_DEFAULT))))
		{
			LanguageTo->SetSelection(0);
		}
		config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPolyglotGetOriginalLanguage(this->LanguageTo->GetStringSelection()));
		buttonShowTranslator->Enable(true);
		buttonCaptureScreen->Enable(true);
	} else
	{
		buttonShowTranslator->Enable(false);
		buttonCaptureScreen->Enable(false);
	}

}

void OPolyglot::OnSelectLanguageFrom( wxCommandEvent& event )
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("OnSelectLanguageFrom"));
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_FROM,OPolyglotGetOriginalLanguage(this->LanguageFrom->GetStringSelection()));
	this->ScanLanguageTo();
}


void OPolyglot::OnSelectLanguageTo( wxCommandEvent& event )
{
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	OPOLYGLOT_MESSAGE(wxT("OnSelectLanguageTo"));
	config.Write(OPOLYGLOT_CONFIG_STRING_LANGUAGE_TO,OPolyglotGetOriginalLanguage(this->LanguageTo->GetStringSelection()));
}


void OPolyglot::OnFinishSetupLanguages(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnFinishSetupLanguages"));
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglot::OnFinishSetupLanguages,this);
	frameDownload->Destroy();
	frameDownload = NULL;
#if 0
	ScanLanguageFrom();
	ScanLanguageTo();
#endif
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
}

void OPolyglot::OnCloseDocumentTranslator(wxThreadEvent &event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnCloseDocumentTranslator"));
	
	this->Unbind(wxEVT_COMMAND_OPOLYGLOT_SETUP,&OPolyglot::OnCloseDocumentTranslator,this);
	wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
	
}


void OPolyglot::OnOpenTranslator( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnOpenTranslator"));
	buttonCaptureScreen->Enable(false);
	buttonShowTranslator->Enable(false);
	if(IS_NULLPTR(frameTranslator))
	{
		frameTranslator = new OPolyglotTranslator(this,LanguageFrom->GetStringSelection(),LanguageTo->GetStringSelection());
		frameTranslator->Show();
	}
}

wxThread::ExitCode OPolyglot::Entry()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::Entry"));
	wxStringInputStream sis(valueXML);
	wxXmlDocument 		inputDoc(sis);
	wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
	if(!inputDoc.GetRoot()->GetName().IsSameAs(wxT("ScreenshotFile")))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot::Entry root node is not (ScreenshotFile) != %s"),inputDoc.GetRoot()->GetName());
		wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Error root node (ScreenshotFile) != "),inputDoc.GetRoot()->GetName()),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return (wxThread::ExitCode)0;
	}
	wxString fileName = inputDoc.GetRoot()->GetAttribute(wxS("fileName"));
	OPOLYGLOT_DEBUG(wxT("OPolyglot::Entry screenshot file %s"),fileName);
	PIX *pixs = pixRead(fileName);
	if(!pixs)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot::Entry error opened file %s"),fileName);
		wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Error opened file "),fileName),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return (wxThread::ExitCode)0;
	}
	PIX* pix_gray = pixConvertRGBToLuminance(pixs);
	pixDestroy(&pixs);
	pixs = pix_gray;
	if(!pixs)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot::Entry error convert to gray %s"),fileName);
		wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Failed to convert image to grayscale"),fileName),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
		return (wxThread::ExitCode)0;
	}
	wxXmlDocument *xmlTranslate = new wxXmlDocument();
	if(!xmlTranslate->Load(OPOLYGLOT_GET_XML_FILE_TRANSLATE))
	{
		OPOLYGLOT_WARNING(wxT("OPolyglot::Entry not load %s"),OPOLYGLOT_GET_XML_FILE_TRANSLATE);
		wxXmlNode *root = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,wxT("Texts"));
		xmlTranslate->SetRoot(root);
	}
	wxString codeLanguageFrom = OPolyglotGetCodeFromLanguage(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()));
	wxString codeLanguageTo = OPolyglotGetCodeFromLanguage(OPolyglotGetOriginalLanguage(LanguageTo->GetStringSelection()));
	for(wxXmlNode *child = inputDoc.GetRoot()->GetChildren();child;child = child->GetNext())
	{
		int x,y,w,h;
		if(child->GetName().IsSameAs(wxT("TextRegion")))
		{
			if(child->GetAttribute(wxS("x")).ToInt(&x)
					&&child->GetAttribute(wxS("y")).ToInt(&y)
					&&child->GetAttribute(wxS("w")).ToInt(&w)
					&&child->GetAttribute(wxS("h")).ToInt(&h))
			{
				OPOLYGLOT_DEBUG(wxT("OPolyglot::Entry start ocr %d %d %dx%d"),x,y,w,h);
				BOX *box = boxCreate(x,y,w,h);
				PIX *pixd = pixClipRectangle(pixs,box,NULL);
				boxDestroy(&box);
				if(!pixd)
				{
					OPOLYGLOT_ERROR(wxT("OPolyglot::Entry error pixClipRectangle(%d,%d,%dx%d)"),x,y,w,h);
					wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Failed pixClipRectangle")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
					msg.ShowModal();
					return (wxThread::ExitCode)0;
				}
				l_int32 tresh,fg_val,bg_val;
				if(pixSplitDistributionFgBg(pixd,0.1f,1,&tresh,&fg_val,&bg_val,NULL) == 0)
				{
				}
				if(!child->GetAttribute(wxS("InvertColor")).IsEmpty())
				{
					pixInvert(pixd,pixd);
				}
				PIX *pixBin =  pixSauvolaOnContrastNorm(pixd,
						static_cast<int>(config.ReadLong(OPOLYGLOT_CONFIG_INT_SAUVOLA_MINDIFF,OPOLYGLOT_CONFIG_INT_SAUVOLA_MINDIFF_DEFAULT)),NULL,NULL);
				if(!pixBin)
				{
					OPOLYGLOT_ERROR(wxT("OPolyglot::Entry error banarization image %dx%d start pixThresholdToBinary"),w,h);
					pixBin = pixThresholdToBinary(pixd, 128);
				}
				pixDestroy(&pixd);
				wxXmlNode *textNode = LibOPolyglotOCRAndTranslate(pixBin,codeLanguageFrom,codeLanguageTo,!child->GetAttribute(wxS("onlyOCR")).IsEmpty());
				pixDestroy(&pixBin);
				if(textNode == NULL)
				{
					OPOLYGLOT_ERROR(wxT("OPolyglot::Entry Failed ocr and translation"));
					wxMessageDialog msg(this,wxString::Format(wxT("%s"),_("Failed ocr and translation")),wxT("OPolyglot"),wxOK|wxICON_ERROR);
					msg.ShowModal();
					return (wxThread::ExitCode)0;
				}
				xmlTranslate->GetRoot()->AddChild(textNode);


			}
		}
	}
	if(!xmlTranslate->Save(OPOLYGLOT_GET_XML_FILE_TRANSLATE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot::Entry failed to save changes  %s"),OPOLYGLOT_GET_XML_FILE_TRANSLATE);
		wxMessageDialog msg(this,wxString::Format(wxT("%s %s"),_("Failed to save changed"),OPOLYGLOT_GET_XML_FILE_TRANSLATE),wxT("OPolyglot"),wxOK|wxICON_ERROR);
		msg.ShowModal();
	}
	delete xmlTranslate;
	pixDestroy(&pixs);
	OPOLYGLOT_DEBUG(wxT("OPolyglot::Entry FINISH"));
	wxQueueEvent(this,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH));
	return (wxThread::ExitCode)0;
}

void OPolyglot::OnFinishThreadTranslator(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnFinishThreadTranslator"));
	this->Enable(true);
	viewTextTranslate->ViewTranslate();	
	progress->Finish();
}

void OPolyglot::OnStartThreadTranslator(wxThreadEvent &event)
{
	if(event.GetString().IsEmpty())
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnStartOCR CANCEL"));
		wxConfig config(OPOLYGLOT_CONFIG_ARGUMENT);
		if(config.ReadBool(OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP,OPOLYGLOT_CONFIG_BOOL_STAY_ON_TOP_DEFAULT))
		{
			this->SetWindowStyle(this->GetWindowStyle()|wxSTAY_ON_TOP);
		} else
		{
			this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
		}
		return;
	}
	this->Enable(false);
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnStartOCR"));
	this->Raise();

	progress = new OPolyglotDialogProgress(this,_("Translating..."));
	progress->Show();
	valueXML = event.GetString();
	if((IS_NULLPTR(GetThread()))||(!(GetThread()->IsRunning())))
	{
		if(CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglot::OnStartOCR could not create the worker thread!"));
			wxMessageDialog msg(this,_("Could not create the worker thread!"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
			msg.ShowModal();
			return;
		}
		OPOLYGLOT_DEBUG(wxT("OPolyglot::OnStartOCR thread created"));
		if(GetThread()->Run() != wxTHREAD_NO_ERROR)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglot::OnStartOCR could not run the worker thread!"));
			wxMessageDialog msg(this,_("Could not run the worker thread!"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
			msg.ShowModal();
			return;
		}
		OPOLYGLOT_DEBUG(wxT("OPolyglot::OnStartOCR thread runing"));
	}  else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot::OnStartOCR thread is running"));
	}
}


void OPolyglot::SetShow(bool flag)
{
	OPOLYGLOT_MESSAGE(wxT("SetShow(%s) %s"),OPOLYGLOT_BOOL_TO_STRING(flag),OPOLYGLOT_BOOL_TO_STRING(this->IsShown()));
	ScanLanguageFrom();
	ScanLanguageTo();
	viewTextTranslate->LoadXML();
	if(flag)
	{

		this->Show(true);
		//viewTextTranslate->Show(true);
		flagShow = true;
	} else
	{
		this->Show(false);
		viewTextTranslate->Show(false);
		flagShow = false;
	}
}



void OPolyglot::OnClose( wxCloseEvent& event ) { 
	OPOLYGLOT_MESSAGE(wxT("OnClose"));
	if(wxTaskBarIcon::IsAvailable())
	{
		OPOLYGLOT_MESSAGE(wxT("hide to tray"));
		wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW));
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("close window"));
		wxQueueEvent(this->handler,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_EXIT));
	}
}

void OPolyglot::OnScreenshot(wxThreadEvent &event)
{
	wxString fileName = event.GetString();
#ifdef __WXGTK__ 
	wxURI uri(fileName);
	if(uri.HasScheme())
	{
		wxString decodedPath = wxURI::Unescape(uri.GetPath());
		wxFileName file(decodedPath);
		fileName = file.GetFullPath();
		OPOLYGLOT_DEBUG(wxT("OPolyglot::OnScreenshot %s is URI %s"),fileName,uri.GetPath());
	
	} else
	{
		OPOLYGLOT_DEBUG(wxT("OPolyglot::OnScreenshot %s is not URI"),fileName);
	}
#endif
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnScreenshot %s"),fileName);
	if(event.GetInt() != 0)
	{
		if(!wxFileName::FileExists(fileName))
		{
			OPOLYGLOT_ERROR(wxT("OPolyglot::OnScreenshot is file not exist %s"),fileName);
			wxMessageDialog msg(this,wxS("not create screenshot"),wxS("OPolyglot"),wxICON_ERROR|wxOK);
			msg.ShowModal();
			return;
		}

		this->SetWindowStyle(this->GetWindowStyle() & (~((long)wxSTAY_ON_TOP)));
		fullscreen = new OPolyglotFullscreenFrame(this,fileName);
		fullscreen->Raise();
	}
}


void OPolyglot::OnShowTranslation( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnShowTranslation"));
	viewTextTranslate->Show(true);
	viewTextTranslate->Raise();
}

void OPolyglot::OnCloseTranslator(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnCloseTranslator"));
	frameTranslator->Destroy();
	frameTranslator = NULL;
	buttonCaptureScreen->Enable(true);
	buttonShowTranslator->Enable(true);
	this->SetFocus();
}

OPolyglotTranslator::OPolyglotTranslator(wxWindow *parent,wxString languageFrom,wxString languageTo) : GUIOPolyglotTranslator(parent)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator %s -> %s"),languageFrom,languageTo);
	this->SetTitle(wxString::Format(wxT("OPolyglot %s"),_("translator")));
#ifdef __WXMSW__
	SetIcon(wxIcon("MAINICON"));
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#else
	SetIcon(wxICON(icon));
#endif
	Bind(wxEVT_THREAD,&OPolyglotTranslator::OnThreadTranslatorFinish,this);
	Bind(wxEVT_TIMER,&OPolyglotTranslator::OnStartTranslator,this);
	startTranslation = new wxTimer(this);
	wxBitmap copyIcon = wxArtProvider::GetBitmap(wxART_COPY,wxART_BUTTON);
	wxBitmap rechangeIcon = wxArtProvider::GetBitmap(OPOLYGLOT_ART_RECHANGE,wxART_BUTTON,copyIcon.GetSize());
	buttonCopy->SetBitmap(copyIcon);
	buttonRechange->SetBitmap(rechangeIcon);
	wxArrayString languages = OPolyglotGetTranslatedLanguages(OPolyglotGetInstalledLanguagesFrom());
	languages.Sort(CompareLocaleNoCase);
	LanguageFrom->Append(languages);
	if(LanguageFrom->GetStrings().Index(languageFrom) != wxNOT_FOUND)
	{
		LanguageFrom->Select(LanguageFrom->GetStrings().Index(languageFrom));
	} else
	{
		LanguageFrom->Select(0);
	}
	languages = OPolyglotGetTranslatedLanguages(
			OPolyglotGetInstalledLanguagesTo(
				OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection())));
	languages.Sort(CompareLocaleNoCase);
	LanguageTo->Append(languages);
	if(LanguageTo->GetStrings().Index(languageTo) != wxNOT_FOUND)
	{
		LanguageTo->Select(LanguageTo->GetStrings().Index(languageTo));
	} else
	{
		LanguageTo->Select(0);
	}
	wxRect rect = parent->GetRect();
	wxPoint pos = GetPosition();
	pos.y = (rect.GetY()+rect.GetHeight()+5);
	SetPosition(pos);
	buttonCopy->Enable(false);
	this->parent = parent;
	configsTranslator = OPolyglotCreateConfigsFromBergamot(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()),OPolyglotGetOriginalLanguage(LanguageTo->GetStringSelection()));
}

OPolyglotTranslator::~OPolyglotTranslator()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::~OPolyglotTranslator"));
	if(startTranslation->IsRunning())
	{
		startTranslation->Stop();
	}
	delete startTranslation;
}

void OPolyglotTranslator::OnClose(wxCloseEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnClose"));
	wxQueueEvent(parent,new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_CLOSE_TRANSLATOR));
}

void OPolyglotTranslator::OnRechange(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnRechange"));
	wxString oldLangFrom = LanguageFrom->GetStringSelection();
	wxString oldLangTo = LanguageTo->GetStringSelection();
	wxArrayString newLanguagesTo = OPolyglotGetTranslatedLanguages(
			OPolyglotGetInstalledLanguagesTo(
				OPolyglotGetOriginalLanguage(oldLangTo)));
	newLanguagesTo.Sort(CompareLocaleNoCase);
	if((newLanguagesTo.Index(oldLangFrom)==wxNOT_FOUND)||(LanguageFrom->GetStrings().Index(oldLangTo) == wxNOT_FOUND))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnRechange not found \"%s -> %s\" in installed languages"),oldLangTo,oldLangFrom);
		wxMessageDialog msg(this,wxString::Format(wxS("%s \"%s -> %s\""),_("Not found in installed languages"),oldLangTo,oldLangFrom),wxT("OPolyglot"),wxICON_ERROR|wxOK);
		msg.ShowModal();
		return;
	}
	OPOLYGLOT_DEBUG(wxT("OPolyglotTranslator::OnRechange old languages \"%s -> %s\""),oldLangFrom,oldLangTo);
	LanguageFrom->Select(LanguageFrom->GetStrings().Index(oldLangTo));
	LanguageTo->Clear();

	LanguageTo->Append(newLanguagesTo);
	LanguageTo->Select(LanguageTo->GetStrings().Index(oldLangFrom));
	buttonCopy->Enable(false);
	if((IS_NULLPTR(GetThread()))||(!GetThread()->IsRunning()))
	{
		if(textTranslate->GetValue().Length() != 0)
		{
			textOriginal->SetValue(textTranslate->GetValue());
		}
	}
	configsTranslator = OPolyglotCreateConfigsFromBergamot(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()),OPolyglotGetOriginalLanguage(LanguageTo->GetStringSelection()));
	buttonCopy->Enable(false);
	buttonRechange->Enable(false);
	startTranslation->Start(50,wxTIMER_ONE_SHOT);
}

void OPolyglotTranslator::OnLanguageFrom(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnLanguageFrom(%s)"),LanguageFrom->GetStringSelection());
	wxString oldLangTo = LanguageTo->GetStringSelection();
	LanguageTo->Clear();
	wxArrayString languages = OPolyglotGetTranslatedLanguages(
			OPolyglotGetInstalledLanguagesTo(
				OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection())));
	languages.Sort(CompareLocaleNoCase);
	LanguageTo->Append(languages);
	if(LanguageTo->GetStrings().Index(oldLangTo) != wxNOT_FOUND)
	{
		LanguageTo->Select(LanguageTo->GetStrings().Index(oldLangTo));
	} else
	{
		LanguageTo->Select(0);
	}
	configsTranslator.Clear();
	configsTranslator = OPolyglotCreateConfigsFromBergamot(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()),OPolyglotGetOriginalLanguage(LanguageTo->GetStringSelection()));
	if(textOriginal->GetValue().Length() != 0)
	{
		startTranslation->Start(50,wxTIMER_ONE_SHOT);
	}

}

void OPolyglotTranslator::OnLanguageTo(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnLanguageTo(%s)"),LanguageTo->GetStringSelection());
	configsTranslator = OPolyglotCreateConfigsFromBergamot(OPolyglotGetOriginalLanguage(LanguageFrom->GetStringSelection()),OPolyglotGetOriginalLanguage(LanguageTo->GetStringSelection()));
	if(textOriginal->GetValue().Length() != 0)
	{
		startTranslation->Start(50,wxTIMER_ONE_SHOT);
	}
}

wxThread::ExitCode OPolyglotTranslator::Entry()
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::Entry"));
	wxString input =textOriginal->GetValue();
	OPOLYGLOT_DEBUG(wxT("Start translator\n%s"),input);
	wxString result = LibOPolyglotTranslator(input,configsTranslator);
	wxThreadEvent *event = new wxThreadEvent();
	event->SetString(wxString(result));
	wxQueueEvent(GetEventHandler(),event);
	return (wxThread::ExitCode)0;
}

void OPolyglotTranslator::OnThreadTranslatorFinish(wxThreadEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnThreadTranslatorFinish"));
	textOriginal->Enable(true);
	textTranslate->Enable(true);
	buttonRechange->Enable(true);
	textTranslate->SetValue(wxT(""));
	if(event.GetString().IsEmpty())
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnThreadTranslatorFinish return IsNull"));
		wxMessageDialog msg(this,wxT("Error: \"OPolyglotTranslator\" return thread isNull"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
		msg.ShowModal();
		return;
	}
	OPOLYGLOT_DEBUG(wxT("OPolyglotTranslator::OnThreadTranslatorFinish %s"),event.GetString());
	textTranslate->SetValue(event.GetString());
	if(0 < event.GetString().Length())
	{
		buttonCopy->Enable(true);
	}
}

void OPolyglotTranslator::OnTextSource(wxCommandEvent& event) 
{
	startTranslation->Start(1500,wxTIMER_ONE_SHOT);
	buttonCopy->Enable(false);
	buttonRechange->Enable(false);
}

void OPolyglotTranslator::OnStartTranslator(wxTimerEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnStartTranslator configsTranslator.GetCount(%zu)"),configsTranslator.GetCount());
	if((IS_NULLPTR(GetThread()))||(!(GetThread()->IsRunning())))
	{
		textTranslate->Enable(false);
		textTranslate->SetValue(_("translation process..."));
		if(CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnStartTranslator could not create the worker thread!"));
			wxMessageDialog msg(this,_("Could not create the worker thread!"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
			msg.ShowModal();
			return;
		}
		OPOLYGLOT_DEBUG(wxT("OPolyglotTranslator::OnStartTranslator thread created"));
		if(GetThread()->Run() != wxTHREAD_NO_ERROR)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnStartTranslator could not run the worker thread!"));
			wxMessageDialog msg(this,_("Could not run the worker thread!"),wxT("OPolyglot"),wxICON_ERROR|wxOK);
			msg.ShowModal();
			return;
		}
		OPOLYGLOT_DEBUG(wxT("OPolyglotTranslator::OnStartTranslator thread runing"));
	} else
	{
		startTranslation->Start(100,wxTIMER_ONE_SHOT);
	}
}


void OPolyglotTranslator::OnCopy( wxCommandEvent& event )
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglotTranslator::OnCopy"));
	if(wxTheClipboard->Open())
	{
		    wxTheClipboard->SetData( new wxTextDataObject(textTranslate->GetValue()) );
		    wxTheClipboard->Close();
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnCopy not open clipboard"));
	}
}
