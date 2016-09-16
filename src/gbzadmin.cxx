// gbzadmin version 2.4.x
// GTKmm bzadmin
// Copyright (c) 2005 - 2014 Michael Sheppard
//
// Code based on BZFlag-2.0.x and SVN 2.4.x
// Portions Copyright (c) 1993 - 2009 Tim Riker
//
//  =====GPL=============================================================
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; version 2 dated June, 1991.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program;  if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
//  =====================================================================
//

#include "gbzadmin.h"

const char *windowTitle = "gBZAdmin3";
const char *configFileName = "/.gbzadmin";
const int maxServersList = 16;

// message mask/prefs dialog widget names
const gchar *msg_names[] = {
    "rabbit",	// new rabbit
    "pause",	// player paused
    "spawn",	// player spawned
    "ping",	// server ping
    "bzdb",	// variables updated
    "join",	// player joined
    "leave",	// player left
    "admin",	// admin info
    "kill",	// player killed
    "score",	// score update
    "chat",	// p2p messages
    "roger",	// auto pilot
    "flags",	// flag update
    "time",	// timestamp join/part
    "teleport", // player teleported
    "gtime",	// game time
    "tupdate",	// time update
};

const int n_msg_masks = (sizeof (msg_names) / sizeof (msg_names[0]) );

gbzadmin::gbzadmin() : Gtk::Window()
{
    connect_dialog = 0;
    pref_dialog = 0;
    input_dialog = 0;
    save_dialog = 0;
    flag_reset_dialog = 0;
    lagwarn_dialog = 0;

    count = prev_count = 0;
    line = prev_line = 0;
    wd_counter = MaxWdTime;
}

bool gbzadmin::isConnected()
{
    return (sock.getState() == Okay);
}

void gbzadmin::power_down()
{
    Gtk::Window *app;
    refBuilder->get_widget ("app", app);

    // save window position and size to conf file
    app->get_size (win_width, win_height);
    app->get_position (win_x, win_y);

    // save pane positions
    Gtk::Paned *pane;
    refBuilder->get_widget ("vpaned1", pane);
    msg_pane = pane->get_position();

    refBuilder->get_widget ("hpaned1", pane);
    game_pane = pane->get_position();

    Glib::ustring configPath (Glib::get_home_dir() );
    configPath += configFileName;

    save_config_file (configPath);

    if (app) {
        app->hide(); // hide() will cause main::run() to end.
    }
}

// in case the user clicks the close button on the
// top level window.
bool gbzadmin::on_delete_event (GdkEventAny *event)
{
    // if we're connected, shutdown here, otherwise just quit
    if (isConnected() ) {
        cmd_str = "/quit";
        process_command();
    } else {
        power_down();
    }
    return true;
}

void gbzadmin::shut_down()
{
    sock.sendExit();
    sock.disconnect();

    power_down();
}

void gbzadmin::on_quit_activate()
{
    if (isConnected() ) {
        if (confirm_quit_and_save() ) {
            shut_down();
        }
    } else {
        shut_down();
    }
}

void gbzadmin::on_connect_activate()
{
    connect_dialog_setup();
}

void gbzadmin::on_connect_signal_to_item()
{
    connect_dialog_setup();
}

void gbzadmin::on_disconnect_activate()
{
    logoff();
}

void gbzadmin::on_disconnect_signal_to_item()
{
    logoff();
}

void gbzadmin::logoff()
{
    if (confirm ("Do you really want to disconnect?", false) == GTK_RESPONSE_YES) {
        conn_timer.disconnect();
        wd_timer.disconnect();
        tcp_data_pending.disconnect();
        sock.sendExit();
        sock.disconnect();
    } else {
        return;
    }
    clean_up();
}

void gbzadmin::clean_up (bool clear_msg_view)
{
    set_status_message (StatusConnTime, "Disconnected");

    if (clear_msg_view) {
        msg_view.clear();
    }

    player_view.clear();
    cmd.clear_targets();
    player_view.clear_players();
    player_combo.clear_items();
    flag_store.clear();

    enable_admin_items (false);

    count = prev_count = 0;
    line = prev_line = 0;

    conn_timer.disconnect();
    wd_timer.disconnect();

    wd_counter = MaxWdTime;

    leaving = false;

    // disable the connected-only items
    enable_connected_items (false);

    // need to prefetch the login token again
    if (sock.get_prefetch() ) {
        sock.preFetchToken (_callsign, _password);
    }
}

void gbzadmin::on_save_activate()
{
    if (!save_dialog) {
        refBuilder->get_widget ("save_dialog", save_dialog);
    }
    save_dialog->show();
}

void gbzadmin::on_save_config_activate()
{
    Glib::ustring configPath (Glib::get_home_dir() );
    configPath += configFileName;

    save_config_file (configPath);
}

void gbzadmin::on_prefs_activate()
{
    pref_dialog_setup();
}

void gbzadmin::on_capture_activate()
{
    toggle_capture_file();
}

void gbzadmin::on_lagwarn_activate()
{
    refBuilder->get_widget ("lagwarn_dialog", lagwarn_dialog);
    if (lagwarn_dialog) {
        lagwarn_dialog->show();
    }
}

void gbzadmin::on_lagwarn_dialog_response (gint response_id)
{
    if (response_id == Gtk::RESPONSE_OK) {
        Glib::ustring command ("/lagwarn ");
        Gtk::Entry *entry;
        refBuilder->get_widget ("lagwarn_entry", entry);
        Glib::ustring warn = entry->get_text();
        if (warn.size() ) {
            command += warn;
        }
        cmd_str = command;
        process_command();
    }
    lagwarn_dialog->hide();
}

void gbzadmin::on_uptime_activate()
{
    do_command_send (ServerUptime);
}

void gbzadmin::on_flag_reset_activate()
{
    refBuilder->get_widget ("flag_reset_dialog", flag_reset_dialog);
    if (flag_reset_dialog) {
        flag_reset_dialog->show();
    }
}

void gbzadmin::on_flag_reset_dialog_response (gint response_id)
{
    if ( (response_id == Gtk::RESPONSE_OK) || (response_id == Gtk::RESPONSE_APPLY) ) {
        Glib::ustring command;
        Gtk::RadioButton *unused, * all;
        refBuilder->get_widget ("unused_button", unused);
        refBuilder->get_widget ("all_button", all);
        if (unused->get_active() ) {
            command = "/flag reset unused";
        } else if (all->get_active() ) {
            command = "/flag reset all";
        } else {
            command = "/flag reset team";
        }
        cmd_str = command;

        if (response_id == Gtk::RESPONSE_OK) {
            flag_reset_dialog->hide();
        }
        process_command();
    } else {
        flag_reset_dialog->hide();
    }
}

void gbzadmin::on_flag_up_activate()
{
    if (confirm ("You are about to remove ALL flags\nDo you really want to do this?", false) == GTK_RESPONSE_YES) {
        do_command_send (FlagUp);
    }
}

void gbzadmin::connect_dialog_setup()
{
    refBuilder->get_widget ("connect_dialog", connect_dialog);
    if (connect_dialog) {
        Gtk::Button *button;
        refBuilder->get_widget ("cancelbutton3", button);
        button->set_sensitive();

        Gtk::Entry *w_callsign, *w_password, *w_motto;
        refBuilder->get_widget ("callsign_entry", w_callsign);
        refBuilder->get_widget ("password_entry", w_password);
        refBuilder->get_widget ("motto_entry", w_motto);

        w_callsign->set_text (_callsign);
        w_password->set_text (_password);
        w_motto->set_text (_motto);

        // add the server:port strings
        server_combo.clear_items();
        std::list<Glib::ustring>::iterator iter;
        int nLines = 1;
        for (iter = server_mru_str.begin(); iter != server_mru_str.end(); iter++) {
            server_combo.append_text (*iter);
            nLines++;
            if (nLines >= maxServersList) {
                break;
            }
        }
        server_combo.set_active_text (server_mru_find (current_server) );

        connect_dialog->show_all();
    }
}

void gbzadmin::on_connect_dialog_response (gint response_id)
{
    Gtk::Entry *w_callsign, *w_password, *w_motto;

    if (response_id == Gtk::RESPONSE_OK) {
        Gtk::Button *button;
        refBuilder->get_widget ("cancelbutton3", button);
        button->set_sensitive (false);
        refBuilder->get_widget ("callsign_entry", w_callsign);
        refBuilder->get_widget ("password_entry", w_password);
        refBuilder->get_widget ("motto_entry", w_motto);

        Glib::ustring addr = server_combo.get_entry_text();
        parse_host_port (addr); // this sets the _server and _port variables

        // check to see if this server is already in the list
        bool found = false;
        std::list<Glib::ustring>::iterator iter = server_mru_str.begin();
        while (iter != server_mru_str.end() ) {
            if (*iter == addr) {
                found = true;
                break; // server:port already in list
            }
            iter++;
        }
        if (!found) { // add server:port string to MRU list
            server_mru_str.push_front (addr);
            current_server = addr;
        }
        // get the rest of the story
        _callsign.assign (w_callsign->get_text(), 0, CallSignLen);
        _password.assign (w_password->get_text(), 0, PasswordLen);
        _motto.assign (w_motto->get_text(), 0, MottoLen);

        logon();
    } else {
        // only hide the connect dialog if canceled.
        // the logon routine will hide it otherwise.
        connect_dialog->hide();
    }
}

void gbzadmin::pref_dialog_setup()
{
    Gtk::Dialog *dlg;
    refBuilder->get_widget ("pref_dialog", dlg);
    if (dlg) {
        Gtk::CheckButton *check;
        // set checkbuttons to current msg mask values
        for (int k = 0; k < n_msg_masks; k++) {
            refBuilder->get_widget (msg_names[k], check);
            check->set_active (msg_mask[msg_names[k]]);
        }
        refBuilder->get_widget ("pref_auto_cmd", check);
        check->set_active (auto_cmd);

        refBuilder->get_widget ("pref_small_toolbar", check);
        check->set_active (small_toolbar);

        refBuilder->get_widget ("pref_pings", check);
        check->set_active (echo_pings);

        refBuilder->get_widget ("pref_save_password", check);
        check->set_active (save_password);

        refBuilder->get_widget ("connect_at_startup", check);
        check->set_active (connect_at_startup);

        refBuilder->get_widget ("line_numbers", check);
        check->set_active (line_numbers);

        refBuilder->get_widget ("debugmsgs", check);
        check->set_active (debugMsgs);

        Glib::ustring sp = "";
        refBuilder->get_widget ("pref_dump_players", check);
        if (sp.empty() ) {
            check->set_sensitive (false);
        } else {
            check->set_active (dump_players);
        }
        refBuilder->get_widget ("net_stats", check);
        check->set_active (sock.netStatsEnabled() );

        refBuilder->get_widget ("pref_wd_time", check);
        check->set_active (display_wd_time);

        Gtk::Entry *entry;
        refBuilder->get_widget ("pref_callsign_entry", entry);
        entry->set_text (_callsign);

        refBuilder->get_widget ("pref_server_entry", entry);
        Glib::ustring host (_server);
        host += ":" + _port_str;
        entry->set_text (host);

        refBuilder->get_widget ("pref_motto_entry", entry);
        entry->set_text (_motto);

        dlg->show();
    }
}

void gbzadmin::on_pref_dialog_response (gint response_id)
{
    if ( (response_id == Gtk::RESPONSE_OK) || (response_id == Gtk::RESPONSE_APPLY) ) {
        Gtk::CheckButton *w;
        bool checked;
        for (int k = 0; k < n_msg_masks; k++) {
            refBuilder->get_widget (msg_names[k], w);
            checked = w->get_active();
            set_message_filter (msg_names[k], checked);
        }

        refBuilder->get_widget ("pref_auto_cmd", w);
        auto_cmd = w->get_active();

        refBuilder->get_widget ("pref_small_toolbar", w);
        small_toolbar = w->get_active();
        // change the style here
        Gtk::Toolbar *toolbar;
        refBuilder->get_widget ("toolbar", toolbar);
        toolbar->set_toolbar_style (small_toolbar ? Gtk::TOOLBAR_ICONS : Gtk::TOOLBAR_BOTH);

        refBuilder->get_widget ("pref_pings", w);
        echo_pings = w->get_active();

        refBuilder->get_widget ("pref_save_password", w);
        save_password = w->get_active();

        refBuilder->get_widget ("connect_at_startup", w);
        connect_at_startup = w->get_active();

        refBuilder->get_widget ("net_stats", w);
        sock.setNetStats (w->get_active() );

        refBuilder->get_widget ("pref_dump_players", w);
        dump_players = w->get_active();

        refBuilder->get_widget ("line_numbers", w);
        line_numbers = w->get_active();
        msg_view.set_line_numbers (line_numbers);

        refBuilder->get_widget ("debugmsgs", w);
        debugMsgs = w->get_active();

        refBuilder->get_widget ("pref_wd_time", w);
        display_wd_time = w->get_active();

        Gtk::Entry *e;
        refBuilder->get_widget ("pref_callsign_entry", e);
        _callsign = e->get_text();

        refBuilder->get_widget ("pref_server_entry", e);
        parse_host_port (e->get_text() );

        refBuilder->get_widget ("pref_motto_entry", e);
        _motto = e->get_text();

        if (response_id == Gtk::RESPONSE_OK) {
            pref_dialog->hide();
        }
    } else {
        pref_dialog->hide();
    }
}

Gtk::Window *gbzadmin::init_gbzadmin (Glib::RefPtr<Gtk::Builder> _refBuilder)
{
    refBuilder = _refBuilder;

    // some defaults
    capturing = false;
    leaving = false;
    dump_players = false;
    auto_cmd = false;
    echo_pings = false;
    save_password = true;
    connect_at_startup = false;
    line_numbers = false;
    debugMsgs = false;
    display_wd_time = false;

    win_x = 0;
    win_y = 0;
    win_width = 800;
    win_height = 600;
    msg_pane = 300;
    game_pane = 400;
    current_server = "";

    current_cmd_type = NoCommand;

    refBuilder->get_widget ("app", app);
    if (!app) {
        return 0;
    }

    app->set_title (Glib::ustring (windowTitle) );

    Glib::ustring configPath (Glib::get_home_dir() );
    configPath += configFileName;

    // check for the config file first
    if (Glib::file_test (configPath, Glib::FILE_TEST_EXISTS) ) {
        parse_config_file (configPath);
    } else {
        // no config file, run preferences dialog for initial setup
        pref_dialog_setup();
    }

    // set window size and position
    app->move (win_x, win_y);
    app->resize (win_width, win_height);

    // set pane positions
    Gtk::Paned *pane;
    refBuilder->get_widget ("vpaned1", pane);
    pane->set_position (msg_pane);

    refBuilder->get_widget ("hpaned1", pane);
    pane->set_position (game_pane);

    // init the views
    player_view.init (refBuilder);
    msg_view.init (refBuilder);
    server_vars_view.init (refBuilder);
    game_view.init (refBuilder);
    server_list_view.init (refBuilder);
    cmd.init (refBuilder);

    // misc view initializations
    msg_view.set_line_numbers (line_numbers);
    defColor = msg_view.Color (WhiteFg);

    add_callbacks();

    // initialize the message handler map
    init_message_handler_map();

    set_status_message (StatusConnTime, "Disconnected");
    set_status_message (StatusNetStats, "Calculating...");

    // replace the input dialog comboboxes with combotextboxes
    replace_input_placeholders();

    // replace the connect dlg combobox
    replace_connect_placeholders();

    if (auto_cmd) {
        cmd.set_auto_cmd();
    }
    // disable the connected-only items
    enable_connected_items (false);

    // prefetch the login token
    if (sock.get_prefetch() ) {
        sock.preFetchToken (_callsign, _password);
    }

    if (connect_at_startup) {
        logon();
    }

    return app;
}

// message code to handler function map initialization
void gbzadmin::init_message_handler_map()
{
    handler_map[MsgPause]				= &gbzadmin::handle_pause_message;
    handler_map[MsgAutoPilot]			= &gbzadmin::handle_autopilot_message;
    handler_map[MsgAddPlayer]			= &gbzadmin::handle_add_player_message;
    handler_map[MsgNewRabbit]			= &gbzadmin::handle_rabbit_message;
    handler_map[MsgRemovePlayer]		= &gbzadmin::handle_remove_player_message;
    handler_map[MsgPlayerInfo]			= &gbzadmin::handle_playerinfo_message;
    handler_map[MsgAdminInfo]			= &gbzadmin::handle_admininfo_message;
    handler_map[MsgKilled]				= &gbzadmin::handle_killed_message;
    handler_map[MsgScore]				= &gbzadmin::handle_score_message;
    handler_map[MsgAlive]				= &gbzadmin::handle_alive_message;
    handler_map[MsgSetVar]				= &gbzadmin::handle_setvar_message;
    handler_map[MsgGrabFlag]			= &gbzadmin::handle_grabflag_message;
    handler_map[MsgDropFlag]			= &gbzadmin::handle_dropflag_message;
    handler_map[MsgFlagUpdate]			= &gbzadmin::handle_flagupdate_message;
    handler_map[MsgTransferFlag]		= &gbzadmin::handle_transferflag_message;
    handler_map[MsgLagPing]				= &gbzadmin::handle_ping_message;
    handler_map[MsgQueryGame]			= &gbzadmin::handle_game_query_message;
    handler_map[MsgTeamUpdate]			= &gbzadmin::handle_teamupdate_message;
    handler_map[MsgMessage]				= &gbzadmin::handle_message_message;
    handler_map[MsgTeleport]			= &gbzadmin::handle_teleport_message;
    handler_map[MsgGameTime]			= &gbzadmin::handle_game_time_message;
    handler_map[MsgTimeUpdate]			= &gbzadmin::handle_time_update_message;
}

void gbzadmin::on_about_activate()
{
    const gchar *license =
        "    This program is free software; you can redistribute it and/or modify\n"
        "it under the terms of the GNU General Public License as published by\n"
        "the Free Software Foundation; either version 2 of the License, or\n"
        "(at your option) any later version.\n\n"
        "    This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n\n"
        "    You should have received a copy of the GNU General Public License\n"
        "along with this program; if not, write to the Free Software\n"
        "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA";

    Glib::ustring comment ("Gtkmm powered BZFlag server administration\nBased on BZFlag's bzadmin\n");
    comment += "GtkBuilder Version\n\n";
    comment += getAppVersion();
    comment += "-";
    comment += getServerVersion();

    Glib::ustring path (PACKAGE_DATA_DIR);
    path += "/";
    path += PACKAGE;
    path += "/pixmaps";

    Glib::ustring pathname ("");
    pathname = Glib::build_filename (path, "background.png");

    Glib::RefPtr<Gdk::Pixbuf> background = Gdk::Pixbuf::create_from_file (pathname);
    if (! (background) ) {
        std::cerr << "Failed to load pixbuf file: %s: " << pathname << std::endl;
    }

    const gchar *authors[] = {
        "Michael Sheppard (gBZAdmin)",
        "BZFlag Development Team (BZFlag source)",
        0
    };

    Gtk::AboutDialog dlg;

    dlg.set_name ("gBZAdmin");
    dlg.set_version (VERSION);
    dlg.set_copyright ("Copyright (c) 2005 - 2014 Michael Sheppard\nPortions Copyright (c) 1993 - 2009 Tim Riker");
    dlg.set_license (license);
    dlg.set_comments (comment);
    dlg.set_authors (authors);
    dlg.set_logo (background);

    dlg.run();
}

void gbzadmin::add_callbacks()
{
    // Tool Button handlers
    connect_signal_to_item ("connect_button", sigc::mem_fun (*this, &gbzadmin::on_connect_signal_to_item) );
    connect_signal_to_item ("disconnect_button", sigc::mem_fun (*this, &gbzadmin::on_disconnect_signal_to_item) );
    connect_signal_to_item ("mute_button", sigc::mem_fun (*this, &gbzadmin::on_mute_button_clicked) );
    connect_signal_to_item ("kick_button", sigc::mem_fun (*this, &gbzadmin::on_kick_button_clicked) );
    connect_signal_to_item ("ban_button", sigc::mem_fun (*this, &gbzadmin::on_ban_button_clicked) );
    connect_signal_to_item ("playerlist_button", sigc::mem_fun (*this, &gbzadmin::on_playerlist_button_clicked) );
    connect_signal_to_item ("clientquery_button", sigc::mem_fun (*this, &gbzadmin::on_clientquery_button_clicked) );
    connect_signal_to_item ("lagstats_button", sigc::mem_fun (*this, &gbzadmin::on_lagstats_button_clicked) );
    connect_signal_to_item ("query_button", sigc::mem_fun (*this, &gbzadmin::on_query_listserver_clicked) );

    // Menu Item handlers
    connect_signal_to_item ("quit", sigc::mem_fun (*this, &gbzadmin::on_quit_activate) );
    connect_signal_to_item ("about", sigc::mem_fun (*this, &gbzadmin::on_about_activate) );
    connect_signal_to_item ("connect", sigc::mem_fun (*this, &gbzadmin::on_connect_activate) );
    connect_signal_to_item ("disconnect", sigc::mem_fun (*this, &gbzadmin::on_disconnect_activate) );
    connect_signal_to_item ("save", sigc::mem_fun (*this, &gbzadmin::on_save_activate) );
    connect_signal_to_item ("save_config", sigc::mem_fun (*this, &gbzadmin::on_save_config_activate) );
    connect_signal_to_item ("preferences", sigc::mem_fun (*this, &gbzadmin::on_prefs_activate) );
    connect_signal_to_item ("message_view_scrolling", sigc::mem_fun (*this, &gbzadmin::on_message_view_scrolling_activate) );
    connect_signal_to_item ("mute", sigc::mem_fun (*this, &gbzadmin::on_mute_activate) );
    connect_signal_to_item ("kick", sigc::mem_fun (*this, &gbzadmin::on_kick_activate) );
    connect_signal_to_item ("ban", sigc::mem_fun (*this, &gbzadmin::on_ban_activate) );
    connect_signal_to_item ("playerlist", sigc::mem_fun (*this, &gbzadmin::on_playerlist_activate) );
    connect_signal_to_item ("clientquery", sigc::mem_fun (*this, &gbzadmin::on_clientquery_activate) );
    connect_signal_to_item ("lagwarn", sigc::mem_fun (*this, &gbzadmin::on_lagwarn_activate) );
    connect_signal_to_item ("query_listserver", sigc::mem_fun (*this, &gbzadmin::on_query_listserver_activate) );
    connect_signal_to_item ("lagstats", sigc::mem_fun (*this, &gbzadmin::on_lagstats_activate) );
    connect_signal_to_item ("uptime", sigc::mem_fun (*this, &gbzadmin::on_uptime_activate) );
    connect_signal_to_item ("flag_reset", sigc::mem_fun (*this, &gbzadmin::on_flag_reset_activate) );
    connect_signal_to_item ("remove_flags", sigc::mem_fun (*this, &gbzadmin::on_flag_up_activate) );
    connect_signal_to_item ("shutdown_server", sigc::mem_fun (*this, &gbzadmin::on_shutdown_server_activate) );
    connect_signal_to_item ("super_kill", sigc::mem_fun (*this, &gbzadmin::on_super_kill_activate) );
    connect_signal_to_item ("player_list_window", sigc::mem_fun (*this, &gbzadmin::on_player_window_activate) );
    connect_signal_to_item ("server_variables_window", sigc::mem_fun (*this, &gbzadmin::on_server_window_activate) );
    connect_signal_to_item ("server_list_window", sigc::mem_fun (*this, &gbzadmin::on_server_list_window_activate) );

    Gtk::CheckMenuItem *check;
    refBuilder->get_widget ("capture", check);
    capture_signal = check->signal_activate().connect (sigc::mem_fun (*this, &gbzadmin::on_capture_activate) );

    // Application events
    app->signal_key_press_event().connect (sigc::mem_fun (*this, &gbzadmin::on_key_press_event), false);
    app->signal_delete_event().connect (sigc::mem_fun (*this, &gbzadmin::on_delete_event) );

    Gtk::Button *button;
    refBuilder->get_widget ("send_button", button);
    button->signal_clicked().connect ( (sigc::mem_fun (*this, &gbzadmin::on_send_button_pressed) ) );

    // dialog response handlers
    refBuilder->get_widget ("connect_dialog", connect_dialog);
    if (connect_dialog) {
        connect_dialog->signal_response().connect (sigc::mem_fun (*this, &gbzadmin::on_connect_dialog_response) );
    }
    // connect dialog server mru combo_changed callback
    server_combo.signal_changed().connect (sigc::mem_fun (*this, &gbzadmin::on_server_combo_changed), false);

    refBuilder->get_widget ("pref_dialog", pref_dialog);
    if (pref_dialog) {
        pref_dialog->signal_response().connect (sigc::mem_fun (*this, &gbzadmin::on_pref_dialog_response) );
    }

    refBuilder->get_widget ("input_dialog", input_dialog);
    if (input_dialog) {
        input_dialog->signal_response().connect (sigc::mem_fun (*this, &gbzadmin::on_input_dialog_response) );
    }

    refBuilder->get_widget ("save_dialog", save_dialog);
    if (save_dialog) {
        save_dialog->signal_response().connect (sigc::mem_fun (*this, &gbzadmin::on_save_dialog_response) );
    }

    refBuilder->get_widget ("flag_reset_dialog", flag_reset_dialog);
    if (flag_reset_dialog) {
        flag_reset_dialog->signal_response().connect (sigc::mem_fun (*this, &gbzadmin::on_flag_reset_dialog_response) );
    }

    refBuilder->get_widget ("lagwarn_dialog", lagwarn_dialog);
    if (lagwarn_dialog) {
        lagwarn_dialog->signal_response().connect (sigc::mem_fun (*this, &gbzadmin::on_lagwarn_dialog_response) );
    }

    // connect the server variable changed signal
    server_vars_view.on_variable_changed.connect (sigc::mem_fun (*this, &gbzadmin::on_variable_changed) );
}

void gbzadmin::on_server_combo_changed()
{
    // update the current server:port string
    current_server = server_combo.get_active_text();
}

void gbzadmin::on_player_window_activate()
{
    Gtk::Notebook *notebook;
    refBuilder->get_widget ("notebook", notebook);
    notebook->set_current_page (0);
}

void gbzadmin::on_server_window_activate()
{
    Gtk::Notebook *notebook;
    refBuilder->get_widget ("notebook", notebook);

    Gtk::Widget *page = notebook->get_nth_page (Page_ServerVars);

    if (!page->is_visible() ) {
        page->show();
        notebook->set_current_page (Page_ServerVars);
    } else {
        notebook->set_current_page (Page_ServerVars);
    }
}

void gbzadmin::on_server_list_window_activate()
{
    Gtk::Notebook *notebook;
    refBuilder->get_widget ("notebook", notebook);

    Gtk::Widget *page = notebook->get_nth_page (Page_ServerList);

    if (!page->is_visible() ) {
        page->show();
        notebook->set_current_page (Page_ServerList);
    } else {
        notebook->set_current_page (Page_ServerList);
    }
}

bool gbzadmin::on_key_press_event (GdkEventKey *event)
{
    switch (event->keyval) {
        default:
            return false;

        case GDK_F6:
        case GDK_slash:
            // grab focus for command widget if not currently focused
            if (!server_vars_view.is_focus() ) {
                if (!cmd.get_focus() ) {
                    cmd.grab_focus();
                }
            }
            break;

        case GDK_Escape:
            // clear the command entry if it has the focus
            if (cmd.get_focus() ) {
                cmd_str.clear();
                cmd.clear();
            }
            break;

        case GDK_F7:
            if (!cmd.get_focus() ) {
                cmd.set_target_admin();
            }
            break;

        case GDK_F8:
            if (!cmd.get_focus() ) {
                cmd.set_target_team();
            }
            break;

        case GDK_F9:
            if (!cmd.get_focus() ) {
                cmd.set_target_all();
            }
            break;

        case GDK_Up: {
            // command history
            if (!cmd.get_focus() ) {
                cmd.grab_focus();
            }
            Gtk::Entry *entry;
            refBuilder->get_widget ("command_entry", entry);
            entry->set_text (cmd.historyUp() );
            // do not pass on to the default handler
            return true;
        }

        case GDK_Down: {
            // command history
            if (!cmd.get_focus() ) {
                cmd.grab_focus();
            }
            Gtk::Entry *entry;
            refBuilder->get_widget ("command_entry", entry);
            entry->set_text (cmd.historyDown() );
            // do not pass on to the default handler
            return true;
        }
    }
    // return 'false' to allow default handler to handle key press events
    return false;
}

void gbzadmin::set_status_message (gint statusbar, const gchar *msg)
{
    push_pop_status (statusbar, msg, true);
    Gdk::flush();
}

void gbzadmin::clear_status_message (gint statusbar)
{
    push_pop_status (statusbar, 0, false);
    Gdk::flush();
}

void gbzadmin::push_pop_status (gint statusbar, const gchar *msg, bool push)
{
    Gtk::Statusbar *status;
    gint id;
    gchar bar[16];

    switch (statusbar) {
        case StatusConnTime:
            g_stpcpy (bar, "statusbar1");
            break;

        case StatusMsgLine:
            g_stpcpy (bar, "statusbar2");
            break;

        case StatusNetStats:
            g_stpcpy (bar, "statusbar3");
            break;

        default:
            break;
    }

    refBuilder->get_widget (bar, status);
    id = status->get_context_id ("msg");
    if (push) {
        status->push (msg, id);
    } else {
        status->pop (id);
    }
}

void gbzadmin::toggle_capture_file()
{
    if (capturing) {	// close the capture file
        msg_view.stop_capture();
        capturing = false;
    } else {
        if (!msg_view.start_capture() ) {
            // if the user cancels the capture dialog
            // the menu item is still checked. Un-checking the item
            // causes this function to be called again and the
            // capture dialog reappears. Need to block the signal
            // before unchecking and unblock afterward.
            capture_signal.block();

            Gtk::CheckMenuItem *check;
            refBuilder->get_widget ("capture", check);
            check->set_active (false);

            capture_signal.unblock();
            capturing = false;
        }
        capturing = true;
    }
}

void gbzadmin::parse_config_file (Glib::ustring filename)
{
    char variable[64];
    char value[1024];
    char buf[2048];
    int result = 0;

    std::ifstream is;
    is.open (filename.c_str(), std::ios::in);
    if (is.is_open() ) {
        do {
            memset (buf, 0, 2048);
            is.getline (buf, 1024);

            result = sscanf (buf, "%64[^#=]=%1024[^\n]\n", variable, value);
            if ( (result < 2) || (result == EOF) ) {
                break;
            }

            // now that we have the variable,value pair we need to assign
            // the value to the class variable

            if (g_ascii_strcasecmp (variable, "callsign") == 0) {
                _callsign = value;
            } else if (g_ascii_strcasecmp (variable, "motto") == 0) {
                _motto = value;
            } else if (g_ascii_strcasecmp (variable, "password") == 0) {
                _password = value;
            } else if (g_ascii_strcasecmp (variable, "server") == 0) {
                _server = value;
            } else if (g_ascii_strcasecmp (variable, "window_x") == 0) {
                win_x = atoi (value);
            } else if (g_ascii_strcasecmp (variable, "window_y") == 0) {
                win_y = atoi (value);
            } else if (g_ascii_strcasecmp (variable, "window_width") == 0) {
                win_width = atoi (value);
            } else if (g_ascii_strcasecmp (variable, "window_height") == 0) {
                win_height = atoi (value);
            } else if (g_ascii_strcasecmp (variable, "msg_pane") == 0) {
                msg_pane = atoi (value);
            } else if (g_ascii_strcasecmp (variable, "game_pane") == 0) {
                game_pane = atoi (value);
            } else if (g_ascii_strcasecmp (variable, "port") == 0) {
                _port_str = value;
                _port = atoi (_port_str.c_str() );
            } else if (g_ascii_strcasecmp (variable, "auto_cmd") == 0) {
                auto_cmd = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "small_toolbar") == 0) {
                small_toolbar = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "echo_pings") == 0) {
                echo_pings = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "dump_players") == 0) {
                dump_players = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "line_numbers") == 0) {
                line_numbers = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "save_password") == 0) {
                save_password = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "server_mru") == 0) {
                Glib::ustring str (value);
                server_mru_str = parse_server_mru (str, ";", maxServersList);
            } else if (g_ascii_strcasecmp (variable, "current_server") == 0) {
                current_server = value;
            } else if (g_ascii_strcasecmp (variable, "prefetch") == 0) {
                sock.set_prefetch (atoi (value) ? true : false);
#ifndef USING_GIO_NETWORK
            } else if (g_ascii_strcasecmp (variable, "blocking") == 0) {
                sock.set_blocking (atoi (value) ? true : false);
#endif
            } else if (g_ascii_strcasecmp (variable, "msg_new_rabbit") == 0) {
                msg_mask["rabbit"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_pause") == 0) {
                msg_mask["pause"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_alive") == 0) {
                msg_mask["spawn"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_lag_ping") == 0) {
                msg_mask["ping"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_set_var") == 0) {
                msg_mask["bzdb"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_add_player") == 0) {
                msg_mask["join"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_remove_player") == 0) {
                msg_mask["leave"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_admin_info") == 0) {
                msg_mask["admin"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_score") == 0) {
                msg_mask["score"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_time_stamp") == 0) {
                msg_mask["time"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_killed") == 0) {
                msg_mask["kill"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_message") == 0) {
                msg_mask["chat"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_roger") == 0) {
                msg_mask["roger"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_flags") == 0) {
                msg_mask["flags"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_teleport") == 0) {
                msg_mask["teleport"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_time_update") == 0) {
                msg_mask["tupdate"] = atoi (value) ? true : false;
            } else if (g_ascii_strcasecmp (variable, "msg_game_time") == 0) {
                msg_mask["gtime"] = atoi (value) ? true : false;
            } else {
                continue;
            }
        } while (result != EOF);
        is.close();
    } else {
        std::cerr << "File could not be opened\n";
    }
}

//
// configuration preferences are preserved by saving to a file
// called ".gbzadmin" in the user's home directory
//
void gbzadmin::save_config_file (Glib::ustring filename)
{
    Glib::ustring buf;
    std::ofstream os;

    os.open (filename.c_str(), std::ios::out | std::ios::trunc);
    if (os.is_open() ) {
        if (!_callsign.empty() ) {
            buf = Glib::ustring::compose ("callsign=%1\n", _callsign);
            os.write (buf.c_str(), buf.length() );
        }
        if (!_password.empty() && save_password) {
            buf = Glib::ustring::compose ("password=%1\n", _password);
            os.write (buf.c_str(), buf.length() );
        }
        if (!_motto.empty() ) {
            buf = Glib::ustring::compose ("motto=%1\n", _motto);
            os.write (buf.c_str(), buf.length() );
        }
        if (!_server.empty() ) {
            buf = Glib::ustring::compose ("server=%1\n", _server);
            os.write (buf.c_str(), buf.length() );
        }
        if (!_port_str.empty() ) {
            buf = Glib::ustring::compose ("port=%1\n", _port_str);
            os.write (buf.c_str(), buf.length() );
        }

        // write the server list MRU
        int idx = 0;
        std::list<Glib::ustring>::iterator it = server_mru_str.begin();
        while (it != server_mru_str.end() ) {
            if (it == server_mru_str.begin() ) {
                buf = Glib::ustring::compose ("server_mru=%1", *it);
                os.write (buf.c_str(), buf.length() );
            } else {
                buf = Glib::ustring::compose (";%1", *it);
                os.write (buf.c_str(), buf.length() );
            }
            it++;
            idx++;
            if (idx >= maxServersList) {
                os.write ("\n", 1); // add newline to end of server mru string
                break;
            }
        }

        buf = Glib::ustring::compose ("current_server=%1\n", current_server);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("prefetch=%1\n", sock.get_prefetch() );
        os.write (buf.c_str(), buf.length() );
#ifndef USING_GIO_NETWORK
        buf = Glib::ustring::compose ("blocking=%1\n", sock.get_blocking() );
        os.write (buf.c_str(), buf.length() );
#endif
        buf = Glib::ustring::compose ("window_x=%1\n", win_x);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("window_y=%1\n", win_y);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("window_height=%1\n", win_height);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("window_width=%1\n", win_width);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_pane=%1\n", msg_pane);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("game_pane=%1\n", game_pane);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("auto_cmd=%1\n", auto_cmd);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("small_toolbar=%1\n", small_toolbar);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("echo_pings=%1\n", echo_pings);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("dump_players=%1\n", dump_players);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("line_numbers=%1\n", line_numbers);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("save_password=%1\n", save_password);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_new_rabbit=%1\n", msg_mask["rabbit"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_pause=%1\n", msg_mask["pause"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_alive=%1\n", msg_mask["spawn"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_lag_ping=%1\n", msg_mask["ping"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_set_var=%1\n", msg_mask["bzdb"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_add_player=%1\n", msg_mask["join"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_remove_player=%1\n", msg_mask["leave"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_admin_info=%1\n", msg_mask["admin"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_score=%1\n", msg_mask["score"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_time_stamp=%1\n", msg_mask["time"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_killed=%1\n", msg_mask["kill"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_message=%1\n", msg_mask["chat"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_roger=%1\n", msg_mask["roger"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_flags=%1\n", msg_mask["flags"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_teleport=%1\n", msg_mask["teleport"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_time_update=%1\n", msg_mask["tupdate"]);
        os.write (buf.c_str(), buf.length() );

        buf = Glib::ustring::compose ("msg_game_time=%1\n", msg_mask["gtime"]);
        os.write (buf.c_str(), buf.length() );

        os.close();
    } else {
        std::cerr << "File could not be opened\n";
    }
}

void gbzadmin::set_message_filter (Glib::ustring type, bool set)
{
    bool tmp = msg_mask[type];
    if (set) {
        msg_mask[type] = true;
        if (tmp != msg_mask[type]) {
            cmd_str = "/show " + type;
        }
    } else {
        msg_mask[type] = false;
        if (tmp != msg_mask[type]) {
            cmd_str = "/hide " + type;
        }
    }
}

Glib::ustring gbzadmin::get_team_str (int t)
{
    if (t < 0 || t > 5) {
        return Glib::ustring ("unknown");
    }

    const char* teams[] = {
        "Rogue",
        "Red",
        "Green",
        "Blue",
        "Purple",
        "Observer"
    };

    Glib::ustring str;
    str = teams[t];
    str += " Team ";

    return str;
}

///////////////////////////////////////////////////////////////////////////////
// message handler functions
//
void gbzadmin::handle_rabbit_message (void *vbuf)
{
    guint8 p;
    vbuf = parser.nboUnpackUByte (vbuf, &p);

    Player* player = player_view.find_player (p);

    if (player) {
        player->set_team (RabbitTeam);
        player_view.change_all_to_hunter_except (p);
        if (msg_mask["rabbit"]) {
            Glib::ustring str ("*** ");
            str += colorize (player) + player->get_callsign() + defColor + " is now the rabbit\n";
            msg_view.add_text (str);
        }
        player_view.update();
    }
}

void gbzadmin::handle_pause_message (void *vbuf)
{
    guint8 p;
    guint8 paused;

    vbuf = parser.nboUnpackUByte (vbuf, &p);
    vbuf = parser.nboUnpackUByte (vbuf, &paused);

    Player* player = player_view.find_player (p);
    if (player) {
        player->set_paused (paused);

        if (msg_mask["pause"]) {
            Glib::ustring str ("*** ");
            str += colorize (player) + player->get_callsign() + defColor + " is " + (paused ? "paused" : "resumed") + "\n";
            msg_view.add_text (str);
        }
        player_view.update();
    }
}

void gbzadmin::handle_alive_message (void *vbuf)
{
    guint8 p;
    vbuf = parser.nboUnpackUByte (vbuf, &p);

    Player *player = player_view.find_player (p);
    if (player) {
        player->set_alive (true);

        if (msg_mask["spawn"]) {
            float pos[3], forward;
            vbuf = parser.nboUnpackVector (vbuf, &pos[0]);
            vbuf = parser.nboUnpackFloat (vbuf, &forward);
            Glib::ustring str ("*** ");
            str += colorize (player) + player->get_callsign() + defColor;
            str += Glib::ustring::compose (" has spawned at [%1:%2:%3] (%4)\n", pos[0], pos[1], pos[2], forward);
            msg_view.add_text (str);
        }
    }
    player_view.update();
}

void gbzadmin::handle_add_player_message (void *vbuf)
{
    guint8 p;
    guint16 team, type, wins, losses, tks;
    gchar callsign[MaxPacketLen];
    gchar motto[MaxPacketLen];

    vbuf = parser.nboUnpackUByte (vbuf, &p);
    vbuf = parser.nboUnpackUShort (vbuf, &type);
    vbuf = parser.nboUnpackUShort (vbuf, &team);
    vbuf = parser.nboUnpackUShort (vbuf, &wins);
    vbuf = parser.nboUnpackUShort (vbuf, &losses);
    vbuf = parser.nboUnpackUShort (vbuf, &tks);
    vbuf = parser.nboUnpackString (vbuf, callsign, CallSignLen);
    vbuf = parser.nboUnpackString (vbuf, motto, MottoLen);

    callsign[CallSignLen] = '\0';
    motto[MottoLen] = '\0';

    // timestamp the arrival
    time_t now;
    now = time (0);
    struct tm *ts = localtime (&now);
    char time_str[128];
    strftime (time_str, 128, " (%T)", ts);

    if (player_view.get_rabbit_mode() && (team == RogueTeam) ) {
        team = HunterTeam;
    }

    // check for duplicate MsgAddPlayer
    if (player_view.find_player (p) ) {
        return;
    }

    // add the player to the target list
    Player *player = new Player;
    player->add (p, type, team, wins, losses, tks, Glib::ustring (callsign), Glib::ustring (motto) );
    player_view.add (player);
    cmd.add_target (p, Glib::ustring (callsign) );

    // checking all known flags for owners
    Glib::ustring flag = flag_has_owner (p);
    if (flag.size() ) {
        player->set_flag (flag);
    }

    // If you are an admin, then MsgAdminInfo will output the message
    me = sock.getId();
    player = player_view.find_player (me);
    if (player) {
        bool i_am_admin = player->get_admin();
        if (msg_mask["joined"] && !i_am_admin && (player->get_id() != me) ) {
            Glib::ustring str (msg_view.Color (GreenFg) );
            str += "*** " + msg_view.Color (WhiteFg);
            str += colorize (player) + Glib::ustring (callsign) + defColor + " joined the " + get_team_str (team);
            str += (msg_mask["time"] ? Glib::ustring (time_str) : "");
            str += "\n";
            msg_view.add_text (str);
        }
    }
    player_view.update();
    sock.send (MsgQueryGame, 2, vbuf);
}

void gbzadmin::handle_remove_player_message (void *vbuf)
{
    guint8 p;

    vbuf = parser.nboUnpackUByte (vbuf, &p);
    Player *player = player_view.find_player (p);

    if (dump_players) {
        if (player) {
            dump_player_stats (player);
        }
    }

    if (msg_mask["leave"]) {
        if (player) {
            // get current time
            time_t now;
            now = time (0);
            struct tm *ts0 = localtime (&now);
            gchar time_str[128];
            strftime (time_str, 64, "(%T)", ts0);
            // estimate time on the server
            double diff = player->duration (now);
            unsigned long ldiff = (unsigned long) diff;
            guint hours = (ldiff / 3600);
            guint minutes = (ldiff / 60) - (hours * 60);
            guint seconds = ldiff % 60;
            gchar dt[64];
            ::snprintf (dt, sizeof (dt), " duration: %02d:%02d:%02d", hours, minutes, seconds);
            g_strlcat (time_str, dt, sizeof (time_str) );
            Glib::ustring str (msg_view.Color (RedFg) );
            str += "*** " + msg_view.Color (WhiteFg);
            if (player->get_team() != ObserverTeam) {
                str += colorize (player) + player->get_callsign() + defColor + " left the game, final score: ";
                str += Glib::ustring::compose ("%1 ", player->get_score() );
                str += (msg_mask["time"] ? Glib::ustring (time_str) : "");
                str += "\n";
            } else {
                str += colorize (player) + player->get_callsign() + defColor + " left the game ";
                str += (msg_mask["time"] ? Glib::ustring (time_str) : "");
                str += "\n";
            }
            msg_view.add_text (str);
        }
    }
    cmd.remove_target (p, player->get_callsign() );
    player_view.remove (player);
    player_view.update();
    sock.send (MsgQueryGame, 2, vbuf);
}

void gbzadmin::handle_playerinfo_message (void *vbuf)
{
    guint8 numPlayers;
    guint8 p;

    vbuf = parser.nboUnpackUByte (vbuf, &numPlayers);

    for (int k = 0; k < numPlayers; ++k) {
        vbuf = parser.nboUnpackUByte (vbuf, &p);
        guint8 info;

        vbuf = parser.nboUnpackUByte (vbuf, &info);
        Player *player = player_view.find_player (p);
        if (player) {
            player->set_status (info);
        }
    }
    player_view.update();
}

void gbzadmin::handle_admininfo_message (void* vbuf)
{
    guint8 p;
    guint8 numIPs, ipsize;
    struct in_addr addr;
    addr.s_addr = htonl (INADDR_ANY);
    Player *player = 0;

    // if you get this you are an admin
    enable_admin_items (true);

    vbuf = parser.nboUnpackUByte (vbuf, &numIPs);
    // Alternative to MsgAddPlayer
    if (numIPs == 1) {
        void *tmpbuf = vbuf;
        tmpbuf = parser.nboUnpackUByte (tmpbuf, &ipsize);
        tmpbuf = parser.nboUnpackUByte (tmpbuf, &p);
        tmpbuf = parser.unpack_address (tmpbuf, &addr);
        player = player_view.find_player (p);
        if (player) {
            player->set_IP (addr);
            if (msg_mask["admin"] && (player->get_id() != sock.getId() ) ) {
                time_t tm;
                tm = time (0);
                struct tm *tim = localtime (&tm);
                char time_str[128];
                strftime (time_str, sizeof (time_str), " (%H:%M:%S)", tim);

                Glib::ustring str (msg_view.Color (GreenFg) );
                str += "*** " + msg_view.Color (WhiteFg);
                str += player->get_callsign() + " joined the " + get_team_str (player->get_team() );
                str += "from " + player->get_IP();
                str += (msg_mask["time"] ? Glib::ustring (time_str) : "");
                str += "\n";
                msg_view.add_text (str);
            }
        }
    }
    if (numIPs != 1) {
        for (int k = 0; k < numIPs; k++) {
            vbuf = parser.nboUnpackUByte (vbuf, &ipsize);
            vbuf = parser.nboUnpackUByte (vbuf, &p);
            vbuf = parser.unpack_address (vbuf, &addr);
            player = player_view.find_player (p);
            if (player) {
                player->set_IP (addr);
                if (msg_mask["admin"] && (player->get_id() != sock.getId() ) ) {
                    Glib::ustring str (msg_view.Color (YellowFg) );
                    Glib::ustring ipstr = player->get_IP();
                    Glib::ustring color = msg_view.get_color (player->get_team() );
                    Glib::ustring callsign = player->get_callsign();
                    str += "*** IPINFO: " + color + callsign + msg_view.Color (CyanFg);
                    for (int i = 0; i < (CallSignLen - (int) callsign.size() ); i++) {
                        str += " ";
                    }
                    str += "From: " + color + ipstr;
                    for (int i = 0; i < (17 - (int) ipstr.size() ); i++) {
                        str += " ";
                    }
                    str += msg_view.Color (WhiteFg) + "(join)\n";
                    msg_view.add_text (str);
                }
            }
        }
    }
    player_view.update();
}

void gbzadmin::handle_killed_message (void *vbuf)
{
    guchar victim, killer;
    gint16 shotId, reason;
    gchar flag_abbrv[3];
    gint k_team, v_team;

    vbuf = parser.nboUnpackUByte (vbuf, &victim);
    vbuf = parser.nboUnpackUByte (vbuf, &killer);
    vbuf = parser.nboUnpackShort (vbuf, &reason);
    vbuf = parser.nboUnpackShort (vbuf, &shotId);
    vbuf = parser.unpack_flag (vbuf, (guchar*) flag_abbrv);
    Glib::ustring flag (flag_abbrv);

    Player *player = 0;

    Glib::ustring killerName ("the server (phydrv)");
    Glib::ustring killerColor;
    if (reason == PhysicsDriverDeath) {
        // killed by a physics driver (server)
        k_team = NoTeam;
    } else {
        // find the player names and build a kill message string
        // killer first...
        player = player_view.find_player (killer);
        k_team = player ? player->get_team() : NoTeam;
        killerName = (player ? player->get_callsign() : "destroyed by the server");
        killerColor = colorize (player); //msg_view.get_color(player->get_team());
    }
    // now the victim
    player = player_view.find_player (victim);
    player->set_alive (false);
    player->clear_flag();
    if (msg_mask["kill"]) {
        v_team = player ? player->get_team() : NoTeam;
        Glib::ustring victimName;
        victimName = (player ? player->get_callsign() : "<unknown victim>");

        Glib::ustring str (msg_view.Color (YellowFg) );
        str += msg_view.colorBullet() + colorize (player) + victimName + " " + defColor;

        Glib::ustring killer_str;
        if ( (k_team == v_team) && (k_team != RogueTeam) && (game_type != OpenFFA) ) {
            killer_str = "teammate ";
            killer_str += killerColor + killerName + msg_view.Color (WhiteFg);
        } else if (killer != victim) {
            killer_str = killerColor + killerName + msg_view.Color (WhiteFg);
        }
        if ( (killer == victim) && (k_team != NoTeam) ) {
            str += "committed suicide!\n";
            msg_view.add_text (str);
            return;
        } else { // informative death message
            if (flag == "L") {
                str += "was fried by " + killer_str + "'s laser";
            } else if (flag == "GM") {
                str += "destroyed by " + killer_str + "'s guided missile";
            } else if (flag == "SW") {
                str += "felt the effects of " + killer_str + "'s shock wave";
            } else if (flag == "IB") {
                str += "didn't see " + killer_str + "'s bullet";
            } else if (flag == "MG") {
                str += "was turned into swiss cheese by " + killer_str + "'s machine gun";
            } else if (flag == "SB") {
                str += "got skewered by " + killer_str + "'s super bullet";
            } else {
                str += "destroyed by " + killer_str;
            }
        }
        str += "\n";
        msg_view.add_text (str);
    }
    player_view.update();
}

void gbzadmin::handle_score_message (void *vbuf)
{
    guint8 numScores;
    guint8 p;
    Player *player = 0;

    vbuf = parser.nboUnpackUByte (vbuf, &numScores);
    for (int k = 0; k < numScores; k++) {
        guint16 wins, losses, tks;
        vbuf = parser.nboUnpackUByte (vbuf, &p);
        vbuf = parser.nboUnpackUShort (vbuf, &wins);
        vbuf = parser.nboUnpackUShort (vbuf, &losses);
        vbuf = parser.nboUnpackUShort (vbuf, &tks);
        player = player_view.find_player (p);
        if (player) {
            player->set_score (wins, losses);
            player->set_tks (tks);
        }
    }
    if (msg_mask["score"]) {
        Glib::ustring str = Glib::ustring::compose ("*** Received %1 score update(s)\n", numScores);
        msg_view.add_text (str, Glib::ustring ("default") );
    }
    player_view.update();
}

void gbzadmin::handle_autopilot_message (void *vbuf)
{
    guchar p;
    guint8 autopilot;
    Player *player = 0;

    vbuf = parser.nboUnpackUByte (vbuf, &p);
    vbuf = parser.nboUnpackUByte (vbuf, &autopilot);
    player = player_view.find_player (p);
    if (player) {
        player->set_autoPilot (autopilot);
        player_view.update();
        if (msg_mask["roger"]) {
            Glib::ustring str ("*** Roger is ");
            str += (autopilot ? "taking the controls for " : "releasing the controls back to "
                    + colorize (player) + player->get_callsign() );
            str += "\n";
            msg_view.add_text (str);
        }
    }
}
void gbzadmin::handle_grabflag_message (void *vbuf)
{
    guint8 p;
    guint16 flag_idx;
    Player *player = 0;
    flag_info fi;

    vbuf = parser.nboUnpackUByte (vbuf, &p);
    vbuf = parser.nboUnpackUShort (vbuf, &flag_idx);
    vbuf = parser.unpack_flag_info (vbuf, &fi);
    player = player_view.find_player (fi.owner);
    Glib::ustring flag ( (gchar*) fi.type);
    if (player) {
        player->set_flag (flag);
        player_view.update();
        if (msg_mask["flags"]) {
            Glib::ustring buffer ("*** ");
            buffer += Glib::ustring::compose ("%1 has picked up the %2 flag\n",
                                              player->get_callsign(), flag.size() ? flag : "<unknown>");

            msg_view.add_text (buffer, Glib::ustring ("default") );
        }
    }
}

void gbzadmin::handle_dropflag_message (void *vbuf)
{
    guint8 p;
    guint16 flag_idx;
    Player *player = 0;
    flag_info fi;

    vbuf = parser.nboUnpackUByte (vbuf, &p);
    vbuf = parser.nboUnpackUShort (vbuf, &flag_idx);
    vbuf = parser.unpack_flag_info (vbuf, &fi);
    fi.idx = flag_idx;
    player = player_view.find_player (fi.owner);
    Glib::ustring flag ( (gchar*) fi.type);
    if (player) {
        player->clear_flag();
        player_view.update();
        if (msg_mask["flags"]) {
            Glib::ustring buffer ("*** ");
            buffer += Glib::ustring::compose ("%1 has dropped the %2 flag\n",
                                              player->get_callsign(), flag.size() ? flag : "<unknown>");

            msg_view.add_text (buffer, Glib::ustring ("default") );
        }
    }
}

void gbzadmin::handle_flagupdate_message (void *vbuf)
{
    guint16 numFlags;
    guint16 flag_idx;
    flag_info *fi;
    Player *player = 0;

    vbuf = parser.nboUnpackUShort (vbuf, &numFlags);
    for (int k = 0; k < numFlags; k++) {
        vbuf = parser.nboUnpackUShort (vbuf, &flag_idx);
        fi = g_new0 (flag_info, 1);
        vbuf = parser.unpack_flag_info (vbuf, fi);
        fi->idx = flag_idx;
        if (fi->status != Parser::FlagNoExist && fi->status != Parser::FlagGoing) {
            add_flag (fi);
        } else if (fi->status == Parser::FlagNoExist || fi->status == Parser::FlagGoing) {
            remove_flag (fi);
        }
        player = player_view.find_player (fi->owner);
        if (player && fi->status == Parser::FlagOnTank) {
            player->set_flag (Glib::ustring ( (gchar*) fi->type) );
            player_view.update();
        }
        player = 0;
    }
}

void gbzadmin::handle_transferflag_message (void *vbuf)
{
    guint8 from_id, to_id;
    guint16 flag_idx;
    flag_info fi;

    vbuf = parser.nboUnpackUByte (vbuf, &from_id);
    vbuf = parser.nboUnpackUByte (vbuf, &to_id);
    vbuf = parser.nboUnpackUShort (vbuf, &flag_idx);
    vbuf = parser.unpack_flag_info (vbuf, &fi);

    fi.idx = flag_idx;
    if (msg_mask["flags"]) {
        Glib::ustring flag ( (gchar*) fi.type);
        Player *from = player_view.find_player (from_id);
        Player *to = player_view.find_player (to_id);
        from->clear_flag();
        to->set_flag (flag);
        Glib::ustring buffer ("*** ");
        Glib::ustring callsign_from (from->get_callsign() );
        Glib::ustring callsign_to (to->get_callsign() );

        buffer += Glib::ustring::compose ("%1 stole %2's flag\n",	callsign_to, callsign_from);
        msg_view.add_text (buffer, Glib::ustring ("default") );
    }
}

void gbzadmin::handle_message_message (void *vbuf)
{
    guint8 src, dst, type;
    Player *src_player, *dst_player;
    gint16 src_team = NoTeam;

    vbuf = parser.nboUnpackUByte (vbuf, &src);
    vbuf = parser.nboUnpackUByte (vbuf, &dst);
    vbuf = parser.nboUnpackUByte (vbuf, &type);

    // process the message if it's chat or action
    if (MessageType (type) != ChatMessage && MessageType (type) != ActionMessage) {
        return;
    }

    // format the message depending on source and destination
    TeamColor dstTeam = PlayerIdToTeam (dst);

    if (msg_mask["chat"]) {
        src_player = player_view.find_player (src);
        dst_player = player_view.find_player (dst);
        Glib::ustring src_callsign ("");
        Glib::ustring dst_callsign ("");
        if (src_player) {
            src_callsign = src_player->get_callsign();
            src_team = src_player->get_team();
        }
        if (dst_player) {
            dst_callsign = dst_player->get_callsign();
        }

        Glib::ustring str (msg_view.Color (OrangeFg) );
        Glib::ustring formatted;
        str += ">>> ";
        str += (src_team == NoTeam) ? msg_view.Color (GoldenFg) : msg_view.get_color (src_team);
        msg_view.format (formatted, Glib::ustring ( (gchar*) vbuf), MessageType (type), src, dst, dstTeam, me, src_callsign, dst_callsign);
        str += formatted;

        str = Glib::convert_with_fallback (str.c_str(), "UTF-8", "ISO-8859-1");
        msg_view.add_text (str);
    }
}

void gbzadmin::handle_setvar_message (void *vbuf)
{
    guint16 numVars;
    guint8 nameLen, valueLen;
    char name[MaxPacketLen];
    char value[MaxPacketLen];

    vbuf = parser.nboUnpackUShort (vbuf, &numVars);

    for (int k = 0; k < numVars; k++) {
        vbuf = parser.nboUnpackUByte (vbuf, &nameLen);
        vbuf = parser.nboUnpackString (vbuf, name, nameLen);
        name[nameLen] = '\0';

        vbuf = parser.nboUnpackUByte (vbuf, &valueLen);
        vbuf = parser.nboUnpackString (vbuf, value, valueLen);
        value[valueLen] = '\0';

        server_vars_view.update (Glib::ustring (name), Glib::ustring (value) );
    }
    if (msg_mask["bzdb"]) {
        Glib::ustring str = Glib::ustring::compose ("*** Received %1 BZDB update(s)\n", numVars);
        msg_view.add_text (str, Glib::ustring ("default") );
    }
}

void gbzadmin::handle_ping_message (void *vbuf)
{
    if (echo_pings) {
        // echo PING msg - this will show true lag eventually
        sock.sendLagPing ( (char *) vbuf);
    }
    // since we are not using seqno, unpack
    // this only when we want to display this msg
    if (msg_mask["ping"]) {
        guint16 seqno;
        parser.nboUnpackUShort (vbuf, &seqno);
        Glib::ustring str = Glib::ustring::compose ("*** Received lag ping from server (%1)\n", seqno);
        msg_view.add_text (str, Glib::ustring ("server") );
    }
    // reset the watchdog counter with ever server ping
    wd_counter = MaxWdTime;
}

void gbzadmin::handle_game_query_message (void *vbuf)
{
    guint16 tmp;
    guint16 tmpRogueSize, tmpRedSize, tmpGreenSize;
    guint16 tmpBlueSize, tmpPurpleSize;

    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setType (tmp);
    game_type = tmp;
    if (tmp == RabbitChase) {
        player_view.set_rabbit_mode (true);
    }

    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setOptions (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setMaxPlayers (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setMaxShots (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmpRogueSize);
    game_view.setRogueSize (tmpRogueSize);
    vbuf = parser.nboUnpackUShort (vbuf, &tmpRedSize);
    game_view.setRedSize (tmpRedSize);
    vbuf = parser.nboUnpackUShort (vbuf, &tmpGreenSize);
    game_view.setGreenSize (tmpGreenSize);
    vbuf = parser.nboUnpackUShort (vbuf, &tmpBlueSize);
    game_view.setBlueSize (tmpBlueSize);
    vbuf = parser.nboUnpackUShort (vbuf, &tmpPurpleSize);
    game_view.setPurpleSize (tmpPurpleSize);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setObsSize (tmp);
    game_view.setTotalObservers (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setRogueMax (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setRedMax (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setGreenMax (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setBlueMax (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setPurpleMax (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setObsMax (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setShakeWins (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setShakeTimeOut (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setMaxPlayerScore (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setMaxTeamScore (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setMaxTime (tmp);
    vbuf = parser.nboUnpackUShort (vbuf, &tmp);
    game_view.setTimeElapsed (tmp);

    game_view.setTotalPlayers (tmpRogueSize + tmpRedSize + tmpGreenSize + tmpBlueSize +	tmpPurpleSize);

    game_view.update();
}

void gbzadmin::handle_teamupdate_message (void *vbuf)
{
    guint8 numTeams;

    vbuf = parser.nboUnpackUByte (vbuf, &numTeams);
    if (numTeams > 5) {
        return;
    }
    guint16 tmp;
    for (int k = 0; k < numTeams; k++) {
        vbuf = parser.nboUnpackUShort (vbuf, &tmp);
        game_view.setTeam (k, tmp);
        vbuf = parser.nboUnpackUShort (vbuf, &tmp); // team_size[team[k]]);
        game_view.setTeamSize (k, tmp);
        vbuf = parser.nboUnpackUShort (vbuf, &tmp); // team_wins[team[k]]);
        game_view.setTeamWins (k, tmp);
        vbuf = parser.nboUnpackUShort (vbuf, &tmp); // team_losses[team[k]]);
        game_view.setTeamLosses (k, tmp);
    }
    game_view.update();
}

void gbzadmin::handle_teleport_message (void *vbuf)
{
    guint8 id;
    guint16 from, to;

    if (msg_mask["teleport"]) {
        vbuf = parser.nboUnpackUByte (vbuf, &id);
        vbuf = parser.nboUnpackUShort (vbuf, &from);
        vbuf = parser.nboUnpackUShort (vbuf, &to);
        Player *player = player_view.find_player (id);

        Glib::ustring str (msg_view.Color (PurpleFg) );
        str += "*** ";
        str += colorize (player);
        str += player->get_callsign();
        str += msg_view.Color (WhiteFg) + " has teleported!";
        str += msg_view.Color (RedFg);
        str += Glib::ustring::compose (" (%1 ==> %2)", from, to);
        str += "\n";
        msg_view.add_text (str);
    }
}

void gbzadmin::handle_time_update_message (void *vbuf)
{
    if (msg_mask["tupdate"]) {
        gint32 timeLeft;
        vbuf = parser.nboUnpackInt (vbuf, &timeLeft);
        Glib::ustring str (msg_view.Color (OrangeFg) );
        str += ">>> ";
        str += msg_view.Color (RedFg);

        if (timeLeft == 0) {
            str += "Time Expired";
            msg_view.add_text (str);
        } else if (timeLeft < 0) {
            str += "Game Paused";
            msg_view.add_text (str);
        } else {
            str += "Time: ";
            str += msg_view.Color (BlueFg);
            str += Glib::ustring::compose ("%1\n", timeLeft);
            msg_view.add_text (str);
        }
    }
}

void gbzadmin::handle_game_time_message (void *vbuf)
{
    if (msg_mask["gtime"]) {
        guint32 msb, lsb;
        vbuf = parser.nboUnpackUInt (vbuf, &msb);
        vbuf = parser.nboUnpackUInt (vbuf, &lsb);
        const gint64 netTime = ( (gint64) msb << 32) + (gint64) lsb;
        Glib::ustring str (msg_view.Color (OrangeFg) );
        str += ">>> ";
        str += msg_view.Color (RedFg);
        str += "Game Time: ";
        str += msg_view.Color (BlueFg);
        str += Glib::ustring::compose ("%1\n", netTime);
        msg_view.add_text (str);
    }
}

Glib::ustring gbzadmin::colorize (Player *player)
{
    return msg_view.get_color (player->get_team() );
}


// Pending data signal handler - an alternative to the idle loop
// message processing. This one uses signals. This
// handler will get called by the TCP signal.
bool gbzadmin::on_read_data (Glib::IOCondition io_condition)
{
    gint result = MsgNull;
    if ( ( (io_condition & Glib::IO_IN) != 0) || ( (io_condition & Glib::IO_PRI) != 0) ) {
        result = get_message();
    } else if ( (io_condition & Glib::IO_ERR) != 0) {
        return true;
    } else if ( (io_condition & Glib::IO_HUP) != 0) {
        // connection hung up
        return false;
    } else if ( (io_condition & Glib::IO_NVAL) != 0) {
        // invaid fd
        return false;
    }
    check_errors (result);

    return true;
}

gint gbzadmin::get_message()
{
    if (!isConnected() || (wd_counter <= 0) ) {
        return CommError;
    }

    guint16 code, len;
    gchar inbuf[MaxPacketLen];

    if (sock.read (code, len, (gchar*) inbuf, BlockTime) == 1) {
        void *vbuf = inbuf;

        if (code == MsgSuperKill) {
            return Superkilled;
        }

        // find and execute the appropriate message handler
        msg_handler_map::const_iterator iter = handler_map.find (code);
        if (iter != handler_map.end() ) {
            messagehandler handler = iter->second;
            ( (this)->*handler) (vbuf);
        } else {
            // print unhandled codes, we may want to monitor some of them
            if (debugMsgs) {
                print_message_code (code);
            }
            return NoMessage;
        }
        return GotMessage;
    }

    return NoMessage;
}

void gbzadmin::check_errors (int result)
{
    Glib::ustring error (msg_view.Color (RedFg) );
    time_t now;
    now = time (0);
    struct tm *ts = localtime (&now);
    char time_str[128];

    switch (result) {
        case Superkilled:
            error += "--- ERROR: Server forced disconnect " + msg_view.Color (YellowFg) + "(Superkilled)\n";
            msg_view.add_text (error);
            app->set_title (windowTitle);
            sock.disconnect();
            clean_up (false);
            break;

        case CommError:
            strftime (time_str, 128, " (%T)", ts);

            if (wd_counter <= 0) {
                error += "--- ERROR: Connection to server lost " + msg_view.Color (YellowFg) + "(WatchDog)";
            } else {
                error += "--- ERROR: Connection to server lost " + msg_view.Color (YellowFg) + "(CommError)";
            }
            error += " at";
            error += time_str;
            error += "\n";

            msg_view.add_text (error);
            app->set_title (windowTitle);
            sock.disconnect();
            clean_up (false);
            break;

        default:
            // nothing to see here folks, move along...
            break;
    }
}

// diagnostic function
void gbzadmin::print_message_code (guint16 code)
{
    if (code != MsgShotBegin && code != MsgShotEnd && code != MsgPlayerUpdateSmall && code != MsgGMUpdate) {
        Glib::ustring msg (msg_view.Color (GoldenFg) );
        msg += "###";
        msg += msg_view.Color (GreenFg);
        msg += " MSG DEBUG: Received ";
        msg += msg_view.Color (RedFg);
        msg += Glib::ustring::compose ("(0X%1)", Glib::ustring::format (std::hex, std::setw (4), code) );
        msg += msg_view.Color (GreenFg);
        msg += " message code\n";
        msg_view.add_text (msg);
    }
}

void gbzadmin::logon()
{
    set_status_message (StatusConnTime, "Connecting...");
    while (Gtk::Main::events_pending() ) { // update the GUI before the connect phase
        Gtk::Main::iteration();
    }
    if (sock.connect (_server, _port) ) {
        if (sock.join (_callsign, _password, _motto) ) {
            // display some server info
            Glib::ustring id_str = Glib::ustring::compose ("%1", sock.getId() );
            Glib::ustring str (msg_view.Color (GoldenFg) );
            str += "--- Server version: " + msg_view.Color (YellowFg) + sock.getVersion() + "\n";
            str += msg_view.Color (GoldenFg) + "--- My player ID: " + msg_view.Color (RedFg) + id_str + "\n";
            str += msg_view.Color (GoldenFg) + "--- Connected to " + msg_view.Color (RedFg) + _server + ":";
            str += msg_view.Color (BlueFg) + _port_str + msg_view.Color (YellowFg) + " (" + sock.get_server_IP() + ")\n";
            str += msg_view.Color (GoldenFg) + "--- ";
            str += msg_view.Color (GreenFg) + _server + " has ACCEPTED me\n";
            str += msg_view.Color (BlueFg) + "--- " + getAppVersion() + "\n";
            msg_view.add_text (str);

            // connect the TCP data pending signal from the socket object
            tcp_data_pending = sock.on_tcp_data_pending.connect (sigc::mem_fun (*this, &gbzadmin::on_read_data) );
            // start the connection timer
            conn_timer = Glib::signal_timeout().connect_seconds (sigc::mem_fun (*this, &gbzadmin::update_timer), 1);
            connection_timer.reset();
            // start the watchdog timer
            wd_timer = Glib::signal_timeout().connect_seconds (sigc::mem_fun (*this, &gbzadmin::update_watchdog), 1);
            watchdog_timer.reset();
            // add callsign@server:port to the title
            app->set_title (Glib::ustring (Glib::ustring (windowTitle) + " - " + _callsign + "@" + _server + ":" + _port_str) );
            // enable the connected only items
            enable_connected_items (true);
        } else {	// failed to join
            Glib::ustring str (msg_view.Color (YellowFg) );
            str += "--- " + msg_view.Color (RedFg) + sock.getRejectionMessage();
            msg_view.add_text (str);
            set_status_message (StatusConnTime, "Disconnected");
        }
    } else { // failed to connect
        get_reason_for_connect_failure();
    }
    // hide the connect dialog here.
    if (connect_dialog->is_visible() ) {
        connect_dialog->hide();
    }
}

void gbzadmin::get_reason_for_connect_failure()
{
    Glib::ustring str (msg_view.Color (YellowFg) );
    switch (sock.getState() ) {
        case BadVersion:
            str += "--- " + msg_view.Color (RedFg) + "Server versions are not compatible\n";
            str += "    " + msg_view.Color (YellowFg) + "You tried to connect to a " + sock.getVersion() + " server\n";
            str += "    using a ";
            str += ServerVersion;
            str += " client\n";
            msg_view.add_text (str);
            break;

        case Refused:
            str += "--- " + msg_view.Color (RedFg) + sock.getRejectionMessage();
            msg_view.add_text (str);
            break;

        case Rejected:
            str += "--- " + msg_view.Color (RedFg) + _server + " has REJECTED me\n";
            str += ">>> " + msg_view.Color (RedFg) + sock.getRejectionMessage();
            msg_view.add_text (str);
            break;

        case ResolveFailure:
            str += ">>> " + msg_view.Color (RedFg) + sock.getRejectionMessage();
            msg_view.add_text (str);
            break;

        default:
            str += "\n--- " + msg_view.Color (RedFg) + "Failed to connect to " + _server;
            str += msg_view.Color (YellowFg) + " (socket error)\n";
            msg_view.add_text (str);
            break;
    }
}

// this function is called once per second to update stuff
// like the connection timer and other status bar stuff
bool gbzadmin::update_timer()
{
    double elapsed;
    char time_str[128];
    int hours, mins, secs;
    bool running = isConnected();
    static int count = 0;

    if (running) {
        elapsed = connection_timer.elapsed();
        hours = int (elapsed / 3600.0);
        mins = int (elapsed / 60.0) % 60;
        secs = int (elapsed) % 60;
        if (display_wd_time) {
            ::snprintf (time_str, sizeof (time_str), "%02d:%02d:%02d  [WD:%2d]", hours, mins, secs, wd_counter);
        } else {
            ::snprintf (time_str, sizeof (time_str), "%02d:%02d:%02d", hours, mins, secs);
        }
        set_status_message (StatusConnTime, time_str);
    } else {
        set_status_message (StatusConnTime, "Disconnected");
    }

    if (sock.netStatsEnabled() ) {
        char str[64];
        float flow_rate = sock.totalBitsPerSecondRate();
        ::sprintf (str, "Flow rate: %5.2fKBps", flow_rate / 8.0); // Kilo Bytes per Second
        set_status_message (StatusNetStats, str);
    } else {
        set_status_message (StatusNetStats, "");
    }
    if ( (line = msg_view.get_buffer_line() ) != prev_line) {
        count = msg_view.get_buffer_size();
        Glib::ustring str = Glib::ustring::compose ("Line: %1  Count: %2", line, count);
        set_status_message (StatusMsgLine, str.c_str() );
        prev_line = line;
    }

    return running;
}

bool gbzadmin::update_watchdog()
{
    bool connected = isConnected();
    wd_counter--;
    if ( (wd_counter > 0) && connected) {
        connected = true;
    } else {
//        on_read_data();     // call this to process the watchdog timeout
        connected = false;  // this will quit the timeout
    }
    return connected;
}

bool gbzadmin::confirm_quit_and_save()
{
    bool result = false;
    switch (confirm ("Quiting...\nDo you want to save the message buffer?", true) ) {
        case GTK_RESPONSE_YES:
            if (!save_dialog) {
                refBuilder->get_widget ("save_dialog", save_dialog);
            }
            if (save_dialog->run() == GTK_RESPONSE_YES) {
                save_the_buffer();
            }
            result = true;
            break;

        case GTK_RESPONSE_NO:
            result = true;
            break;

        case GTK_RESPONSE_CANCEL:
            result = false;
            break;
    }
    return result;
}

int gbzadmin::confirm (Glib::ustring msg, bool use_cancel = false)
{
    Gtk::MessageDialog msg_dialog (msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, true);
    if (use_cancel) {
        msg_dialog.add_button (GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
    }

    msg_dialog.add_button (GTK_STOCK_YES, GTK_RESPONSE_YES);
    msg_dialog.add_button (GTK_STOCK_NO, GTK_RESPONSE_NO);
    msg_dialog.set_default_response (GTK_RESPONSE_NO);

    return msg_dialog.run();
}

void gbzadmin::on_message_view_scrolling_activate()
{
    Gtk::CheckMenuItem *scroll;
    refBuilder->get_widget ("message_view_scrolling", scroll);
    msg_view.set_scroll (scroll->get_active() );
}

void gbzadmin::send_message (const Glib::ustring message, guint8 target)
{
    char msg[MaxPacketLen];
    void* buf = msg;

    buf = parser.nboPackUByte (buf, guint8 (target) );
    buf = parser.nboPackString (buf, message.c_str(), MessageLen);

    sock.send (MsgMessage, (uint16_t) ( (char *) buf - msg), msg);
}

void gbzadmin::on_send_button_pressed()
{
    Gtk::Entry *entry;
    refBuilder->get_widget ("command_entry", entry);
    // retrieve the string from the entry widget
    Glib::ustring tmp_cmd;
    tmp_cmd = entry->get_text();

    if (tmp_cmd.size() == 0) {
        return;
    }

    // clear the command entry
    entry->set_text ("");

    if (tmp_cmd.substr (0, 1) != "/") {
        // Dumbass check:
        // make sure is not an ill-formed password string !!
        if (tmp_cmd.substr (0, 9) == "password ") {
            Glib::ustring msg ("You are about to send your password to EVERYONE!\n"
                               "Try using a '/' in front of 'password'\n\nYou're Welcome");
            post_message (msg, Gtk::MESSAGE_WARNING);
            return;
        } else { // hope its a message
            msg_str = tmp_cmd;
            if (!isConnected() ) {
                msg_str.clear();
            }
        }
    } else if (tmp_cmd.substr (0, 1) == "/") { // check if is command
        if (!isConnected() && tmp_cmd == "/quit") { // offline quit command
            shut_down();
            return;
        }
        cmd_str = tmp_cmd;
    }
    process_command();
}

void gbzadmin::process_command()
{
    Glib::ustring substr;
    Glib::ustring part_msg;
    // check for local commands first
    if (cmd_str.size() ) {
        // add this cmd to the history buffer
        cmd.historyAdd (cmd_str);
        // test command string for quit with a message
        if (cmd_str.substr (0, cmd_str.find_first_of (" ") ) == "/quit") {
            if (cmd_str.length() > 5) { // found a parting message
                part_msg = cmd_str.substr (6);
                if (part_msg.length() ) { // only send if there is an actual message
                    send_message (part_msg, cmd.get_current_target_id() );
                }
            }
            cmd_str.clear();
            if (confirm_quit_and_save() ) {
                shut_down();
            } else {
                return;
            }
        } else if (cmd_str == "/join") {
            // join immediately using current server:port and callsign/password
            logon();
        } else if (cmd_str == "/leave") {
            logoff();
        } else if (cmd_str == "/list") {
            query_listServer();
        } else if (cmd_str.substr (0, cmd_str.find_first_of (" ") ) == "/reverse") {
            if (cmd_str.length() > 8) {
                displayHostName (cmd_str.substr (9) );
            } else {
                show_help (cmd_str);
            }
        } else if (cmd_str.substr (0, cmd_str.find_first_of (" ") ) == "/show") {
            // handle message filter show command
            Glib::ustring type (cmd_str.substr (6) );
            set_message_filter (type, true);
            Glib::ustring str ("--- Message's of type ");
            str += "'" + type + "'" + " will now be shown\n";
            msg_view.add_text (str, Glib::ustring ("rogue") );
        } else if (cmd_str.substr (0, cmd_str.find_first_of (" ") ) == "/hide") {
            // handle message filter hide command
            Glib::ustring type (cmd_str.substr (6) );
            set_message_filter (type, false);
            Glib::ustring str ("--- Message's of type ");
            str += "'" + type + "'" + " will now be hidden\n";
            msg_view.add_text (str, Glib::ustring ("rogue") );
        } else if (cmd_str.substr (0, cmd_str.find_first_of (" ") ) == "/mottos") {
            show_mottos();
        } else { // or send the command to the server
            send_message (cmd_str, ServerPlayer);
        }
        cmd_str.clear();
    } else if (msg_str.size() ) { // otherwise it's a message
        send_message (msg_str, cmd.get_current_target_id() );
        msg_str.clear();
    } else {
        return;
    }
}

// TODO: add command help system
void gbzadmin::show_help (Glib::ustring what)
{
    Glib::ustring help ("--- Help:\n        Sorry, no help available for ");

    if (what == "/reverse")	{
        help = "Usage: /reverse <callsign> | <#slot>\n";
        help += "       Finds the hostname of the given player\n";
        msg_view.add_text (help, Glib::ustring ("rogue") );
    } else {
        help += what + "\n";
    }
}

void gbzadmin::on_variable_changed (Glib::ustring cmd)
{
    // update the variable data
    cmd_str = cmd;
    process_command();
}

void gbzadmin::post_message (Glib::ustring msg, Gtk::MessageType type)
{
    Gtk::MessageDialog msg_dialog (msg, false, type);
    msg_dialog.run();
}

void gbzadmin::on_mute_button_clicked()
{
    do_mute_player();
}

void gbzadmin::on_kick_button_clicked()
{
    do_kick_player();
}

void gbzadmin::on_ban_button_clicked()
{
    do_ban_player();
}

void gbzadmin::on_mute_activate()
{
    do_mute_player();
}

void gbzadmin::on_kick_activate()
{
    do_kick_player();
}

void gbzadmin::on_ban_activate()
{
    do_ban_player();
}

void gbzadmin::on_playerlist_activate()
{
    do_command_send (PlayerList);
}

void gbzadmin::on_clientquery_activate()
{
    do_client_query();
}

void gbzadmin::on_lagstats_activate()
{
    do_command_send (LagStats);
}

void gbzadmin::on_query_listserver_activate()
{
    if (isConnected() ) {
        cmd_str = "/list";
        process_command();
    } else {
        query_listServer();
    }
    on_server_list_window_activate();
}

void gbzadmin::on_query_listserver_clicked()
{
    if (isConnected() ) {
        cmd_str = "/list";
        process_command();
    } else {
        query_listServer();
    }
    on_server_list_window_activate();
}

void gbzadmin::query_listServer()
{
    time_t tm;
    tm = time (NULL);

    Glib::Timer clock;	// starts the clock too

    std::vector<serverInfo*> si;

    listServer.getServerList (si);

    clock.stop();
    double elapsed = clock.elapsed();

    server_list_view.display (si);

    // ctime str is terminated with '\n'
    Glib::ustring str (msg_view.Color (YellowFg) );
    str += Glib::ustring::compose ("--- queried (%1) list server in %2 seconds on %3", ServerVersion, elapsed, ctime (&tm) );

    msg_view.add_text (str);
}

void gbzadmin::do_mute_player()
{
    Gtk::Label *label, *all_label, *ban_label;
    Gtk::Entry *ban_entry;
    Gtk::CheckButton *check;

    input_dialog->Gtk::Window::set_title ("Mute Player");

    refBuilder->get_widget ("reason_label", label);
    refBuilder->get_widget ("bantime_label", ban_label);
    refBuilder->get_widget ("bantime_entry", ban_entry);
    refBuilder->get_widget ("all_label", all_label);
    refBuilder->get_widget ("subnet_checkbutton", check);

    populate_player_combo();

    player_combo.set_active (0);
    reason_combo.set_active (0);
    current_cmd_type = MutePlayer;

    player_combo.show();
    all_label->hide();
    label->hide();
    reason_combo.hide();
    ban_label->hide();
    ban_entry->hide();
    check->hide();
    input_dialog->show();
}

void gbzadmin::do_kick_player()
{
    Gtk::Label *label, *all_label, *ban_label;
    Gtk::Entry *ban_entry;
    Gtk::CheckButton *check;

    input_dialog->Gtk::Window::set_title ("Kick Player");

    refBuilder->get_widget ("reason_label", label);
    refBuilder->get_widget ("bantime_label", ban_label);
    refBuilder->get_widget ("bantime_entry", ban_entry);
    refBuilder->get_widget ("all_label", all_label);
    refBuilder->get_widget ("subnet_checkbutton", check);

    populate_player_combo();

    player_combo.set_active (0);
    reason_combo.set_active (0);
    current_cmd_type = KickPlayer;

    label->show();
    reason_combo.show();
    player_combo.show();
    all_label->hide();
    ban_label->hide();
    ban_entry->hide();
    check->hide();
    input_dialog->show();
}

void gbzadmin::do_ban_player()
{
    Gtk::Label *all_label;
    Gtk::Entry *ban_entry;

    input_dialog->Gtk::Window::set_title ("Ban Player");

    refBuilder->get_widget ("bantime_entry", ban_entry);
    refBuilder->get_widget ("all_label", all_label);

    populate_player_combo();

    player_combo.set_active (0);
    reason_combo.set_active (0);
    current_cmd_type = BanPlayer;

    // "no soup for you, come back, one year!"
    ban_entry->set_text ("short");

    all_label->hide();
    reason_combo.show();
    player_combo.show();
    input_dialog->show_all();
}

void gbzadmin::on_playerlist_button_clicked()
{
    do_command_send (PlayerList);
}

void gbzadmin::on_clientquery_button_clicked()
{
    do_client_query();
}

void gbzadmin::do_client_query()
{
    Gtk::Entry *bantime_entry;
    Gtk::Label *reason_label, *bantime_label, *all_label;

    input_dialog->Gtk::Window::set_title ("Client Query");

    current_cmd_type = ClientQuery;
    refBuilder->get_widget ("reason_label", reason_label);
    refBuilder->get_widget ("all_label", all_label);
    refBuilder->get_widget ("bantime_label", bantime_label);
    refBuilder->get_widget ("bantime_entry", bantime_entry);

    all_label->hide();
    reason_label->hide();
    reason_combo.hide();
    bantime_label->hide();
    bantime_entry->hide();
    player_combo.show();

    populate_player_combo();

    player_combo.prepend_text ("All");
    player_combo.set_active (0);

    input_dialog->show();
}

void gbzadmin::on_lagstats_button_clicked()
{
    do_command_send (LagStats);
}

// this function builds commands that have
// no parameters.
void gbzadmin::do_command_send (gint type)
{
    current_cmd_type = type;

    switch (type) {
        case PlayerList:
            cmd_str = "/playerlist";
            break;

        case LagStats:
            cmd_str = "/lagstats";
            break;

        case ShutDown:
            cmd_str = "/shutdownserver";
            break;

        case FlagUp:
            cmd_str = "/flag up";
            break;

        case SuperKill:
            cmd_str = "/superkill";
            break;

        case ReLoad:
            cmd_str = "/reload";
            break;

        case ServerUptime:
            cmd_str = "/uptime";
            break;

        default:
            break;
    }
    process_command();
}

// retrieve input from the dialog depending on
// the visible entry widgets
void gbzadmin::on_input_dialog_response (gint response_id)
{
    Gtk::Entry *bantime;
    Gtk::CheckButton *subnet;
    Glib::ustring callsign_str, reason_str, bantime_str;
    Glib::ustring command;

    if (response_id == Gtk::RESPONSE_OK) {
        if (player_combo.is_visible() ) {
            callsign_str = player_combo.get_active_text();
        }
        if (reason_combo.is_visible() ) {
            reason_str = reason_combo.get_active_text();
        }
        refBuilder->get_widget ("bantime_entry", bantime);
        if (bantime->is_visible() ) {
            bantime_str = bantime->get_text();
        }
        switch (current_cmd_type) {
            case ClientQuery:
                if (callsign_str.length() ) {
                    if (callsign_str == "All") {
                        command = "/clientquery";
                    } else {
                        command = "/clientquery \"" + callsign_str + "\"";
                    }
                } else {
                    command = "/clientquery";
                }
                break;

            case MutePlayer:
                if (callsign_str.length() ) {
                    command = "/mute \"" + callsign_str + "\"";
                }
                break;

            case KickPlayer:
                if (callsign_str.length() ) {
                    command = "/kick \"" + callsign_str + "\"";
                }
                if (reason_str.length() ) {
                    command += reason_str;
                } else {
                    post_message ("You must supply a reason", Gtk::MESSAGE_ERROR);
                    return;
                }
                break;

            case BanPlayer:
                refBuilder->get_widget ("subnet_checkbutton", subnet);
                // if this option is checked the callsign is replaced by the
                // IP in which the last octet is replaced by a '*', banning
                // an entire subnet.
                if (subnet->get_active() ) {
                    Glib::ustring ip = player_view.get_IP (callsign_str);
                    ip.erase (ip.find_last_of (".") + 1);
                    ip += "*";
                    command = "/ban " + ip + " ";
                } else if (callsign_str.length() ) {
                    command = "/ban \"" + callsign_str + "\" ";
                }
                if (bantime_str.length() ) {
                    command += bantime_str;
                } else { // default to short ban time
                    command += "short";
                }
                if (reason_str.length() ) {
                    command += " (" + callsign_str + ") " + reason_str;
                } else {
                    post_message ("You must supply a reason", Gtk::MESSAGE_ERROR);
                    return;
                }
                break;

            default:
                break;
        }
        cmd_str = command;
    }
    process_command();
    input_dialog->hide();
}

void gbzadmin::on_save_dialog_response (gint response_id)
{
    if (response_id == Gtk::RESPONSE_OK) {
        save_the_buffer();
    }
    save_dialog->hide();
}

void gbzadmin::save_the_buffer()
{
    std::ofstream os;

    if (!save_dialog) {
        refBuilder->get_widget ("save_dialog", save_dialog);
    }

    Glib::ustring filename = save_dialog->get_filename();
    os.open (filename.c_str() );
    if (os) {
        Glib::ustring text = msg_view.get_buffer_text();
        os << text;
        os.close();
    }
    save_dialog->hide();
}

void gbzadmin::replace_input_placeholders()
{
    // Remove the placeholder Gtk::ComboBox and add a Gtk::ComboTextBox
    // We don't need the combobox and it's TreeModel, we just want to
    // display strings. These comboboxes are on the input_dialog.
    Gtk::VBox *box;
    refBuilder->get_widget ("vbox3", box);

    Gtk::ComboBox *crap;
    refBuilder->get_widget ("player_placeholder", crap);

    box->remove (*crap);
    box->pack_start (player_combo);
    box->reorder_child (player_combo, 1);

    refBuilder->get_widget ("reason_placeholder", crap);

    box->remove (*crap);
    box->pack_start (reason_combo);
    box->reorder_child (reason_combo, 4);

    // populate the reason box
    const char* reasons[] = {
        "Cheating",
        "Player Abuse",
        "Spamming",
        "NR",
        "Idle too long",
        "Team killing",
        "Inappropriate Callsign",
        "slot hog",
        "I don't like you",
        "'cause I can",
        "Take a hike!",
        "No reason",
        0
    };

    gint k = 0;
    while (reasons[k]) {
        reason_combo.append_text (Glib::ustring (reasons[k++]) );
    }
}

void gbzadmin::replace_connect_placeholders()
{
    Gtk::VBox *box;
    refBuilder->get_widget ("vbox5", box);

    Gtk::Entry *crap;
    refBuilder->get_widget ("server_entry", crap);

    box->remove (*crap);
    box->pack_start (server_combo);
    box->reorder_child (server_combo, 7);

    // add a delete icon and callback
    Gtk::Entry *entry = server_combo.get_entry();
    entry->set_icon_from_stock (Gtk::Stock::CLOSE);
    entry->signal_icon_press().connect (sigc::mem_fun (*this, &gbzadmin::on_icon_pressed) );
}

void gbzadmin::on_icon_pressed (Gtk::EntryIconPosition icon_pos, const GdkEventButton* event)
{
    // get the active text
    Glib::ustring addr = server_combo.get_entry_text();
    // delete this text entry
    server_mru_delete (addr);
    // re-populate the combo
    server_mru_populate();
    // set the active index
    server_combo.set_active (0);
}

void gbzadmin::populate_player_combo()
{
    player_combo.clear_items();
    for (int k = 0; k < player_view.get_n_players(); k++) {
        Player* player = player_view.get_player (k);
        player_combo.append_text (player->get_callsign() );
    }
    for (int k = 0; k < player_view.get_n_observers(); k++) {
        Player* player = player_view.get_observer (k);
        player_combo.append_text (player->get_callsign() );
    }
}

void gbzadmin::enable_admin_items (bool set)
{
    server_vars_view.set_editable (set);

    Gtk::MenuItem *item = 0;
    refBuilder->get_widget ("shutdown_server", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("super_kill", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("playerlist", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("mute", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("kick", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("ban", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("flag_reset", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("remove_flags", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("lagwarn", item);
    item->set_sensitive (set);

    Gtk::ToolButton *button = 0;
    refBuilder->get_widget ("mute_button", button);
    button->set_sensitive (set);

    button = 0;
    refBuilder->get_widget ("kick_button", button);
    button->set_sensitive (set);

    button = 0;
    refBuilder->get_widget ("ban_button", button);
    button->set_sensitive (set);

    button = 0;
    refBuilder->get_widget ("playerlist_button", button);
    button->set_sensitive (set);
}

void gbzadmin::enable_connected_items (bool set)
{

    Gtk::MenuItem *item = 0;
    refBuilder->get_widget ("save", item);
    item->set_sensitive (set);

    refBuilder->get_widget ("clientquery", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("lagstats", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("capture", item);
    item->set_sensitive (set);

    item = 0;
    refBuilder->get_widget ("uptime", item);
    item->set_sensitive (set);

    Gtk::ToolButton *button = 0;
    refBuilder->get_widget ("clientquery_button", button);
    button->set_sensitive (set);

    button = 0;
    refBuilder->get_widget ("lagstats_button", button);
    button->set_sensitive (set);

    // enable connect button/menu item if not connected
    // otherwise disable the items
    Gtk::ToolButton *connect_button;
    refBuilder->get_widget ("connect_button", connect_button);
    connect_button->set_sensitive (!isConnected() );

    Gtk::MenuItem *connect;
    refBuilder->get_widget ("connect", connect);
    connect->set_sensitive (!isConnected() );

    // disable disconnect button/menuitem if not connected
    // otherwise disable the items
    Gtk::ToolButton *disconnect_button;
    refBuilder->get_widget ("disconnect_button", disconnect_button);
    disconnect_button->set_sensitive (isConnected() );

    Gtk::MenuItem *disconnect;
    refBuilder->get_widget ("disconnect", disconnect);
    disconnect->set_sensitive (isConnected() );
}

Glib::ustring gbzadmin::flag_has_owner (guint8 id)
{
    Glib::ustring flag ("");

    std::vector<flag_info*>::iterator it;
    bool found = false;

    // search the flag store
    it = flag_store.begin();
    while (it != flag_store.end() ) {
        if ( ( (*it)->owner == id) && ( (*it)->status == Parser::FlagOnTank) ) {
            flag = Glib::ustring::compose ("%1", (gchar*) ( (*it)->type) );
            found = true;
            break;
        }
        it++;
    }
    if (!found) {
        flag.clear();
    }

    return flag;
}

void gbzadmin::add_flag (flag_info *fi)
{
    std::vector<flag_info*>::iterator it = flag_store.begin();
    bool found = false;

    while (it != flag_store.end() ) {
        if ( (*it)->idx == fi->idx) { // if flag does not exist add it
            found = true;
            break;
        }
        it++;
    }
    if (!found) {
        flag_store.push_back (fi);
    }
}

void gbzadmin::remove_flag (flag_info *fi)
{
    std::vector<flag_info*>::iterator it = flag_store.begin();

    while (it != flag_store.end() ) {
        if ( (*it)->idx == fi->idx) { // if flag exists erase it
            flag_store.erase (it);
            break;
        }
        it++;
    }
}

void gbzadmin::on_shutdown_server_activate()
{
    if (confirm ("Do you really want to shutdown the server?") == GTK_RESPONSE_YES) {
        do_command_send (ShutDown);
    }
}

void gbzadmin::on_super_kill_activate()
{
    if (confirm ("Really...?\nYou REALLY want to kill _EVERYONE_?\nYourself included!") == GTK_RESPONSE_YES) {
        do_command_send (SuperKill);
    }
}

// this is from connect_clicked in libgtkmm
void gbzadmin::connect_signal_to_item (const Glib::ustring& name, const sigc::slot<void>& slot_)
{
    Gtk::Widget* pWidget = 0;
    refBuilder->get_widget (name, pWidget);

    Gtk::ToolButton* pButton = dynamic_cast<Gtk::ToolButton*> (pWidget);
    Gtk::MenuItem* pMenuItem = dynamic_cast<Gtk::MenuItem*> (pWidget);

    if (pButton) {
        pButton->signal_clicked().connect (slot_);
    }

    if (pMenuItem) {
        pMenuItem->signal_activate().connect (slot_);
    }
}

//
// open|create 'playerstats.txt' text file
// comma seperate the data fields
// this data can be opened in a spreadsheet application
// as TXT|CSV or parsed with Perl.
//
void gbzadmin::dump_player_stats (Player *player)
{
    std::ofstream os;

    Glib::ustring statsPath (Glib::get_home_dir() );
    statsPath += "/Documents/playerstats.txt";

    os.open (statsPath.c_str(), std::ios_base::app);

    if (os) {
        os << player->get_callsign() << ";";
        os << player->get_motto() << ";";
        os << (int) (player->get_id() ) << ";";
        os << player->get_team() << ";";
        os << player->get_registered() << ";";
        os << player->get_verified() << ";";
        os << player->get_admin() << ";";
        os << player->get_IP() << ";";
        os << player->get_wins() << ";";
        os << player->get_losses() << ";";
        os << player->get_score() << ";";
        os << player->get_tks() << ";";
        os << player->get_strength_index() << ";";

        time_t now = time (NULL);
        const time_t joined = player->get_time_joined();
        struct tm *ts = localtime (&joined);
        char str[64];
        strftime (str, 64, "%T", ts);

        os << str << ";";	// time joined

        ts = localtime (&now);
        strftime (str, 64, "%T", ts);

        os << str << ";";	// time left

        double diff = difftime (now, player->get_time_joined() );
        unsigned long ldiff = (unsigned long) diff;
        guint hours = (ldiff / 3600);
        guint minutes = (ldiff / 60) - (hours * 60);
        guint seconds = ldiff % 60;

        os << hours << ":" << minutes << ":" << seconds << ";"; // duration
        // add the date
        strftime (str, 64, "%F;", ts);
        os << str;
        // what server:port is this?
        os << _server << ":" << _port;
        // done, <CR><LF>
        os << "\n";

        os.close();
    }
}

// param is cmd_str.substr(9)
void gbzadmin::displayHostName (Glib::ustring param)
{
    Player *player = 0;
    Glib::ustring substr;

    if (param.at (0) == '#') { // player ID
        substr = param.substr (1); // remove the '#'
        gint idx = atoi (substr.c_str() );
        player = player_view.find_player (idx);
    } else { // player callsign
        player = player_view.find_player (param);
    }
    if (player) {
        if (player->get_IP().length() ) {
            Glib::ustring str ("### ");
            Glib::ustring host = sock.reverseResolve (player->get_IP(), player->get_callsign() );
            str += host;
            msg_view.add_text (str, Glib::ustring ("rogue") );
        } else {
            Glib::ustring str ("### Player IP was not found, cannot resolve hostname!\n");
            msg_view.add_text (str, Glib::ustring ("rogue") );
        }
    } else {
        Glib::ustring str ("### Player was not found, invalid player ID or callsign?\n");
        msg_view.add_text (str, Glib::ustring ("rogue") );
    }
}

void gbzadmin::show_mottos()
{
    Glib::ustring str ("");
    for (int k = 0; k < player_view.get_n_players(); k++) {
        Player* player = player_view.get_player (k);
        str += msg_view.Color (Cyan4Fg) + ">>> ";
        str += player->get_callsign();
        str += "'s motto is ";
        str += msg_view.Color (GoldenFg) + "'";
        str += player->get_motto();
        str += "'\n";
    }
    for (int k = 0; k < player_view.get_n_observers(); k++) {
        Player* player = player_view.get_observer (k);
        str += msg_view.Color (Cyan4Fg) + ">>> ";
        str += player->get_callsign();
        str += "'s motto is ";
        str += msg_view.Color (GoldenFg) + "'";
        str += player->get_motto();
        str += "'\n";
    }
    msg_view.add_text (str);
}

// server MRU stuff

// parse the config file MRU string
std::list<Glib::ustring> gbzadmin::parse_server_mru (const Glib::ustring& in, const Glib::ustring &delims, int lines_max)
{
    std::list<Glib::ustring> tokens;
    Glib::ustring currentToken ("");
    const Glib::ustring::size_type len = in.size();
    Glib::ustring::size_type pos = in.find_first_not_of (delims);
    int currentChar = (pos == Glib::ustring::npos) ? -1 : in[pos];
    int nLines = 1;

    while (pos < len && pos != Glib::ustring::npos) {
        if (nLines >= lines_max) {
            break;
        }
        bool tokenDone = false;
        currentChar = (pos < len) ? in[pos] : -1;
        while ( (currentChar != -1) && !tokenDone) {
            tokenDone = false;
            if (delims.find (char (currentChar) ) != Glib::ustring::npos) {
                pos ++;
                break;
            }
            currentToken += char (currentChar);
            pos++;
            currentChar = (pos < len) ? in[pos] : -1;
        }
        if (currentToken.size() > 0) {
            tokens.push_back (currentToken);
            currentToken.clear();
        }
        if ( (pos < len) && (pos != std::string::npos) ) {
            pos = in.find_first_not_of (delims, pos);
        }
        nLines++;
    }
    if (pos != std::string::npos) {
        std::string lastToken = in.substr (pos);
        if (lastToken.size() > 0) {
            tokens.push_back (lastToken);
        }
    }
    return tokens;
}

// populate the server combo box
void gbzadmin::server_mru_populate()
{
    server_combo.clear_items();
    std::list<Glib::ustring>::iterator iter;
    int nLines = 1;
    for (iter = server_mru_str.begin(); iter != server_mru_str.end(); iter++) {
        server_combo.append_text (*iter);
        nLines++;
        if (nLines >= maxServersList) {
            break;
        }
    }
    iter = server_mru_str.begin();
    server_combo.set_active_text (*iter);
}

// delete an entry from the server combo box and server_mru list
void gbzadmin::server_mru_delete (Glib::ustring line)
{
    std::list<Glib::ustring>::iterator it = server_mru_str.begin();
    while (it != server_mru_str.end() ) {
        std::list<Glib::ustring>::iterator thisone = it;
        it++;
        if (*thisone == line) {
            server_mru_str.erase (thisone);
        }
    }
}

// find an entry in the server mru list
Glib::ustring gbzadmin::server_mru_find (Glib::ustring find)
{
    Glib::ustring result ("");
    std::list<Glib::ustring>::iterator it = server_mru_str.begin();
    while (it != server_mru_str.end() ) {
        std::list<Glib::ustring>::iterator thisone = it;
        it++;
        if (*thisone == find) {
            result = *thisone;
        }
    }
    return result;
}

// END server MRU stuff

TeamColor gbzadmin::PlayerIdToTeam (guint8 id)
{
    return (LastRealPlayer < id && id <= FirstTeam ? TeamColor (FirstTeam - id) : NoTeam);
}

// parse the address string (server:port) and populate the _server, _port_str and _port variables
void gbzadmin::parse_host_port (Glib::ustring addr)
{
    size_t idx = addr.find_last_of (":");
    if (idx == std::string::npos) { // host:port separator ":" is missing
        _server = addr;
        _port_str = "5154";
        _port = 5154; // use default port
    } else { // parse the server name and port
        _server = addr.substr (0, idx);
        _port_str = addr.substr (idx + 1);
        _port = atoi (_port_str.c_str() );
    }
}

