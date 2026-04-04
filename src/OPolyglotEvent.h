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


#pragma once
#include <wx/event.h>

wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_EXIT,	wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_THREAD_FINISH,			wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_SETUP,					wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_ABOUT,					wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_CHANGE_SHOW,			wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_CANCEL_USER,			wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_SCREENSHOT_FINISH,		wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_SCREENSHOT_START,		wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_OCR_START,			wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_OPOLYGLOT_CLOSE_TRANSLATOR,	wxThreadEvent);
