#pragma once
#include <wx/timer.h>
#include <wx/dcscreen.h>
#include <wx/taskbar.h>
#include "GuiOPolyglot.h"
#if 0
#include "OPolyglotDialogTranslation.h"
#endif
#include "OPolyglotDownloadLanguage.h"
#include <wx/dynarray.h>
#include <wx/uiaction.h>
#include <wx/dynlib.h>
//#include "ThreadClipboard.h"
//

/*
 * SetString(file for OCR)
 * SetString(wxEmptyString) not select aread
 */
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_SELECT_AREA,	wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_EXIT_PROGRAMM,	wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_EXIT_THREAD_TRANSLATION,	wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_SET_TEXT_ORIGINAL,		wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_SETUP_LANGUAGES,	wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_FINISH_SETUP_LANGUAGES,	wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_UPDATE_PROGRESS_MESSAGE,	wxThreadEvent);



class OPolyglot;

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

class OPolyglotThreadTranslator : public wxThread
{
	public:
		OPolyglotThreadTranslator(OPolyglot *handler,wxString dirOCR,wxString landOCR,wxArrayString *configsYml,wxString textOriginal,wxString fileForOcr);
		~OPolyglotThreadTranslator();
	protected:
		virtual ExitCode Entry() wxOVERRIDE;
		virtual void OnExit() wxOVERRIDE;
		virtual void OnKill() wxOVERRIDE;
	private:
		OPolyglot *handler;
		wxString dirOCR;
		wxString langOCR;
		wxArrayString *configsYmlTranslator;
		wxString textOriginal;
		wxString filenameImageAreaForOCR;
		wxDynamicLibrary 		*library;
};

class OPolyglot : public GuiOPolyglot 
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
		void OnSetTextOriginal(wxThreadEvent &event);
		void OnUpdateProgressMessage(wxThreadEvent &event);
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

		wxString 		GetLangCodeForOCR();
		wxTimer			*timerClipboardChecking;
		wxTimer			*timerMouseState;
		wxTimer			*timerProgressOcrTranslation;
		bool 			mouseLeftButtonPressed;
		wxProgressDialog *progressThreadTranslation;
		wxString		messageProgressThreadTranslation;
		wxMutex 		mutexProgressThreadTranslation;
		OPolyglotThreadTranslator	*threadOCRTranslator;
		bool		flagThreadOCRTranslationIsRun = false;
		int coordStartX;
		int coordStartY;
		wxString lastClipboardText;
		OPolyglotTaskBar *taskBar;
		bool viewDialogTranslator;
		wxString textForTranslate;
		wxArrayString codeTranslateLanguageFrom;
		bool flagShow = true;

		FullscreenFrame *fullscreen = nullptr;
		wxArrayString configTranslatorFileYml;
		wxArrayString codeLanguageFrom;
		wxArrayString codeLanguageTo;
		wxArrayString codesTranslator;
		wxArrayString idsTranslator;
		wxString filenameImageAreaForOCR;
		//OPolyglotDownloadLanguage	*frameDownloadsLanguage;
};

