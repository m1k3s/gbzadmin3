#ifndef _players_h_
#define _players_h_
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

#include <gtkmm.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/utsname.h>
#include <cstdio>

#include "config.h"
#include "common.h"

// a single player class
class Player
{
public:
	Player() {}
	~Player() {}
	
	void add(guint8 _id, guint16 _type, guint16 _team, guint16 _wins, guint16 _losses, guint16 _tks, 
						Glib::ustring _callsign, Glib::ustring _motto, time_t joined);
						
	void add(guint8 _id, guint16 _type, guint16 _team, guint16 _wins, guint16 _losses, guint16 _tks, 
						Glib::ustring _callsign, Glib::ustring _motto);
	
	void change_to_hunter() { team = HunterTeam; }
	double duration(time_t now) { return difftime(now, time_joined); }
						
	void set_id(guint8 _id) { id = _id; }
	void set_type(guint16 _type) { type = _type; }
	void set_team(guint16 _team) { team = _team; }
	void set_wins(guint16 _wins) { wins = _wins; }
	void set_losses(guint16 _losses) { losses = _losses; }
	void set_tks(guint16 _tks) { tks = _tks; }
	void set_score(guint16 _wins, guint16 _losses) { wins = _wins; losses = _losses; }
	void set_callsign(Glib::ustring _callsign) { callsign = _callsign; }
	void set_motto(Glib::ustring _motto) { motto = _motto; }
	void set_time_joined(time_t _joined) { memcpy((time_t*)&time_joined, &_joined, sizeof(time_t)); }
	void set_paused(bool _paused) { paused = _paused; }
	void set_alive(bool _alive) { alive = _alive; }
	void set_autoPilot(bool is_auto) { autopilot = is_auto; }
	void set_status(guint8 _status) { status = _status; }
	void set_registered(guint8 registered) { status |= registered; }
	void set_verified(guint8 verified) { status |= verified; }
	void set_admin(guint8 admin) { status |= admin; }
	void set_IP(Glib::ustring _ip) { IP = _ip; }
	void set_IP(struct in_addr addr);
	void set_flag(Glib::ustring _flag) { flag = _flag; flagged = true; }
	void clear_flag() { flag.clear(); flagged = false; }
	
	Glib::ustring get_IP() { return IP; }
	Glib::ustring get_callsign() { return callsign; }
	Glib::ustring get_motto() { return motto; }
	guint8 get_id() { return id; }
	guint8 get_status() { return status; }
	bool get_registered() { return ((status & IsRegistered) != 0); }
	bool get_verified() { return ((status & IsVerified) != 0); }
	bool get_admin() { return ((status & IsAdmin) != 0); }
	bool get_paused() { return paused; }
	bool get_alive() { return alive; }
	bool get_autopilot() { return autopilot; }
	bool get_observer() { return (team == ObserverTeam); }
	bool has_flag() { return flagged; }
	gfloat get_strength_index();
	guint16 get_team() { return team; }
	gint16 get_score() { return (gint16)(wins - losses); }
	guint16 get_wins() { return wins; }
	guint16 get_losses() { return losses; }
	guint16 get_tks() { return tks; }
	gint get_rabbit_score();
	Glib::ustring get_flag() { return flag; }
	gint get_type() { return type; }
	time_t get_time_joined() { return time_joined; }
	
protected:
	
	
private:
	Glib::ustring callsign;
	Glib::ustring IP;
	guint16 team;					// team index, rogue, red, green, blue, purple, obs
	guint16 wins, losses;
	guint16 tks;					// team kills
	guint8 id;						// player ID, server slot
	Glib::ustring motto;
	gint type;						// type, TankPlayer | ComputerPlayer
	guint8 status;				// player Registered | Verified | Admin
	bool paused;
	bool alive;
	bool autopilot;
	bool flagged;					// whether or not player has a flag
	time_t time_joined;
	Glib::ustring flag;		// players flag description, if carrying
};

#endif /* _players_h_ */

