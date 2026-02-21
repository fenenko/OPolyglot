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

	wxString LanguageFromChoices[] = { _("ADD LANGUAGE") };
	int LanguageFromNChoices = sizeof( LanguageFromChoices ) / sizeof( wxString );
	LanguageFrom = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LanguageFromNChoices, LanguageFromChoices, 0 );
	LanguageFrom->SetSelection( 0 );
	h_box1->Add( LanguageFrom, 0, wxALL, 5 );

	labelDirect = new wxStaticText( this, wxID_ANY, _("->"), wxDefaultPosition, wxDefaultSize, 0 );
	labelDirect->Wrap( -1 );
	h_box1->Add( labelDirect, 0, wxALIGN_CENTER|wxALL, 0 );

	wxString LanguageToChoices[] = { _("ADD LANGUAGE") };
	int LanguageToNChoices = sizeof( LanguageToChoices ) / sizeof( wxString );
	LanguageTo = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LanguageToNChoices, LanguageToChoices, 0 );
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
	bSizer9->Add( buttonShowOriginal, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer9->Add( 0, 0, 1, wxEXPAND, 5 );

	buttonStartTranslate = new wxButton( translatePanel, wxID_ANY, _("Start a translation"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer9->Add( buttonStartTranslate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	ButtonCopyTranslate = new wxBitmapButton( translatePanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );

	ButtonCopyTranslate->SetBitmap( wxNullBitmap );
	ButtonCopyTranslate->Enable( false );

	bSizer9->Add( ButtonCopyTranslate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


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
	menuBar = new wxMenuBar( 0 );
	menuSettings = new wxMenu();
	wxMenuItem* menuSetup;
	menuSetup = new wxMenuItem( menuSettings, wxID_ANY, wxString( _("Setup") ) , wxEmptyString, wxITEM_NORMAL );
	menuSettings->Append( menuSetup );

	menuBar->Append( menuSettings, _("Settings") );

	menuHelp = new wxMenu();
	wxMenuItem* menuAbout;
	menuAbout = new wxMenuItem( menuHelp, wxID_ANY, wxString( _("About") ) , wxEmptyString, wxITEM_NORMAL );
	menuHelp->Append( menuAbout );

	menuBar->Append( menuHelp, _("Help") );

	this->SetMenuBar( menuBar );


	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GuiOPolyglot::OnClose ) );
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( GuiOPolyglot::OnSize ) );
	LanguageFrom->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GuiOPolyglot::OnSelectLanguageFrom ), NULL, this );
	LanguageTo->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GuiOPolyglot::OnSelectLanguageTo ), NULL, this );
	EnableAutoTranslate->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnEnableClipboard ), NULL, this );
	OCRTranslate->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnOCRTranslate ), NULL, this );
	buttonShowTranslate->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnShowTranslate ), NULL, this );
	buttonShowOriginal->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnShowOriginal ), NULL, this );
	buttonStartTranslate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnStartTranslate ), NULL, this );
	ButtonCopyTranslate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnCopyTextTranslate ), NULL, this );
	menuSettings->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( GuiOPolyglot::OnMenuSetup ), this, menuSetup->GetId());
	menuHelp->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( GuiOPolyglot::OnMenuAbout ), this, menuAbout->GetId());
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

	StartDownload = new wxButton( this, wxID_ANY, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
	v_box->Add( StartDownload, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( v_box );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotDownloadLanguage::OnClose ) );
	StartDownload->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotDownloadLanguage::OnStartDownload ), NULL, this );
}

GUIOPolyglotDownloadLanguage::~GUIOPolyglotDownloadLanguage()
{
}

GUIOPolyglotProgressOCRTranslator::GUIOPolyglotProgressOCRTranslator( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxVERTICAL );

	ProgressLabel = new wxStaticText( this, wxID_ANY, _("OCR and Translator"), wxDefaultPosition, wxDefaultSize, 0 );
	ProgressLabel->Wrap( -1 );
	bSizer28->Add( ProgressLabel, 0, wxALL, 5 );

	Progress = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	Progress->SetValue( 0 );
	bSizer28->Add( Progress, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );


	bSizer29->Add( 0, 0, 1, wxEXPAND, 5 );

	Cancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	Cancel->Hide();

	bSizer29->Add( Cancel, 0, wxALL, 5 );


	bSizer28->Add( bSizer29, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( bSizer28 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotProgressOCRTranslator::OnCancel ), NULL, this );
}

GUIOPolyglotProgressOCRTranslator::~GUIOPolyglotProgressOCRTranslator()
{
}

GUIOPolyglotSetup::GUIOPolyglotSetup( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainBox = new wxBoxSizer( wxVERTICAL );

	ButtonSetupLanguages = new wxButton( this, wxID_ANY, _("Install languages"), wxDefaultPosition, wxDefaultSize, 0 );
	ButtonSetupLanguages->SetHelpText( _("Help") );

	MainBox->Add( ButtonSetupLanguages, 0, wxALL|wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	MainBox->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );

	HBox1 = new wxBoxSizer( wxHORIZONTAL );

	labelTypeMethodTranslate = new wxStaticText( this, wxID_ANY, _("Preferred method of translation"), wxDefaultPosition, wxDefaultSize, 0 );
	labelTypeMethodTranslate->Wrap( -1 );
	HBox1->Add( labelTypeMethodTranslate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox1->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString MethodTranslationChoices[] = { _("BEST"), _("FAST") };
	int MethodTranslationNChoices = sizeof( MethodTranslationChoices ) / sizeof( wxString );
	MethodTranslation = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, MethodTranslationNChoices, MethodTranslationChoices, 0 );
	MethodTranslation->SetSelection( 0 );
	HBox1->Add( MethodTranslation, 0, wxALL, 5 );


	MainBox->Add( HBox1, 0, wxEXPAND, 5 );

	HBox2 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText81 = new wxStaticText( this, wxID_ANY, _("Preffered method of OCR"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	HBox2->Add( m_staticText81, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox2->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString MethodOCRChoices[] = { _("BEST"), _("FAST") };
	int MethodOCRNChoices = sizeof( MethodOCRChoices ) / sizeof( wxString );
	MethodOCR = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, MethodOCRNChoices, MethodOCRChoices, 0 );
	MethodOCR->SetSelection( 0 );
	HBox2->Add( MethodOCR, 0, wxALL, 5 );


	MainBox->Add( HBox2, 0, wxEXPAND, 5 );

	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	MainBox->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );

	HBox3 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText17 = new wxStaticText( this, wxID_ANY, _("Text creation mode for translation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	HBox3->Add( m_staticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox3->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString ModeCreationTextChoices[] = { _("NEW"), _("APPEND") };
	int ModeCreationTextNChoices = sizeof( ModeCreationTextChoices ) / sizeof( wxString );
	ModeCreationText = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, ModeCreationTextNChoices, ModeCreationTextChoices, 0 );
	ModeCreationText->SetSelection( 0 );
	HBox3->Add( ModeCreationText, 0, wxALL, 5 );


	MainBox->Add( HBox3, 0, wxALL|wxEXPAND, 0 );

	HBox4 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText18 = new wxStaticText( this, wxID_ANY, _("Enable text pre-processing before translation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	HBox4->Add( m_staticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox4->Add( 0, 0, 1, wxEXPAND, 5 );

	RulesPreprocessing = new wxButton( this, wxID_ANY, _("Rules"), wxDefaultPosition, wxDefaultSize, 0 );
	HBox4->Add( RulesPreprocessing, 0, wxALL, 5 );

	EnablePreprocessing = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	HBox4->Add( EnablePreprocessing, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox4, 1, wxEXPAND, 5 );

	HBox5 = new wxBoxSizer( wxHORIZONTAL );

	LabelPostprocessing = new wxStaticText( this, wxID_ANY, _("Enable post-processing of text after translation"), wxDefaultPosition, wxDefaultSize, 0 );
	LabelPostprocessing->Wrap( -1 );
	HBox5->Add( LabelPostprocessing, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox5->Add( 0, 0, 1, wxEXPAND, 5 );

	RulesPostprocessing = new wxButton( this, wxID_ANY, _("Rules"), wxDefaultPosition, wxDefaultSize, 0 );
	HBox5->Add( RulesPostprocessing, 0, wxALL, 5 );

	EnablePostprocessing = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	HBox5->Add( EnablePostprocessing, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox5, 0, wxEXPAND, 5 );

	wxBoxSizer* HBox6;
	HBox6 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText9 = new wxStaticText( this, wxID_ANY, _("Stay on top of all other windows"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	HBox6->Add( m_staticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox6->Add( 0, 0, 1, wxEXPAND, 5 );

	StyleStayOnTop = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	StyleStayOnTop->SetValue(true);
	HBox6->Add( StyleStayOnTop, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox6, 0, wxEXPAND, 5 );

	wxBoxSizer* HBox7;
	HBox7 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText8 = new wxStaticText( this, wxID_ANY, _("Log level"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	HBox7->Add( m_staticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox7->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString LogLevelChoices[] = { _("DEBUG"), _("MESSAGE"), _("WARNING"), _("ERROR") };
	int LogLevelNChoices = sizeof( LogLevelChoices ) / sizeof( wxString );
	LogLevel = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LogLevelNChoices, LogLevelChoices, 0 );
	LogLevel->SetSelection( 0 );
	HBox7->Add( LogLevel, 0, wxALL, 5 );


	MainBox->Add( HBox7, 0, wxEXPAND, 5 );


	this->SetSizer( MainBox );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotSetup::OnClose ) );
	ButtonSetupLanguages->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotSetup::OnSetupLanguages ), NULL, this );
	MethodTranslation->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSetup::OnSelectMethodTranslation ), NULL, this );
	MethodOCR->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSetup::OnSelectMethodOCR ), NULL, this );
	ModeCreationText->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSetup::OnModeCreationText ), NULL, this );
	RulesPreprocessing->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotSetup::OnRulesPreprocessing ), NULL, this );
	EnablePreprocessing->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GUIOPolyglotSetup::OnEnablePreprocessing ), NULL, this );
	RulesPostprocessing->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotSetup::OnRulesPostprocessing ), NULL, this );
	EnablePostprocessing->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GUIOPolyglotSetup::OnEnablePostprocessing ), NULL, this );
	StyleStayOnTop->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GUIOPolyglotSetup::OnChangeStayOnTop ), NULL, this );
	LogLevel->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSetup::OnChangeLogLevel ), NULL, this );
}

GUIOPolyglotSetup::~GUIOPolyglotSetup()
{
}

GUIOPolyglotProgressInstallLanguage::GUIOPolyglotProgressInstallLanguage( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainBox = new wxBoxSizer( wxVERTICAL );

	HBox1 = new wxBoxSizer( wxHORIZONTAL );

	Labeltimeelapsed = new wxStaticText( this, wxID_ANY, _("The time elapsed"), wxDefaultPosition, wxDefaultSize, 0 );
	Labeltimeelapsed->Wrap( -1 );
	HBox1->Add( Labeltimeelapsed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox1->Add( 0, 0, 1, wxEXPAND, 5 );

	TimeElapsed = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	TimeElapsed->Wrap( -1 );
	TimeElapsed->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	HBox1->Add( TimeElapsed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox1, 1, wxEXPAND, 5 );

	HBox2 = new wxBoxSizer( wxHORIZONTAL );

	Labeltimeremaining = new wxStaticText( this, wxID_ANY, _("Time remaining"), wxDefaultPosition, wxDefaultSize, 0 );
	Labeltimeremaining->Wrap( -1 );
	HBox2->Add( Labeltimeremaining, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox2->Add( 0, 0, 1, wxEXPAND, 5 );

	TimeRemaining = new wxStaticText( this, wxID_ANY, _("∞"), wxDefaultPosition, wxDefaultSize, 0 );
	TimeRemaining->Wrap( -1 );
	TimeRemaining->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	HBox2->Add( TimeRemaining, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox2, 1, wxEXPAND, 5 );

	HBox3_1 = new wxBoxSizer( wxHORIZONTAL );

	LabelAllProgress = new wxStaticText( this, wxID_ANY, _("All progress downloaded"), wxDefaultPosition, wxDefaultSize, 0 );
	LabelAllProgress->Wrap( -1 );
	HBox3_1->Add( LabelAllProgress, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox3_1->Add( 0, 0, 1, wxEXPAND, 5 );

	SizeAll = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	SizeAll->Wrap( -1 );
	SizeAll->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	HBox3_1->Add( SizeAll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox3_1, 1, wxEXPAND, 0 );

	AllProgress = new wxGauge( this, wxID_ANY, 1000, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	AllProgress->SetValue( 0 );
	MainBox->Add( AllProgress, 0, wxALL|wxEXPAND, 5 );

	HBox3 = new wxBoxSizer( wxHORIZONTAL );

	Labelspeed = new wxStaticText( this, wxID_ANY, _("Speed"), wxDefaultPosition, wxDefaultSize, 0 );
	Labelspeed->Wrap( -1 );
	HBox3->Add( Labelspeed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox3->Add( 0, 0, 1, wxEXPAND, 5 );

	Speed = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	Speed->Wrap( -1 );
	Speed->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	HBox3->Add( Speed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox3, 1, wxEXPAND, 0 );

	HBox3_2 = new wxBoxSizer( wxHORIZONTAL );

	LabelFileProgress = new wxStaticText( this, wxID_ANY, _("File progress"), wxDefaultPosition, wxDefaultSize, 0 );
	LabelFileProgress->Wrap( -1 );
	HBox3_2->Add( LabelFileProgress, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox3_2->Add( 0, 0, 1, wxEXPAND, 5 );

	SizeFile = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	SizeFile->Wrap( -1 );
	SizeFile->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	HBox3_2->Add( SizeFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox3_2, 1, wxEXPAND, 5 );

	FileProgress = new wxGauge( this, wxID_ANY, 1000, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	FileProgress->SetValue( 0 );
	MainBox->Add( FileProgress, 0, wxALL|wxEXPAND, 5 );

	HBox4 = new wxBoxSizer( wxHORIZONTAL );


	HBox4->Add( 0, 0, 1, wxEXPAND, 5 );

	ButtonCancel = new wxButton( this, wxID_ANY, _("Cancel install"), wxDefaultPosition, wxDefaultSize, 0 );
	HBox4->Add( ButtonCancel, 0, wxALL, 5 );


	MainBox->Add( HBox4, 1, wxEXPAND, 5 );


	this->SetSizer( MainBox );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotProgressInstallLanguage::OnClose ) );
	ButtonCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotProgressInstallLanguage::OnCancel ), NULL, this );
}

GUIOPolyglotProgressInstallLanguage::~GUIOPolyglotProgressInstallLanguage()
{
}

GUIOPolyglotEditorRule::GUIOPolyglotEditorRule( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainBox = new wxBoxSizer( wxVERTICAL );

	NumberRule = new wxStaticText( this, wxID_ANY, _("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
	NumberRule->Wrap( -1 );
	MainBox->Add( NumberRule, 0, wxALL, 5 );

	Comment = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	MainBox->Add( Comment, 0, wxALL|wxEXPAND, 5 );

	m_staticText21 = new wxStaticText( this, wxID_ANY, _("Regular expression:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText21->Wrap( -1 );
	MainBox->Add( m_staticText21, 0, wxALL, 5 );

	RegEx = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	MainBox->Add( RegEx, 0, wxALL|wxEXPAND, 5 );

	m_staticText22 = new wxStaticText( this, wxID_ANY, _("Replacement rule:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText22->Wrap( -1 );
	MainBox->Add( m_staticText22, 0, wxALL, 5 );

	ReplacementRule = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	MainBox->Add( ReplacementRule, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* HBox1;
	HBox1 = new wxBoxSizer( wxHORIZONTAL );

	Test = new wxButton( this, wxID_ANY, _("Test"), wxDefaultPosition, wxDefaultSize, 0 );
	HBox1->Add( Test, 0, wxALL, 5 );


	HBox1->Add( 0, 0, 1, wxEXPAND, 5 );

	Save = new wxButton( this, wxID_ANY, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	HBox1->Add( Save, 0, wxALL, 5 );

	Cancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	HBox1->Add( Cancel, 0, wxALL, 5 );


	MainBox->Add( HBox1, 0, wxEXPAND, 5 );


	this->SetSizer( MainBox );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotEditorRule::OnClose ) );
	Test->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotEditorRule::OnTest ), NULL, this );
	Save->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotEditorRule::OnSave ), NULL, this );
	Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotEditorRule::OnCancel ), NULL, this );
}

GUIOPolyglotEditorRule::~GUIOPolyglotEditorRule()
{
}

GUIOPolyglotMultilineText::GUIOPolyglotMultilineText( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainBox = new wxBoxSizer( wxVERTICAL );

	Value = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	MainBox->Add( Value, 1, wxALL|wxEXPAND, 5 );

	ValueRO = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	ValueRO->Hide();

	MainBox->Add( ValueRO, 1, wxALL|wxEXPAND, 5 );

	VBox = new wxBoxSizer( wxHORIZONTAL );


	VBox->Add( 0, 0, 1, wxEXPAND, 5 );

	Ok = new wxButton( this, wxID_ANY, _("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
	VBox->Add( Ok, 0, wxALL, 5 );

	Cancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	VBox->Add( Cancel, 0, wxALL, 5 );


	MainBox->Add( VBox, 0, wxEXPAND, 5 );


	this->SetSizer( MainBox );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotMultilineText::OnClose ) );
	Ok->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotMultilineText::OnOk ), NULL, this );
	Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotMultilineText::OnCancel ), NULL, this );
}

GUIOPolyglotMultilineText::~GUIOPolyglotMultilineText()
{
}

GUIOPolyglotListRules::GUIOPolyglotListRules( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainBox = new wxBoxSizer( wxVERTICAL );

	ListRules = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_EDIT_LABELS|wxLC_REPORT );
	MainBox->Add( ListRules, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* HBox1;
	HBox1 = new wxBoxSizer( wxHORIZONTAL );

	Add = new wxButton( this, wxID_ANY, _("Add rule"), wxDefaultPosition, wxDefaultSize, 0 );
	HBox1->Add( Add, 0, wxALL, 5 );

	Test = new wxButton( this, wxID_ANY, _("Test all rules"), wxDefaultPosition, wxDefaultSize, 0 );
	HBox1->Add( Test, 0, wxALL, 5 );


	HBox1->Add( 0, 0, 1, wxEXPAND, 5 );

	Save = new wxButton( this, wxID_ANY, _("Save changes"), wxDefaultPosition, wxDefaultSize, 0 );
	Save->Enable( false );

	HBox1->Add( Save, 0, wxALL, 5 );


	MainBox->Add( HBox1, 0, wxEXPAND, 5 );


	this->SetSizer( MainBox );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotListRules::OnClose ) );
	ListRules->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( GUIOPolyglotListRules::OnSelectItem ), NULL, this );
	ListRules->Connect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, wxListEventHandler( GUIOPolyglotListRules::OnListRulesMenu ), NULL, this );
	Add->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotListRules::OnAdd ), NULL, this );
	Test->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotListRules::OnTest ), NULL, this );
	Save->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotListRules::OnSave ), NULL, this );
}

GUIOPolyglotListRules::~GUIOPolyglotListRules()
{
}

GUIAbout::GUIAbout( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* Sizer;
	Sizer = new wxBoxSizer( wxVERTICAL );

	Info = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	Sizer->Add( Info, 1, wxALL|wxEXPAND, 5 );


	this->SetSizer( Sizer );
	this->Layout();

	this->Centre( wxBOTH );
}

GUIAbout::~GUIAbout()
{
}
