///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6-dirty)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "GuiOPolyglot.h"

///////////////////////////////////////////////////////////////////////////

GuiOPolyglot::GuiOPolyglot( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ) );

	MainVBox = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* h_box1;
	h_box1 = new wxBoxSizer( wxHORIZONTAL );

	wxArrayString LanguageFromChoices;
	LanguageFrom = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LanguageFromChoices, 0 );
	LanguageFrom->SetSelection( 0 );
	h_box1->Add( LanguageFrom, 0, wxALL, 5 );

	labelDirect = new wxStaticText( this, wxID_ANY, _("->"), wxDefaultPosition, wxDefaultSize, 0 );
	labelDirect->Wrap( -1 );
	h_box1->Add( labelDirect, 0, wxALIGN_CENTER|wxALL, 0 );

	wxArrayString LanguageToChoices;
	LanguageTo = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LanguageToChoices, 0 );
	LanguageTo->SetSelection( 0 );
	h_box1->Add( LanguageTo, 0, wxALL, 5 );

	EnableAutoTranslate = new wxCheckBox( this, wxID_ANY, _("auto translate clipboards"), wxDefaultPosition, wxDefaultSize, 0 );
	h_box1->Add( EnableAutoTranslate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	OCRTranslate = new wxCheckBox( this, wxID_ANY, _("screanshot translate"), wxDefaultPosition, wxDefaultSize, 0 );
	h_box1->Add( OCRTranslate, 0, wxALL|wxEXPAND, 5 );


	h_box1->Add( 0, 0, 1, wxEXPAND, 5 );

	buttonShowTranslate = new wxToggleButton( this, wxID_ANY, _("show translation"), wxDefaultPosition, wxDefaultSize, 0 );
	h_box1->Add( buttonShowTranslate, 0, wxALL, 5 );


	MainVBox->Add( h_box1, 0, wxALL|wxEXPAND, 0 );

	translatePanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	translatePanel->Hide();

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );

	buttonShowOriginal = new wxToggleButton( translatePanel, wxID_ANY, _("show the text of the original"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer9->Add( buttonShowOriginal, 0, wxALL, 5 );


	bSizer9->Add( 0, 0, 1, wxEXPAND, 5 );

	buttonStartTranslate = new wxButton( translatePanel, wxID_ANY, _("Start a translation"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer9->Add( buttonStartTranslate, 0, wxALL, 5 );


	bSizer8->Add( bSizer9, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	textOriginal = new wxTextCtrl( translatePanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	textOriginal->Hide();

	bSizer10->Add( textOriginal, 1, wxALL|wxEXPAND, 0 );

	textTranslation = new wxTextCtrl( translatePanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	bSizer10->Add( textTranslation, 1, wxALL|wxEXPAND, 0 );


	bSizer8->Add( bSizer10, 6, wxALL|wxEXPAND, 5 );


	translatePanel->SetSizer( bSizer8 );
	translatePanel->Layout();
	bSizer8->Fit( translatePanel );
	MainVBox->Add( translatePanel, 1, wxEXPAND | wxALL, 0 );


	this->SetSizer( MainVBox );
	this->Layout();
	MainVBox->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GuiOPolyglot::OnClose ) );
	LanguageFrom->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GuiOPolyglot::OnSelectLanguageFrom ), NULL, this );
	LanguageTo->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GuiOPolyglot::OnSelectLanguageTo ), NULL, this );
	EnableAutoTranslate->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnEnableClipboard ), NULL, this );
	OCRTranslate->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnOCRTranslate ), NULL, this );
	buttonShowTranslate->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnShowTranslate ), NULL, this );
	buttonShowOriginal->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnShowOriginal ), NULL, this );
	buttonStartTranslate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnStartTranslate ), NULL, this );
}

GuiOPolyglot::~GuiOPolyglot()
{
}

GUIFullscreen::GUIFullscreen( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );


	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( GUIFullscreen::OnMouseLeftUp ) );
}

GUIFullscreen::~GUIFullscreen()
{
}

GUIOPolyglotDownloadLanguage::GUIOPolyglotDownloadLanguage( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	v_box = new wxBoxSizer( wxVERTICAL );

	labelSetupLanguages = new wxStaticText( this, wxID_ANY, _("Select languages for install:"), wxDefaultPosition, wxDefaultSize, 0 );
	labelSetupLanguages->Wrap( -1 );
	v_box->Add( labelSetupLanguages, 0, wxALL, 5 );

	wxArrayString ListLanguageChoices;
	ListLanguage = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, ListLanguageChoices, 0 );
	v_box->Add( ListLanguage, 1, wxALL|wxEXPAND, 5 );

	StartDownload = new wxButton( this, wxID_ANY, _("Setup languages"), wxDefaultPosition, wxDefaultSize, 0 );
	v_box->Add( StartDownload, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( v_box );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	StartDownload->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotDownloadLanguage::OnStartDownload ), NULL, this );
}

GUIOPolyglotDownloadLanguage::~GUIOPolyglotDownloadLanguage()
{
}

GUIOPolyglotSetup::GUIOPolyglotSetup( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxHORIZONTAL );

	label1 = new wxStaticText( this, wxID_ANY, _("Setup Language"), wxDefaultPosition, wxDefaultSize, 0 );
	label1->Wrap( -1 );
	bSizer17->Add( label1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer17->Add( 0, 0, 1, wxEXPAND, 5 );

	buttonSetupLanguages = new wxButton( this, wxID_ANY, _("install or remove"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer17->Add( buttonSetupLanguages, 0, wxALL|wxEXPAND, 5 );


	bSizer7->Add( bSizer17, 0, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer7->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );

	labelTypeMethodTranslate = new wxStaticText( this, wxID_ANY, _("Preferred method of translation"), wxDefaultPosition, wxDefaultSize, 0 );
	labelTypeMethodTranslate->Wrap( -1 );
	bSizer12->Add( labelTypeMethodTranslate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer12->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString MethodTranslationChoices[] = { _("BEST"), _("FAST") };
	int MethodTranslationNChoices = sizeof( MethodTranslationChoices ) / sizeof( wxString );
	MethodTranslation = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, MethodTranslationNChoices, MethodTranslationChoices, 0 );
	MethodTranslation->SetSelection( 0 );
	bSizer12->Add( MethodTranslation, 0, wxALL, 5 );


	bSizer7->Add( bSizer12, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText81 = new wxStaticText( this, wxID_ANY, _("Preffered method of OCR"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	bSizer19->Add( m_staticText81, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer19->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString MethodOCRChoices[] = { _("BEST"), _("FAST") };
	int MethodOCRNChoices = sizeof( MethodOCRChoices ) / sizeof( wxString );
	MethodOCR = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, MethodOCRNChoices, MethodOCRChoices, 0 );
	MethodOCR->SetSelection( 0 );
	bSizer19->Add( MethodOCR, 0, wxALL, 5 );


	bSizer7->Add( bSizer19, 0, wxEXPAND, 5 );

	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer7->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );

	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText9 = new wxStaticText( this, wxID_ANY, _("Stay on top of all other windows"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	bSizer18->Add( m_staticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer18->Add( 0, 0, 1, wxEXPAND, 5 );

	StyleStayOnTop = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	StyleStayOnTop->SetValue(true);
	bSizer18->Add( StyleStayOnTop, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer7->Add( bSizer18, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText8 = new wxStaticText( this, wxID_ANY, _("Log level"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	bSizer15->Add( m_staticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer15->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString LogLevelChoices[] = { _("DEBUG"), _("MESSAGE"), _("WARNING"), _("ERROR") };
	int LogLevelNChoices = sizeof( LogLevelChoices ) / sizeof( wxString );
	LogLevel = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LogLevelNChoices, LogLevelChoices, 0 );
	LogLevel->SetSelection( 0 );
	bSizer15->Add( LogLevel, 0, wxALL, 5 );


	bSizer7->Add( bSizer15, 0, wxEXPAND, 5 );

	m_staticline3 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer7->Add( m_staticline3, 0, wxEXPAND | wxALL, 5 );


	this->SetSizer( bSizer7 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotSetup::OnClose ) );
	buttonSetupLanguages->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotSetup::OnSetupLanguages ), NULL, this );
	MethodTranslation->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSetup::OnSelectMethodTranslation ), NULL, this );
	MethodOCR->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSetup::OnSelectMethodOCR ), NULL, this );
	StyleStayOnTop->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GUIOPolyglotSetup::OnChangeStayOnTop ), NULL, this );
	LogLevel->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSetup::OnChangeLogLevel ), NULL, this );
}

GUIOPolyglotSetup::~GUIOPolyglotSetup()
{
}
