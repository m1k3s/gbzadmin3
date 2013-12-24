#ifndef _gbzadmin_h_
#define _gbzadmin_h_

// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2012 Michael Sheppard
//  
// Code based on BZFlag-2.0.x and SVN 2.99
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

#include <gtkmm.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sys/types.h>
#include <sys/utsname.h>
#include <cstdio>
#include <map>

#include "config.h"
#include "utilities.h"
#include "common.h"
#include "player_view.h"
#include "msg_view.h"
#include "server_vars_view.h"
#include "game_stat_view.h"
#include "serverlist_view.h"
#include "cmdLine.h"
#include "parser.h"
#include "gSocket.h"
#include "gListServer.h"

const int MaxWdTime = 30;

class gbzadmin : public Gtk::Window
{
public:
	gbzadmin();
	~gbzadmin() {}

	Gtk::Window *init_gbzadmin(Glib::RefPtr <Gtk::Builder> _refBuilder);
	Glib::RefPtr <Gtk::Builder> getRefXml() { return refBuilder; }
	playerView& getPlayerView() { return player_view; }
	msgView& getMsgView() { return msg_view; }
	guint8 get_id() { return me; }
	
protected:
	void add_callbacks();
	void connect_clicked(const Glib::ustring& name, const sigc::slot<void>& slot_);
	
	// member callbacks
	void on_quit_activate();
	void on_about_activate();
	bool on_key_press_event (GdkEventKey *event);
	void on_connect_activate();
	void on_disconnect_activate();
	void on_connect_clicked();
	void on_disconnect_clicked();
	void on_capture_activate();
	void on_save_activate();
	void on_prefs_activate();
	void on_message_view_scrolling_activate();
	void on_send_button_pressed();
	void on_variable_changed(Glib::ustring cmd);
	void on_mute_button_clicked();
	void on_kick_button_clicked();
	void on_ban_button_clicked();
	void on_mute_activate();
	void on_kick_activate();
	void on_ban_activate();
	void on_lagwarn_activate();
	void on_uptime_activate();
	void on_flag_reset_activate();
	void on_flag_up_activate();
	void on_playerlist_activate();
	void on_clientquery_activate();
	void on_lagstats_activate();
	void on_playerlist_button_clicked();
	void on_clientquery_button_clicked();
	void on_lagstats_button_clicked();
	void on_shutdown_server_activate();
	void on_super_kill_activate();
	void on_player_window_activate();
	void on_server_window_activate();
	void on_server_list_window_activate();
	bool on_delete_event (GdkEventAny *event);
	void on_query_listserver_activate();
	void on_save_config_activate();
	
	// signal callback function
	void on_read_data();
	
	// member functions
	void query_listServer();
	void set_status_message(gint statusbar, const gchar *msg);
	void clear_status_message(gint statusbar);
	void push_pop_status(gint statusbar, const gchar *msg, bool push);
	void toggle_capture_file();
	void shut_down();
	void set_message_filter(Glib::ustring type, bool set);
	Glib::ustring get_team_str(int t);
	void logon();
	void logoff();
	bool confirm_quit_and_save();
	int confirm(Glib::ustring msg, bool use_cancel);
	void send_message(const Glib::ustring msg, guint8 target);
	void post_message(Glib::ustring msg, Gtk::MessageType type);
	void do_mute_player();
	void do_kick_player();
	void do_ban_player();
	void do_command_send(gint type);
	void do_client_query();
	void save_the_buffer();
	void replace_input_placeholders();
	void populate_player_combo();
	void enable_admin_items(bool set);
	void enable_connected_items(bool set);
	void clean_up(bool);
	void power_down();
	void process_command();
	gint get_message();
	bool isConnected();
	void dump_player_stats(Player *p);
	void show_help(Glib::ustring what);
	void displayHostName(Glib::ustring param);
	void save_config_file(Glib::ustring filename);
	void parse_config_file(Glib::ustring filename);
	Glib::ustring colorize(Player* player);
	void init_message_handler_map();
	void print_message_code(guint16 code);
	
	// timer functions
	bool update_timer();
	bool update_watchdog();
	
	// dialog setup activation
	void connect_dialog_setup();
	void pref_dialog_setup();
	
	// dialog response handlers
	void on_connect_dialog_response(gint response_id);
	void on_pref_dialog_response(gint response_id);
	void on_input_dialog_response(gint response_id);
	void on_save_dialog_response(gint response_id);
	void on_flag_reset_dialog_response(gint response_id);
	void on_lagwarn_dialog_response(gint response_id);
	
	// network message handlers
	void handle_rabbit_message(void *vbuf);
	void handle_pause_message(void *vbuf);
	void handle_alive_message(void *vbuf);
	void handle_add_player_message(void *vbuf);
	void handle_remove_player_message(void *vbuf);
	void handle_playerinfo_message(void *vbuf);
	void handle_admininfo_message(void* vbuf);
	void handle_killed_message(void *vbuf);
	void handle_captureflag_message(void *vbuf);
	void handle_grabflag_message(void *vbuf);
	void handle_dropflag_message(void *vbuf);
	void handle_transferflag_message(void *vbuf);
	void handle_autopilot_message(void *vbuf);
	void handle_udplinkestablished_message(void *vbuf);
	void handle_udplinkrequest_message(void *vbuf);
	void handle_message_message(void *vbuf);
	void handle_score_message(void *vbuf);
	void handle_setvar_message(void *vbuf);
	void handle_flagupdate_message(void *vbuf);
	void handle_ping_message(void *vbuf);
	void handle_game_query_message(void *vbuf);
	void handle_teamupdate_message(void *vbuf);
	void handle_joinserver_message(void *vbuf);
	void handle_teleport_message(void *vbuf);
	
private:
	Gtk::Window *app;
	Glib::Timer connection_timer;
	Glib::Timer watchdog_timer;
	Glib::RefPtr<Gtk::Builder> refBuilder;
	Glib::ustring cmd_str;
	Glib::ustring msg_str;
	Glib::ustring defColor;
	
	Gtk::Dialog *connect_dialog;
	Gtk::Dialog *pref_dialog;
	Gtk::Dialog *input_dialog;
	Gtk::Dialog *flag_reset_dialog;
	Gtk::Dialog *lagwarn_dialog;
	Gtk::FileChooserDialog *save_dialog;
	
	// Unfortunately Glade Interface Builder does not support
	// Gtk::ComboBoxText widgets. So we must supply our own.
	Gtk::ComboBoxText player_combo;
	Gtk::ComboBoxText reason_combo;

	// widgets and objects
	playerView player_view;
	msgView msg_view;
	serverVars server_vars_view;
	gameStatView game_view;
	serverListView server_list_view;
	cmdLine cmd;
	Parser parser;
	gSocket sock;
	gListServer listServer;
	
	// member properties
	Glib::ustring path; // to the config file
	Glib::ustring msg_self;
	Glib::ustring serverName;
	Glib::ustring referrer;
	guint8 me;
	gint current_cmd_type;
	gint count, prev_count;
	gint line, prev_line;
	gint wd_counter;
	bool capturing;
	bool leaving;
	int _team;
	float rank;
	gint win_x, win_y;
	gint win_width, win_height;
	int msg_pane, game_pane;
	
	// message handler function map
	typedef void (gbzadmin::*messagehandler)(void*);
    typedef std::map<guint16, messagehandler> msg_handler_map;
    msg_handler_map handler_map;
	
	// need to have a flag list in order to 
	// see the players existing flags on startup
	std::vector<flag_info*> flag_store;
	Glib::ustring flag_has_owner(guint8 id);
	void add_flag(flag_info *fi);
	void remove_flag(flag_info *fi);
	
	// preferences data
	Glib::ustring _callsign;
	Glib::ustring _password;
	Glib::ustring _server;
	Glib::ustring _port_str;
	Glib::ustring _motto;
	int _port;
	bool auto_cmd;
	bool useUDP;
	bool dump_players;
	std::map<Glib::ustring, bool> msg_mask;
	std::map<Glib::ustring, int> target_map;
	bool echo_pings;
	bool save_password;
	bool connect_at_startup;
	bool line_numbers;
	
	Glib::ustring msg_view_fg, msg_view_bg;
	Glib::ustring player_view_fg, player_view_bg;
	Glib::ustring gamestat_view_fg, gamestat_view_bg;
	Glib::ustring serverlist_view_fg, serverlist_view_bg;
	
	sigc::connection conn_timer;
	sigc::connection wd_timer;
	sigc::connection tcp_data_pending;
	sigc::connection udp_data_pending;
	sigc::connection capture_signal;

	enum { // command enumerators
		NoCommand = -1, PlayerList =  0, ClientQuery = 1,	LagStats,	MutePlayer,	KickPlayer,
		BanPlayer, ShutDown,	FlagReset, FlagUp, SuperKill,	LagWarn, SetGroup,
		RemoveGroup, ReLoad, ServerUptime
	};
	enum {
		StatusConnTime=0,
		StatusMsgLine,
		StatusNetStats
	};
	
};

#endif // _gbzadmin_h_
