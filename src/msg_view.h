#ifndef _msg_view_h_
#define _msg_view_h_
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

#include <gtkmm.h>
#include <iostream>

#include "config.h"
#include "common.h"
#include "view.h"
#include "players.h"

class msgView : public view
{
public:
	msgView();
	~msgView();
	void init(Glib::RefPtr <Gtk::Builder> _refBuilder);
	void format(Glib::ustring& formatted, Glib::ustring msg, guint8 src, guint8 dst,
						guint16 dstTeam, guint8 me, Glib::ustring src_callsign, Glib::ustring dst_callsign);
	Glib::ustring colorBullet();
	void set_line_numbers(bool set) { line_numbers = set; }
	bool get_line_numbers() { return line_numbers; }
	
	// overrides
	void add_text(const gchar *str, const gchar *tag); // TextTag based versions
	void add_text(Glib::ustring str, Glib::ustring tag);
	void add_text(Glib::ustring &str); // ANSI color code version
	
private:
	bool line_numbers;
};

#endif // _msg_view_h_
