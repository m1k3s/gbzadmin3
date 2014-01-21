#ifndef _cmdLine_h_
#define _cmdLine_h_
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

#include "config.h"
#include "common.h"

class cmdLine : public Gtk::Entry
{
    public:
        cmdLine();
        ~cmdLine() {}
        void init(Glib::RefPtr <Gtk::Builder> _refBuilder);
        void grab_focus();
        void clear();
        bool get_focus();
        void add_target(guint8 id, Glib::ustring callsign);
        void remove_target(guint8 id, Glib::ustring callsign);
        int get_current_target_id();
        void clear_targets();
        void set_target_admin();
        void set_target_all();
        void set_target_team();
        void set_auto_cmd();
        void historyAdd(Glib::ustring line);
        Glib::ustring historyUp();
        Glib::ustring historyDown();
        void historyDelete(Glib::ustring line);
        void historyReset();

    protected:
    	void on_icon_pressed(Gtk::EntryIconPosition icon_pos, const GdkEventButton* event);
        Glib::RefPtr<Gtk::TreeModel> create_completion_model();

        struct cmdColumn : public Gtk::TreeModel::ColumnRecord {
            public:
                Gtk::TreeModelColumn<Glib::ustring> cmd;

                cmdColumn() {
                    add(cmd);
                }
        };

    private:
        Gtk::ComboBoxText _targets;
        Gtk::Entry *command;
        Gtk::Button *send_button;
        Glib::RefPtr <Gtk::Builder> refBuilder;
        Glib::ustring cmd_str;
        Glib::ustring msg_str;
        std::map<Glib::ustring, int> target_map;
        std::list<Glib::ustring> history;
        std::list<Glib::ustring>::iterator hist_it;
        const cmdColumn column;
        bool hist_reset;
};

#endif // _cmdLine_h_
