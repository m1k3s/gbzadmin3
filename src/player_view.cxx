// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2014 Michael Sheppard
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

#include "player_view.h"

// score compare function to sort the player list
bool score_cmp(Player * a, Player * b)
{
    return (a->get_score() > b->get_score());
}

playerView::playerView()
    :	rabbitMode(false)
{

}

void playerView::init(Glib::RefPtr < Gtk::Builder > _refBuilder)
{
    view::init(_refBuilder, "player_view");
    set_scroll(false);
}

// add a player to the view
void playerView::add(Player * player)
{
    guint16 team = player->get_team();
    if (team == ObserverTeam) {
        observers.push_back(player);
    } else {
        players.push_back(player);
    }
}

// remove a player from the view
void playerView::remove(Player * player)
{
    std::vector < Player * >::iterator it;

    guint16 team = player->get_team();
    if (team == ObserverTeam) {
        it = observers.begin();
        while (it != observers.end()) {
            if ((*it)->get_callsign() == player->get_callsign()) {
                observers.erase(it);
                break;
            }
            it++;
        }
    } else {
        it = players.begin();
        while (it != players.end()) {
            if ((*it)->get_callsign() == player->get_callsign()) {
                players.erase(it);
                break;
            }
            it++;
        }
    }
}

Player *playerView::find_player(int id)
{
    Player *p = 0;
    std::vector < Player * >::iterator it;
    bool found = false;

    // search the players first
    it = players.begin();
    while (it != players.end()) {
        if ((*it)->get_id() == id) {
            found = true;
            p = (*it);
            break;
        }
        it++;
    }
    // if we didn't find the player, must be an observer
    if (!found) {
        it = observers.begin();
        while (it != observers.end()) {
            if ((*it)->get_id() == id) {
                found = true;
                p = (*it);
                break;
            }
            it++;
        }
    }
    return p;
}

Player *playerView::find_player(Glib::ustring callsign)
{
    Player *p = 0;
    std::vector < Player * >::iterator it;
    bool found = false;

    // search the players first
    it = players.begin();
    while (it != players.end()) {
        if ((*it)->get_callsign() == callsign) {
            found = true;
            p = (*it);
            break;
        }
        it++;
    }
    // if we didn't find the player, must be an observer
    if (!found) {
        it = observers.begin();
        while (it != observers.end()) {
            if ((*it)->get_callsign() == callsign) {
                found = true;
                p = (*it);
                break;
            }
            it++;
        }
    }
    return p;
}

Glib::ustring playerView::get_IP(Glib::ustring callsign)
{
    std::vector < Player * >::iterator it;
    bool found = false;
    Glib::ustring ip;

    // search the players first
    it = players.begin();
    while (it != players.end()) {
        if ((*it)->get_callsign() == callsign) {
            found = true;
            ip = (*it)->get_IP();
            break;
        }
        it++;
    }
    // if we didn't find the player, must be an observer
    if (!found) {
        it = observers.begin();
        while (it != observers.end()) {
            if ((*it)->get_callsign() == callsign) {
                found = true;
                ip = (*it)->get_IP();
                break;
            }
            it++;
        }
    }
    return ip;
}

// update the playerView with new players/data
void playerView::update()
{
    Glib::ustring tanks;
    Glib::ustring obs;
    std::vector < Player * >::iterator it;

    // clear the text_view
    clear();

    // sort the players in descending order by score
    sort(players.begin(), players.end(), score_cmp);
    for (it = players.begin(); it != players.end(); it++) {
        tanks = format_player(*it);
        add_text(tanks);
    }
    // separate the observers from the players
    add_text("\n\n", NULL);

    for (it = observers.begin(); it != observers.end(); it++) {
        obs = format_observer(*it);
        add_text(obs);
    }
}

// format a player in the view
Glib::ustring playerView::format_player(Player * player)
{
    gint rabbit_score = 0;
    if (rabbitMode) {
        rabbit_score = player->get_rabbit_score();
    }

    gint16 score = player->get_score();
    gfloat si = player->get_strength_index();

    Glib::ustring attrib("(");

    if (player->get_registered()) {
        attrib += "Reg";
    }
    if (player->get_verified()) {
        attrib += "/Ver";
    }
    if (player->get_admin()) {
        if (attrib.size() == 1) {
            attrib += "Adm";
        } else {
            attrib += "/Adm";
        }
    }
    if (attrib.size() == 1) {
        attrib += "Anon)";
    } else {
        attrib += ")";
    }

    Glib::ustring playerflag = player->get_flag();

    // set the player and team flag colors
    Glib::ustring tag;
    tag = get_color(player->get_team());

    Glib::ustring flagtag;

    if (player->has_flag()) {
        if (playerflag[1] == '*') {	// it's a team flag
            flagtag = get_color(get_team_from_team_flag(playerflag.c_str()));
        } else if (playerflag == "SW" || playerflag == "G" || playerflag == "L" || playerflag == "GM" ||
                   playerflag == "ST" || playerflag == "CL" || playerflag == "WG" || playerflag == "SB") {
            flagtag = Color(YellowFg);
        } else {
            flagtag = Color(CyanFg);
        }
    }
    // player's accoutrements
    Glib::ustring flag("/");
    if (player->has_flag()) {	// player has a flag
        if (playerflag[1] == '*') {	// team flag
            flag += get_team_flag_desc(playerflag);
        } else {
            flag += playerflag;
        }
    } else {
        flag.clear();
    }
    Glib::ustring type;
    if (player->get_autopilot()) {
        type = " [auto]";
    } else if (player->get_type() == ComputerPlayer) {
        type = " [robot]";
    } else {
        type.clear();
    }
    Glib::ustring paused;
    if (player->get_paused()) {
        paused = "[paused]";
    } else {
        paused.clear();
    }
    Glib::ustring dead;
    if (player->get_alive()) {
        dead.clear();
    } else {					// player is dead
        dead = "[dead]";
    }
    // limit callsign to MAX_CALLSIGN_LEN
    Glib::ustring callsign;
    callsign.assign(player->get_callsign(), 0, MAX_CALLSIGN_LEN);

    int wins = player->get_wins();
    int losses = player->get_losses();
    int tks = player->get_tks();
    int id = player->get_id();

    // FIXME: using C style formating
    // score (wins - losses) [tks] strength index, player id), callsign
    char tmp_buf[256];
    Glib::ustring fmt_str("");
    if (rabbitMode) {
        ::snprintf(tmp_buf, sizeof(tmp_buf), "%2d%% %5d (%4d - %4d) [%2d] % 10.3f %3d) %s",
                   rabbit_score, score, wins, losses, tks, si, id, callsign.c_str());
    } else {
        ::snprintf(tmp_buf, sizeof(tmp_buf), "    %5d (%4d - %4d) [%2d] % 10.3f %3d) %s",
                   score, wins, losses, tks, si, id, callsign.c_str());
    }

    Glib::ustring text;
    text = tag + Glib::ustring(tmp_buf);
    // render the flag string in its own color if its a team flag
    text += flagtag + flag;
    // left justify and pad segment to MAX_COLUMN_LEN chars wide
    int len = MAX_COLUMN_LEN - (flag.size() + type.size() + paused.size() + callsign.size() + dead.size());
    ::snprintf(tmp_buf, sizeof(tmp_buf), "%s%s%s%*s", type.c_str(), dead.c_str(), paused.c_str(), len, "");
    text += Color(OrangeFg) + Glib::ustring(tmp_buf);
    // player attributes (reg, ver, adm) and IP address
    ::snprintf(tmp_buf, sizeof(tmp_buf), "%-15s%-15s\n", attrib.c_str(), player->get_IP().c_str());
    text += tag + Glib::ustring(tmp_buf);

    return text;
}

// format an observer in the view
Glib::ustring playerView::format_observer(Player * player)
{
    Glib::ustring attrib("(");
    if (player->get_registered()) {
        attrib += "Reg";
    }
    if (player->get_verified()) {
        attrib += "/Ver";
    }
    if (player->get_admin()) {
        attrib += "/Adm";
    }
    if (attrib.size() == 1) {
        attrib += "Anon)";
    } else {
        attrib += ")";
    }

    Glib::ustring paused;
    if (player->get_paused()) {
        paused = "[paused]";
    } else {
        paused.clear();
    }
    // limit callsign to MAX_CALLSIGN_LEN
    Glib::ustring callsign;
    callsign.assign(player->get_callsign(), 0, MAX_CALLSIGN_LEN);

    // FIXME: using C style formatting
    // whitespace padding + player id) callsign
    gint tmpl_len = Glib::ustring("XXX XXXXX (XXXX - XXXX) [XX] -XXXXX.XXX").length();
    char tmp_buf[256];
    ::snprintf(tmp_buf, sizeof(tmp_buf), "%*s %3d) %s", tmpl_len, "", player->get_id(), callsign.c_str());
    Glib::ustring text;
    text = Color(CyanFg) + Glib::ustring(tmp_buf);
    // left justify and pad segment to MAX_COLUMN_LEN chars wide
    int len = MAX_COLUMN_LEN - (paused.size() + callsign.size());
    ::snprintf(tmp_buf, sizeof(tmp_buf), "%s%s%*s", "", paused.c_str(), len, "");
    text += Color(OrangeFg) + Glib::ustring(tmp_buf);
    // player attributes (reg, adm, etc.) and IP address
    ::snprintf(tmp_buf, sizeof(tmp_buf), "%-15s%-15s\n", attrib.c_str(), player->get_IP().c_str());
    text += Color(CyanFg) + Glib::ustring(tmp_buf);

    return text;
}

gint playerView::get_team_from_team_flag(Glib::ustring abbrv)
{
    gint team;

    if ((abbrv[0] == 'R' || abbrv[0] == 'r') && abbrv[1] == '*') {
        team = RedTeam;
    } else if ((abbrv[0] == 'G' || abbrv[0] == 'g') && abbrv[1] == '*') {
        team = GreenTeam;
    } else if ((abbrv[0] == 'B' || abbrv[0] == 'b') && abbrv[1] == '*') {
        team = BlueTeam;
    } else if ((abbrv[0] == 'P' || abbrv[0] == 'p') && abbrv[1] == '*') {
        team = PurpleTeam;
    } else {
        team = NoTeam;
    }

    return team;
}

Glib::ustring playerView::get_team_flag_desc(Glib::ustring abbrv)
{
    Glib::ustring desc;

    switch (abbrv[0]) {
        case 'R':
        case 'r':
            desc = "Red Team";
            break;
        case 'G':
        case 'g':
            desc = "Green Team";
            break;
        case 'B':
        case 'b':
            desc = "Blue Team";
            break;
        case 'P':
        case 'p':
            desc = "Purple Team";
            break;
        default:
            desc = abbrv;
            break;
    }
    return desc;
}

void playerView::change_all_to_hunter_except(guint8 id)
{
    std::vector < Player * >::iterator it;

    it = players.begin();
    while (it != players.end()) {
        if ((*it)->get_id() != id) {
            (*it)->set_team(HunterTeam);
        }
        it++;
    }
}

Player *playerView::get_observer(gint n)
{
    return observers[n];
}

gint playerView::get_n_observers()
{
    return observers.size();
}

Player *playerView::get_player(gint n)
{
    return players[n];
}

gint playerView::get_n_players()
{
    return players.size();
}

void playerView::clear_players()
{
    players.clear();
    observers.clear();
}
