#ifndef _serverlist_view_h_
#define _serverlist_view_h_

// GTKmm bzadmin
// Copyright (c) 2005 - 2012 Michael Sheppard
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
#include <fstream>
#include <sys/types.h>
#include <sys/utsname.h>
#include <cstdio>

#include "config.h"
#include "utilities.h"
#include "common.h"
#include "view.h"
#include "parser.h"
#include "gSocket.h"

class serverListView : public view
{
    public:
        serverListView();
        ~serverListView();

        void init(Glib::RefPtr <Gtk::Builder> _refBuilder);
        void display(std::vector<serverInfo*> &list);
        void accumulate_serverStats(std::vector<serverInfo*> & si, serverStats& ss);
        void display_serverStats(serverStats & ss);
        void display_listServers(std::vector<serverInfo*> & si);

    protected:
        void on_list_tab_close_button_pressed();

    private:
        Glib::RefPtr<Gtk::Builder> refBuilder;
};

#endif // _serverlist_view_h_
