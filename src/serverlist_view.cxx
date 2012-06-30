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

#include "serverlist_view.h"

// score compare function to sort the server list
static bool total_cmp(serverInfo* a, serverInfo* b)
{
	return (a->sih.totalPlayers > b->sih.totalPlayers);
}


serverListView::serverListView()
{

}

serverListView::~serverListView()
{

}

void serverListView::init(Glib::RefPtr <Gtk::Builder> _refBuilder, Glib::RefPtr<Gnome::Conf::Client> _client)
{
	view::init(_refBuilder, "serverlist_view", _client);
	refBuilder = _refBuilder;
	
	// connect the close button signal handler
	Gtk::Button *button;
	refBuilder->get_widget("list_tab_close_button", button);
	button->signal_clicked().connect((sigc::mem_fun(*this, &serverListView::on_list_tab_close_button_pressed)));
	
	add_text("Server List has not been queried\n", "default");
}

void serverListView::on_list_tab_close_button_pressed()
{
	Gtk::Notebook *nb;
	refBuilder->get_widget("notebook", nb);
	Gtk::Widget *page = nb->get_nth_page(Page_ServerList);
	
	page->hide();
}


// display the serverInfo list
void serverListView::display(std::vector<serverInfo*> &list)
{
	// clear the text buffer
	clear();

	if (list.size() <= 0) {
		Glib::ustring msg(Color(RedFg));
		msg += ">>> ";
		msg += "No ";
		msg += ServerVersion;
		msg += " servers found <<<\n";
		add_text(msg);
		return;
	}
		
	// sort the list by total number of players
	sort(list.begin(), list.end(), total_cmp);
	
	Glib::ustring server(Color(RedFg));
	server += "Server List - sorted";
	server += Glib::ustring::compose("...(%1 found)\n", list.size());

	server += Color(GreenFg) + "=================================================\n";

	add_text(server);
	
	// create the server statistics
	serverStats ss;
	accumulate_serverStats(list, ss);
	// display the statistics
	display_serverStats(ss);
	// and of course, a list of game servers...
	display_listServers(list);
	
	server = Color(GreenFg) + "=================================================\n";
	server += Color(RedFg) + "End of Server List\n";
	add_text(server);
}

void serverListView::accumulate_serverStats(std::vector<serverInfo*> & si, serverStats& ss)
{
	std::vector<serverInfo*>::iterator it = si.begin();
	memset(&ss, 0, sizeof(serverStats));
	while (it != si.end()) {
		ss.total_players += (*it)->sih.totalPlayers;
		ss.total_rogues += (*it)->sih.rogueCount;
		ss.total_reds += (*it)->sih.redCount;
		ss.total_greens += (*it)->sih.greenCount;
		ss.total_blues += (*it)->sih.blueCount;
		ss.total_purples += (*it)->sih.purpleCount;
		ss.total_observers += (*it)->sih.observerCount;
		it++;
	}
}

void serverListView::display_serverStats(serverStats & ss)
{
	Glib::ustring str(Color(YellowFg));

	str += Glib::ustring::compose("      Rogues:      %1\n", ss.total_rogues);
	str += Color(RedFg);
	str += Glib::ustring::compose("      Reds:        %1\n", ss.total_reds);
	str += Color(GreenFg);
	str += Glib::ustring::compose("      Greens:      %1\n", ss.total_greens);
	str += Color(BlueFg);
	str += Glib::ustring::compose("      Blues:       %1\n", ss.total_blues);
	str += Color(PurpleFg);
	str += Glib::ustring::compose("      Purples:     %1\n", ss.total_purples);
	str += Color(CyanFg);
	str += Glib::ustring::compose("Total Observers:   %1\n", ss.total_observers);
	str += Color(GoldenFg);
	str += Glib::ustring::compose("Total Players:     %1\n", ss.total_players - ss.total_observers);
	str += Color(Cyan4Fg);
	str += Glib::ustring::compose("Total:             %1\n", ss.total_players);
	str += Color(WhiteFg);
	str += "============================================================\n";
	
	add_text(str);
}

void serverListView::display_listServers(std::vector<serverInfo*> & si)
{
	Glib::ustring listing("");
	int idx = 1;
	std::vector<serverInfo*>::iterator it = si.begin();
	
	while (it != si.end()) {
		listing.erase(); // clear the ustring
		listing = Color(WhiteFg) + Glib::ustring::compose("%1) ", idx);
		listing += Color(BlueFg) + Glib::ustring::compose("%1", (*it)->name);
		if ((*it)->port != DefaultPort) {
			listing += Color(RedFg) + Glib::ustring::compose(":%1", (*it)->port);
		}
		listing += Color(GreenFg) + Glib::ustring::compose(";  %1\n", (*it)->desc);
		
		if ((*it)->sih.gameType == ClassicCTF) {
			listing += Color(GreenFg) + "Classic CTF";
		} else if ((*it)->sih.gameType == RabbitChase) {
			listing += Color(GreenFg) + "Rabbit Chase";
		} else if ((*it)->sih.gameType == OpenFFA) {
			listing += Color(GreenFg) + "Open FFA (Teamless)";
		} else {
			listing += Color(GreenFg) + "Team FFA";
		}
		
		Glib::ustring oridinance(Color(YellowFg));
		guint16 shots = (*it)->sih.maxShots;
		oridinance += Glib::ustring::compose(" %1 shot%2", shots, shots > 1 ? "s" : "");
		listing += oridinance;
		
		Glib::ustring options(Color(RedFg));
		if ((*it)->sih.gameOptions & SuperFlagGameStyle) {
			options += " SuperFlags,";
		}
		if ((*it)->sih.gameOptions & JumpingGameStyle) {
			options += " Jumping,";
		}
		if ((*it)->sih.gameOptions & RicochetGameStyle) {
			options += " Ricochet,";
		}
		if ((*it)->sih.gameOptions & ShakableGameStyle) {
			options += " Shaking,";
		}
		if ((*it)->sih.gameOptions & AntidoteGameStyle) {
			options += " Antidote,";
		}
		if ((*it)->sih.gameOptions & HandicapGameStyle) {
			options += " Handicap,";
		}
		guint16 timeLimit = (*it)->sih.maxTime;
		if (timeLimit != 0) {
			char buf[60];
			if (timeLimit >= 3600)
	      sprintf(buf, " %d:%02d:%02d", timeLimit / 3600, (timeLimit / 60) % 60, timeLimit % 60);
	    else if (timeLimit >= 60)
	      sprintf(buf, " %d:%02d", timeLimit / 60, timeLimit % 60);
	    else
	      sprintf(buf, " 0:%02d", timeLimit);
	      
	    options += buf;
			options += " Time Limit";
		}
		listing += options + "\n";

		listing += Color(OrangeFg) + Glib::ustring::compose("(%1) ", (*it)->sih.totalPlayers);
		
		if ((*it)->sih.rogueMax > 0) {
			listing += Color(YellowFg) + Glib::ustring::compose("(rogue %1/%2) ", (*it)->sih.rogueCount, (*it)->sih.rogueMax);
		}
		if ((*it)->sih.redMax > 0) {
			listing += Color(RedFg) + Glib::ustring::compose("(red %1/%2) ", (*it)->sih.redCount, (*it)->sih.redMax);
		}
		if ((*it)->sih.greenMax > 0) {
			listing += Color(GreenFg) + Glib::ustring::compose("(green %1/%2) ", (*it)->sih.greenCount, (*it)->sih.greenMax);
		}
		if ((*it)->sih.blueMax > 0) {
			listing += Color(BlueFg) + Glib::ustring::compose("(blue %1/%2) ", (*it)->sih.blueCount, (*it)->sih.blueMax);
		}
		if ((*it)->sih.purpleMax > 0) {
			listing += Color(PurpleFg) + Glib::ustring::compose("(purple %1/%2) ", (*it)->sih.purpleCount, (*it)->sih.purpleMax);
		}
		if ((*it)->sih.observerMax > 0) {
			listing += Color(CyanFg) + Glib::ustring::compose("(observer %1/%2)", (*it)->sih.observerCount, (*it)->sih.observerMax);
		}
		listing += Color(WhiteFg) + "\n============================================================\n";
		
		add_text(listing);
		
		it++;
		idx++;
	}
}

