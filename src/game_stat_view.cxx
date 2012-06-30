// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2009 Michael Sheppard
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

#include "game_stat_view.h"

const char *team_str[] = {
	"Rogue Team",
	"Red Team",
	"Green Team",
	"Blue Team",
	"Purple Team",
	"Observer Team",
	"Rabbit Team",
	"Hunter Team",
	NULL
};

void gameStatView::init(Glib::RefPtr <Gtk::Builder> _refBuilder, Glib::RefPtr<Gnome::Conf::Client> _client)
{
	view::init(_refBuilder, "game_view", _client);
	set_scroll(false);
}

void gameStatView::update()
{
	Glib::ustring output(Color(RedFg));

	clear();
	
	// print info
	output += "Game Stats\n";
	
	output += Color(GreenFg);
	output += "=======================================================\n";

	output += Color(RedFg);
	
	output += "Type: ";
	output += Color(WhiteFg);
	
	if (type == ClassicCTF) {
		output += "Classic CTF";
	} else if (type == RabbitChase) {
		output += "Rabbit Chase";
	} else if (type == OpenFFA) {
		output += "Open FFA (Teamless)";
	} else {
		output += "Team FFA";
	}
	output += Color(RedFg);
	output += "  Options:";
	output += Color(WhiteFg);
	
	if (options & SuperFlagGameStyle) {
		output += " SuperFlags";
	}
	if (options & JumpingGameStyle) {
		output += " Jumping";
	}
	if (options & RicochetGameStyle) {
		output += " Ricochet";
	}
	if (options & ShakableGameStyle) {
		output += " Shaking";
	}
	if (options & AntidoteGameStyle) {
		output += " Antidote";
	}
	if (options & HandicapGameStyle) {
		output += " Handicap";
	}
	output += "\n";
	add_text(output);

	output = Color(GreenFg);
	output += "=======================================================\n";
	//add_text(output);
	
	// FIXME: C-Style formatting
	char tmp_buf[256];
	output += Color(GoldenFg);
	::snprintf(tmp_buf, sizeof(tmp_buf), "maximum players: %d\n", maxPlayers);
	output += Glib::ustring(tmp_buf);
	
	::snprintf(tmp_buf, sizeof(tmp_buf), "maximum shots: %d\n", maxShots);
	output += Glib::ustring(tmp_buf);
	
	::snprintf(tmp_buf, sizeof(tmp_buf), "team sizes: %2d %2d %2d %2d %2d %2d "
									"(rogue red green blue purple observer)\n",
						rogueMax, redMax, greenMax, blueMax, purpleMax, obsMax);
	output += Glib::ustring(tmp_buf);
	
	::snprintf(tmp_buf, sizeof(tmp_buf), "current sizes: %2d %2d %2d %2d %2d %2d\n",
									rogueSize, redSize, greenSize, blueSize, purpleSize, obsSize);
	output += Glib::ustring(tmp_buf);
	
	if (options & ShakableGameStyle) {
		::snprintf(tmp_buf, sizeof(tmp_buf), "wins to shake bad flag: %d\n"
										"time to shake bad flag: %d\n", shakeWins, shakeTimeout/10);
		output += Glib::ustring(tmp_buf);
	}
	::snprintf(tmp_buf, sizeof(tmp_buf), "max player score: %d\n", maxPlayerScore);
	output += Glib::ustring(tmp_buf);
	
	::snprintf(tmp_buf, sizeof(tmp_buf), "max team score: %d\n", maxTeamScore);
	output += Glib::ustring(tmp_buf);
	
	::snprintf(tmp_buf, sizeof(tmp_buf), "max time: %d\n", maxTime / 10);
	output += Glib::ustring(tmp_buf);
	
	::snprintf(tmp_buf, sizeof(tmp_buf), "time elapsed: %d\n", timeElapsed / 10);
	output += Glib::ustring(tmp_buf);
	
	::snprintf(tmp_buf, sizeof(tmp_buf), "total players: %d\n", total_players);
	output += Glib::ustring(tmp_buf);

	::snprintf(tmp_buf, sizeof(tmp_buf), "total observers: %d\n", total_observers);
	output += Glib::ustring(tmp_buf);
	
	output += Color(GreenFg);
	output += "=======================================================\n";
	
	for (int k = 0; k < MaxTeams - 2; k++) {
		output += get_color(k);
		::snprintf(tmp_buf, sizeof(tmp_buf), "%-15s: %2d players, score: %3d  (%3d wins, %3d losses)\n",
										team_str[k], team_size[k],
										team_wins[k] - team_losses[k],
										team_wins[k], team_losses[k]);
		output += Glib::ustring(tmp_buf);
	}
	output += Color(GreenFg);
	output += "=======================================================\n";

	add_text(output);
}

void gameStatView::setTeam(int idx, guint16 _team)
{
	team[idx] = _team;
}

// FIXME: these functions assume team[] has been set
void gameStatView::setTeamSize(int idx, guint16 _size)
{
	team_size[team[idx]] = _size;
}

void gameStatView::setTeamWins(int idx, guint16 _wins)
{
	team_wins[team[idx]] = _wins;
}

void gameStatView::setTeamLosses(int idx, guint16 _losses)
{
	team_losses[team[idx]] = _losses;
}
// END_FIXME
