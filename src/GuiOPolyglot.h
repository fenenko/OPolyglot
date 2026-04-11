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
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/menu.h>
#include <wx/frame.h>
#include <wx/scrolwin.h>
#include <wx/gauge.h>
#include <wx/statline.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/stc/stc.h>
#include <wx/bmpbuttn.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class GuiOPolyglot
///////////////////////////////////////////////////////////////////////////////
class GuiOPolyglot : public wxFrame
{
	private:

	protected:
		wxBoxSizer* MainVBox;
		wxBoxSizer* h_box1;
		wxPanel* panelMain;
		wxChoice* LanguageFrom;
		wxStaticText* labelDirect;
		wxChoice* LanguageTo;
		wxButton* buttonViewResult;
		wxButton* buttonShowTranslator;
		wxButton* buttonCaptureScreen;
		wxMenuBar* menuBar;
		wxMenu* menuSettings;
		wxMenu* menuHelp;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnSize( wxSizeEvent& event ) { event.Skip(); }
		virtual void OnSelectLanguageFrom( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSelectLanguageTo( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowTranslation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOpenTranslator( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCaptureScreen( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuSetup( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuAbout( wxCommandEvent& event ) { event.Skip(); }


	public:

		GuiOPolyglot( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxSYSTEM_MENU|wxTAB_TRAVERSAL );

		~GuiOPolyglot();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIFullscreen
///////////////////////////////////////////////////////////////////////////////
class GUIFullscreen : public wxFrame
{
	private:

	protected:

	public:

		GUIFullscreen( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxFRAME_FLOAT_ON_PARENT|wxSTAY_ON_TOP|wxBORDER_NONE|wxTAB_TRAVERSAL );

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
		wxScrolledWindow* ListLanguages;
		wxBoxSizer* box;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotDownloadLanguage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot setup languages"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 640,320 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIOPolyglotDownloadLanguage();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotProgressOCRTranslator
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotProgressOCRTranslator : public wxFrame
{
	private:

	protected:
		wxBoxSizer* vBox;
		wxStaticText* ProgressLabel;
		wxGauge* Progress;
		wxButton* Cancel;

		// Virtual event handlers, override them in your derived class
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotProgressOCRTranslator( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Progress OCR and Translate"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIOPolyglotProgressOCRTranslator();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotSettings
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotSettings : public wxFrame
{
	private:

	protected:
		wxBoxSizer* MainBox;
		wxBoxSizer* HBox0;
		wxStaticText* LabelInterface;
		wxChoice* SelectInterfaceLanguage;
		wxStaticLine* m_staticline4;
		wxScrolledWindow* ListLanguages;
		wxBoxSizer* boxLanguages;
		wxBoxSizer* HBox3;
		wxStaticText* m_staticText25;
		wxChoice* additionalLanguageOCR;
		wxBoxSizer* HBox2;
		wxStaticText* m_staticText81;
		wxChoice* MethodOCR;
		wxBoxSizer* HBox1;
		wxStaticText* labelTypeMethodTranslate;
		wxChoice* MethodTranslation;
		wxStaticLine* m_staticline2;
		wxBoxSizer* HBox4;
		wxStaticText* m_staticText18;
		wxButton* RulesPreprocessing;
		wxCheckBox* EnablePreprocessing;
		wxBoxSizer* HBox5;
		wxStaticText* LabelPostprocessing;
		wxButton* RulesPostprocessing;
		wxCheckBox* EnablePostprocessing;
		wxStaticText* m_staticText9;
		wxCheckBox* StyleStayOnTop;
		wxStaticText* m_staticText8;
		wxChoice* LogLevel;
		wxButton* ViewLog;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnSelectInterfaceLanguage( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAdditionalLanguage( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSelectMethodOCR( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSelectMethodTranslation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRulesPreprocessing( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEnablePreprocessing( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRulesPostprocessing( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEnablePostprocessing( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnChangeStayOnTop( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnChangeLogLevel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnViewLog( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotSettings( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 680,560 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIOPolyglotSettings();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotProgressInstallLanguage
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotProgressInstallLanguage : public wxFrame
{
	private:

	protected:
		wxBoxSizer* MainBox;
		wxBoxSizer* HBox1;
		wxStaticText* Labeltimeelapsed;
		wxStaticText* TimeElapsed;
		wxBoxSizer* HBox2;
		wxStaticText* Labeltimeremaining;
		wxStaticText* TimeRemaining;
		wxBoxSizer* HBox3;
		wxStaticText* Labelspeed;
		wxStaticText* Speed;
		wxBoxSizer* HBox3_2;
		wxStaticText* LabelFileProgress;
		wxStaticText* SizeFile;
		wxGauge* FileProgress;
		wxBoxSizer* HBox3_1;
		wxStaticText* LabelAllProgress;
		wxStaticText* SizeAll;
		wxGauge* AllProgress;
		wxBoxSizer* HBox4;
		wxButton* ButtonCancel;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotProgressInstallLanguage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot install languages"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 640,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxTAB_TRAVERSAL );

		~GUIOPolyglotProgressInstallLanguage();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotEditorRule
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotEditorRule : public wxFrame
{
	private:

	protected:
		wxBoxSizer* MainBox;
		wxStaticText* NumberRule;
		wxTextCtrl* Comment;
		wxStaticText* m_staticText21;
		wxTextCtrl* RegEx;
		wxStaticText* m_staticText22;
		wxTextCtrl* ReplacementRule;
		wxButton* Test;
		wxButton* Save;
		wxButton* Cancel;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnTest( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSave( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotEditorRule( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot editing the rule"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxICONIZE|wxTAB_TRAVERSAL );

		~GUIOPolyglotEditorRule();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotMultilineText
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotMultilineText : public wxDialog
{
	private:

	protected:
		wxBoxSizer* MainBox;
		wxTextCtrl* Value;
		wxTextCtrl* ValueRO;
		wxBoxSizer* VBox;
		wxButton* Ok;
		wxButton* Cancel;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnOk( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotMultilineText( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 480,240 ), long style = wxDEFAULT_DIALOG_STYLE );

		~GUIOPolyglotMultilineText();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotListRules
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotListRules : public wxFrame
{
	private:

	protected:
		wxBoxSizer* MainBox;
		wxListCtrl* ListRules;
		wxButton* Add;
		wxButton* Test;
		wxButton* Save;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnSelectItem( wxListEvent& event ) { event.Skip(); }
		virtual void OnListRulesMenu( wxListEvent& event ) { event.Skip(); }
		virtual void OnAdd( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTest( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSave( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotListRules( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot list rules"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIOPolyglotListRules();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIAbout
///////////////////////////////////////////////////////////////////////////////
class GUIAbout : public wxFrame
{
	private:

	protected:
		wxStaticText* labelOpolyglot;
		wxTextCtrl* licensesOpolyglot;

	public:

		GUIAbout( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("About OPolyglot"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 647,643 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIAbout();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIViewLog
///////////////////////////////////////////////////////////////////////////////
class GUIViewLog : public wxFrame
{
	private:

	protected:
		wxStyledTextCtrl* Log;

	public:

		GUIViewLog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot log view"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,480 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIViewLog();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotViewTextTranslate
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotViewTextTranslate : public wxFrame
{
	private:

	protected:
		wxBitmapButton* buttonClear;
		wxBitmapButton* buttonCopy;
		wxStyledTextCtrl* textTranslate;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnClear( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCopy( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotViewTextTranslate( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OPolyglot"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,480 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIOPolyglotViewTextTranslate();

};

///////////////////////////////////////////////////////////////////////////////
/// Class GUIOPolyglotTranslator
///////////////////////////////////////////////////////////////////////////////
class GUIOPolyglotTranslator : public wxFrame
{
	private:

	protected:
		wxChoice* LanguageFrom;
		wxTextCtrl* textOriginal;
		wxBitmapButton* buttonRechange;
		wxChoice* LanguageTo;
		wxBitmapButton* buttonCopy;
		wxTextCtrl* textTranslate;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnLanguageFrom( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTextSource( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRechange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLanguageTo( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCopy( wxCommandEvent& event ) { event.Skip(); }


	public:

		GUIOPolyglotTranslator( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,480 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~GUIOPolyglotTranslator();

};

