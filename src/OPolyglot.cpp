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
#include "OPolyglotDownload.h"
#include "OPolyglotSettings.h"
#include "OPolyglotDialogError.h"
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
	#include <gio/gio.h>
#endif

enum{
	TIMER_ID,
	TIMER_CLIPBOARD_TRANSLATION_ID,
};


#if __WXGTK__
	static wxMutex 		mutex;
	static wxWindow		*parent;
	static wxString portalCopySession=wxEmptyString;

static void on_response(GDBusConnection *conn,
                        const char *sender_name,
                        const char *object_path,
                        const char *interface_name,
						const char *signal_name,
                        GVariant *parameters,
                        gpointer user_data)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot.cpp::on_response"));
    uint32_t responce;
    GVariant *results;
    g_variant_get(parameters, "(u@a{sv})", &responce,  &results);

    // We only care about the request we issued
	if(responce == 0) {
		const gchar *uri = NULL;
		if(g_variant_lookup(results,"uri","&s",&uri))
		{
			OPOLYGLOT_MESSAGE(wxT("OPolyglot.cpp::on_response uri = %s"),uri);
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SCREENSHOT_FINISH);
			event->SetInt(1);
			event->SetString(wxString::FromUTF8(uri));
			wxQueueEvent(parent,event);
		} else
		{
			OPOLYGLOT_ERROR(wxT("OPolyglot.cpp::on_response not received uri"));
		}
	} else if(responce == 1)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot.cpp::on_response cancelled user"));
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot.cpp::on_response error capture screen"));
	}
    g_main_loop_quit(static_cast<GMainLoop *>(g_object_get_data(G_OBJECT(conn), "loop")));
}

static void
on_create_session_response(GDBusConnection *connection,
                           const gchar *sender_name,
                           const gchar *object_path,
                           const gchar *interface_name,
                           const gchar *signal_name,
                           GVariant *parameters,
                           gpointer user_data)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::on_create_session_response"));
    GVariant *results = NULL;
	gboolean response_code;


    g_variant_get(parameters, "(u@a{sv})",
                  &response_code, &results);

    if (response_code == 0) {
		const gchar *session_handle = NULL;
		if(g_variant_lookup(results,"session_handle","&s",&session_handle))
		{
			portalCopySession= wxString::FromUTF8(session_handle);
		} else
		{
			OPOLYGLOT_ERROR(wxT("OPolyglot::on_create_session_response Response did not contain session_handle"));
			portalCopySession = wxEmptyString;
		}
#if 0
        GVariant *handle = g_variant_lookup_value(
            results, "session_handle", G_VARIANT_TYPE_OBJECT_PATH);

        if (handle) {
			gchar *session_handle = g_variant_dup_string(handle, NULL);
            g_variant_unref(handle);
			portalCopySession = wxString::FromUTF8(session_handle);
        } else {
			OPOLYGLOT_ERROR(wxT("OPolyglot::on_create_session_response Response did not contain session_handle"));
			portalCopySession = wxEmptyString;
        }
#endif
    }

    g_variant_unref(results);

    g_main_loop_quit(static_cast<GMainLoop *>(g_object_get_data(G_OBJECT(connection), "loop")));
	//g_main_loop_quit(loopCreateSession);
}

static void
on_select_device_response(GDBusConnection *connection,
                           const gchar *sender_name,
                           const gchar *object_path,
                           const gchar *interface_name,
                           const gchar *signal_name,
                           GVariant *parameters,
                           gpointer user_data)
{
    bool *status = static_cast<bool *>(user_data);
	guint response_code;
    GVariant *results = NULL;

    g_variant_get(parameters, "(u@a{sv})",
                  &response_code, &results);
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::on_select_devices_response %d\n"),response_code);
	if(response_code)
	{
		*status = false;
	} else
	{
		*status = true;
	}

    g_variant_unref(results);
    g_main_loop_quit(static_cast<GMainLoop *>(g_object_get_data(G_OBJECT(connection), "loop")));
}

static void
on_start_response(GDBusConnection *connection,
            const gchar *sender_name,
            const gchar *object_path,
            const gchar *interface_name,
            const gchar *signal_name,
            GVariant *parameters,
            gpointer user_data)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::on_start_response"));
    guint response_code;
    GVariant *results;
    gboolean clipboard_enabled = FALSE;
    bool *status = static_cast<bool *>(user_data);

    g_variant_get(parameters, "(u@a{sv})", &response_code, &results);

    if (response_code == 0 &&
        g_variant_lookup(results, "clipboard_enabled", "b",
                         &clipboard_enabled) &&
        clipboard_enabled) {
		*status = true;
    } else if (response_code == 0) {
		*status = false;
		OPOLYGLOT_ERROR("OPolyglot::on_start_response clipboard disabled");
    } else {
		OPOLYGLOT_ERROR("OPolyglot::on_start_response Failed %u",response_code);
    }

    g_variant_unref(results);
    g_main_loop_quit(static_cast<GMainLoop *>(g_object_get_data(G_OBJECT(connection), "loop")));
}


static void PortalInit(wxWindow *w)
{
	wxMutexLocker lock(mutex);
	parent = w;
}

static wxString portalReadClipboardText(
		const char* mime_type = "text/plain")
{
    GError* error = nullptr;
    GUnixFDList* fd_list = nullptr;

	gchar *uuid = g_uuid_string_random();
	g_strdelimit(uuid, "-", '_');
	gchar *token = g_strdup_printf("OPolyglot_%s", uuid);
	g_free(uuid);
	GError *err = nullptr;
	GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
	if (!conn) {
		OPOLYGLOT_ERROR(wxT("OPolyglot::portalReadClipboardText Failed to connect to session bus: %s"),err->message);
        throw std::runtime_error("Failed to connect to session bus");
	}
	const gchar *unique_name = g_dbus_connection_get_unique_name(conn);
	gchar *sender = g_strdup(unique_name+1);
	g_strdelimit(sender,".",'_');
	gchar *request_path = g_strdup_printf(
			"/org/freedesktop/portal/desktop/session/%s/%s",
			sender,
			token);
    GVariant* reply = g_dbus_connection_call_with_unix_fd_list_sync(
        conn,
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Clipboard",
        "SelectionRead",
        g_variant_new("(os)", portalCopySession.utf8_str().data(), mime_type),
        G_VARIANT_TYPE("(h)"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,           // timeout
        nullptr,      // вхідний список fd
        &fd_list,     // список fd з відповіді
        nullptr,
        &error);

    if (!reply) {
        std::string message = error ? error->message : "D-Bus call failed";
        g_clear_error(&error);
        if (fd_list) g_object_unref(fd_list);
        throw std::runtime_error(message);
    }

    // Значення h — індекс fd у переданому D-Bus списку дескрипторів.
    gint handle_index = -1;
    g_variant_get(reply, "(h)", &handle_index);
    g_variant_unref(reply);

    int fd = g_unix_fd_list_get(fd_list, handle_index, &error);
    g_object_unref(fd_list);

    if (fd < 0) {
        std::string message = error ? error->message : "Could not get clipboard fd";
        g_clear_error(&error);
        throw std::runtime_error(message);
    }

    std::string text;
    char buffer[4096];

    for (;;) {
        ssize_t n = read(fd, buffer, sizeof(buffer));
        if (n > 0) {
            text.append(buffer, static_cast<std::size_t>(n));
        } else if (n == 0) {
            break; // кінець даних
        } else if (errno != EINTR) {
			break;
#if 0
            close(fd);
            throw std::runtime_error("Reading clipboard fd failed");
#endif
        }
    }

    close(fd);
    return wxString::FromUTF8(text);
}



static bool portalHasClipboardInterface(GError** error)
{
    GDBusConnection* bus =
        g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, error);
    if (!bus)
        return false;

    GVariant* reply = g_dbus_connection_call_sync(
        bus,
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.DBus.Introspectable",
        "Introspect",
        nullptr,
        G_VARIANT_TYPE("(s)"),
        G_DBUS_CALL_FLAGS_NONE,
        3000,
        nullptr,
        error);

    g_object_unref(bus);

    if (!reply)
        return false;

    const gchar* xml = nullptr;
    g_variant_get(reply, "(&s)", &xml);

    GDBusNodeInfo* node = g_dbus_node_info_new_for_xml(xml, error);
    const bool found =
        node &&
        g_dbus_node_info_lookup_interface(
            node, "org.freedesktop.portal.Clipboard") != nullptr;

    if (node)
        g_dbus_node_info_unref(node);
    g_variant_unref(reply);

    return found;
}

static bool portalHasScreenshotInterface(GError** error)
{
    GDBusConnection* bus =
        g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, error);
    if (!bus)
        return false;

    GVariant* reply = g_dbus_connection_call_sync(
        bus,
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.DBus.Introspectable",
        "Introspect",
        nullptr,
        G_VARIANT_TYPE("(s)"),
        G_DBUS_CALL_FLAGS_NONE,
        3000,
        nullptr,
        error);

    g_object_unref(bus);

    if (!reply)
        return false;

    const gchar* xml = nullptr;
    g_variant_get(reply, "(&s)", &xml);

    GDBusNodeInfo* node = g_dbus_node_info_new_for_xml(xml, error);
    const bool found =
        node &&
        g_dbus_node_info_lookup_interface(
            node, "org.freedesktop.portal.Screenshot") != nullptr;

    if (node)
        g_dbus_node_info_unref(node);
    g_variant_unref(reply);

    return found;
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
	timerClipboardTranslation = new wxTimer(this,TIMER_CLIPBOARD_TRANSLATION_ID);
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
	this->Bind(wxEVT_TIMER,wxTimerEventHandler(OPolyglot::OnTimeCheckClipboard),this,TIMER_CLIPBOARD_TRANSLATION_ID);
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
#if __WXGTK__
	GError* error = nullptr;
    flagCopyClipboardOnlyPortal = portalHasClipboardInterface(&error);
    if (error) {
		OPOLYGLOT_ERROR("OPolyglot not check portal %s",error->message);
        g_error_free(error);
    } else
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglot portal has clipboard interface %s"),OPOLYGLOT_BOOL_TO_STRING(flagCopyClipboardOnlyPortal));
	}
    flagCreateScreenshotOnlyPortal = portalHasScreenshotInterface(&error);
    if (error) {
		OPOLYGLOT_ERROR("OPolyglot not check portal %s",error->message);
        g_error_free(error);
    } else
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglot portal has screenshot interface %s"),OPOLYGLOT_BOOL_TO_STRING(flagCreateScreenshotOnlyPortal));
	}
#endif
#if __WXMSW__
	flagCopyClipboardOnlyPortal = false;
	flagCreateScreenshotOnlyPortal = false;
#endif
	if(flagCreateScreenshotOnlyPortal)
	{
#if __WXGTK__
		OPOLYGLOT_MESSAGE(wxT("OPolyglot use libportal to capture the screen"));
		PortalInit(this);
		//PortalTakeScreenshot(this);
#else
		OPOLYGLOT_ERROR(wxT("OPolyglot not supported screen capture"));
		OPolyglotDialogError dlgErr(this,wxS("OPolyglot not supported screen capture"));
#endif
	} else
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglot available screen capture"));
	}
	if(flagCopyClipboardOnlyPortal)
	{
#if __WXGTK__
		OPOLYGLOT_MESSAGE(wxT("OPolyglot using portal for access to clipboard"));
		GError* error = nullptr;
		GDBusConnection *bus = g_bus_get_sync(G_BUS_TYPE_SESSION,NULL,&error);
		if(!bus)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglot Session bus: %s\n"),error->message);
			g_error_free(error);
		} else
		{
			gchar *uuid = g_uuid_string_random();
			g_strdelimit(uuid,"-",'_');
			const gchar *unique_name = g_dbus_connection_get_unique_name(bus);
			gchar *sender = g_strdup(unique_name+1);
			g_strdelimit(sender,".",'_');
			OPOLYGLOT_MESSAGE(wxT("OPolyglot %s %s"),sender,uuid);
			GVariantBuilder options;
			g_variant_builder_init(&options,G_VARIANT_TYPE("a{sv}"));
			g_variant_builder_add(&options,"{sv}","handle_token",
					g_variant_new_string(sender));
			g_variant_builder_add(&options,"{sv}","session_handle_token",
					g_variant_new_string(uuid));
			g_free(sender);
			g_free(uuid);
			GError *err = nullptr;
			GMainLoop *loop = g_main_loop_new(NULL,FALSE);
			GVariant *reply = g_dbus_connection_call_sync(
					bus,
					"org.freedesktop.portal.Desktop",
					"/org/freedesktop/portal/desktop",
					"org.freedesktop.portal.RemoteDesktop",
					"CreateSession",
					g_variant_new("(a{sv})", &options),
					G_VARIANT_TYPE("(o)"),
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&err);
			if(reply)
			{
				const gchar *returned_path = nullptr;
				g_variant_get(reply,"(&o)",&returned_path);
				guint subscription_id = g_dbus_connection_signal_subscribe(
						bus,
						"org.freedesktop.portal.Desktop",
						"org.freedesktop.portal.Request",
						"Response",
						returned_path,
						NULL,
						G_DBUS_SIGNAL_FLAGS_NONE,
						on_create_session_response,
						NULL,
						NULL);
				g_object_set_data(G_OBJECT(bus), "loop", loop);
				g_main_loop_run(loop);
				g_dbus_connection_signal_unsubscribe(bus, subscription_id);
				g_variant_unref(reply);
				g_variant_builder_init(&options, G_VARIANT_TYPE_VARDICT);
				reply = g_dbus_connection_call_sync(
						bus,
						"org.freedesktop.portal.Desktop",
						"/org/freedesktop/portal/desktop",
						"org.freedesktop.portal.Clipboard",
						"RequestClipboard",
						g_variant_new("(o@a{sv})",
							portalCopySession.utf8_str().data(),
							g_variant_builder_end(&options)),
						G_VARIANT_TYPE("()"),
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&err);
				if(reply)
				{
					g_variant_unref(reply);
					uuid = g_uuid_string_random();
					g_strdelimit(uuid,"-",'_');
					g_variant_builder_init(&options, G_VARIANT_TYPE_VARDICT);
					g_variant_builder_add(&options, "{sv}", "handle_token",
							g_variant_new_string(uuid));
					g_variant_builder_add(&options, "{sv}", "types",
							g_variant_new_uint32(1 | 2));
					g_free(uuid);
					reply = g_dbus_connection_call_sync(
							bus,
							"org.freedesktop.portal.Desktop",
							"/org/freedesktop/portal/desktop",
							"org.freedesktop.portal.RemoteDesktop",
							"SelectDevices",
							g_variant_new("(o@a{sv})",
								portalCopySession.utf8_str().data(),
								g_variant_builder_end(&options)),
							G_VARIANT_TYPE("(o)"),   /* повертається шлях Request */
							G_DBUS_CALL_FLAGS_NONE,
							-1,
							NULL,
							&err);
					if(reply)
					{
						const gchar *returned_path = nullptr;
						g_variant_get(reply,"(&o)",&returned_path);
						bool status_select_device = false;
						subscription_id = g_dbus_connection_signal_subscribe(
								bus,
								"org.freedesktop.portal.Desktop",
								"org.freedesktop.portal.Request",
								"Response",
								returned_path,
								NULL,
								G_DBUS_SIGNAL_FLAGS_NONE,
								on_select_device_response,
								&status_select_device,
								NULL);
						g_object_set_data(G_OBJECT(bus),"loop",loop);
						g_main_loop_run(loop);
						g_dbus_connection_signal_unsubscribe(bus, subscription_id);
						g_variant_unref(reply);
						uuid = g_uuid_string_random();
						g_strdelimit(uuid,"-",'_');
						g_variant_builder_init(&options, G_VARIANT_TYPE("a{sv}"));
						g_variant_builder_add(&options, "{sv}", "handle_token",
								g_variant_new_string(uuid));
						g_free(uuid);
						reply = g_dbus_connection_call_sync(
								bus,
								"org.freedesktop.portal.Desktop",
								"/org/freedesktop/portal/desktop",
								"org.freedesktop.portal.RemoteDesktop",
								"Start",
								g_variant_new("(os@a{sv})",
									portalCopySession.utf8_str().data(),
									"",  
									g_variant_builder_end(&options)),
								G_VARIANT_TYPE("(o)"),
								G_DBUS_CALL_FLAGS_NONE,
								-1,
								NULL,
								&err);
						if(reply)
						{

							const gchar *returned_path = nullptr;
							g_variant_get(reply,"(&o)",&returned_path);
							bool status_clipboard_enable = false;
							OPOLYGLOT_MESSAGE(wxT("OPolyglot %s"),returned_path);
							subscription_id = g_dbus_connection_signal_subscribe(
									bus,
									"org.freedesktop.portal.Desktop",
									"org.freedesktop.portal.Request",
									"Response",
									returned_path,
									NULL,
									G_DBUS_SIGNAL_FLAGS_NONE,
									on_start_response,
									&status_clipboard_enable,
									NULL);
							g_object_set_data(G_OBJECT(bus),"loop",loop);
							g_main_loop_run(loop);
							g_dbus_connection_signal_unsubscribe(bus, subscription_id);
							g_variant_unref(reply);
							
							g_main_loop_unref(loop);
							g_object_unref(bus);
							if(status_clipboard_enable)
							{
								if(portalCopySession.IsEmpty())
								{
									OPOLYGLOT_ERROR(wxT("OPolyglot Failed creating session"));
									OPolyglotDialogError dlgErr(this,wxS("OPolyglot Failed creating session"));
								}
								OPOLYGLOT_MESSAGE(wxT("OPolyglot Session copy clipboard %s"),portalCopySession);
							} else
							{
								OPOLYGLOT_ERROR(wxT("OPolyglot Failed access to clipboard"));
								checkClipboardTranslation->Enable(false);
								portalCopySession = wxEmptyString;
							}
						} else
						{
							OPOLYGLOT_ERROR(wxT("OPolyglot Failed Start %s"),err->message);
							checkClipboardTranslation->Enable(false);
							portalCopySession = wxEmptyString;
						}
					} else
					{
						OPOLYGLOT_ERROR("OPolyglot Failed SelectDevices %s",err->message);
						checkClipboardTranslation->Enable(false);
						portalCopySession = wxEmptyString;
					}
				} else
				{
					OPOLYGLOT_ERROR(wxT("OPolyglot Failed RequestClipboard %s"),err->message);
					checkClipboardTranslation->Enable(false);
					portalCopySession = wxEmptyString;
				}
			} else
			{
				OPOLYGLOT_ERROR(wxT("OPolyglot Failed CreatedSession %s"),err->message);
				checkClipboardTranslation->Enable(false);
				portalCopySession = wxEmptyString;
			}
		}
#else
		OPOLYGLOT_ERROR(wxT("OPolyglot not supported copy clipboard"));
		OPolyglotDialogError dlgErr(this,wxS("OPolyglot not supported copy clipboard"));
#endif
	} else
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot available clipboard"));
	}

	viewTextTranslate = new OPolyglotViewTextTranslate(this,OPOLYGLOT_GET_XML_FILE_TRANSLATE);
	oldValueClipboard = wxEmptyString;
}



OPolyglot::~OPolyglot()
{
	OPOLYGLOT_MESSAGE(wxT("~OPolyglot"));
	if(!IS_NULLPTR(frameDownload ))
	{
		delete frameDownload;
	}
	delete LanguageFrom;
	delete timerClipboardTranslation;
}


void OPolyglot::OnTimeCheckClipboard(wxTimerEvent& event)
{
	wxString dataToTranslate = wxEmptyString;
	if(flagCopyClipboardOnlyPortal)
	{
#if __WXGTK__
		try{
			wxString value = portalReadClipboardText();
			if(value != oldValueClipboard)
			{
				oldValueClipboard = value;
				dataToTranslate = oldValueClipboard;
			}
		}catch(const std::exception& e)
		{
			timerClipboardTranslation->Stop();
			checkClipboardTranslation->SetValue(false);
			checkClipboardTranslation->Enable(false);
			OPOLYGLOT_ERROR(wxT("OPolyglot::OnTimeCheckClipboard %s %s"),wxS("Error reading the clipboard on the portal."),e.what());
			OPolyglotDialogError ode(this,_("Error reading the clipboard on the portal."));

		}
#endif
	} else
	{
		if(wxTheClipboard->Open())	
		{
			if(wxTheClipboard->IsSupported(wxDF_TEXT))
			{
				wxTextDataObject data;
				wxTheClipboard->GetData(data);
				if(data.GetText() != oldValueClipboard)
				{
					oldValueClipboard = data.GetText();
					dataToTranslate = oldValueClipboard;
				}
			}
			wxTheClipboard->Close();
		} else
		{
			timerClipboardTranslation->Stop();
			checkClipboardTranslation->SetValue(false);
			checkClipboardTranslation->Enable(false);
			OPOLYGLOT_ERROR(wxT("OPolyglot::OnTimeCheckClipboard error opening the clipboard"));
			OPolyglotDialogError msg(this,wxString::Format(wxT("%s"),_("error opening the clipboard")));
		}
	}
	if(!dataToTranslate.IsEmpty())
	{
		OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnTimeCheckClipboard data to translate %s"),dataToTranslate);
	}
}

void OPolyglot::OnClipboardTranslation(wxCommandEvent& event)
{
	OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnClipboardTranslation(%s)"),OPOLYGLOT_BOOL_TO_STRING(checkClipboardTranslation->GetValue()));
	if(checkClipboardTranslation->GetValue())
	{
		timerClipboardTranslation->Start(1500);
	} else
	{
		timerClipboardTranslation->Stop();
	}
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
		if((!flagCreateScreenshotOnlyPortal))
		{
			OPOLYGLOT_MESSAGE(wxT("OnCaptureScreen(%dx%d)"),w,h);
			wxScreenDC dc;
			dc.GetSize(&w,&h);
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
				OPolyglotDialogError msg(this,wxString::Format(wxT("%s %s"),_("error saving screenshot"),fileName));
				return;
			}
			wxThreadEvent *event = new wxThreadEvent(wxEVT_COMMAND_OPOLYGLOT_SCREENSHOT_FINISH);
			event->SetInt(-1);
			event->SetString(fileName);
			wxQueueEvent(this,event);
		} else
		{
			OPOLYGLOT_MESSAGE(wxT("OnCaptureScreen using libportal"));
#if __WXGTK__
			//PortalTakeScreenshot(this);
			GError *err = nullptr;
			GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
			if (!conn) {
				OPOLYGLOT_ERROR(wxT("OPolyglot::OnCaptureScreen Failed to connect to session bus: %s"),err->message);
				return ;
			}
			gchar *uuid = g_uuid_string_random();
			g_strdelimit(uuid, "-", '_');
			gchar *token = g_strdup_printf("OPolyglot_%s", uuid);
			g_free(uuid);
			OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnCaptureScreen token %s"),token);
			const gchar *unique_name = g_dbus_connection_get_unique_name(conn);
			gchar *sender = g_strdup(unique_name+1);
			g_strdelimit(sender,".",'_');
			OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnCaptureScreen sender %s"),sender);
			gchar *request_path = g_strdup_printf(
					"/org/freedesktop/portal/desktop/request/%s/%s",
					sender,
					token);
			g_free(sender);
			OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnCaptureScreen request_path %s"),request_path);
			guint subscription_id = g_dbus_connection_signal_subscribe(
					conn,
					"org.freedesktop.portal.Desktop",
					"org.freedesktop.portal.Request",
					"Response",
					request_path,
					NULL,
					G_DBUS_SIGNAL_FLAGS_NONE,
					on_response,
					this,
					NULL);

			GVariantBuilder options;
			g_variant_builder_init(&options, G_VARIANT_TYPE_VARDICT);
			g_variant_builder_add(&options,
					"{sv}",
					"handle_token",
					g_variant_new_string(token));
			GVariant *var = g_variant_builder_end(&options);
			OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnCaptureScreen"));
			GVariant *reply = g_dbus_connection_call_sync(
					conn,
					"org.freedesktop.portal.Desktop",
					"/org/freedesktop/portal/desktop",
					"org.freedesktop.portal.Screenshot",
					"Screenshot",
					g_variant_new("(s@a{sv})", "", var),
					G_VARIANT_TYPE("(o)"),
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&err);
			if(!reply)
			{
				OPOLYGLOT_ERROR(wxT("OPolyglot::OnCaptureScreen %s"),err->message);
			}
			const gchar *returned_path = NULL;
			g_variant_get(reply,"(&o)",&returned_path);
			OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnCaptureScreen returned_path %s"),returned_path);
			GMainLoop *loop = g_main_loop_new(NULL,FALSE);
			g_object_set_data(G_OBJECT(conn), "loop", loop);
			g_main_loop_run(loop);
			OPOLYGLOT_MESSAGE(wxT("OPolyglot::OnCaptureScreen finish"));
			g_dbus_connection_signal_unsubscribe(conn,subscription_id);
			g_main_loop_unref(loop);
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
		buttonOpenDocument->Enable(true);
	} else
	{
		buttonShowTranslator->Enable(false);
		buttonCaptureScreen->Enable(false);
		buttonOpenDocument->Enable(false);
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
		buttonOpenDocument->Enable(true);
	} else
	{
		buttonShowTranslator->Enable(false);
		buttonCaptureScreen->Enable(false);
		buttonOpenDocument->Enable(false);
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
		OPolyglotDialogError msg(this,wxString::Format(wxT("%s %s"),_("Error root node (ScreenshotFile) != "),inputDoc.GetRoot()->GetName()));
		return (wxThread::ExitCode)0;
	}
	wxString fileName = inputDoc.GetRoot()->GetAttribute(wxS("fileName"));
	OPOLYGLOT_DEBUG(wxT("OPolyglot::Entry screenshot file %s"),fileName);
	PIX *pixs = pixRead(fileName);
	if(!pixs)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot::Entry error opened file %s"),fileName);
		OPolyglotDialogError msg(this,wxString::Format(wxT("%s %s"),_("Error opened file "),fileName));
		return (wxThread::ExitCode)0;
	}
	PIX* pix_gray = pixConvertRGBToLuminance(pixs);
	pixDestroy(&pixs);
	pixs = pix_gray;
	if(!pixs)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot::Entry error convert to gray %s"),fileName);
		OPolyglotDialogError msg(this,wxString::Format(wxT("%s %s"),_("Failed to convert image to grayscale"),fileName));
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
					OPolyglotDialogError msg(this,wxString::Format(wxT("%s"),_("Failed pixClipRectangle")));
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
					OPolyglotDialogError msg(this,wxString::Format(wxT("%s"),_("Failed ocr and translation")));
					return (wxThread::ExitCode)0;
				}
				xmlTranslate->GetRoot()->AddChild(textNode);


			}
		}
	}
	if(!xmlTranslate->Save(OPOLYGLOT_GET_XML_FILE_TRANSLATE))
	{
		OPOLYGLOT_ERROR(wxT("OPolyglot::Entry failed to save changes  %s"),OPOLYGLOT_GET_XML_FILE_TRANSLATE);
		OPolyglotDialogError msg(this,wxString::Format(wxT("%s %s"),_("Failed to save changed"),OPOLYGLOT_GET_XML_FILE_TRANSLATE));
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
			OPolyglotDialogError msg(this,_("Could not create the worker thread!"));
			return;
		}
		OPOLYGLOT_DEBUG(wxT("OPolyglot::OnStartOCR thread created"));
		if(GetThread()->Run() != wxTHREAD_NO_ERROR)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglot::OnStartOCR could not run the worker thread!"));
			OPolyglotDialogError msg(this,_("Could not run the worker thread!"));
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
			OPolyglotDialogError msg(this,wxS("not create screenshot"));
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
		OPolyglotDialogError msg(this,wxString::Format(wxS("%s \"%s -> %s\""),_("Not found in installed languages"),oldLangTo,oldLangFrom));
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
	wxString result = wxEmptyString;
	if(!input.IsEmpty())
	{
		result = LibOPolyglotTranslator(input,configsTranslator);
	}
	wxThreadEvent *event = new wxThreadEvent();
	if(((input.IsEmpty())&&(result.IsEmpty()))||(!result.IsEmpty()))
	{
		event->SetInt(0);
	} else
	{
		//Error LibOPolyglotTranslator
		event->SetInt(-1);
	}
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
	if(event.GetInt()!=0)
	{
		OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnThreadTranslatorFinish return IsNull"));
		OPolyglotDialogError msg(this,wxT("Error: \"OPolyglotTranslator\" return thread is error"));
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
			OPolyglotDialogError msg(this,_("Could not create the worker thread!"));
			return;
		}
		OPOLYGLOT_DEBUG(wxT("OPolyglotTranslator::OnStartTranslator thread created"));
		if(GetThread()->Run() != wxTHREAD_NO_ERROR)
		{
			OPOLYGLOT_ERROR(wxT("OPolyglotTranslator::OnStartTranslator could not run the worker thread!"));
			OPolyglotDialogError msg(this,_("Could not run the worker thread!"));
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
