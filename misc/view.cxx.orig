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
// 
// Base class for the views

#include "view.h"

// ANSI code Gtk::TextTag equivalents
const gchar *TermColors[] = {
  // foreground colors ////
	"rabbit",		// white, actually
	"red", 			// red
	"green", 		// green
	"rogue",		// yellow
	"blue", 		// blue
	"purple", 		// purple
	"observer", 	// cyan3
	"default", 		// gainesboro
	// custom fg colors /////
	"hunter",  		// orange
	"server",		// goldenrod
	"brdcast",		// cyan4
	"srvdirect",	// PaleGreen4
	// background colors ////
	"black_bg",		// black
	"red_bg", 		// red
	"green_bg", 	// green
	"yellow_bg",	// yellow
	"blue_bg", 		// blue
	"purple_bg", 	// purple
	"cyan3_bg",		// cyan3
	"white_bg", 	// gainesboro
	// custom bg colors /////
	"orange_bg",  	// orange
	"golden_bg",	// goldenrod
	"cyan4_bg",		// cyan4
	"pgreen4",		// PaleGreen4
};

view::view()
	: Gtk::TextView(), capture(false), viewFont("Monospace 9")
{
	capture_dialog = 0;
	view_dialog = 0;
	color_fg = "white";
	color_bg = "black";
	cnxn = 0;
}

view::~view()
{
	if(capture)
		cap_os.close();
}

Glib::ustring view::Color(int c)
{
	return AnsiCodes[c];
}

// initialize colors to ANSI color strings, sorta
// some are "custom" colors
void view::initialize_colors()
{
	AnsiCodes[BlackFg]		= Glib::ustring("\033[30m");	// black foreground
	AnsiCodes[RedFg] 		= Glib::ustring("\033[31m");	// red foreground
	AnsiCodes[GreenFg] 		= Glib::ustring("\033[32m");	// green foreground
	AnsiCodes[YellowFg] 	= Glib::ustring("\033[33m");	// yellow foreground
	AnsiCodes[BlueFg] 		= Glib::ustring("\033[34m");	// blue foreground
	AnsiCodes[PurpleFg] 	= Glib::ustring("\033[35m");	// purple foreground
	AnsiCodes[CyanFg] 		= Glib::ustring("\033[36m");	// cyan3 foreground
	AnsiCodes[WhiteFg]		= Glib::ustring("\033[37m");	// gainsboro foreground

	AnsiCodes[OrangeFg] 	= Glib::ustring("\033[130m");	// orange foreground
	AnsiCodes[GoldenFg] 	= Glib::ustring("\033[131m"); 	// goldenrod foreground
	AnsiCodes[Cyan4Fg] 		= Glib::ustring("\033[132m");	// cyan4 foreground
	AnsiCodes[PGreen4Fg] 	= Glib::ustring("\033[133m");	// palegreen4 foreground

	AnsiCodes[BlackBg] 		= Glib::ustring("\033[40m");	// black background
	AnsiCodes[RedBg] 		= Glib::ustring("\033[41m");	// red background
	AnsiCodes[GreenBg] 		= Glib::ustring("\033[42m");	// green background
	AnsiCodes[YellowBg] 	= Glib::ustring("\033[43m");	// yellow background
	AnsiCodes[BlueBg] 		= Glib::ustring("\033[44m");	// blue background
	AnsiCodes[PurpleBg] 	= Glib::ustring("\033[45m");	// purple background
	AnsiCodes[CyanBg] 		= Glib::ustring("\033[46m");	// cyan background
	AnsiCodes[WhiteBg] 		= Glib::ustring("\033[47m");	// gainesboro background

	AnsiCodes[OrangeBg] 	= Glib::ustring("\033[140m");	// orange background
	AnsiCodes[GoldenBg] 	= Glib::ustring("\033[141m");	// goldenrod background
	AnsiCodes[Cyan4Bg] 		= Glib::ustring("\033[142m");	// cyan4 background
	AnsiCodes[PGreen4Bg] 	= Glib::ustring("\033[143m");	// palegreen4 background
}

void view::initialize_tag_table()
{
	tag_table = Gtk::TextTagTable::create();
    
  // create the tag table, foreground colors first
	Glib::RefPtr<Gtk::TextTag> tag_fg = Gtk::TextTag::create("rabbit");
	tag_fg->property_foreground() = "white"; // Color(WhiteFg)
 	tag_table->add(tag_fg);
 	
 	tag_fg = Gtk::TextTag::create("red");
	tag_fg->property_foreground() = "red"; // Color(RedFg)
 	tag_table->add(tag_fg);
 	
 	tag_fg = Gtk::TextTag::create("green");
	tag_fg->property_foreground() = "green"; // Color(GreenFg)
 	tag_table->add(tag_fg);
 	
	tag_fg = Gtk::TextTag::create("rogue");
	tag_fg->property_foreground() = "yellow"; // Color(YellowFg)
	tag_table->add(tag_fg);
	
 	tag_fg = Gtk::TextTag::create("blue");
	tag_fg->property_foreground() = "DodgerBlue3"; // Color(BlueFg)
 	tag_table->add(tag_fg);
 	
 	tag_fg = Gtk::TextTag::create("purple");
	tag_fg->property_foreground() = "purple"; // Color(PurpleFg)
 	tag_table->add(tag_fg);
 	
 	tag_fg = Gtk::TextTag::create("observer");
	tag_fg->property_foreground() = "cyan3"; // Color(CyanFg)
 	tag_table->add(tag_fg);
 	
	tag_fg = Gtk::TextTag::create("default");
	tag_fg->property_foreground() = "gainsboro"; // Color(WhiteFg)
 	tag_table->add(tag_fg);

 	tag_fg = Gtk::TextTag::create("hunter");
	tag_fg->property_foreground() = "orange"; // Color(OrangeFg)
 	tag_table->add(tag_fg);
 	
 	tag_fg = Gtk::TextTag::create("server");
	tag_fg->property_foreground() = "goldenrod"; // Color(GoldenFg)
 	tag_table->add(tag_fg);
 	
	tag_fg = Gtk::TextTag::create("brdcast");
	tag_fg->property_foreground() = "cyan4"; // Color(Cyan4Fg)
	tag_fg->property_weight() = Pango::WEIGHT_BOLD;
	tag_fg->property_style() = Pango::STYLE_ITALIC;
 	tag_table->add(tag_fg);
	
 	tag_fg = Gtk::TextTag::create("srvdirect");
	tag_fg->property_foreground() = "PaleGreen4";	 // Color(PGreen4Fg)
 	tag_table->add(tag_fg);
	
	// background colors
	tag_fg = Gtk::TextTag::create("black_bg");	 // Color(BlackBg]
	tag_fg->property_background() = "black";
	tag_table->add(tag_fg);

	tag_fg = Gtk::TextTag::create("red_bg");
	tag_fg->property_background() = "red";	 // Color(RedBg]
	tag_table->add(tag_fg);
	
	tag_fg = Gtk::TextTag::create("green_bg");
	tag_fg->property_background() = "green";	 // Color(GreenBg]
	tag_table->add(tag_fg);
	
	tag_fg = Gtk::TextTag::create("yellow_bg");
	tag_fg->property_background() = "yellow";	 // Color(YellowBg]
	tag_table->add(tag_fg);
	
	tag_fg = Gtk::TextTag::create("blue_bg");
	tag_fg->property_background() = "blue";	 // Color(BlueBg]
	tag_table->add(tag_fg);
	
	tag_fg = Gtk::TextTag::create("purple_bg");
	tag_fg->property_background() = "purple";	 // Color(PurpleBg]
	tag_table->add(tag_fg);
	
	tag_fg = Gtk::TextTag::create("cyan3_bg");
	tag_fg->property_background() = "cyan3";	 // Color(CyanBg]
	tag_table->add(tag_fg);
	
	tag_fg = Gtk::TextTag::create("white_bg");
	tag_fg->property_background() = "white";	 // Color(WhiteBg]
	tag_table->add(tag_fg);
	
	tag_fg = Gtk::TextTag::create("orange_bg");
	tag_fg->property_background() = "orange";	 // Color(OrangeBg]
	tag_table->add(tag_fg);
	
	tag_fg = Gtk::TextTag::create("golden_bg");
	tag_fg->property_background() = "goldenrod";	 // Color(GoldenBg]
	tag_table->add(tag_fg);
	
	tag_fg = Gtk::TextTag::create("cyan4_bg");
	tag_fg->property_background() = "cyan4";	 // Color(Cyan4Bg]
	tag_table->add(tag_fg);
	
	tag_fg = Gtk::TextTag::create("pgreen4_bg");
	tag_fg->property_background() = "PaleGreen4";	 // Color(PGreen4Bg]
	tag_table->add(tag_fg);
}

void view::init(Glib::RefPtr <Gtk::Builder> _refBuilder, const gchar* which)
{
	refBuilder = _refBuilder;
	theView = which;
	
	refBuilder->get_widget(theView, me);
	refBuilder->get_widget("capture_dialog", capture_dialog);
	refBuilder->get_widget("view_dialog", view_dialog);
	
	// set default font
	me->Gtk::Widget::modify_font(Pango::FontDescription(viewFont));
	
	view_scroll = false;
	capture = false;
	do_shrink = false;

	// intercept the popup menu and add font selection item	
	me->signal_populate_popup().connect(sigc::mem_fun(*this, &view::on_view_populate_popup));
	
	// set up the view colors
	Gdk::Color color;
	color.parse(color_fg);
	me->Gtk::Widget::modify_text(Gtk::STATE_NORMAL, color);
	color.parse(color_bg);
	me->Gtk::Widget::modify_base(Gtk::STATE_NORMAL, color);
	
	initialize_colors();
	initialize_tag_table();
	add_buffer();
}

void view::set_fg(Glib::ustring fg)
{
	color_fg = fg;
	Gdk::Color color;
	color.parse(color_fg);
	me->Gtk::Widget::modify_text(Gtk::STATE_NORMAL, color);
}

void view::set_bg(Glib::ustring bg)
{
	color_bg = bg;
	Gdk::Color color;
	color.parse(color_bg);
	me->Gtk::Widget::modify_base(Gtk::STATE_NORMAL, color);
}

void view::add_buffer()
{
	buffer = Gtk::TextBuffer::create(tag_table);
	me->set_buffer(buffer);
	set_scroll_mark();
}

// add colorized text to the view using the tagtable
void view::add_text(const gchar *str, const gchar *tag)
{
	Gtk::TextIter iter;
	std::list<const gchar*> tags(1, tag);
	
	Glib::ustring str2 = Glib::convert_with_fallback(str, "UTF-8", "ISO-8859-1");

	iter = buffer->end();
	if (tag == NULL)
		buffer->insert(iter, str2.c_str());
	else
		buffer->insert_with_tags_by_name(iter, str2.c_str(), tags);
		
	scroll();
	shrink();
	if (capture)
		cap_os << str;
}

// add colorized text to the view using the tagtable
void view::add_text(Glib::ustring str, Glib::ustring tag)
{
	Gtk::TextIter iter;
	std::list<const gchar*> tags(1, tag.c_str());
	
	str = Glib::convert_with_fallback(str.c_str(), "UTF-8", "ISO-8859-1");

	iter = buffer->end();
	if (tag.size() == 0)
		buffer->insert(iter, str.c_str());
	else
		buffer->insert_with_tags_by_name(iter, str.c_str(), tags);
		
	scroll();
	shrink();
	if (capture)
		cap_os << str;
}

// This is the ANSI version
// str can have embedded ANSI codes
// parse the ANSI codes and colorize the string
void view::add_text(Glib::ustring &str)
{
	if (str.size() == 0)
    return;

	// Break the text every time an ANSI code
	// is encountered and do an add_text() for each
	// segment, with the appropriate tagColor

  Glib::ustring tagColor;

 	tagColor = TermColors[WhiteFg];

  // ANSI code interpretation is customized
  bool done = false;
  int start = 0;
  int end = (int)str.find("\033[", start);
  
  // run at least once
  if (end == -1) {
    end = (int)str.size();
    done = true;
  }

  // split string into parts based on the embedded ANSI codes, render each separately
  while (end >= 0) {
   // render text
    int len = end - start;
    if (len > 0) {
      Glib::ustring tmpText;
      tmpText.assign(str, start , len);
      add_text(Glib::convert_with_fallback(tmpText.c_str(), "UTF-8", "ISO-8859-1"), tagColor);
    }
    if (!done) {
      start = (int)str.find('m', end) + 1;
    }
    // we stopped sending text at an ANSI code, find out what it is
    // and do something about it
    if (end != (int)str.size()) {
      Glib::ustring tmpText = str.substr(end, (str.find('m', end) - end) + 1);
      for (int i = BlackFg; i < LastColor; i++) {
				if (tmpText == Color(i)) {
					tagColor = TermColors[i];
					break;
				}
      }
    }
    end = (int)str.find("\033[", start);
    if ((end == -1) && !done) {
      end = (int)str.size();
      done = true;
    }
  }
  scroll();
  shrink();
}

// create a 'scroll' mark to use in scrolling the message view
void view::set_scroll_mark()
{
	Gtk::TextIter iter = buffer->end();
	buffer->create_mark ("scroll", iter);
}

void view::scroll()
{
	// keep the current line in view by scrolling to the latest insertion point
	if (view_scroll) {
		Gtk::TextIter iter = buffer->end();
		iter.set_line_offset(0);
		Glib::RefPtr<Gtk::TextMark> mark = buffer->get_mark("scroll");
		buffer->move_mark(mark, iter);
		me->scroll_mark_onscreen(mark);
	} else {
		return;
	}
}

void view::clear()
{
	buffer->erase(buffer->begin(), buffer->end());
}

void view::shrink()
{
	if (do_shrink) {
		const int lines = buffer->get_line_count();
		
		if (lines > MaxLineCount) {
			Gtk::TextIter end_line = buffer->get_iter_at_line(lines - MinLineCount);
			buffer->erase(buffer->begin(), end_line);
		}
	} else {
		return;
	}
}

Glib::ustring view::get_color(int idx)
{
	Glib::ustring color;
	
	switch (idx) {
	case RogueTeam:
		color = Color(YellowFg);
		break;
	case RedTeam:
		color = Color(RedFg);
		break;
	case GreenTeam:
		color = Color(GreenFg);
		break;
	case BlueTeam:
		color = Color(BlueFg);
		break;
	case PurpleTeam:
		color = Color(PurpleFg);
		break;
	case ObserverTeam:
		color = Color(CyanFg);
		break;
	case RabbitTeam:
		color = Color(WhiteFg);
		break;
	case HunterTeam:
		color = Color(OrangeFg);
		break;
	default:
		color = Color(WhiteFg);
		break;
	}
	return color;
}

bool view::start_capture()
{
	if (capture_dialog->run() == Gtk::RESPONSE_OK) {
		Glib::ustring filename = capture_dialog->get_filename();
		Gtk::CheckButton *save_all;
		refBuilder->get_widget("save_all", save_all);
		bool saveAll = save_all->get_active();
		
		cap_os.open(filename.c_str(), std::ios::app);
		if (cap_os.is_open()) {
			if (saveAll) {
				Glib::ustring text;
				text = get_buffer_text();
				gint size = get_buffer_size();
				cap_os.write(text.c_str(), size);
			}
		} else {
			Glib::ustring str;
			str = Color(GoldenFg) + "*** capture operation has failed\n";
			this->add_text(str);
			capture = false;
			return capture;
		}
		capture = true;
		Glib::ustring str = Color(PurpleFg) + "*** message capture is on\n";
		this->add_text(str);
	}
	capture_dialog->hide();
	return capture;
}

void view::stop_capture()
{
	Glib::ustring str = Color(PurpleFg) + "*** message capture is off\n";
	this->add_text(str);
	capture = false;
	cap_os.close();
}

void view::on_set_view_prefs()
{
	Glib::ustring fontname;
	
	Gtk::FontSelection *fs = 0;
	refBuilder->get_widget("fontselection", fs);
	Gtk::ColorButton *fg = 0;
	refBuilder->get_widget("fg_color", fg);
	Gtk::ColorButton *bg = 0;
	refBuilder->get_widget("bg_color", bg);
	
	// setup the prefs dialog
	fs->set_font_name(viewFont);
	
	Gdk::Color fg_col, bg_col;
		
	fg_col.parse(color_fg);
	fg->set_color((const Gdk::Color)color_fg);
	
	bg_col.parse(color_bg);
	bg->set_color((const Gdk::Color)color_bg);
	
	// run the dialog
	if (view_dialog->run() == Gtk::RESPONSE_OK) {
		// retrieve the results	and update the view	
		fg_col = fg->get_color();
		color_fg = fg_col.to_string();
		me->Gtk::Widget::modify_text(Gtk::STATE_NORMAL, fg_col);
		
		bg_col = bg->get_color();
		color_bg = bg_col.to_string();
		me->Gtk::Widget::modify_base(Gtk::STATE_NORMAL, bg_col);
		
		fontname = fs->get_font_name();
		me->Gtk::Widget::modify_font(Pango::FontDescription(fontname));
		viewFont = fontname;
	}
	view_dialog->hide();
}

void view::on_view_populate_popup(Gtk::Menu* menu)
{
	Gtk::ImageMenuItem *prefsItem = new Gtk::ImageMenuItem(Gtk::Stock::PREFERENCES);
	prefsItem->signal_activate().connect(sigc::mem_fun(*this, &view::on_set_view_prefs));
	prefsItem->show_all();
	menu->prepend(*prefsItem);
}

void view::set_view_font(Glib::ustring font)
{ 
	viewFont = font;
	me->Gtk::Widget::modify_font(Pango::FontDescription(viewFont));
}

std::vector<Glib::ustring> view::split(const Glib::ustring& in, const Glib::ustring &delims)
{
	std::vector<Glib::ustring> tokens;
	Glib::ustring currentToken("");
	const Glib::ustring::size_type len = in.size();
	Glib::ustring::size_type pos = in.find_first_not_of(delims);
	int currentChar = (pos == Glib::ustring::npos) ? -1 : in[pos];

	while (pos < len && pos != Glib::ustring::npos) {
	  bool tokenDone = false;
	  currentChar = (pos < len) ? in[pos] : -1;
	  while ((currentChar != -1) && !tokenDone) {
			tokenDone = false;
			if (delims.find(char(currentChar)) != Glib::ustring::npos) {
				pos ++;
				break;
			}
			currentToken += char(currentChar);
			pos++;
			currentChar = (pos < len) ? in[pos] : -1;
	  }
	  if (currentToken.size() > 0) {
			tokens.push_back(currentToken);
			currentToken.clear();
	  }
	  if ((pos < len) && (pos != std::string::npos)) {
			pos = in.find_first_not_of(delims,pos);
	  }
	}
	if (pos != std::string::npos) {
	  std::string lastToken = in.substr(pos);
	  if (lastToken.size() > 0)
			tokens.push_back(lastToken);
	}
	return tokens;
}


