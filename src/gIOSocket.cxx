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
#include "gIOSocket.h"

const char* const BZ_Connect_Header = "BZFLAG\r\n\r\n";
const char* const BanRefusalString = "REFUSED:";

gIOSocket::gIOSocket()
{
	state = SocketError;
	netStats = true;
    prev_flow = 0.0;
    prefetch_token = false;
    token = "";
    bytesSent = 0;
    bytesReceived = 0;
//    packetsSent = 0;
//    packetsReceived = 0;
}

gIOSocket::~gIOSocket()
{
    disconnect();
}

void gIOSocket::preFetchToken(Glib::ustring callsign, Glib::ustring password)
{
	token = ""; // clear the token
	listServer.queryListServer(callsign, password, token);
}

void gIOSocket::disconnect()
{
    if (state != Okay) {
        return;
    }

    tcp_read.disconnect();
	try {
		socket->shutdown(true, true);
		socket->close();
	} catch (const Gio::Error& error) {
	    std::cerr << Glib::ustring::compose ("socket->shutdown/close: %1\n", error.what ());
    }
    state = SocketError;
}

bool gIOSocket::connect(Glib::ustring host, int p)
{
	if (state == Okay) { // already connected
        return false;
    }
	serverName = host;
    port = p;
    
	try {
        connectable = Gio::NetworkAddress::parse(host, p);
    } catch (const Gio::Error& error) {
        std::cerr << Glib::ustring::compose ("Gio::NetworkAddress::parse: %1\n", error.what ());
        return false;
    }
    
    try {
        socket = Gio::Socket::create (Gio::SOCKET_FAMILY_IPV4, Gio::SOCKET_TYPE_STREAM, Gio::SOCKET_PROTOCOL_DEFAULT);
    } catch (const Gio::Error& error) {
        std::cerr << Glib::ustring::compose ("Gio::Socket::create: %1\n", error.what ());
        return false;
    }
    
    enumerator = connectable->enumerate ();
    while (true) {
        try {
            address = enumerator->next();
            if (!address) {
                return false;
            }
        } catch (const Gio::Error& error) {
            std::cerr << Glib::ustring::compose ("enumerator->next: %1\n", error.what ());
            return false;
        }
		if (address->get_family() == Gio::SOCKET_FAMILY_IPV4) {
		    try {
		        socket->connect (address);
		        serverIP = get_ip_str(address);
		        break;
		    } catch (const Gio::Error& error) {
			    std::cerr << Glib::ustring::compose ("socket->connect: %1\n", error.what ());
		        return false;
		    }
		}
    }
    
    try {
    	socket->set_blocking(true);
    } catch (const Gio::Error& error) {
	    std::cerr << Glib::ustring::compose ("socket->set_blocking(true): %1\n", error.what ());
        return false;
    }
    
    socket->send(BZ_Connect_Header, (int)strlen(BZ_Connect_Header));
    
    // check server version
    try {
        socket->condition_wait (Glib::IO_IN);
    } catch (const Gio::Error& error) {
        std::cerr << Glib::ustring::compose("socket->condition_wait (Glib::IO_IN): %1\n", error.what ());
        return false;
    }
    
    version = "BZFS0000";
    try {
    	char vstr[8]; // buffer for server version (8 bytes)
    	socket->receive_with_blocking((char*)&vstr[0], 8, true);
    	Glib::ustring str(vstr);
    	version = str.substr(0, 8); // server version
    	std::cout << "received: " << str << std::endl;
    } catch (const Gio::Error& error) {
	    std::cerr << Glib::ustring::compose("socket->receive(version): %1\n", error.what());
        return false;
    }
    
    if (ServerVersion != version) {
        state = BadVersion;
		if (BanRefusalString == version) {
            state = Refused;
            char message[512];
            int len = socket->receive((char*)message, 512);
            if (len > 0) {
                message[len - 1] = 0;
            } else {
                message[0] = 0;
            }
            rejectionMessage = message;
        }
        return false;
    }
    
	// get my player ID
    try {
		char* idstr = new char[1];
		socket->receive_with_blocking((char*)&idstr[0], sizeof(idstr), false);
		id = (unsigned char)atoi(idstr);     // my ID
		if (id == 0xff) {
			state = Rejected;
			return false;
		}
	} catch (const Gio::Error& error) {
		std::cerr << Glib::ustring::compose("socket->receive(id): %1\n", error.what ());
        return false;
	}
    
    try {
    	socket->set_blocking(true);
    } catch (const Gio::Error& error) {
	    std::cerr << Glib::ustring::compose ("socket->set_blocking(true): %1\n", error.what ());
        return false;
    }
    
    // set tcp no delay
    setTcpNoDelay();

    state = Okay;
	return true;
}

bool gIOSocket::join(Glib::ustring callsign, Glib::ustring password, Glib::ustring motto)
{
    uint16_t code, rejCode;
    Glib::ustring reason;

    // query the list server to get a token
    if (!prefetch_token) {
    	token = "";
    	listServer.queryListServer(callsign, password, token);
    }
    // request to enter the game -- sending callsign and token
    sendEnter(TankPlayer, ObserverTeam, callsign, motto, token);

    // now see if we were accepted
    if (readEnter(reason, code, rejCode)) {
        // we're in! connect the TCP signal
        Glib::IOCondition flags = Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL;
        Glib::RefPtr<Glib::IOChannel> channel = Glib::IOChannel::create_from_fd(socket->get_fd());
        tcp_read = Glib::signal_io().connect(sigc::mem_fun(*this, &gIOSocket::tcp_data_pending), channel, flags);

        return true;
    }
    // we have failed to join
    rejectionMessage = reason;
    return false;
}

bool gIOSocket::tcp_data_pending(Glib::IOCondition cond)
{
    on_tcp_data_pending(cond);
    return true;
}

int gIOSocket::send(guint16 code, guint16 len, const void* msg)
{
    if (state != Okay) {
        return -1;
    }
    char msgbuf[MaxPacketLen];
    void* buf = msgbuf;
    ssize_t sent = 0;

    buf = parser.nboPackUShort(buf, len);
    buf = parser.nboPackUShort(buf, code);

    if (msg && len != 0) {
        buf = parser.nboPackString(buf, msg, len);
    }
	try {
        socket->condition_wait(Glib::IO_OUT);
    } catch (const Gio::Error& error) {
        std::cerr << Glib::ustring::compose("%1\n", error.what ());
        return -1;
    }
    try {
		sent = socket->send(msgbuf, len + 4);
	} catch (const Gio::Error& error) {
        std::cerr << Glib::ustring::compose("%1\n", error.what ());
        return -1;
    }
    
    if (netStats) {
        bytesSent += sent;
//        packetsSent++;
    }
    return sent;
}

// no need to select in this function, the IO signal was received so we 
// should have data available.
int gIOSocket::read(uint16_t& code, uint16_t& len, void* msg, int blockTime)
{
    code = MsgNull;
    len = 0;

    if (state != Okay) {
        return -1;
    }
    // get packet header
    char headerBuffer[4];
    int rlen = 0;
	try {
		rlen = socket->receive(headerBuffer, 4);
    } catch (const Gio::Error& error) {
        std::cerr << Glib::ustring::compose ("Error receiving from socket: %1\n", error.what ());
        return -1;
    }
    if (netStats) {
        bytesReceived += 4;
//        packetsReceived++;
    }
    // unpack header and get message
    void* buf = headerBuffer;
    buf = parser.nboUnpackUShort(buf, &len);
    buf = parser.nboUnpackUShort(buf, &code);
    
    if (len > 0) {
		try {
			rlen = socket->receive((char*)msg, (int)len);
		} catch (const Gio::Error& error) {
			std::cerr << Glib::ustring::compose ("Error receiving from socket: %1\n", error.what ());
			return -1;
		}
    } else {
        rlen = 0;
    }
    if (rlen == int(len)) {	// got the whole message, done
    	if (netStats) {
		    bytesReceived += rlen;
		}
    } 
    return 1;
}

void gIOSocket::sendEnter(unsigned char type, unsigned int team, Glib::ustring callsign, Glib::ustring motto, Glib::ustring token)
{
    if (state != Okay) {
        return;
    }
    char msg[PlayerIdPLen + 4 + CallSignLen + MottoLen + TokenLen + VersionLen] = {0};
    void* buf = msg;
    const char* versionStr = getAppVersion().c_str();

    buf = parser.nboPackUShort(buf, uint16_t(type));
    buf = parser.nboPackUShort(buf, uint16_t(team));
    ::memcpy(buf, callsign.c_str(), callsign.size());
    buf = (void*)((char*)buf + CallSignLen);
    ::memcpy(buf, motto.c_str(), motto.size());
    buf = (void*)((char*)buf + MottoLen);
    ::memcpy(buf, token.c_str(), token.size());
    buf = (void*)((char*)buf + TokenLen);
    ::memcpy(buf, versionStr, ::strlen(versionStr) + 1);
    buf = (void*)((char*)buf + VersionLen);

    this->send(MsgEnter, sizeof(msg), msg);
}

bool gIOSocket::readEnter(Glib::ustring& reason, uint16_t& code, uint16_t& rejcode)
{
    guint16 len;
    char msg[MaxPacketLen];

    while (true) {
        if (this->read(code, len, msg, -1) < 0) {
            reason = "Communication error joining game [No immediate respose].\n";
            return false;
        }

        if (code == MsgAccept) {
            return true;
        } else if (code == MsgSuperKill) {
            reason = "Server forced disconnection.\n";
            return false;
        } else if (code == MsgReject) {
            void *buf;
            char buffer[MessageLen];
            buf = parser.nboUnpackUShort (msg, &rejcode); // filler for now
            buf = parser.nboUnpackString (buf, buffer, MessageLen);
            buffer[MessageLen - 1] = '\0';
            reason = buffer;
            return false;
        }
    }

    return true;
}

void gIOSocket::setTcpNoDelay()
{
    gint off = 0;
    struct protoent *p = ::getprotobyname("tcp");

    if (p) {
    	try {
    	socket->set_option(p->p_proto, TCP_NODELAY, off);
		} catch (const Gio::Error& error) {
			std::cerr << Glib::ustring::compose ("socket->set_option(TCP_NODELAY): %1\n", error.what ());
		}
	}
}

void gIOSocket::resetNetStats()
{
    startTime = time(0);
    bytesSent = 0;
    bytesReceived = 0;
//    packetsSent = 0;
//    packetsReceived = 0;
}

float gIOSocket::totalBitsPerSecondRate()
{
    time_t now = time(0);
    double dt = difftime(now, startTime);

	// one minute moving (exponential decaying) average
    float bitsTotal = (float)(bytesReceived + bytesSent) * 8.0;
    float cur_flow = ((prev_flow - bitsTotal) * exp(-dt / 60.0)) + bitsTotal;
    prev_flow = cur_flow;

    resetNetStats();

    return cur_flow;
}

void gIOSocket::sendLagPing(char pingRequest[2])
{
    char msg[3];
    void* buf = msg;

    buf = parser.nboPackUByte(buf, guint8(getId()));
    buf = parser.nboPackString(buf, pingRequest, 2);

    this->send(MsgLagPing, sizeof(msg), msg);
}

void gIOSocket::sendExit()
{
    char msg[1];

    msg[0] = getId();

    this->send(MsgExit, sizeof(msg), msg);
}

Glib::ustring gIOSocket::reverseResolve(Glib::ustring ip, Glib::ustring callsign)
{
    struct sockaddr_in addr;
    ::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
	inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr));
    char host[256];
    Glib::ustring result("");

    int status = getnameinfo((struct sockaddr*)&addr, sizeof(addr), host, 256, (char*)NULL, 0, 0);
    if (check_status(status)) {
	    result = errorMsg;
    } else {
    	result = callsign + "'s IP resolved to " + host + "\n";
    }
    return result;
}

int gIOSocket::check_status(int status_code)
{
	switch (status_code) {
        default: // no error (status_code == 0)
            break;

        case EAI_ADDRFAMILY:
            errorMsg = "The specified network host does not have any network addresses in the requested address family.\n";
            break;

        case EAI_AGAIN:
            errorMsg = "The name could not be resolved at this time.\n";
            break;

        case EAI_BADFLAGS:
            errorMsg = "The flags had an invalid value.\n";
            break;

        case EAI_FAIL:
            errorMsg = "A non-recoverable error occurred.\n";
            break;

        case EAI_FAMILY:
            errorMsg = "The address family was not recognized or the address length was invalid.\n";
            break;

        case EAI_MEMORY:
            errorMsg = "There was a memory allocation failure.\n";
            break;

        case EAI_NONAME:
            errorMsg = "The name does not resolve for the supplied parameters.\n";
            break;

        case EAI_NODATA:
            errorMsg = "The specified network host exists, but does not have any network addresses defined.\n";
            break;

        case EAI_OVERFLOW:
            errorMsg = "An argument buffer overflowed.\n";
            break;

        case EAI_SYSTEM:
            errorMsg = "A system error occurred. The error code can be found in errno. \n";
            break;

        case EAI_SERVICE:
            errorMsg = "The requested service is not available for the requested socket type.\n";
            break;

        case EAI_SOCKTYPE:
            errorMsg = "The requested socket type is not supported at all.\n";
            break;
    }
    return status_code;
}

Glib::ustring gIOSocket::get_ip_str(const Glib::RefPtr<Gio::SocketAddress>& address)
{
	Glib::RefPtr<Gio::InetAddress> inet_address;
    Glib::ustring str, res;
    int port;

    Glib::RefPtr<Gio::InetSocketAddress> isockaddr = Glib::RefPtr<Gio::InetSocketAddress>::cast_dynamic(address);
    if (!isockaddr) {
        return Glib::ustring();
    }
    inet_address = isockaddr->get_address();
    str = inet_address->to_string();
    port = isockaddr->get_port();
    return Glib::ustring::compose("%1:%2", str, port);
//    return res;
}
