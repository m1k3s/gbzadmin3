// gbzadmin
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

#include "msg_view.h"

msgView::msgView()
{

}

msgView::~msgView()
{
//	save_gconf();
}

void msgView::init(Glib::RefPtr <Gtk::Builder> _refBuilder, Glib::RefPtr<Gnome::Conf::Client> _client)
{
	view::init(_refBuilder, "msg_view", _client);
	
	set_scroll(true);
	set_shrink(true);
}

Glib::ustring msgView::colorBullet()
{
	Glib::ustring bullet(Color(RedFg));
	bullet += "*";
	bullet += Color(YellowFg) + "*";
	bullet += Color(GreenFg) + "* ";
	
	return bullet;
}

void msgView::format(Glib::ustring& formatted, Glib::ustring msg, guint8 src, guint8 dst,
						guint16 dstTeam, guint8 me, Glib::ustring src_callsign, Glib::ustring dst_callsign)
{
	bool isAction = false;
	Glib::ustring message("");
	
	// get sender and receiver
	Glib::ustring srcName(src == ServerPlayer ? "SERVER" : (src_callsign.size() ? src_callsign : "(UNKNOWN)"));
	Glib::ustring dstName((dst_callsign.size() ? dst_callsign : "(UNKNOWN)"));
	
	// display action messages differently
  if ((msg[0] == '*') && (msg[1] == ' ') && (msg[msg.size()-1] == '*') && (msg[msg.size()-2] == '\t')) {
    isAction = true;
    message = msg.substr(2, msg.size()-4);
  } else {
    message = msg;
  }
	// direct message to or from me
	if ((dst == me) || (dst_callsign.size())) {
		if (!(src == me && dst == me)) {
			if (src == me) {
				if (isAction) {
					formatted = "[->" + message + "]\n";
				} else {
					formatted = "[->" + dstName + "] " + Color(CyanFg) + message + "\n";
				}
			} else {
				if (isAction) {
					formatted = "[" + message + "->]\n";
				} else {
					if (src == ServerPlayer)
						formatted = "[" + srcName + "->] " + Color(PGreen4Fg) + message + "\n";
					else
						formatted = "[" + srcName + "->] " + Color(CyanFg) + message + "\n";
				}
			}
		} else {
			formatted += Color(CyanFg) + message + "\n";
		}
	}	else {  // public, admin or team message
    if (dst == AdminPlayers)
      formatted = "[Admin] ";
    else if ((gint16)dstTeam != NoTeam)
      formatted = "[Team] ";

    if (!isAction) {
      formatted += srcName;
      formatted += ": ";
    }
    formatted += (src == ServerPlayer) ? Color(Cyan4Fg) : Color(CyanFg);

    formatted += message + "\n";
  }
}
//
// overrides
//

// add colorized text to the view using the tagtable
void msgView::add_text(const gchar *str, const gchar *tag)
{
	if (line_numbers) {
		char tmp[512];
		::snprintf(tmp, sizeof(tmp), "[%04d] ", buffer->get_line_count());
		strcat(tmp, str);
		view::add_text(tmp, tag);
	} else {
		view::add_text(str, tag);
	}
}

// add colorized text to the view using the tagtable
void msgView::add_text(Glib::ustring str, Glib::ustring tag)
{
	if (line_numbers) {
		char tmp[512];
		::snprintf(tmp, sizeof(tmp), "[%04d] ", buffer->get_line_count());
		Glib::ustring lino(Glib::ustring(tmp) += str);
		view::add_text(lino, tag);
	} else {
		view::add_text(str, tag);
	}
}

// add text using ANSI color codes
void msgView::add_text(Glib::ustring &str)
{
	if (line_numbers) {
		char tmp[512];
		::snprintf(tmp, sizeof(tmp), "%s[%04d] ", Color(PGreen4Fg).c_str(), buffer->get_line_count());
		Glib::ustring lino(Glib::ustring(tmp) += str);
		view::add_text(lino);
	} else {
		view::add_text(str);
	}
}


