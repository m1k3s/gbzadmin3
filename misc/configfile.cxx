// gbzadmin version 2.4.x
// GTKmm bzadmin
// Copyright (c) 2005 - 2012 Michael Sheppard
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

#include "configfile.h"

const char *configFileName = "/.gbzadmin";

ConfigFile::ConfigFile()
{

}

ConfigFile::~ConfigFile()
{

}

void ConfigFile::init()
{
    Glib::ustring configPath(Glib::get_home_dir());
    configPath += configFileName;
}

void ConfigFile::parse_config_file()
{
    char variable[64];
    char value[512];
    char buf[1024];
    int result = 0;

    std::ifstream is;
    is.open(filename.c_str(), std::ios::in);
    if (is.is_open()) {
        do {
            memset(buf, 0, 1024);
            is.getline(buf, 256);

            result = sscanf(buf, "%32[^#=]=%256[^\n]\n", variable, value);
            if ((result < 2) || (result == EOF)) {
                break;
            }

            // now that we have the variable,value pair we need to assign
            // the value to the prefs.value

            if (g_ascii_strcasecmp(variable, "callsign") == 0) {
                _callsign = value;
            } else if (g_ascii_strcasecmp(variable, "motto") == 0) {
                _motto = value;
            } else if (g_ascii_strcasecmp(variable, "password") == 0) {
                _password = value;
            } else if (g_ascii_strcasecmp(variable, "server") == 0) {
                _server = value;
            } else if (g_ascii_strcasecmp(variable, "window_x") == 0) {
                win_x = atoi(value);
            } else if (g_ascii_strcasecmp(variable, "window_y") == 0) {
                win_y = atoi(value);
            } else if (g_ascii_strcasecmp(variable, "window_width") == 0) {
                win_width = atoi(value);
            } else if (g_ascii_strcasecmp(variable, "window_height") == 0) {
                win_height = atoi(value);
            } else if (g_ascii_strcasecmp(variable, "msg_pane") == 0) {
                msg_pane = atoi(value);
            } else if (g_ascii_strcasecmp(variable, "game_pane") == 0) {
                game_pane = atoi(value);
            } else if (g_ascii_strcasecmp(variable, "port") == 0) {
                _port_str = value;
            } else if (g_ascii_strcasecmp(variable, "auto_cmd") == 0) {
                auto_cmd = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "echo_pings") == 0) {
                echo_pings = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "dump_players") == 0) {
                dump_players = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "line_numbers") == 0) {
                line_numbers = atoi(value) ? true : false;
//	 		} else if (g_ascii_strcasecmp(variable, "msg_view_fg") == 0) {
//	 			msg_view_fg = value;
//	 		} else if (g_ascii_strcasecmp(variable, "msg_view_bg") == 0) {
//	 			msg_view_bg = value;
//	 		} else if (g_ascii_strcasecmp(variable, "player_view_fg") == 0) {
//	 			player_view_fg = value;
//	 		} else if (g_ascii_strcasecmp(variable, "player_view_bg") == 0) {
//	 			player_view_bg = value;
//	 		} else if (g_ascii_strcasecmp(variable, "gamestat_view_fg") == 0) {
//	 			gamestat_view_fg = value;
//	 		} else if (g_ascii_strcasecmp(variable, "gamestat_view_bg") == 0) {
//	 			gamestat_view_bg = value;
//	 		} else if (g_ascii_strcasecmp(variable, "serverlist_view_fg") == 0) {
//	 			serverlist_view_fg = value;
//	 		} else if (g_ascii_strcasecmp(variable, "serverlist_view_bg") == 0) {
//	 			serverlist_view_bg = value;
            } else if (g_ascii_strcasecmp(variable, "save_password") == 0) {
                save_password = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_new_rabbit") == 0) {
                rabbit = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_pause") == 0) {
                pause = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_alive") == 0) {
                spawn = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_lag_ping") == 0) {
                ping = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_set_var") == 0) {
                bzdb = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_add_player") == 0) {
                join = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_remove_player") == 0) {
                leave = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_admin_info") == 0) {
                admin = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_score") == 0) {
                score = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_time_stamp") == 0) {
                time = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_killed") == 0) {
                kill = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_message") == 0) {
                chat = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_roger") == 0) {
                roger = atoi(value) ? true : false;
            } else if (g_ascii_strcasecmp(variable, "msg_flags") == 0) {
                flags = atoi(value) ? true : false;
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
// configfileuration preferences are preserved by saving to a file
// called ".gbzadmin" in the user's home directory
//
void ConfigFile::save_config_file()
{
    Glib::ustring buf;
    std::ofstream os;

    os.open(filename.c_str(), std::ios::out | std::ios::trunc);
    if (os.is_open()) {
        if (!_callsign.empty()) {
            buf = Glib::ustring::compose("callsign=%1\n", callsign);
            os.write(buf.c_str(), buf.length());
        }
        if (!_password.empty()) {
            buf = Glib::ustring::compose("password=%1\n", password);
            os.write(buf.c_str(), buf.length());
        }
        if (!_motto.empty()) {
            buf = Glib::ustring::compose("motto=%1\n", motto);
            os.write(buf.c_str(), buf.length());
        }
        if (!_server.empty()) {
            buf = Glib::ustring::compose("server=%1\n", server);
            os.write(buf.c_str(), buf.length());
        }
        if (!_port_str.empty()) {
            buf = Glib::ustring::compose("port=%1\n", port);
            os.write(buf.c_str(), buf.length());
        }
        buf = Glib::ustring::compose("window_x=%1\n", win_x);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("window_y=%1\n", win_y);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("window_height=%1\n", win_height);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("window_width=%1\n", win_width);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_pane=%1\n", msg_pane);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("game_pane=%1\n", game_pane);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("auto_cmd=%1\n", auto_cmd);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("echo_pings=%1\n", echo_pings);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("dump_players=%1\n", dump_players);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("line_numbers=%1\n", line_numbers);
        os.write(buf.c_str(), buf.length());

//		buf = Glib::ustring::compose("msg_view_fg=%1\n", msg_view_fg);
//		os.write(buf.c_str(), buf.length());
//
//		buf = Glib::ustring::compose("msg_view_bg=%1\n", msg_view_bg);
//		os.write(buf.c_str(), buf.length());
//
//		buf = Glib::ustring::compose("player_view_fg=%1\n", player_view_fg);
//		os.write(buf.c_str(), buf.length());
//
//		buf = Glib::ustring::compose("player_view_bg=%1\n", player_view_bg);
//		os.write(buf.c_str(), buf.length());
//
//		buf = Glib::ustring::compose("serverlist_view_fg=%1\n", serverlist_view_fg);
//		os.write(buf.c_str(), buf.length());
//
//		buf = Glib::ustring::compose("serverlist_view_bg=%1\n", serverlist_view_bg);
//		os.write(buf.c_str(), buf.length());
//
//		buf = Glib::ustring::compose("gamestat_view_fg=%1\n", gamestat_view_fg);
//		os.write(buf.c_str(), buf.length());
//
//		buf = Glib::ustring::compose("gamestat_view_bg=%1\n", gamestat_view_bg);
//		os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("save_password=%1\n", save_password);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_new_rabbit=%1\n", rabbit);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_pause=%1\n", pause);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_alive=%1\n", spawn);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_lag_ping=%1\n", ping);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_set_var=%1\n", bzdb);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_add_player=%1\n", join);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_remove_player=%1\n", leave);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_admin_info=%1\n", admin);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_score=%1\n", score);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_time_stamp=%1\n", time);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_killed=%1\n", kill);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_message=%1\n", chat);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_roger=%1\n", roger);
        os.write(buf.c_str(), buf.length());

        buf = Glib::ustring::compose("msg_flags=%1\n", flags);
        os.write(buf.c_str(), buf.length());

        os.close();
    } else {
        std::cerr << "File could not be opened\n";
    }
}
