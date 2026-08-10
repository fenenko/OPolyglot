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

	h_box1 = new wxBoxSizer( wxHORIZONTAL );

	panelMain = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer( wxHORIZONTAL );

	wxString LanguageFromChoices[] = { _("ADD LANGUAGE") };
	int LanguageFromNChoices = sizeof( LanguageFromChoices ) / sizeof( wxString );
	LanguageFrom = new wxChoice( panelMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, LanguageFromNChoices, LanguageFromChoices, 0 );
	LanguageFrom->SetSelection( 0 );
	LanguageFrom->SetToolTip( _("Source language") );

	bSizer38->Add( LanguageFrom, 1, wxALL, 5 );

	labelDirect = new wxStaticText( panelMain, wxID_ANY, _("->"), wxDefaultPosition, wxDefaultSize, 0 );
	labelDirect->Wrap( -1 );
	bSizer38->Add( labelDirect, 0, wxALIGN_CENTER|wxALL, 0 );

	wxString LanguageToChoices[] = { _("ADD LANGUAGE") };
	int LanguageToNChoices = sizeof( LanguageToChoices ) / sizeof( wxString );
	LanguageTo = new wxChoice( panelMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, LanguageToNChoices, LanguageToChoices, 0 );
	LanguageTo->SetSelection( 0 );
	LanguageTo->SetToolTip( _("Target language") );

	bSizer38->Add( LanguageTo, 1, wxALL, 5 );


	bSizer38->Add( 0, 0, 2, wxALL|wxEXPAND, 5 );

	buttonShowTranslator = new wxButton( panelMain, wxID_ANY, _("Open Translator"), wxDefaultPosition, wxDefaultSize, 0 );
	buttonShowTranslator->SetToolTip( _("Open text translator") );

	bSizer38->Add( buttonShowTranslator, 0, wxALL, 5 );

	m_staticline5 = new wxStaticLine( panelMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	bSizer38->Add( m_staticline5, 0, wxEXPAND | wxALL, 5 );

	buttonViewResult = new wxButton( panelMain, wxID_ANY, _("Show Translation"), wxDefaultPosition, wxDefaultSize, 0 );
	buttonViewResult->SetToolTip( _("View screen translation results") );

	bSizer38->Add( buttonViewResult, 0, wxALL, 5 );

	buttonCaptureScreen = new wxButton( panelMain, wxID_ANY, _("Screen Translator"), wxDefaultPosition, wxDefaultSize, 0 );
	buttonCaptureScreen->SetToolTip( _("Translate selected screen areas") );

	bSizer38->Add( buttonCaptureScreen, 0, wxALL, 5 );

	buttonOpenDocument = new wxButton( panelMain, wxID_ANY, _("Document Translator"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer38->Add( buttonOpenDocument, 0, wxALL, 5 );


	panelMain->SetSizer( bSizer38 );
	panelMain->Layout();
	bSizer38->Fit( panelMain );
	h_box1->Add( panelMain, 1, wxEXPAND | wxALL, 0 );


	MainVBox->Add( h_box1, 0, wxALL|wxEXPAND, 0 );


	this->SetSizer( MainVBox );
	this->Layout();
	menuBar = new wxMenuBar( 0 );
	menuSettings = new wxMenu();
	wxMenuItem* menuSetup;
	menuSetup = new wxMenuItem( menuSettings, wxID_ANY, wxString( _("Settings") ) , wxEmptyString, wxITEM_NORMAL );
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
	buttonShowTranslator->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnOpenTranslator ), NULL, this );
	buttonViewResult->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnShowTranslation ), NULL, this );
	buttonCaptureScreen->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnCaptureScreen ), NULL, this );
	buttonOpenDocument->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GuiOPolyglot::OnDocumentTranslator ), NULL, this );
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
}

GUIFullscreen::~GUIFullscreen()
{
}

GUIOPolyglotDownloadLanguage::GUIOPolyglotDownloadLanguage( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	v_box = new wxBoxSizer( wxVERTICAL );

	ListLanguages = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME|wxVSCROLL );
	ListLanguages->SetScrollRate( 5, 5 );
	ListLanguages->SetFont( wxFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	box = new wxBoxSizer( wxVERTICAL );


	ListLanguages->SetSizer( box );
	ListLanguages->Layout();
	box->Fit( ListLanguages );
	v_box->Add( ListLanguages, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( v_box );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotDownloadLanguage::OnClose ) );
}

GUIOPolyglotDownloadLanguage::~GUIOPolyglotDownloadLanguage()
{
}

GUIOPolyglotDialogProgress::GUIOPolyglotDialogProgress( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	vBox = new wxBoxSizer( wxVERTICAL );

	ProgressLabel = new wxStaticText( this, wxID_ANY, _("progress"), wxDefaultPosition, wxDefaultSize, 0 );
	ProgressLabel->Wrap( -1 );
	vBox->Add( ProgressLabel, 0, wxALL, 5 );

	Progress = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	Progress->SetValue( 0 );
	vBox->Add( Progress, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );


	bSizer29->Add( 0, 0, 1, wxEXPAND, 5 );

	Cancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	Cancel->Hide();

	bSizer29->Add( Cancel, 0, wxALL, 5 );


	vBox->Add( bSizer29, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( vBox );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotDialogProgress::OnCancel ), NULL, this );
}

GUIOPolyglotDialogProgress::~GUIOPolyglotDialogProgress()
{
}

GUIOPolyglotSettings::GUIOPolyglotSettings( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainBox = new wxBoxSizer( wxVERTICAL );

	HBox0 = new wxBoxSizer( wxHORIZONTAL );

	LabelInterface = new wxStaticText( this, wxID_ANY, _("Select the interface language."), wxDefaultPosition, wxDefaultSize, 0 );
	LabelInterface->Wrap( -1 );
	HBox0->Add( LabelInterface, 0, wxALIGN_CENTER|wxALL, 5 );


	HBox0->Add( 0, 0, 1, wxEXPAND, 5 );

	wxArrayString SelectInterfaceLanguageChoices;
	SelectInterfaceLanguage = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, SelectInterfaceLanguageChoices, 0 );
	SelectInterfaceLanguage->SetSelection( 0 );
	HBox0->Add( SelectInterfaceLanguage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox0, 0, wxEXPAND, 5 );

	m_staticline4 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	MainBox->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );

	ListLanguages = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME|wxVSCROLL );
	ListLanguages->SetScrollRate( 5, 5 );
	boxLanguages = new wxBoxSizer( wxVERTICAL );


	ListLanguages->SetSizer( boxLanguages );
	ListLanguages->Layout();
	boxLanguages->Fit( ListLanguages );
	MainBox->Add( ListLanguages, 1, wxEXPAND | wxALL, 5 );

	HBox3 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText25 = new wxStaticText( this, wxID_ANY, _("Additional OCR languages"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText25->Wrap( -1 );
	HBox3->Add( m_staticText25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox3->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString additionalLanguageOCRChoices[] = { _("NONE") };
	int additionalLanguageOCRNChoices = sizeof( additionalLanguageOCRChoices ) / sizeof( wxString );
	additionalLanguageOCR = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, additionalLanguageOCRNChoices, additionalLanguageOCRChoices, 0 );
	additionalLanguageOCR->SetSelection( 0 );
	HBox3->Add( additionalLanguageOCR, 0, wxALL, 5 );


	MainBox->Add( HBox3, 0, wxEXPAND, 5 );

	HBox2 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText81 = new wxStaticText( this, wxID_ANY, _("Preferred OCR method"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	HBox2->Add( m_staticText81, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox2->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString MethodOCRChoices[] = { _("BEST"), _("FAST") };
	int MethodOCRNChoices = sizeof( MethodOCRChoices ) / sizeof( wxString );
	MethodOCR = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, MethodOCRNChoices, MethodOCRChoices, 0 );
	MethodOCR->SetSelection( 0 );
	HBox2->Add( MethodOCR, 0, wxALL, 5 );


	MainBox->Add( HBox2, 0, wxEXPAND, 5 );

	HBox1 = new wxBoxSizer( wxHORIZONTAL );

	labelTypeMethodTranslate = new wxStaticText( this, wxID_ANY, _("Preferred translation method"), wxDefaultPosition, wxDefaultSize, 0 );
	labelTypeMethodTranslate->Wrap( -1 );
	HBox1->Add( labelTypeMethodTranslate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox1->Add( 0, 0, 1, wxEXPAND, 5 );

	wxString MethodTranslationChoices[] = { _("BEST"), _("FAST") };
	int MethodTranslationNChoices = sizeof( MethodTranslationChoices ) / sizeof( wxString );
	MethodTranslation = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, MethodTranslationNChoices, MethodTranslationChoices, 0 );
	MethodTranslation->SetSelection( 0 );
	HBox1->Add( MethodTranslation, 0, wxALL, 5 );


	MainBox->Add( HBox1, 0, wxEXPAND, 5 );

	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	MainBox->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );

	HBoxSauvola = new wxBoxSizer( wxHORIZONTAL );

	m_staticText251 = new wxStaticText( this, wxID_ANY, _("Minimum threshold for image binarization (1 - 255)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText251->Wrap( -1 );
	HBoxSauvola->Add( m_staticText251, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBoxSauvola->Add( 0, 0, 1, wxEXPAND, 5 );

	sauvolaMindiff = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 255, 15 );
	sauvolaMindiff->SetToolTip( _("(mindiff) from pixSauvolaOnContrastNorm") );

	HBoxSauvola->Add( sauvolaMindiff, 0, wxALL, 5 );

	sauvolaEnabled = new wxCheckBox( this, wxID_ANY, _("Sauvola"), wxDefaultPosition, wxDefaultSize, 0 );
	sauvolaEnabled->Enable( false );
	sauvolaEnabled->Hide();

	HBoxSauvola->Add( sauvolaEnabled, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBoxSauvola, 0, wxEXPAND, 5 );

	HBox4 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText18 = new wxStaticText( this, wxID_ANY, _("Enable text pre-processing before translation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	HBox4->Add( m_staticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox4->Add( 0, 0, 1, wxEXPAND, 5 );

	RulesPreprocessing = new wxButton( this, wxID_ANY, _("Rules"), wxDefaultPosition, wxDefaultSize, 0 );
	HBox4->Add( RulesPreprocessing, 0, wxALL, 5 );

	EnablePreprocessing = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	EnablePreprocessing->SetValue(true);
	HBox4->Add( EnablePreprocessing, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox4, 0, wxEXPAND, 5 );

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

	m_staticText9 = new wxStaticText( this, wxID_ANY, _("Always on top"), wxDefaultPosition, wxDefaultSize, 0 );
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

	wxString LogLevelChoices[] = { _("MESSAGE"), _("WARNING"), _("ERROR") };
	int LogLevelNChoices = sizeof( LogLevelChoices ) / sizeof( wxString );
	LogLevel = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LogLevelNChoices, LogLevelChoices, 0 );
	LogLevel->SetSelection( 0 );
	HBox7->Add( LogLevel, 0, wxALL, 5 );

	ViewLog = new wxButton( this, wxID_ANY, _("View log"), wxDefaultPosition, wxDefaultSize, 0 );
	ViewLog->SetToolTip( _("Open log file") );

	HBox7->Add( ViewLog, 0, wxALL, 5 );


	MainBox->Add( HBox7, 0, wxEXPAND, 5 );


	this->SetSizer( MainBox );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotSettings::OnClose ) );
	SelectInterfaceLanguage->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSettings::OnSelectInterfaceLanguage ), NULL, this );
	additionalLanguageOCR->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSettings::OnAdditionalLanguage ), NULL, this );
	MethodOCR->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSettings::OnSelectMethodOCR ), NULL, this );
	MethodTranslation->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSettings::OnSelectMethodTranslation ), NULL, this );
	sauvolaMindiff->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GUIOPolyglotSettings::OnSauvolaMindiff ), NULL, this );
	sauvolaEnabled->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GUIOPolyglotSettings::OnSauvolaEnabled ), NULL, this );
	RulesPreprocessing->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotSettings::OnRulesPreprocessing ), NULL, this );
	EnablePreprocessing->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GUIOPolyglotSettings::OnEnablePreprocessing ), NULL, this );
	RulesPostprocessing->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotSettings::OnRulesPostprocessing ), NULL, this );
	EnablePostprocessing->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GUIOPolyglotSettings::OnEnablePostprocessing ), NULL, this );
	StyleStayOnTop->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GUIOPolyglotSettings::OnChangeStayOnTop ), NULL, this );
	LogLevel->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotSettings::OnChangeLogLevel ), NULL, this );
	ViewLog->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotSettings::OnViewLog ), NULL, this );
}

GUIOPolyglotSettings::~GUIOPolyglotSettings()
{
}

GUIOPolyglotInstallLanguages::GUIOPolyglotInstallLanguages( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainBox = new wxBoxSizer( wxVERTICAL );

	HBox1 = new wxBoxSizer( wxHORIZONTAL );

	Labeltimeelapsed = new wxStaticText( this, wxID_ANY, _("Time elapsed"), wxDefaultPosition, wxDefaultSize, 0 );
	Labeltimeelapsed->Wrap( -1 );
	HBox1->Add( Labeltimeelapsed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox1->Add( 0, 0, 1, wxEXPAND, 5 );

	TimeElapsed = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	TimeElapsed->Wrap( -1 );
	TimeElapsed->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	HBox1->Add( TimeElapsed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox1, 0, wxEXPAND, 5 );

	HBox2 = new wxBoxSizer( wxHORIZONTAL );

	Labeltimeremaining = new wxStaticText( this, wxID_ANY, _("Time remaining"), wxDefaultPosition, wxDefaultSize, 0 );
	Labeltimeremaining->Wrap( -1 );
	HBox2->Add( Labeltimeremaining, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox2->Add( 0, 0, 1, wxEXPAND, 5 );

	TimeRemaining = new wxStaticText( this, wxID_ANY, _("∞"), wxDefaultPosition, wxDefaultSize, 0 );
	TimeRemaining->Wrap( -1 );
	TimeRemaining->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	HBox2->Add( TimeRemaining, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox2, 0, wxEXPAND, 5 );

	HBox3 = new wxBoxSizer( wxHORIZONTAL );

	Labelspeed = new wxStaticText( this, wxID_ANY, _("Speed"), wxDefaultPosition, wxDefaultSize, 0 );
	Labelspeed->Wrap( -1 );
	HBox3->Add( Labelspeed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox3->Add( 0, 0, 1, wxEXPAND, 5 );

	Speed = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	Speed->Wrap( -1 );
	Speed->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	HBox3->Add( Speed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox3, 0, wxEXPAND, 0 );

	HBox3_2 = new wxBoxSizer( wxHORIZONTAL );

	LabelFileProgress = new wxStaticText( this, wxID_ANY, _("File progress"), wxDefaultPosition, wxDefaultSize, 0 );
	LabelFileProgress->Wrap( -1 );
	HBox3_2->Add( LabelFileProgress, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox3_2->Add( 0, 0, 1, wxEXPAND, 5 );

	SizeFile = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	SizeFile->Wrap( -1 );
	SizeFile->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	HBox3_2->Add( SizeFile, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox3_2, 0, wxEXPAND, 5 );

	FileProgress = new wxGauge( this, wxID_ANY, 1000, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	FileProgress->SetValue( 0 );
	FileProgress->SetToolTip( _("file progress") );

	MainBox->Add( FileProgress, 0, wxALL|wxEXPAND, 5 );

	HBox3_1 = new wxBoxSizer( wxHORIZONTAL );

	LabelAllProgress = new wxStaticText( this, wxID_ANY, _("Remaining to download."), wxDefaultPosition, wxDefaultSize, 0 );
	LabelAllProgress->Wrap( -1 );
	HBox3_1->Add( LabelAllProgress, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	HBox3_1->Add( 0, 0, 1, wxEXPAND, 5 );

	SizeAll = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	SizeAll->Wrap( -1 );
	SizeAll->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	HBox3_1->Add( SizeAll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	MainBox->Add( HBox3_1, 0, wxEXPAND, 0 );

	AllProgress = new wxGauge( this, wxID_ANY, 1000, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	AllProgress->SetValue( 0 );
	AllProgress->SetToolTip( _("Total progress") );

	MainBox->Add( AllProgress, 0, wxALL|wxEXPAND, 5 );

	HBox4 = new wxBoxSizer( wxHORIZONTAL );


	HBox4->Add( 0, 0, 1, wxEXPAND, 5 );

	ButtonCancel = new wxButton( this, wxID_ANY, _("Cancel install"), wxDefaultPosition, wxDefaultSize, 0 );
	HBox4->Add( ButtonCancel, 0, wxALL, 5 );


	MainBox->Add( HBox4, 1, wxEXPAND, 5 );


	this->SetSizer( MainBox );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotInstallLanguages::OnClose ) );
	ButtonCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotInstallLanguages::OnCancel ), NULL, this );
}

GUIOPolyglotInstallLanguages::~GUIOPolyglotInstallLanguages()
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

	Ok = new wxButton( this, wxID_ANY, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
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

	labelOpolyglot = new wxStaticText( this, wxID_ANY, _("Opolyglot version dev"), wxDefaultPosition, wxDefaultSize, 0 );
	labelOpolyglot->Wrap( -1 );
	labelOpolyglot->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

	Sizer->Add( labelOpolyglot, 0, wxALL|wxEXPAND, 5 );

	data = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	panelReadme = new wxPanel( data, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxVERTICAL );

	readmeOpolyglot = new wxHtmlWindow( panelReadme, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	bSizer48->Add( readmeOpolyglot, 1, wxALL|wxEXPAND, 5 );


	panelReadme->SetSizer( bSizer48 );
	panelReadme->Layout();
	bSizer48->Fit( panelReadme );
	data->AddPage( panelReadme, _("Readme"), false );
	panelLicenses = new wxPanel( data, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer47;
	bSizer47 = new wxBoxSizer( wxVERTICAL );

	licensesOpolyglot = new wxTextCtrl( panelLicenses, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	licensesOpolyglot->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

	bSizer47->Add( licensesOpolyglot, 3, wxALL|wxEXPAND, 5 );


	panelLicenses->SetSizer( bSizer47 );
	panelLicenses->Layout();
	bSizer47->Fit( panelLicenses );
	data->AddPage( panelLicenses, _("Licenses"), false );

	Sizer->Add( data, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( Sizer );
	this->Layout();

	this->Centre( wxBOTH );
}

GUIAbout::~GUIAbout()
{
}

GUIViewLog::GUIViewLog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxVERTICAL );

	Log = new wxStyledTextCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxEmptyString );
	Log->SetUseTabs( true );
	Log->SetTabWidth( 9 );
	Log->SetIndent( 9 );
	Log->SetTabIndents( true );
	Log->SetBackSpaceUnIndents( true );
	Log->SetViewEOL( false );
	Log->SetViewWhiteSpace( false );
	Log->SetMarginWidth( 2, 0 );
	Log->SetIndentationGuides( true );
	Log->SetReadOnly( false );
	Log->SetMarginType( 1, wxSTC_MARGIN_SYMBOL );
	Log->SetMarginMask( 1, wxSTC_MASK_FOLDERS );
	Log->SetMarginWidth( 1, 16);
	Log->SetMarginSensitive( 1, true );
	Log->SetProperty( wxT("fold"), wxT("1") );
	Log->SetFoldFlags( wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED );
	Log->SetMarginType( 0, wxSTC_MARGIN_NUMBER );
	Log->SetMarginWidth( 0, Log->TextWidth( wxSTC_STYLE_LINENUMBER, wxT("_99999") ) );
	Log->MarkerDefine( wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS );
	Log->MarkerSetBackground( wxSTC_MARKNUM_FOLDER, wxColour( wxT("BLACK") ) );
	Log->MarkerSetForeground( wxSTC_MARKNUM_FOLDER, wxColour( wxT("WHITE") ) );
	Log->MarkerDefine( wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS );
	Log->MarkerSetBackground( wxSTC_MARKNUM_FOLDEROPEN, wxColour( wxT("BLACK") ) );
	Log->MarkerSetForeground( wxSTC_MARKNUM_FOLDEROPEN, wxColour( wxT("WHITE") ) );
	Log->MarkerDefine( wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY );
	Log->MarkerDefine( wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS );
	Log->MarkerSetBackground( wxSTC_MARKNUM_FOLDEREND, wxColour( wxT("BLACK") ) );
	Log->MarkerSetForeground( wxSTC_MARKNUM_FOLDEREND, wxColour( wxT("WHITE") ) );
	Log->MarkerDefine( wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS );
	Log->MarkerSetBackground( wxSTC_MARKNUM_FOLDEROPENMID, wxColour( wxT("BLACK") ) );
	Log->MarkerSetForeground( wxSTC_MARKNUM_FOLDEROPENMID, wxColour( wxT("WHITE") ) );
	Log->MarkerDefine( wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY );
	Log->MarkerDefine( wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY );
	Log->SetSelBackground( true, wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	Log->SetSelForeground( true, wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT ) );
	bSizer32->Add( Log, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( bSizer32 );
	this->Layout();

	this->Centre( wxBOTH );
}

GUIViewLog::~GUIViewLog()
{
}

GUIOPolyglotViewTextTranslate::GUIOPolyglotViewTextTranslate( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxHORIZONTAL );

	buttonClear = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	bSizer34->Add( buttonClear, 0, wxALL, 5 );

	buttonCopy = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( -1,-1 ), wxBU_AUTODRAW|0 );
	buttonCopy->Enable( false );
	buttonCopy->SetToolTip( _("copy the entire document to the clipboard") );

	bSizer34->Add( buttonCopy, 0, wxALL, 5 );


	bSizer33->Add( bSizer34, 0, wxEXPAND, 5 );

	textTranslate = new wxStyledTextCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxEmptyString );
	textTranslate->SetUseTabs( true );
	textTranslate->SetTabWidth( 4 );
	textTranslate->SetIndent( 4 );
	textTranslate->SetTabIndents( true );
	textTranslate->SetBackSpaceUnIndents( true );
	textTranslate->SetViewEOL( false );
	textTranslate->SetViewWhiteSpace( false );
	textTranslate->SetMarginWidth( 2, 0 );
	textTranslate->SetIndentationGuides( true );
	textTranslate->SetReadOnly( false );
	textTranslate->SetMarginType( 1, wxSTC_MARGIN_SYMBOL );
	textTranslate->SetMarginMask( 1, wxSTC_MASK_FOLDERS );
	textTranslate->SetMarginWidth( 1, 16);
	textTranslate->SetMarginSensitive( 1, true );
	textTranslate->SetProperty( wxT("fold"), wxT("1") );
	textTranslate->SetFoldFlags( wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED );
	textTranslate->SetMarginType( 0, wxSTC_MARGIN_NUMBER );
	textTranslate->SetMarginWidth( 0, textTranslate->TextWidth( wxSTC_STYLE_LINENUMBER, wxT("_99999") ) );
	textTranslate->MarkerDefine( wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS );
	textTranslate->MarkerSetBackground( wxSTC_MARKNUM_FOLDER, wxColour( wxT("BLACK") ) );
	textTranslate->MarkerSetForeground( wxSTC_MARKNUM_FOLDER, wxColour( wxT("WHITE") ) );
	textTranslate->MarkerDefine( wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS );
	textTranslate->MarkerSetBackground( wxSTC_MARKNUM_FOLDEROPEN, wxColour( wxT("BLACK") ) );
	textTranslate->MarkerSetForeground( wxSTC_MARKNUM_FOLDEROPEN, wxColour( wxT("WHITE") ) );
	textTranslate->MarkerDefine( wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY );
	textTranslate->MarkerDefine( wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS );
	textTranslate->MarkerSetBackground( wxSTC_MARKNUM_FOLDEREND, wxColour( wxT("BLACK") ) );
	textTranslate->MarkerSetForeground( wxSTC_MARKNUM_FOLDEREND, wxColour( wxT("WHITE") ) );
	textTranslate->MarkerDefine( wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS );
	textTranslate->MarkerSetBackground( wxSTC_MARKNUM_FOLDEROPENMID, wxColour( wxT("BLACK") ) );
	textTranslate->MarkerSetForeground( wxSTC_MARKNUM_FOLDEROPENMID, wxColour( wxT("WHITE") ) );
	textTranslate->MarkerDefine( wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY );
	textTranslate->MarkerDefine( wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY );
	textTranslate->SetSelBackground( true, wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	textTranslate->SetSelForeground( true, wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT ) );
	bSizer33->Add( textTranslate, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( bSizer33 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotViewTextTranslate::OnClose ) );
	buttonClear->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotViewTextTranslate::OnClear ), NULL, this );
	buttonCopy->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotViewTextTranslate::OnCopy ), NULL, this );
}

GUIOPolyglotViewTextTranslate::~GUIOPolyglotViewTextTranslate()
{
}

GUIOPolyglotTranslator::GUIOPolyglotTranslator( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxVERTICAL );

	wxArrayString LanguageFromChoices;
	LanguageFrom = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LanguageFromChoices, 0 );
	LanguageFrom->SetSelection( 0 );
	bSizer33->Add( LanguageFrom, 0, wxALL, 5 );

	textOriginal = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	bSizer33->Add( textOriginal, 1, wxALL|wxEXPAND, 5 );


	bSizer32->Add( bSizer33, 1, wxALL|wxEXPAND, 0 );

	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxVERTICAL );

	buttonRechange = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	bSizer35->Add( buttonRechange, 0, wxALL, 5 );


	bSizer32->Add( bSizer35, 0, wxALL|wxEXPAND, 0 );

	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer37;
	bSizer37 = new wxBoxSizer( wxHORIZONTAL );

	wxArrayString LanguageToChoices;
	LanguageTo = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LanguageToChoices, 0 );
	LanguageTo->SetSelection( 0 );
	bSizer37->Add( LanguageTo, 0, wxALL, 5 );


	bSizer37->Add( 0, 0, 1, wxEXPAND, 5 );

	buttonCopy = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
	bSizer37->Add( buttonCopy, 0, wxALL, 5 );


	bSizer36->Add( bSizer37, 0, wxEXPAND, 5 );

	textTranslate = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	bSizer36->Add( textTranslate, 1, wxALL|wxEXPAND, 5 );


	bSizer32->Add( bSizer36, 1, wxALL|wxEXPAND, 0 );


	this->SetSizer( bSizer32 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOPolyglotTranslator::OnClose ) );
	LanguageFrom->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotTranslator::OnLanguageFrom ), NULL, this );
	textOriginal->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GUIOPolyglotTranslator::OnTextSource ), NULL, this );
	buttonRechange->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotTranslator::OnRechange ), NULL, this );
	LanguageTo->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotTranslator::OnLanguageTo ), NULL, this );
	buttonCopy->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotTranslator::OnCopy ), NULL, this );
}

GUIOPolyglotTranslator::~GUIOPolyglotTranslator()
{
}

GUIOpolyglotEditTranslating::GUIOpolyglotEditTranslating( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* mainBox;
	mainBox = new wxBoxSizer( wxVERTICAL );

	vBox1 = new wxBoxSizer( wxVERTICAL );

	hBox1_1 = new wxBoxSizer( wxHORIZONTAL );

	imageView = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	hBox1_1->Add( imageView, 1, wxEXPAND | wxALL, 5 );

	vScroll = new wxScrollBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL );
	hBox1_1->Add( vScroll, 0, wxALL|wxEXPAND, 5 );


	vBox1->Add( hBox1_1, 1, wxEXPAND, 5 );

	hBox1_2 = new wxBoxSizer( wxHORIZONTAL );

	hScroll = new wxScrollBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL );
	hBox1_2->Add( hScroll, 1, wxALL, 5 );


	vBox1->Add( hBox1_2, 0, wxEXPAND, 0 );

	m_staticline3 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	vBox1->Add( m_staticline3, 0, wxEXPAND | wxALL, 5 );


	mainBox->Add( vBox1, 1, wxEXPAND, 5 );

	wxBoxSizer* hBox2;
	hBox2 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText26 = new wxStaticText( this, wxID_ANY, _("OCR text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText26->Wrap( -1 );
	hBox2->Add( m_staticText26, 0, wxALL, 5 );


	hBox2->Add( 0, 0, 1, wxEXPAND, 5 );

	Translate = new wxButton( this, wxID_ANY, _("Save and Translate"), wxDefaultPosition, wxDefaultSize, 0 );
	Translate->Enable( false );

	hBox2->Add( Translate, 0, wxALL, 5 );


	mainBox->Add( hBox2, 0, wxEXPAND, 5 );

	textOCR = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	mainBox->Add( textOCR, 1, wxALL|wxEXPAND, 5 );

	m_staticline4 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	mainBox->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );

	wxBoxSizer* hBox3;
	hBox3 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText27 = new wxStaticText( this, wxID_ANY, _("Translation text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText27->Wrap( -1 );
	hBox3->Add( m_staticText27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	hBox3->Add( 0, 0, 1, wxEXPAND, 5 );

	Save = new wxButton( this, wxID_ANY, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	Save->Enable( false );

	hBox3->Add( Save, 0, wxALL, 5 );


	mainBox->Add( hBox3, 0, wxEXPAND, 5 );

	textTranslate = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	mainBox->Add( textTranslate, 1, wxALL|wxEXPAND, 5 );


	this->SetSizer( mainBox );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIOpolyglotEditTranslating::OnClose ) );
	vScroll->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnVScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GUIOpolyglotEditTranslating::OnHScroll ), NULL, this );
	Translate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOpolyglotEditTranslating::OnSaveAndTranslating ), NULL, this );
	Save->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOpolyglotEditTranslating::OnSave ), NULL, this );
	textTranslate->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GUIOpolyglotEditTranslating::OnTextTranslate ), NULL, this );
}

GUIOpolyglotEditTranslating::~GUIOpolyglotEditTranslating()
{
}

GUIOPolyglotDocumentView::GUIOPolyglotDocumentView( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	vBox1 = new wxBoxSizer( wxVERTICAL );

	m_toolBar1 = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL );
	wxString LanguageFromChoices[] = { _("ADD LANGUAGE") };
	int LanguageFromNChoices = sizeof( LanguageFromChoices ) / sizeof( wxString );
	LanguageFrom = new wxChoice( m_toolBar1, wxID_ANY, wxDefaultPosition, wxDefaultSize, LanguageFromNChoices, LanguageFromChoices, 0 );
	LanguageFrom->SetSelection( 0 );
	m_toolBar1->AddControl( LanguageFrom );
	m_staticText28 = new wxStaticText( m_toolBar1, wxID_ANY, _("->"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText28->Wrap( -1 );
	m_toolBar1->AddControl( m_staticText28 );
	wxString LanguageToChoices[] = { _("ADD LANGUAGE") };
	int LanguageToNChoices = sizeof( LanguageToChoices ) / sizeof( wxString );
	LanguageTo = new wxChoice( m_toolBar1, wxID_ANY, wxDefaultPosition, wxDefaultSize, LanguageToNChoices, LanguageToChoices, 0 );
	LanguageTo->SetSelection( 0 );
	m_toolBar1->AddControl( LanguageTo );
	m_staticline7 = new wxStaticLine( m_toolBar1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	m_toolBar1->AddControl( m_staticline7 );
	currentPage = new wxSpinCtrl( m_toolBar1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	m_toolBar1->AddControl( currentPage );
	m_staticText29 = new wxStaticText( m_toolBar1, wxID_ANY, _("of "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText29->Wrap( -1 );
	m_toolBar1->AddControl( m_staticText29 );
	allCountPage = new wxStaticText( m_toolBar1, wxID_ANY, _("88888888"), wxDefaultPosition, wxDefaultSize, 0 );
	allCountPage->Wrap( -1 );
	m_toolBar1->AddControl( allCountPage );
	m_toolBar1->AddSeparator();

	wxString DocumentZoomChoices[] = { _("50%"), _("75%"), _("100%"), _("150%"), _("200%") };
	int DocumentZoomNChoices = sizeof( DocumentZoomChoices ) / sizeof( wxString );
	DocumentZoom = new wxChoice( m_toolBar1, wxID_ANY, wxDefaultPosition, wxDefaultSize, DocumentZoomNChoices, DocumentZoomChoices, 0 );
	DocumentZoom->SetSelection( 2 );
	m_toolBar1->AddControl( DocumentZoom );
	InvertColorOCR = new wxCheckBox( m_toolBar1, wxID_ANY, _("Invert colors"), wxDefaultPosition, wxDefaultSize, 0 );
	InvertColorOCR->SetToolTip( _("Invert background and text colors (swaps black and white mutually)") );

	m_toolBar1->AddControl( InvertColorOCR );
	OnlyOCR = new wxCheckBox( m_toolBar1, wxID_ANY, _("Only OCR"), wxDefaultPosition, wxDefaultSize, 0 );
	OnlyOCR->SetToolTip( _("Recognition only, without translation") );

	m_toolBar1->AddControl( OnlyOCR );
	buttonViewResult = new wxButton( m_toolBar1, wxID_ANY, _("Show Translation"), wxDefaultPosition, wxDefaultSize, 0 );
	buttonViewResult->SetToolTip( _("Using the mouse to select an area for translation") );

	m_toolBar1->AddControl( buttonViewResult );
	m_toolBar1->Realize();

	vBox1->Add( m_toolBar1, 0, wxALL|wxEXPAND, 5 );

	hBox1 = new wxBoxSizer( wxHORIZONTAL );

	vBox2 = new wxBoxSizer( wxVERTICAL );

	documentView = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	documentView->SetToolTip( _("Using the mouse to select an area for translation") );

	vBox2->Add( documentView, 1, wxEXPAND | wxALL, 0 );

	hScroll = new wxScrollBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL );
	vBox2->Add( hScroll, 0, wxALL|wxEXPAND, 5 );


	hBox1->Add( vBox2, 1, wxALL|wxEXPAND, 5 );

	vScroll = new wxScrollBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL );
	hBox1->Add( vScroll, 0, wxALL|wxEXPAND, 5 );


	vBox1->Add( hBox1, 1, wxALL|wxEXPAND, 5 );


	this->SetSizer( vBox1 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( GUIOPolyglotDocumentView::OnRenderPage ) );
	LanguageFrom->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotDocumentView::OnSelectLanguageFrom ), NULL, this );
	LanguageTo->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotDocumentView::OnSelectLanguageTo ), NULL, this );
	currentPage->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GUIOPolyglotDocumentView::OnSetCurrentPage ), NULL, this );
	DocumentZoom->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GUIOPolyglotDocumentView::OnDocumentZoom ), NULL, this );
	buttonViewResult->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotDocumentView::OnShowTranslation ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GUIOPolyglotDocumentView::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GUIOPolyglotDocumentView::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GUIOPolyglotDocumentView::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GUIOPolyglotDocumentView::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GUIOPolyglotDocumentView::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GUIOPolyglotDocumentView::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GUIOPolyglotDocumentView::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GUIOPolyglotDocumentView::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GUIOPolyglotDocumentView::OnHScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GUIOPolyglotDocumentView::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GUIOPolyglotDocumentView::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GUIOPolyglotDocumentView::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GUIOPolyglotDocumentView::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GUIOPolyglotDocumentView::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GUIOPolyglotDocumentView::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GUIOPolyglotDocumentView::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GUIOPolyglotDocumentView::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GUIOPolyglotDocumentView::OnVScroll ), NULL, this );
}

GUIOPolyglotDocumentView::~GUIOPolyglotDocumentView()
{
}

GUIOPolyglotDebugViewImage::GUIOPolyglotDebugViewImage( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer52;
	bSizer52 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer53;
	bSizer53 = new wxBoxSizer( wxHORIZONTAL );

	imageView = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizer53->Add( imageView, 1, wxEXPAND | wxALL, 0 );

	vScroll = new wxScrollBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL );
	bSizer53->Add( vScroll, 0, wxALL|wxEXPAND, 5 );


	bSizer52->Add( bSizer53, 1, wxEXPAND, 5 );

	hScroll = new wxScrollBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL );
	bSizer52->Add( hScroll, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer54;
	bSizer54 = new wxBoxSizer( wxHORIZONTAL );


	bSizer54->Add( 0, 0, 1, wxEXPAND, 5 );

	nextButton = new wxButton( this, wxID_ANY, _("Next"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer54->Add( nextButton, 0, wxALL, 5 );


	bSizer52->Add( bSizer54, 0, wxEXPAND, 5 );


	this->SetSizer( bSizer52 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( GUIOPolyglotDebugViewImage::OnSize ) );
	vScroll->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnVScroll ), NULL, this );
	vScroll->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnVScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnHScroll ), NULL, this );
	hScroll->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GUIOPolyglotDebugViewImage::OnHScroll ), NULL, this );
	nextButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIOPolyglotDebugViewImage::OnNext ), NULL, this );
}

GUIOPolyglotDebugViewImage::~GUIOPolyglotDebugViewImage()
{
}
