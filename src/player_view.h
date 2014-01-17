#ifndef _player_view_h_
#define _player_view_h_
// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2014 Michael Sheppard
//
// Code based on BZFlag-2.0.x
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
#include <vector>
#include <algorithm>

#include "config.h"
#include "common.h"
#include "view.h"
#include "players.h"

#define MAX_CALLSIGN_LEN	24
#define MAX_COLUMN_LEN		36

class playerView : public view
{
    public:
        playerView();
        ~playerView() {}
        void init(Glib::RefPtr <Gtk::Builder> _refBuilder);
        void add(Player* player);
        void remove(Player* player);
        Player* get_player(gint n);
        gint get_n_players();
        Player* get_observer(gint n);
        gint get_n_observers();
        void update();
        Player* find_player(int id);
        Player* find_player(Glib::ustring callsign);
        void set_rabbit_mode(bool set) {
            rabbitMode = set;
        }
        bool get_rabbit_mode() {
            return rabbitMode;
        }
        void change_all_to_hunter_except(guint8 id);
        void clear_players();
        Glib::ustring get_IP(Glib::ustring callsign);

    protected:
        Glib::ustring format_player(Player *player);
        Glib::ustring format_observer(Player *player);
        gint get_team_from_team_flag(Glib::ustring abbrv);
        Glib::ustring get_team_flag_desc(Glib::ustring abbrv);

    private:
        std::vector<Player*> players; // unsorted player vector
        std::vector<Player*> observers; // observer vector
        bool rabbitMode;
};

#endif // _player_view_h_
