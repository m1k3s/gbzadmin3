#ifndef _gIOSocket_h_
#define _gIOSocket_h_
// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2014 Michael Sheppard
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

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <cstring>
#include <errno.h>

#include <ctype.h>
#include <time.h>
#include <vector>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>

#include "utilities.h"
#include "common.h"
#include "config.h"
#include "parser.h"
#include "gListServer.h"
#include "socketStates.h"

class gIOSocket : public sigc::trackable
{
    public:

        gIOSocket();
        ~gIOSocket();
		
        bool connect(Glib::ustring host, int p = 5154);
        bool join(Glib::ustring callsign, Glib::ustring password, Glib::ustring motto);
        void disconnect();
        int send(guint16 code, guint16 len, const void* msg);
        int read(guint16& code, guint16& len, void* msg, int millisecondsToBlock = 0);
        void sendLagPing(char pingRequest[2]);
        void sendExit();
        Glib::ustring reverseResolve(Glib::ustring ip, Glib::ustring callsign);
        void preFetchToken(Glib::ustring callsign, Glib::ustring password);
        void set_prefetch(bool set) { prefetch_token = set; }
        bool get_prefetch() { return prefetch_token; }

        State getState() const;
        const unsigned char& getId() const;
        const char* getVersion() const;
        const Glib::ustring& get_server_IP() const;
        const Glib::ustring& get_server_name() const;
        const Glib::ustring& getRejectionMessage() const;
        int get_port() const;
        
        time_t get_startTime() {
            return startTime;
        }
        guint32 get_bytesSent() {
            return bytesSent;
        }
        guint32 get_bitsSent() {
            return bytesSent * 8;
        }
        guint32 get_bytesReceived() {
            return bytesReceived;
        }
        guint32 get_bitsReceived() {
            return bytesReceived * 8;
        }
        guint32 get_packetsSent() {
            return packetsSent;
        }
        guint32 get_packetsRecevied() {
            return packetsReceived;
        }
        guint32 get_bitsTotal() {
            return (bytesReceived + bytesSent) * 8;
        }
        bool netStatsEnabled() {
            return netStats;
        }
        void resetNetStats();
        void setNetStats(bool set) {
            netStats = set;
            resetNetStats();
        }
        float bitFlow();

        bool resolve_host(const Glib::ustring& host, const Glib::ustring& port);

        sigc::signal<bool, Glib::IOCondition> on_tcp_data_pending;

    protected:
        void sendEnter(unsigned char type, unsigned int team, Glib::ustring callsign, Glib::ustring motto, Glib::ustring token);
        bool readEnter(Glib::ustring& reason, uint16_t& code, uint16_t& rejcode);
		void setTcpNoDelay();
        int check_status(int status_code);
        
        Glib::ustring get_ip_str(const Glib::RefPtr<Gio::SocketAddress>& address);

    private:
    	State state;
        Parser parser;
        gListServer listServer;
        
        bool netStats;
        float prev_flow;
        unsigned char id;
        Glib::ustring version;
		Glib::RefPtr<Gio::Socket> socket;
		Glib::RefPtr<Gio::SocketAddress> address;
		GError *error = NULL;
		Glib::RefPtr<Gio::SocketAddressEnumerator> enumerator;
		Glib::RefPtr<Gio::SocketConnectable> connectable;
        Glib::ustring token;
        bool prefetch_token;

        Glib::ustring rejectionMessage;
        Glib::ustring serverName;
        Glib::ustring serverIP;
        Glib::ustring errorMsg;
        int port;

        sigc::connection tcp_read;
        bool tcp_data_pending(Glib::IOCondition);

        // net stats
        time_t	startTime;
        guint32	bytesSent;
        guint32	bytesReceived;
        guint32	packetsSent;
        guint32	packetsReceived;
};

//class gIOSocketException : public std::exception
//{
//    const char *error;
//    public:
//        gIOSocketException(const char *e) : error(e) { }
//        const char * what() const throw() {
//            return error;
//        }
//};

inline const Glib::ustring& gIOSocket::getRejectionMessage() const
{
    return rejectionMessage;
}

inline State gIOSocket::getState() const
{
    return state;
}

inline const unsigned char& gIOSocket::getId() const
{
    return id;
}

inline const char* gIOSocket::getVersion() const
{
    return version.c_str();
}

inline const Glib::ustring& gIOSocket::get_server_IP() const
{
    return serverIP;
}

inline const Glib::ustring& gIOSocket::get_server_name() const
{
    return serverName;
}

inline int gIOSocket::get_port() const
{
    return port;
}

#endif // _gIOSocket_h_

