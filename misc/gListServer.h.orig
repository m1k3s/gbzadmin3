#ifndef _gListServer_h_
#define _gListServer_h_
// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2012 Michael Sheppard
//  
// Code based on BZFlag-2.0.x and SVN 2.99
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
#include <string>

#include "utilities.h"
#include "common.h"
#include "config.h"
#include "parser.h"

// list server address
const char* const default_list_server_url = "http://my.BZFlag.org/db/";

// static functions for cURL
size_t write_function(void *ptr, size_t size, size_t nmemb, void *userp);
void collect_data(gchar* ptr, int len);

class gListServer 
{
public:
	gListServer();
	~gListServer() {}
	void getServerList(std::vector<serverInfo*>& si);
	void queryListServer(Glib::ustring callsign, Glib::ustring password, Glib::ustring& token);
	void getToken(Glib::ustring callsign, Glib::ustring password);
	void parseToken(char* token);
	void parseServerList(std::vector<serverInfo*>& si_array);
	std::vector<serverInfo*> getServerList() { return si; }

protected:
	
private:
	Parser parser;
	std::vector<serverInfo*> si;
};
#endif // _gListServer_h_

