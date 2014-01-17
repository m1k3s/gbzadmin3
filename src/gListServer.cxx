// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2014 Michael Sheppard
//
// Code based on BZFlag-2.0.x and SVN 2.4.x
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
#include "gListServer.h"

// system headers
#include <string.h>

static unsigned char *curlData = 0;
static unsigned int curlDataLen;

gListServer::gListServer()
{

}

void gListServer::queryListServer(Glib::ustring callsign, Glib::ustring password, Glib::ustring& token)
{
    char _token[TokenLen];
    ::memset(_token, 0, TokenLen);
    getToken(callsign, password);
    parseToken(_token);
    token = Glib::ustring(_token);

    delete curlData;
}

void gListServer::getServerList(std::vector<serverInfo*>& si)
{
    getToken(Glib::ustring(""), Glib::ustring(""));
    parseServerList(si);

    delete curlData;
    curlData = 0;
}

void gListServer::getToken(Glib::ustring callsign, Glib::ustring password)
{
    CURLcode result;
    CURL *handle;
    Glib::ustring msg;
    int timeout = 15;

    // build the POST statement
    msg = "action=LIST&version=";
    msg += ServerVersion;
    msg += "&clientversion";
    msg += url_encode(getAppVersion());
    msg += "&callsign=";
    msg += url_encode(callsign.c_str());
    msg += "&password=";
    msg += url_encode(password.c_str());

    curlData = (unsigned char*)g_try_new(gchar, 1024);

    if ((result = curl_global_init(CURL_GLOBAL_ALL))) {
        std::cerr << "cURL Global init Error\n";
        return;
    }

    handle = curl_easy_init();
    if (!handle) {
        std::cerr << "Something wrong with CURL\n";
        return;
    }

    result = curl_easy_setopt(handle, CURLOPT_URL, default_list_server_url);
    if (result != CURLE_OK) {
        std::cerr << "CURLOPT_URL failed\n";
    }

    result = curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, timeout);
    if (result != CURLE_OK) {
        std::cerr << "CURLOPT_CONNECTTIMEOUT error\n";
    }

    // anything under 1Kbyte/sec is junk
    result = curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, 1024);
    if (result != CURLE_OK) {
        std::cerr << "CURLOPT_LOW_SPEED_LIMIT error\n";
    }

    result = curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, timeout);
    if (result != CURLE_OK) {
        std::cerr << "CURLOPT_LOW_SPEED_TIME error\n";
    }

    result = curl_easy_setopt(handle, CURLOPT_TIMEOUT, timeout);
    if (result != CURLE_OK) {
        std::cerr << "CURLOPT_TIMEOUT failed\n";
    }

    result = curl_easy_setopt(handle, CURLOPT_POSTFIELDS, msg.c_str());
    if (result != CURLE_OK) {
        std::cerr << "CURLOPT_POSTFIELDS failed\n";
    }

    result = curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, msg.size());
    if (result != CURLE_OK) {
        std::cerr << "CURLOPT_POSTFIELDSIZE failed\n";
    }

    result = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_function);
    if (result != CURLE_OK) {
        std::cerr << "CURLOPT_WRITEFUNCTION failed\n";
    }

    result = curl_easy_setopt(handle, CURLOPT_POST, 1);
    if (result != CURLE_OK) {
        std::cerr << "CURLOPT_POST failed\n";
    }

    result = curl_easy_setopt(handle, CURLOPT_NOPROGRESS, TRUE);
    if (result != CURLE_OK) {
        std::cerr << "CURLOPT_NOPROGRESS failed\n";
    }

    result = curl_easy_perform(handle);
    if (result != CURLE_OK) {
        std::cerr << "curl_easy_perform(handle) failed\n";
    }

    curl_easy_cleanup(handle);
}

/////////////////////////////////////////////
// ** static functions for cURL
////////////////////////////////////////////

size_t write_function(void *ptr, size_t size, size_t nmemb, void *userp)
{
    int len = size * nmemb;
    collect_data((char*)ptr, len);

    return len;
}

void collect_data(char* ptr, int len)
{
    unsigned char *new_data = (unsigned char*)g_try_realloc(curlData, curlDataLen + len);
    if (!new_data) {
        std::cerr << "*** ERROR: memory allocation failure!\n";
    } else {
        ::memcpy(new_data + curlDataLen, ptr, len);
        curlDataLen += len;
        curlData = new_data;
    }
}

//////////////////////////////////////////
// ** end of static cURL functions
//////////////////////////////////////////

void gListServer::parseToken(char* _token)
{
    char *base = (char*)curlData;
    char *endS = base + curlDataLen;
    const char *tokenId   = "TOKEN: ";
    const char *noTokenId = "NOTOK: ";
    const char *errorId   = "ERROR: ";
    const char *noticeId  = "NOTICE: ";

    // walks entire reply including HTTP headers
    while (base < endS) {
        // find next newline
        char* scan = base;
        while (scan < endS && *scan != '\n') {
            scan++;
        }

        // if no newline then no more complete replies
        if (scan >= endS) {
            break;
        }
        *scan++ = '\0';

        // look for TOKEN: and save token if found. Also look for NOTOK:
        // and record "badtoken" into the token string.
        if (::strncmp(base, tokenId, ::strlen(tokenId)) == 0) {
            ::strncpy(_token, (char *)(base + ::strlen(tokenId)), TokenLen);
            base = scan;
            continue;
        }	else if (::strncmp(base, noTokenId, ::strlen(noTokenId)) == 0) {
            ::strcpy(_token, "badtoken");
            base = scan;
            continue;
        }	else if (::strncmp(base, errorId, ::strlen(errorId)) == 0) {
            ::strcpy(_token, "badtoken");
            base = scan;
            continue;
        } else if (::strncmp(base, noticeId, ::strlen(noticeId)) == 0) {
            std::cout << "received NOTICE: " << base << " from list server\n";
            base = scan;
            continue;
        }
        // next reply
        base = scan;
    }
    // remove parsed replies
    curlDataLen -= (int)(base - (char *)curlData);
    ::memmove(curlData, base, curlDataLen);
}

void gListServer::parseServerList(std::vector<serverInfo*>& si_array)
{
    char *base = (char*)curlData;
    char *endS = base + curlDataLen;
    const char *tokenId   = "TOKEN: ";
    const char *noTokenId = "NOTOK: ";
    const char *errorId   = "ERROR: ";
    const char *noticeId  = "NOTICE: ";
    char token[TokenLen];
    gchar *scan2, *name, *cversion, *infoServer, *address, *title;
    gint port;

    si_array.clear();

    // walks entire reply including HTTP headers
    while (base < endS) {
        // find next newline
        char* scan = base;
        while (scan < endS && *scan != '\n') {
            scan++;
        }

        // if no newline then no more complete replies
        if (scan >= endS) {
            break;
        }
        *scan++ = '\0';

        // we won't save the token or any other info here
        // we only want the server list
        if (::strncmp(base, tokenId, ::strlen(tokenId)) == 0) {
            ::strncpy(token, (char *)(base + ::strlen(tokenId)), TokenLen);
            base = scan;
            continue;
        }	else if (::strncmp(base, noTokenId, ::strlen(noTokenId)) == 0) {
            ::strcpy(token, "badtoken");
            base = scan;
            continue;
        }	else if (::strncmp(base, errorId, ::strlen(errorId)) == 0) {
            ::strcpy(token, "badtoken");
            base = scan;
            continue;
        } else if (::strncmp(base, noticeId, ::strlen(noticeId)) == 0) {
            std::cout << "received NOTICE: " << base << " from list server\n";
            base = scan;
            continue;
        }
        // now the servers
        name = base;
        cversion = name;
        while (*cversion && !g_ascii_isspace(*cversion)) {
            cversion++;
        }
        while (*cversion &&  g_ascii_isspace(*cversion)) {
            *cversion++ = 0;
        }

        infoServer = cversion;
        while (*infoServer && !g_ascii_isspace(*infoServer)) {
            infoServer++;
        }
        while (*infoServer &&  g_ascii_isspace(*infoServer)) {
            *infoServer++ = 0;
        }

        address = infoServer;
        while (*address && !g_ascii_isspace(*address)) {
            address++;
        }
        while (*address &&  g_ascii_isspace(*address)) {
            *address++ = 0;
        }

        title = address;
        while (*title && !g_ascii_isspace(*title)) {
            title++;
        }
        while (*title &&  g_ascii_isspace(*title)) {
            *title++ = 0;
        }

        // extract port number from address
        port = DefaultPort;
        scan2 = strchr(name, ':');
        if (scan2) {
            port = atoi(scan2 + 1);
            *scan2 = 0;
        }
        // check info
        if ((g_ascii_strcasecmp(cversion, ServerVersion) == 0) &&
                strlen(infoServer) == PingPacketHexPackedSize &&
                (port >= 1) && (port <= 65535)) {
            serverInfo *si = g_try_new0(serverInfo, 1);
            si->port = DefaultPort;

            parser.unpack_hex(infoServer, &si->sih);

            gint dot[4] = {127, 0, 0, 1};
            if (sscanf(address, "%d.%d.%d.%d", dot + 0, dot + 1, dot + 2, dot + 3) == 4) {
                if (dot[0] >= 0 && dot[0] <= 255 && dot[1] >= 0 && dot[1] <= 255 &&
                        dot[2] >= 0 && dot[2] <= 255 && dot[3] >= 0 && dot[3] <= 255) {
                    struct in_addr addr;
                    guchar* paddr = (guchar*)&addr.s_addr;
                    paddr[0] = (guchar)dot[0];
                    paddr[1] = (guchar)dot[1];
                    paddr[2] = (guchar)dot[2];
                    paddr[3] = (guchar)dot[3];
                    memcpy(&si->addr, &addr, sizeof(struct in_addr));
                }
            }
            // construct and store the server description
            gchar *name_str = g_strndup(name, ServerNameLen);
            g_stpcpy(si->name, name_str);
            g_free(name_str);
            if (port != DefaultPort) {
                si->port = port;
            }
            if (strlen(title) > 0) {
                gchar *title_str = g_strndup(title, DescLen);
                g_stpcpy(si->desc, title_str);
                g_free(title_str);
            }
            // si->sih.totalPlayers is used as a sort key
            si->sih.totalPlayers = si->sih.rogueCount + si->sih.redCount + si->sih.greenCount +
                                   si->sih.blueCount + si->sih.purpleCount + si->sih.observerCount;

            si_array.push_back(si);
        }
        // next reply
        base = scan;
    }
    // remove parsed replies
    curlDataLen -= (int)(base - (char *)curlData);
    ::memmove(curlData, base, curlDataLen);
}

