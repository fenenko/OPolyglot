#pragma once
#include <wx/timer.h>
#include <wx/dcscreen.h>
#include <wx/taskbar.h>
#include "GuiOPolyglot.h"
#include "OPolyglotType.h"
#include "OPolyglotDownloadLanguage.h"
#include "OPolyglotFullscreenFrame.h"
#include "OPolyglotThread.h"
#include <wx/dynarray.h>
#include <wx/dynlib.h>

/*
 * SetString(file for OCR)
 * SetString(wxEmptyString) not select aread
 */


class OPolyglotProgress : public GUIOPolyglotProgressOCRTranslator
{
	protected:
		wxWindow *parent;
		void OnUpdateProgress(wxTimerEvent &event);
		wxTimer timerUpdate;
		wxMutex mutex;
	public:
		OPolyglotProgress(wxWindow *parent);
		~OPolyglotProgress();
		void Finish();
};


class OPolyglot : public GuiOPolyglot 
{
	public:
		OPolyglot(wxEvtHandler *handler);
		~OPolyglot();
		void OnEnableClipboard( wxCommandEvent& event ) wxOVERRIDE;
		void OnShowTranslate( wxCommandEvent& event ) wxOVERRIDE;
		void OnShowOriginal(wxCommandEvent& event) wxOVERRIDE;
	void OnClose( wxCloseEvent& event ) wxOVERRIDE;
		void OnTimeCheckClipboard(wxTimerEvent &event);
		void OnTimeCheckMouseState(wxTimerEvent &event);
		void OnOCRTranslate( wxCommandEvent& event ) wxOVERRIDE;
		void OnPaint(wxPaintEvent &event);
		void OnReceivImage(wxThreadEvent &event);
		void OnFinishSetupLanguages(wxThreadEvent &event);
		void OnRightClick(wxMouseEvent &event);
		void OnExitThreadTranslation(wxThreadEvent &event);
		void OnExitThreadOCR(wxThreadEvent &event);
		void OnSelectLanguageFrom( wxCommandEvent& event ) wxOVERRIDE;
		void OnSelectLanguageTo( wxCommandEvent& event ) wxOVERRIDE;
		void OnStartTranslate(wxCommandEvent& event) wxOVERRIDE;
		void ScanLangs();
		void ScanLanguageFrom();
		void ScanLanguageTo();
		void SetShow(bool flag);
		void FinishThread();
		void OnCopyTextTranslate( wxCommandEvent& event ) ;
		void AddOrSetOriginalText(wxString text);
		void StartTranslation();
		void StartThreadTranslation();
		void CreateTranslatorConfig();
		
	protected:
	private:
		wxEvtHandler *handler;
		wxTimer			*timerClipboardChecking;
		wxTimer			*timerMouseState;
		bool 			mouseLeftButtonPressed;
		//wxProgressDialog *progressThreadTranslation;
		OPolyglotProgress *progress;
		wxString		messageProgressThreadTranslation;
		wxMutex 		mutexProgressThreadTranslation;
		OPolyglotThreadTranslator	*threadTranslator = NULL;
		OPolyglotThreadOCR			*threadOCR = NULL;
		OPolyglotDownloadLanguage	*frameDownload = NULL;
		int coordStartX;
		int coordStartY;
		wxString lastClipboardText;
		bool viewDialogTranslator;
		wxString textForTranslate;
		wxArrayString codeTranslateLanguageFrom;
		bool flagShow = true;

		OPolyglotFullscreenFrame *fullscreen = nullptr;
		wxArrayString configTranslatorFileYml;
		wxArrayString installLanguageFrom;
		wxArrayString installLanguageTo;
		wxArrayString installCodeTranslator;
		wxArrayString preProcessingRegex;
		wxArrayString preProcessingReplace;
		wxArrayString postProcessingRegex;
		wxArrayString postProcessingReplace;
		OPolyglotImage	*imageForOCR;
};

