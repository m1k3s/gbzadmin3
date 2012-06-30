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
 
#include "players.h"


void Player::add(guint8 _id, guint16 _type, guint16 _team, guint16 _wins, guint16 _losses, guint16 _tks, 
						Glib::ustring _callsign, Glib::ustring _motto, time_t joined)
{
	id = _id;
	team = _team;
	wins = _wins;
	losses = _losses;
	tks = _tks;
	callsign = _callsign;
	motto = _motto;
	type = _type;
	memcpy(&time_joined, &joined, sizeof(time_t));
	IP = "";
	
	status = 0;
	autopilot = false;
	alive = true;
	paused = false;
	flagged = false;
}

void Player::add(guint8 _id, guint16 _type, guint16 _team, guint16 _wins, guint16 _losses, guint16 _tks, 
						Glib::ustring _callsign, Glib::ustring _motto)
{
	id = _id;
	team = _team;
	wins = _wins;
	losses = _losses;
	tks = _tks;
	callsign = _callsign;
	motto = _motto;
	type = _type;
	IP = "";
	
	time_joined = time(NULL);
	
	status = 0;
	autopilot = false;
	alive = true;
	paused = false;
	flagged = false;
}

//
// from http://my.bzflag.org/help.php#strengthindex
//
gfloat Player::get_strength_index()
{
	gfloat score = wins - losses;

	// avoid divide by zero
	gfloat tmp = (wins == 0.0f) ? 1.0f : wins;
	
	gfloat si = (score / tmp) * fabs(score / 2.0f);
	
	return si;
}

gint Player::get_rabbit_score()
{
	guint16 sum = wins + losses;
	if (sum == 0)
		return 0.5f;

	float average = (float)wins / (float)sum;
	float penalty = 1.0f - 0.5f / sqrt(sum);
	gint score = (gint)((average * penalty) * 100.0f);
	
	return score;
}

void Player::set_IP(struct in_addr addr)
{
	gchar *ip_str = g_strndup(inet_ntoa(addr), IpLen);
	IP = Glib::ustring(ip_str);
	g_free(ip_str);
}

