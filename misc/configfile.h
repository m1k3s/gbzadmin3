#ifndef _configfile_h_
#define _configfile_h_

// gbzadmin
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

#include <gtkmm.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/utsname.h>
#include <cstdio>

#include "config.h"

class ConfigFile
{
    public:
        ConfigFile();
        ~ConfigFile();

        void save_configfile_file();
        void parse_configfile_file();
        void init();

        // set methods
        void set_callsign(Glib::ustring str) {
            callsign = str;
        }
        void set_motto(Glib::ustring str) {
            motto = str;
        }

        // get methods
        Glib::ustring get_callsign() {
            return callsign;
        }
        Glib::ustring get_motto() {
            return motto;
        }

    private:
        Glib::ustring filename;
        Glib::ustring callsign;
        Glib::ustring motto;
        Glib::ustring password;
        Glib::ustring server;
        Glib::ustring port;
        Glib::ustring win_x, win_y;
        Glib::ustring win_width, win_height;
        Glib::ustring msg_pane, game_pane;
        bool auto_cmd;
        bool echo_pings;
        bool dump_players;
        bool line_numbers;
        bool save_password;
        bool rabbit;
        bool pause;
        bool spawn;
        bool ping;
        bool bzdb;
        bool join;
        bool leave;
        bool admin;
        bool score;
        bool time;
        bool kill;
        bool chat;
        bool roger;
};

#endif // _configfile_h_
