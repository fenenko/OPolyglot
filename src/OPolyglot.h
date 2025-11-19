#pragma once
#include <wx/timer.h>
#include <wx/dcscreen.h>
#include <wx/taskbar.h>
#include "GuiOPolyglot.h"
#include "Translator.h"
#if 0
#include "OPolyglotDialogTranslation.h"
#endif
#include "OPolyglotDownloadLanguage.h"
#include <wx/dynarray.h>
#include <wx/uiaction.h>
#define OPOLYGLOT_H_VERSION	"$Id: OPolyglot.h,v 1.18 2025/11/17 08:52:39 oleksandr Exp oleksandr $"
//#include "ThreadClipboard.h"
//

/*
 * SetString(file for OCR)
 * SetString(wxEmptyString) not select aread
 */
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_SELECT_AREA,	wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_EXIT_PROGRAMM,	wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION,	wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_SETUP_LANGUAGES,	wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES,	wxThreadEvent);





class FullscreenFrame : public GUIFullscreen
{
	public:
		FullscreenFrame(wxWindow *parent);
		~FullscreenFrame();
		void OnTimeMouseState(wxTimerEvent &event);
		void OnPaint(wxPaintEvent &event);
#if 1
		//void OnMouseLeftDown( wxMouseEvent& event )  wxOVERRIDE; 
		void OnMouseLeftUp( wxMouseEvent& event ) wxOVERRIDE; 
	//	void OnMouseEvent( wxMouseEvent& event ) wxOVERRIDE; 
//		void OnAux1Up( wxMouseEvent& event ) wxOVERRIDE; 
#endif
	private:
		wxUIActionSimulator action;
		wxTimer *timer;
		int startX;
		int startY;
		int oldX;
		int oldY;
		wxWindow *parent;
		wxBitmap bitmap;
		int timePressedLeft = 0;
		
};

class OPolyglotTaskBar : public wxTaskBarIcon
{
	public:
		OPolyglotTaskBar(wxWindow *parent);
		void OnView(wxCommandEvent& event);
		void OnMenuExit(wxCommandEvent &event);
		void OnSetupLanguage(wxCommandEvent &event);
		void OnLeftDown(wxTaskBarIconEvent &event);
		virtual wxMenu *CreatePopupMenu() wxOVERRIDE;

	private:
		wxWindow *parent;
		bool viewTranslator = false;	
};

class OPolyglot : public GuiOPolyglot ,protected wxThreadHelper
{
	public:
		OPolyglot(wxFrame *frame);
		~OPolyglot();
		void OnEnableClipboard( wxCommandEvent& event ) wxOVERRIDE;
		void OnShowTranslate( wxCommandEvent& event ) wxOVERRIDE;
		void OnShowOriginal(wxCommandEvent& event) wxOVERRIDE;
		void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnTimeCheckClipboard(wxTimerEvent &event);
		void OnTimeCheckMouseState(wxTimerEvent &event);
		void OnTimerProgressOCRTranslation(wxTimerEvent &event);
		void OnOCRTranslate( wxCommandEvent& event ) wxOVERRIDE;
		void OnPaint(wxPaintEvent &event);
		void OnSelectArea(wxThreadEvent &event);
		void OnSetupLanguages(wxThreadEvent &event);
		void OnFinishSetupLanguages(wxThreadEvent &event);
		void OnRightClick(wxMouseEvent &event);
		void OnExitProgramm(wxThreadEvent &event);
		void OnExitThreadTranslation(wxThreadEvent &event);
#if 0
		void OnSelectLanguage(wxCommandEvent& event);
#endif
		void OnSelectLanguageFrom( wxCommandEvent& event ) wxOVERRIDE;
		void OnSelectLanguageTo( wxCommandEvent& event ) wxOVERRIDE;
		void OnStartTranslate(wxCommandEvent& event) wxOVERRIDE;
		void ScanLangs();
		void ScanLanguageFrom();
		void ScanLanguageTo();
		void SetVisible(bool flag);
#if 0
		void Hide();
#endif
		void StartTranslation();
		void StartThreadTranslation();
		void CreateTranslatorConfig();
		
	protected:
	private:
		virtual wxThread::ExitCode Entry() wxOVERRIDE;

		wxString 		GetLangCodeForOCR();
		wxTimer			*timerClipboardChecking;
		wxTimer			*timerMouseState;
		wxTimer			*timerProgressOcrTranslation;
		bool 			mouseLeftButtonPressed;
		wxProgressDialog *progressThreadTranslation;
		wxString		messageProgressThreadTranslation;
		wxMutex 		mutexProgressThreadTranslation;
		int coordStartX;
		int coordStartY;
		wxString lastClipboardText;
		OPolyglotTaskBar *taskBar;
		bool viewDialogTranslator;
		bool flagShow = true;

		FullscreenFrame *fullscreen = nullptr;
		wxArrayString configTranslatorFileYml;
		wxString filenameImageAreaForOCR;
		//OPolyglotDownloadLanguage	*frameDownloadsLanguage;
};

