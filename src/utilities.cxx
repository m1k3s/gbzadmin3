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
//
// Utility functions
#include "utilities.h"


//char *getAppVersion()
//{
//    char buf[128];
//    Glib::ustring os_name, machine;
//    os_name = getOsInfo("sysname");
//    machine = getOsInfo("machine");

//	snprintf(buf, sizeof(buf), "gbzadmin-%s.%d-%s-%s-%s", VERSION, getBuildDate(), Gbzadmin_Build_Type, os_name.c_str(), machine.c_str());

//    return ::strdup(buf);
//}

Glib::ustring getAppVersion()
{
	Glib::ustring str("");
	Glib::ustring osName(getOsInfo("sysname"));
	Glib::ustring machine(getOsInfo("machine"));
	
	str = Glib::ustring::compose("gbzadmin-%1.%2-%3-%4-%5", VERSION, getBuildDate(), Gbzadmin_Build_Type, osName, machine);
	std::cout << str << std::endl;
	return str;
}

int getBuildDate()
{
    Glib::Date d;
    d.set_time_current();

    Glib::Date::Year year = d.get_year();
    Glib::Date::Month month = d.get_month();
    Glib::Date::Day day = d.get_day();

    return (year * 10000) + (month * 100) + day;
}

Glib::ustring getOsInfo(Glib::ustring info)
{
	struct utsname os;
	Glib::ustring value("unknown");
	
	if (uname(&os)) {
		return value;
	} else if (info == "sysname") {
		value = Glib::ustring(os.sysname);
	} else if (info == "machine") {
		value = Glib::ustring(os.machine);
	}
	return value;
}

const char *getServerVersion()
{
    return ServerVersion;
}

//char *url_encode(const char *text)
//{
//    char hex[5];
//    int text_size = strlen(text);
//    char *destination = g_new0(char, text_size + 20);
//    g_assert(destination != NULL);

//    for (int j = 0, i = 0; i < text_size; i++) {
//        char c = text[i];
//        if (g_ascii_isalnum(c)) {
//            memset(&destination[j++], c, sizeof(char));
//        } else if (g_ascii_isspace(c)) {
//            memset(&destination[j++], '+', sizeof(char));
//        } else {
//            memset(&destination[j++], '%', sizeof(char));
//            snprintf(hex, sizeof(hex), "%-2.2X", c);
//            memcpy(&destination[j], hex, strlen(hex));
//            j += strlen(hex);
//        }
//    }
//    return destination;
//}

Glib::ustring UrlEncode(Glib::ustring text)
{
	Glib::ustring hex("");
	Glib::ustring destination("");
//	int len = text.size();
	
	Glib::ustring::iterator it = text.begin();
	while (it != text.end()) {
		if (Glib::Ascii::isalnum(*it)) {
			destination += *it;
			it++;
		} else if (Glib::Ascii::isspace(*it)) {
			destination += "+";
			it++;
		} else {
			destination += "%";
			destination += Glib::ustring::compose("%1", Glib::ustring::format(std::hex, std::setw(2), std::setprecision(2), std::left, *it));
			it++;
		}
	}
	return destination;
}

