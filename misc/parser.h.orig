#ifndef _parser_h_
#define _parser_h_
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
#include <fstream>
#include <sys/types.h>
#include <sys/utsname.h>
#include <cstdio>

#include "config.h"
#include "common.h"


#define	ADV(_b, _t)	((void*)(((char*)(_b)) + sizeof(_t)))

typedef union {
	guint32 a;
	float b;
} dtype;

struct flag_info {
	gint idx;
	guchar type[3];
	guint16 status;
	guint16 endurance;
	guint8 owner;
	float position[3];
	float launch_pos[3];
	float landing_pos[3];
	float flight_time;
	float flight_end;
	float initial_velocity;
};

struct serverInfoHex {
	guint16 gameType, gameOptions, maxShots, shakeWins, shakeTimeout;
	guint16 maxPlayerScore, maxTeamScore, maxTime, totalPlayers;
	guint8 maxPlayers, rogueCount, redCount, greenCount, blueCount;
	guint8 purpleCount, observerCount, rogueMax, redMax;
	guint8 greenMax, blueMax, purpleMax, observerMax;
};

struct serverInfo {
	gchar name[ServerNameLen];
	gint port;
	gchar desc[DescLen];
	serverInfoHex sih;
	struct in_addr addr;
};

struct serverStats {
	guint total_players;
	guint total_rogues;
	guint total_reds;
	guint total_greens;
	guint total_blues;
	guint total_purples;
	guint total_observers;
};

class Parser
{
public:
	Parser();
	~Parser() {}
	
	void* unpack_address(void* _buf, struct in_addr *addr);
	void* unpack_flag(void* buf, guchar *abbv);
	void* unpack_flag_info(void* buf, flag_info *fi);
	void *nboUnpackFloat(void *b, float *v);
	void *nboUnpackVector(void *b, float *v);
	void *nboUnpackString(void *b, void *m, int len);
	gint64 unpack_game_time(void *buf);
	void* nboUnpackUByte(void* b, guint8* v);
	void* nboUnpackShort(void* b, gint16* v);
	void* nboUnpackInt(void* b, gint32* v);
	void* nboUnpackUShort(void* b, guint16* v);
	void* nboUnpackUInt(void* b, guint32* v);
	void unpack_hex(gchar* buf, serverInfoHex *si);
	void *nboUnpackStdString(void* b, Glib::ustring& str);
	
	void* nboPackUByte(void* b, guint8 v);
	void* nboPackShort(void* b, gint16 v);
	void* nboPackInt(void* b, gint32 v);
	void* nboPackUShort(void* b, guint16 v);
	void* nboPackUInt(void* b, guint32 v);
	void *nboPackFloat(void *b, float v);
	void *nboPackVector(void *b, const float *v);
	void *nboPackString(void *b, const void *m, int len);
	void *nboPackStdString(void* b, const Glib::ustring& str);
	
	enum {
		FlagNoExist = 0, FlagOnGround, FlagOnTank, FlagInAir,
		FlagComing, FlagGoing
	};
	
	enum { BadFlag=0, GoodFlag, TeamFlag };
	
protected:
	gint hex2bin(gchar d);
	gchar bin2hex(gint d);
	gchar* unpack_hex8(gchar* buf, guint8 *v);
	gchar* unpack_hex16(gchar* buf, guint16 *v);
	void nboUseErrorChecking(bool checking);
	bool nboGetBufferError();
	void nboClearBufferError(void);
	void nboSetBufferLength(unsigned int length);
	unsigned int nboGetBufferLength(void);
	
private:
	bool Error;
	bool ErrorChecking;
	unsigned int Length;
	
	flag_info fi;
};

#endif // _parser_h_
