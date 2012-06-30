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
#include "cmdLine.h"

cmdLine::cmdLine() : Gtk::Entry()
{

}

void cmdLine::init(Glib::RefPtr <Gtk::Builder> _refBuilder)
{
	refBuilder = _refBuilder;
	refBuilder->get_widget("command_entry", command);
	grab_focus();
	
	// Remove the placeholder Gtk::ComboBox and add a Gtk::ComboTextBox
	// We don't need the combobox and it's TreeModel, we just want to
	// display strings. This is the current target that we send messages
	// and commands to.
	Gtk::HBox *box;
	refBuilder->get_widget("hbox5", box);
	
	Gtk::ComboBox *crap;
	refBuilder->get_widget("placeholder", crap);
	
	box->remove(*crap);
	box->pack_start(_targets, 0 , 0, 5);
	
	// clear the history & set the history iterator
	history.clear();
	hist_it = history.begin();
	hist_reset = true;
	
	// default message targets
	// players will be added as they join
	add_target(AllPlayers, "All");
	add_target(MyTeam, "Team");
	add_target(AdminPlayers, "Admin");
	
	_targets.set_tooltip_text("F7 - Admin, F8 - Team, F9 - All");
	_targets.set_has_tooltip();
	_targets.set_active(0);
	_targets.show();
}

void cmdLine::grab_focus()
{
	command->grab_focus();
}

void cmdLine::clear()
{
	command->set_text("");
}

bool cmdLine::get_focus()
{
	return command->is_focus();
}

void cmdLine::add_target(guint8 id, Glib::ustring callsign)
{
	// add the player to the target map
	target_map[callsign] = id;
	// add the callsign to the combobox
	_targets.append_text(callsign);
}

void cmdLine::remove_target(guint8 id, Glib::ustring callsign)
{
	// if current target is to be removed, change current target to "All"	
	if (id == target_map[_targets.get_active_text()])
		_targets.set_active(0);
	// remove from the target map
	std::map<Glib::ustring, int>::iterator it = target_map.find(callsign);
	target_map.erase(it);
	// remove from item from the combobox
	_targets.remove_text(callsign);
}

int cmdLine::get_current_target_id()
{
	// return the PlayerID of the current message target
	return target_map[_targets.get_active_text()];
}

void cmdLine::clear_targets()
{
	// clear all the targets
	_targets.clear_items();
	// add the default targets back in
	add_target(AllPlayers, "All");
	add_target(MyTeam, "Team");
	add_target(AdminPlayers, "Admin");
	_targets.set_active(0);
}

void cmdLine::set_target_admin()
{
	_targets.set_active(2);
}

void cmdLine::set_target_all()
{
	_targets.set_active(0);
}

void cmdLine::set_target_team()
{
	_targets.set_active(1);
}

void cmdLine::set_auto_cmd()
{
	Glib::RefPtr<Gtk::EntryCompletion> completion;
	completion = Gtk::EntryCompletion::create();
	
	Glib::RefPtr<Gtk::TreeModel> completion_model;
	completion_model = create_completion_model();
	
	completion->set_model(completion_model);
	completion->set_text_column(0);
	
	completion->set_popup_single_match(false);
	completion->set_inline_completion(true);
	completion->set_inline_selection(true);
	completion->set_popup_completion(false);
	
	command->set_completion(completion);
}

Glib::RefPtr<Gtk::TreeModel> cmdLine::create_completion_model()
{
	const gchar *cmds[] = {
		"/?",
		"/ban",
		"/banlist",
		"/checkip",
		"/clientquery",
		"/countdown",
		"/date",
		"/flag",
		"/flaghistory",
		"/gameover",
		"/grouplist",
		"/groupperms",
		"/help",
		"/hostban",
		"/hostbanlist",
		"/hostunban",
		"/idban",
		"/idbanlist",
		"/idlestats",
		"/idletime",
		"/idlist",
		"/idunban",
		"/jitterdrop",
		"/jitterwarn",
		"/join",
		"/kick",
		"/kill",
		"/lagdrop",
		"/lagstats",
		"/lagwarn",
		"/leave",
		"/luaserver",
		"/list",
		"/listplugins",
		"/loadplugin",
		"/masterban",
		"/modcount",
		"/msg",
		"/mute",
		"/owner",
		"/packetlossdrop",
		"/packetlosswarn",
		"/part",
		"/password",
		"/playerlist",
		"/poll",
		"/quit",
		"/record",
		"/reload", 
	  "/removegroup",
	  "/replay",
	  "/report",
	  "/reset",
	  "/reverse",
		"/say",
		"/sendhelp",
		"/serverdebug",
		"/serverquery",
		"/set",
		"/setgroup",
		"/showgroup",
		"/showperms",
		"/shutdownserver",
		"/superkill",
		"/time",
		"/unban",
		"/unmute",
		"/unloadplugin",
		"/uptime",
		"/veto",
		"/viewreports",
		"/vote",
		NULL, // NULL terminate the list
	};
	
	Glib::RefPtr<Gtk::ListStore> store;
	store = Gtk::ListStore::create(column);

	gint k = 0;
	while (cmds[k] != NULL) {
		Gtk::TreeRow row = *(store->append());
		row[column.cmd] = Glib::ustring(cmds[k]);
		k++;
	}
	return store;
}

void cmdLine::historyAdd(Glib::ustring line)
{
	// add the line to the history
	history.push_front(line);
	// reset the beginning of the list
	hist_it = history.begin();
	hist_reset = true;
}

Glib::ustring cmdLine::historyUp()
{
	if (hist_it != history.end() && !hist_reset) {
		hist_it++;
		if (hist_it == history.end()) { // hist_it is one element past the end of the list
			hist_it--;									  // back up
		}
	} else {
		hist_reset = false;
	}
	Glib::ustring line("");
	line = *hist_it;
	
	return line;
}

Glib::ustring cmdLine::historyDown()
{
	if (hist_it == history.begin()) {
		Glib::ustring emptyLine("");
		return emptyLine;
	}
	if (hist_it != history.begin())
		hist_it--;
				
	Glib::ustring line("");
	line = *hist_it;
	
	return line;
}

void cmdLine::historyDelete(Glib::ustring line)
{
	std::list<Glib::ustring>::iterator it = history.begin();
   while(it != history.end() ) {
     std::list<Glib::ustring>::iterator thisone = it;
     it++;
     if (*thisone == line) {
        history.erase(thisone);
     }
   }
}

