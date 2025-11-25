///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6-dirty)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/choice.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/tglbtn.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/checklst.h>
#include <wx/statline.h>
#include <wx/gauge.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class GuiOPolyglot
///////////////////////////////////////////////////////////////////////////////
class GuiOPolyglot : public wxFrame
{
	private:

	protected:
		wxBoxSizer* MainVBox;
		wxChoice* LanguageFrom;
		wxStaticText* labelDirect;
		wxChoice* LanguageTo;
		wxCheckBox* EnableAutoTranslate;
		wxCheckBox* OCRTranslate;
		wxToggleButton* buttonShowTranslate;
		wxPanel* translatePanel;
		wxToggleButton* buttonShowOriginal;
		wxButton* buttonStartTranslate;
		wxTextCtrl* textOriginal;
		wxTextCtrl* textTranslation;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnSelectLanguageFrom( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSelectLanguageTo( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEnableClipboard( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOCRTranslate( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowTranslate( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowOriginal( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStartTranslate( wxCommandEvent& event ) { event.Skip(); }


	public:

		GuiOPolyglot( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxSTAY_ON_TOP|wxTAB_TRAVERSAL );

		~GuiOPolyglot();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIFullscreen
///////////////////////////////////////////////////////////////////////////////
class GUIFullscreen : public wxFrame
{
	private:

	protected:

		// Virtual event handlers, override them in your derived class
		virtual void OnMouseLeftUp( wxMouseEvent& event ) { event.Skip(); }


	public:

		GUIFullscreen( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIFullscreen();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotDownloadLanguage
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotDownloadLanguage : public wxFrame
{
	private:

	protected:
		wxBoxSizer* v_box;
		wxStaticText* labelSetupLanguages;
		wxCheckListBox* ListLanguage;
		wxButton* StartDownload;

		// Virtual event handlers, override them in your derived class
		virtual void OnStartDownload( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotDownloadLanguage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot download language"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 439,163 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIOPolyglotDownloadLanguage();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotSetup
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotSetup : public wxFrame
{
	private:

	protected:
		wxStaticText* label1;
		wxButton* buttonSetupLanguages;
		wxStaticLine* m_staticline1;
		wxStaticText* labelTypeMethodTranslate;
		wxChoice* MethodTranslation;
		wxStaticText* m_staticText81;
		wxChoice* MethodOCR;
		wxStaticLine* m_staticline2;
		wxStaticText* m_staticText9;
		wxCheckBox* StyleStayOnTop;
		wxStaticText* m_staticText8;
		wxChoice* LogLevel;
		wxStaticLine* m_staticline3;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnSetupLanguages( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSelectMethodTranslation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSelectMethodOCR( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnChangeStayOnTop( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnChangeLogLevel( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotSetup( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot setup"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 556,264 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIOPolyglotSetup();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotProgressInstallLanguage
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotProgressInstallLanguage : public wxDialog
{
	private:

	protected:
		wxStaticText* LabelProgress;
		wxStaticText* LabelTimeRemaining;
		wxStaticText* LabelSpeed;
		wxStaticLine* m_staticline4;
		wxStaticText* LabelAllProgress;
		wxGauge* AllProgress;
		wxStaticLine* m_staticline5;
		wxStaticText* LabelFileProgress;
		wxGauge* FileProgress;
		wxButton* ButtonCancel;

		// Virtual event handlers, override them in your derived class
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotProgressInstallLanguage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot download languages"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 480,-1 ), long style = wxDEFAULT_DIALOG_STYLE );

		~GUIOPolyglotProgressInstallLanguage();

};

