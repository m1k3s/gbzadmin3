#ifndef _view_h_
#define _view_h_
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
//#include <gconfmm.h>
#include <iostream>
#include <fstream>

#include "common.h"

enum {
	BlackFg = 0, RedFg, GreenFg, YellowFg, BlueFg, PurpleFg, CyanFg, WhiteFg,
	OrangeFg, GoldenFg, Cyan4Fg, PGreen4Fg,
	BlackBg, RedBg, GreenBg, YellowBg, BlueBg, PurpleBg, CyanBg, WhiteBg,
	OrangeBg, GoldenBg, Cyan4Bg, PGreen4Bg,
	
	LastColor,
};

// the text buffer will grow to MaxLineCount
// and will then be shrunk to MinLineCount
const int MaxLineCount = 2048;
const int MinLineCount = 1024;

class view : public Gtk::TextView
{
public:
	view();
	~view();
	void init(Glib::RefPtr <Gtk::Builder> _refBuilder, const gchar* which);
	Glib::ustring get_buffer_text() { return buffer->get_text(); }
	gint get_buffer_size() { return buffer->get_char_count(); }
	gint get_buffer_line() { return buffer->get_line_count(); }
	void add_text(const gchar *str, const gchar *tag); // TextTag based versions
	void add_text(Glib::ustring str, Glib::ustring tag);
	void add_text(Glib::ustring &str); // ANSI color code version
	Glib::RefPtr <Gtk::Builder> get_xml() { return refBuilder; }
	void set_scroll(bool set) { view_scroll = set; }
	bool get_scroll() { return view_scroll; }
	void set_shrink(bool set) { do_shrink = set; }
	bool get_shrink() { return do_shrink; }
	void clear();
	Glib::ustring get_color(int idx);
	bool start_capture();
	void stop_capture();
	void set_view_font(Glib::ustring font);
	Glib::ustring get_view_font() { return viewFont; }
	Glib::ustring Color(int c);
	void initialize_colors();
	void set_bg(Glib::ustring bg);
	void set_fg(Glib::ustring fg);
	
protected:
	Glib::RefPtr<Gtk::TextTagTable> get_tag_table() { return tag_table; }
	Glib::RefPtr <Gtk::TextBuffer> get_buffer() { return buffer; }
	void add_buffer();
	Glib::ustring parse_string(Glib::ustring str);
	Glib::ustring parse_string(const gchar *str);
	void initialize_tag_table();
	void scroll();
	void set_scroll_mark();
	void shrink();
	void on_view_populate_popup(Gtk::Menu* menu);
	void on_set_view_prefs();
	std::vector<Glib::ustring> split(const Glib::ustring& in, const Glib::ustring &delims);
	
//	bool line_numbers;
	Glib::RefPtr <Gtk::TextBuffer> buffer;
		
private:
	Glib::RefPtr <Gtk::TextTagTable> tag_table;
	Glib::RefPtr <Gtk::Builder> refBuilder;
	Gtk::TextView *me;
	bool view_scroll;
	bool capture;
	bool do_shrink;
	std::ofstream cap_os;
	Gtk::FileChooserDialog *capture_dialog;
	Gtk::Dialog *view_dialog;
	Glib::ustring viewFont;
	Glib::ustring theView;
	int cnxn;
	Glib::ustring color_fg, color_bg;
	std::map<int, Glib::ustring> AnsiCodes;
};

// strip ANSI codes from a string
inline Glib::ustring stripAnsiCodes(const Glib::ustring &text)
{
  Glib::ustring str("");

  int length = (int)text.size();
  for (int i = 0; i < length; i++) {
    if (text[i] == ((char) 0x1B)) { // esc char to begin ANSI codes (\033)
      i++;
      if ((i < length) && (text[i] == '[')) {
				i++;
				while ((i < length) && ((text[i] == ';') || (Glib::Ascii::isdigit(text[i])))) {
	  			i++;
				}
      }
		} else {
      str += text[i];
    }
  }
  return str;
}

#endif // _view_h_
