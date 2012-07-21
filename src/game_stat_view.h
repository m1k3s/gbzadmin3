#ifndef _game_stat_view_h_
#define _game_stat_view_h_
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
#include <vector>
#include <algorithm>

#include "config.h"
#include "common.h"
#include "view.h"

#define MaxTeams 7

class gameStatView : public view
{
public:
	gameStatView() {}
	~gameStatView() {}
	void init(Glib::RefPtr <Gtk::Builder> _refBuilder/*, Glib::RefPtr<Gnome::Conf::Client> _client*/);
	void update();
	void setType(guint16 tmp) { type = tmp; }
	void setOptions(guint16 tmp) { options = tmp; }
	void setMaxPlayers(guint16 tmp) { maxPlayers = tmp; }
	void setMaxShots(guint16 tmp) { maxShots = tmp; }
	void setTotalPlayers(guint16 tmp) { total_players = tmp; }
	void setRogueSize(guint16 tmp) { rogueSize = tmp; }
	void setRedSize(guint16 tmp) { redSize = tmp; }
	void setGreenSize(guint16 tmp) { greenSize = tmp; }
	void setBlueSize(guint16 tmp) { blueSize = tmp; }
	void setPurpleSize(guint16 tmp) { purpleSize = tmp; }
	void setObsSize(guint16 tmp) { obsSize = tmp; }
	void setRogueMax(guint16 tmp) { rogueMax = tmp; }
	void setRedMax(guint16 tmp) { redMax = tmp; }
	void setGreenMax(guint16 tmp) { greenMax = tmp; }
	void setBlueMax(guint16 tmp) { blueMax = tmp; }
	void setPurpleMax(guint16 tmp) { purpleMax = tmp; }
	void setObsMax(guint16 tmp) { obsMax = tmp; }
	void setShakeWins(guint16 tmp) { shakeWins = tmp; }
	void setShakeTimeOut(guint16 tmp) { shakeTimeout = tmp; }
	void setMaxPlayerScore(guint16 tmp) { maxPlayerScore = tmp; }
	void setMaxTeamScore(guint16 tmp) { maxTeamScore = tmp; }
	void setMaxTime(guint16 tmp) { maxTime = tmp; }
	void setTimeElapsed(guint16 tmp) { timeElapsed = tmp; }
	void setTotalObservers(guint16 tmp) { total_observers = tmp; }
	void setTeam(int idx, guint16 _team);
	void setTeamSize(int idx, guint16 _size);
	void setTeamWins(int idx, guint16 _wins);
	void setTeamLosses(int idx, guint16 _losses);
	
protected:


private:
	guint16 type, options, maxPlayers, maxShots, total_players;
	guint16 rogueSize, redSize, greenSize, blueSize;
	guint16 purpleSize, obsSize, rogueMax, redMax;
	guint16 greenMax, blueMax, purpleMax, obsMax;
	guint16 shakeWins, shakeTimeout, maxPlayerScore;
	guint16 maxTeamScore, maxTime, timeElapsed;
	guint16 team[MaxTeams], team_size[MaxTeams];
	guint16 team_wins[MaxTeams], team_losses[MaxTeams];
	guint16 total_observers;
};

#endif // _game_stat_view_h_
