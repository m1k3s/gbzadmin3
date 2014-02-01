#ifndef _gSocket_h_
#define _gSocket_h_
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

//#define USING_GIO_NETWORK


class gSocket : public sigc::trackable
{
    public:
        enum State {
            Okay = 0,
            SocketError,
            Rejected,
            BadVersion,
            CrippledVersion,
            Refused,
            ResolveFailure
        };

        gSocket();
        ~gSocket();
		
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
#ifndef USING_GIO_NETWORK
        bool get_blocking() { return blocking; }
        void set_blocking(bool set) { blocking = set; }
#endif
        float bitFlow();

        bool resolve_host(const Glib::ustring& host, const Glib::ustring& port);

        sigc::signal<bool, Glib::IOCondition> on_tcp_data_pending;

    protected:
        void sendEnter(unsigned char type, unsigned int team, Glib::ustring callsign, Glib::ustring motto, Glib::ustring token);
        bool readEnter(Glib::ustring& reason, uint16_t& code, uint16_t& rejcode);
#ifndef USING_GIO_NETWORK
        bool connect_blocking(int _sockfd, const struct sockaddr *addr, socklen_t addrlen);
        bool connect_nonblocking(int _sockfd, const struct sockaddr *addr, socklen_t addrlen, int timeout);

        int setNonBlocking(int fd);
        int setBlocking(int fd);
        bool select(int _fd);
        int select(int _fd, int blockTime);
        int select_write(int _fd, int blockTime);
        Glib::ustring get_ip_str(const struct addrinfo *ai);
        void* get_in_addr(struct sockaddr *sa);
        bool check_server_version(int _sockfd);
        bool get_my_id(int _sockfd, unsigned char& _id);
        bool create_connection(int& _sockfd, bool blocking);
        int send_connection_header(int _sockfd);
#endif
		void setTcpNoDelay(int fd);
        int check_status(int status_code);
        
#ifdef USING_GIO_NETWORK
        Glib::ustring get_ip_str(const Glib::RefPtr<Gio::SocketAddress>& address);
#endif

    private:
    	State state;
#ifndef USING_GIO_NETWORK
        int sockfd;
        struct addrinfo server_info;
        bool blocking;
#endif
        Parser parser;
        gListServer listServer;
        
        bool netStats;
        float prev_flow;
        unsigned char id;
        Glib::ustring version;
#ifdef USING_GIO_NETWORK
		Glib::RefPtr<Gio::Socket> socket;
		Glib::RefPtr<Gio::SocketAddress> address;
		GError *error = NULL;
		Glib::RefPtr<Gio::SocketAddressEnumerator> enumerator;
		Glib::RefPtr<Gio::SocketConnectable> connectable;
#endif
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

//class gSocketException : public std::exception
//{
//    const char *error;
//    public:
//        gSocketException(const char *e) : error(e) { }
//        const char * what() const throw() {
//            return error;
//        }
//};

inline const Glib::ustring& gSocket::getRejectionMessage() const
{
    return rejectionMessage;
}

//#ifndef USING_GIO_NETWORK
inline gSocket::State gSocket::getState() const
{
    return state;
}
//#endif

inline const unsigned char& gSocket::getId() const
{
    return id;
}

inline const char* gSocket::getVersion() const
{
    return version.c_str();
}

inline const Glib::ustring& gSocket::get_server_IP() const
{
    return serverIP;
}

inline const Glib::ustring& gSocket::get_server_name() const
{
    return serverName;
}

inline int gSocket::get_port() const
{
    return port;
}

#endif // _gSocket_h_

