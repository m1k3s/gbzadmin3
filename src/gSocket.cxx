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
#include "gSocket.h"


gSocket::gSocket()
    :	state(SocketError), fd(-1), netStats(true)
{
    prev_flow = 0.0;
}

gSocket::~gSocket()
{
    disconnect();
}

void gSocket::disconnect()
{
    if (state != Okay) {
        return;
    }

    tcp_read.disconnect();

    ::shutdown(fd, 2);
    ::close(fd);
    fd = -1;

    state = SocketError;
}

bool gSocket::connect(Glib::ustring host, int p)
{
    if (state == Okay) { // already connected
        return false;
    }

    int recvd;
    serverName = host;
    port = p;
    const char* const BanRefusalString = "REFUSED:";

    // initialize version string
    ::strcpy(version, "BZFS0000");

    // resolve the host name address
    if (!resolveHost(host, Glib::ustring::compose("%1", port))) {
        state = ResolveFailure;
        rejectionMessage = "DNS: Failed to resolve host name\n";
        return false;
    }

	char addrStr[32];
	inet_ntop(server_info.ai_family, server_info.ai_addr, addrStr, 32);
	serverIP = addrStr;

    // open connection to server.
    int query = ::socket(server_info.ai_family, server_info.ai_socktype, 0);
    if (query < 0) {
        return false;
    }

    // this is where we actually connect. Hopefully.
	if (::connect(query, server_info.ai_addr, server_info.ai_addrlen) < 0) {
        if (errno != EINPROGRESS) {
            ::close(query);
            return false;
        }
        if (!select(query)) {
            ::close(query);
            return false;
        }
        // check for connection errors
        int connectError;
        socklen_t errorLen = sizeof(int);
        if (::getsockopt(query, SOL_SOCKET, SO_ERROR, &connectError, &errorLen)	< 0) {
            ::close(query);
            return false;
        }
        if (connectError != 0) {
            ::close(query);
            return false;
        }
    }
    // send the connection header
    ::send(query, BZ_CONNECT_HEADER, (int)strlen(BZ_CONNECT_HEADER), 0);

    bool gotNetData = false;

    int loopCount = 0;
    while (!gotNetData) {
        loopCount++;
        // get server version and verify
        if (!select(query)) {
            ::close(query);
            return false;
        }
        recvd = ::recv(query, (char*)version, 8, 0);
        if (recvd > 0) {
            gotNetData = true;
        } else if (loopCount > 20) {
            ::close(query);
            return false;
        }
        Glib::usleep(G_USEC_PER_SEC / 4); // 1/4 second
    }

    if (recvd < 8) {
        close(query);
        return false;
    }

    if (setNonBlocking(query) < 0) {
        close(query);
        return false;
    }

    if (::strcmp(version, ServerVersion) != 0) {
        std::cerr << "BAD VERSION: " << version << std::endl;
        state = BadVersion;
        if (::strcmp(version, BanRefusalString) == 0) {
            state = Refused;
            char message[512];
            int len = ::recv(query, (char*)message, 512, 0);
            if (len > 0) {
                message[len - 1] = 0;
            } else {
                message[0] = 0;
            }
            rejectionMessage = message;
        }
        ::close(query);
        return false;
    }

    // read local player's id
    if (!select(query)) {
        ::close(query);
        return false;
    }
    recvd = ::recv(query, (char *) &id, sizeof(id), 0);
    if (recvd < (int) sizeof(id)) {
        return false;
    }

    if (id == 0xff) {
        state = Rejected;
        ::close(query);
        return false;
    }

    if (setBlocking(query) < 0) {
        ::close(query);
        return false;
    }

    fd = query;

    // turn on TCP no delay
    setTcpNoDelay(fd);

    if (netStats) {
        resetNetStats();
    }

    state = Okay;
    return true;
}

bool gSocket::resolveHost(const Glib::ustring& host, const Glib::ustring& port)
{
    struct addrinfo hints, *res;
    char addrstr[100];

    ::memset(&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    int error = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
    if (check_for_error(error)) {
    	return false;
    }

    while (res) {
        inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

        switch (res->ai_family) {
            case AF_INET:
                ::memcpy(&server_info, res, sizeof(struct addrinfo));
                break;
            case AF_INET6:
                // we don't support IPV6 at this time
                //::memcpy(&server_info, res, sizeof(struct addrinfo));
                return false;
        }
        res = res->ai_next;
    }
    freeaddrinfo(res);
    return true;
}

bool gSocket::join(Glib::ustring callsign, Glib::ustring password, Glib::ustring motto)
{
    uint16_t code, rejCode;
    Glib::ustring reason;
    Glib::ustring token("");

    // query the list server to get a token
    listServer.queryListServer(callsign, password, token);
    // request to enter the game -- sending callsign and token
    sendEnter(TankPlayer, ObserverTeam, callsign, motto, token);

    // now see if we were accepted
    if (readEnter(reason, code, rejCode)) {
        // we're in! connect the TCP signal
        tcp_read = Glib::signal_io().connect(sigc::mem_fun(*this, &gSocket::tcp_data_pending),
                                              fd, Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);

        return true;
    }
    // we have failed to join
    rejectionMessage = reason;
    return false;
}

bool gSocket::tcp_data_pending(Glib::IOCondition)
{
    on_tcp_data_pending();
    return true;
}

bool gSocket::select(int _fd)
{
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET((unsigned int)_fd, &set);
    timeout.tv_sec = long(5);
    timeout.tv_usec = 0;
    int nfound = ::select(_fd + 1, (fd_set*)&set, NULL, NULL, &timeout);
    if (nfound <= 0) {
        std::cerr << "gSocket::select(int _fd) Failed!\n";
        return false;
    }
    return true;
}

int gSocket::select(int _fd, int blockTime)
{
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET((unsigned int)_fd, &set);
    timeout.tv_sec = blockTime / 1000;
    timeout.tv_usec = blockTime - 1000 * timeout.tv_sec;
    return ::select(_fd + 1, (fd_set*)&set, NULL, NULL, (struct timeval*)(blockTime >= 0 ? &timeout : NULL));
}

int gSocket::send(guint16 code, guint16 len,	const void* msg)
{
    if (state != Okay) {
        return -1;
    }
    char msgbuf[MaxPacketLen];
    void* buf = msgbuf;
    ssize_t sent = 0, usent = 0;

    buf = parser.nboPackUShort(buf, len);
    buf = parser.nboPackUShort(buf, code);

    if (msg && len != 0) {
        buf = parser.nboPackString(buf, msg, len);
    }
    sent = ::send(fd, (const char*)msgbuf, len + 4, 0);
    if (netStats) {
        bytesSent += sent;
        bytesSent += usent;
        packetsSent++;
    }
    return sent;
}

int gSocket::read(uint16_t& code, uint16_t& len, void* msg, int blockTime)
{
    code = MsgNull;
    len = 0;

    if (state != Okay) {
        return -1;
    }

    /////////////////////////////////////////////////////////////////////
    // In this version the Gtkmm framework is emitting a signal when the
    // socket has pending data to be read. We know there is data when
    // we get here. No need to check to see if that is true. Just read
    // the data.
    /////////////////////////////////////////////////////////////////////

    // block for specified period. default is no blocking (polling)

    // only check server
    int nfound = select(fd, blockTime);

    if (nfound == 0) {
        return 0;
    }
    if (nfound < 0) {
        return -1;
    }

    // get packet header
    char headerBuffer[4];

    int rlen = 0;
    rlen = ::recv(fd, (char*)headerBuffer, 4, 0);

    int tlen = rlen;
    while (rlen >= 1 && tlen < 4) {
        nfound = select(fd, -1);
        if (nfound == 0) {
            continue;
        }
        if (nfound < 0) {
            return -1;
        }
        rlen = ::recv(fd, (char*)headerBuffer + tlen, 4 - tlen, 0);
        if (rlen >= 0) {
            tlen += rlen;
        }
    }
    if (tlen < 4) {
        return -1;
    }

    if (netStats) {
        bytesReceived += 4;
        packetsReceived++;
    }
    // unpack header and get message
    void* buf = headerBuffer;
    buf = parser.nboUnpackUShort(buf, &len);
    buf = parser.nboUnpackUShort(buf, &code);

    if (len > MaxPacketLen) {
        return -1;
    }
    if (len > 0) {
        rlen = ::recv(fd, (char*)msg, int(len), 0);
    } else {
        rlen = 0;
    }
    if (rlen == int(len)) {	// got the whole thing, DONE!
        return 1;
    }

    if (netStats && rlen >= 0) {
        bytesReceived += rlen;
    }
    // keep reading until we get the whole message
    tlen = rlen;
    while (rlen >= 1 && tlen < int(len)) {
        nfound = select(fd, -1);
        if (nfound == 0) {
            continue;
        }
        if (nfound < 0) {
            return -1;
        }
        rlen = ::recv(fd, (char*)msg + tlen, int(len) - tlen, 0);
        if (rlen >= 0) {
            tlen += rlen;
        }
    }

    if (netStats && rlen >= 0) {
        bytesReceived += rlen;
    }

    if (tlen < int(len)) {
        return -1;
    }

    return 1;
}

void gSocket::sendEnter(unsigned char type, unsigned int team, Glib::ustring callsign,
                         Glib::ustring motto, Glib::ustring token)
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

bool gSocket::readEnter (Glib::ustring& reason, uint16_t& code, uint16_t& rejcode)
{
    // wait for response
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

int gSocket::setNonBlocking(int fd)
{
    int mode = fcntl(fd, F_GETFL, 0);
    if (mode == -1 || fcntl(fd, F_SETFL, mode | O_NDELAY) < 0) {
        return -1;
    }
    return 0;
}

int gSocket::setBlocking(int fd)
{
    int mode = fcntl(fd, F_GETFL, 0);
    if (mode == -1 || fcntl(fd, F_SETFL, mode & ~O_NDELAY) < 0) {
        return -1;
    }
    return 0;
}

void gSocket::setTcpNoDelay(int fd)
{
    gint off = 0;
    struct protoent *p = ::getprotobyname("tcp");

    if (p) {
        ::setsockopt(fd, p->p_proto, TCP_NODELAY, &off, sizeof(off));
    }
}

void gSocket::resetNetStats()
{
    startTime = time(0);
    bytesSent = 0;
    bytesReceived = 0;
    packetsSent = 0;
    packetsReceived = 0;
}

float gSocket::bitFlow()
{
    time_t now = time(0);
    double dt = difftime(now, startTime);

    // one minute moving average
    // average = ((prev_avg - bitspersec) * exp(-dt/secondsperminute)) + bitspersec
    float bitsTotal = (float)(bytesReceived + bytesSent) * 8.0;
    float bps = bitsTotal / 1000.0; // kilobits/sec
    float flow = ((prev_flow - bps) * exp(-dt / 60.0)) + bps;
    prev_flow = flow; // member variable

    resetNetStats();

    return flow;
}

void gSocket::sendLagPing(char pingRequest[2])
{
    char msg[3];
    void* buf = msg;

    buf = parser.nboPackUByte(buf, guint8(getId()));
    buf = parser.nboPackString(buf, pingRequest, 2);

    this->send(MsgLagPing, sizeof(msg), msg);
}

void gSocket::sendExit()
{
    char msg[1];

    msg[0] = getId();

    this->send(MsgExit, sizeof(msg), msg);
}

Glib::ustring gSocket::reverseResolve(Glib::ustring ip, Glib::ustring callsign)
{
    struct sockaddr_in addr;
    ::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
	inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr));
    char host[256];
    Glib::ustring result("");

    int error = getnameinfo((struct sockaddr*)&addr, sizeof(addr), host, 256, (char*)NULL, 0, 0);
    if (check_for_error(error)) {
	    result = Glib::ustring::compose("An error has occured, could not resolve IP! (%1)\n", error);
    } else {
    	result = callsign + "'s IP resolved to " + host + "\n";
    }
    return result;
}

int gSocket::check_for_error(int error_code)
{
	switch (error_code) {
        default: // no error (error_code == 0)
            break;

        case EAI_ADDRFAMILY:
            std::cerr << "The specified network host does not have any network addresses in the requested address family.\n";
            break;

        case EAI_AGAIN:
            std::cerr << "The name could not be resolved at this time.\n";
            break;

        case EAI_BADFLAGS:
            std::cerr << "The flags had an invalid value.\n";
            break;

        case EAI_FAIL:
            std::cerr << "A non-recoverable error occurred.\n";
            break;

        case EAI_FAMILY:
            std::cerr << "The address family was not recognized or the address length was invalid.\n";
            break;

        case EAI_MEMORY:
            std::cerr << "There was a memory allocation failure.\n";
            break;

        case EAI_NONAME:
            std::cerr << "The name does not resolve for the supplied parameters.\n";
            break;

        case EAI_NODATA:
            std::cerr << "The specified network host exists, but does not have any network addresses defined.\n";
            break;

        case EAI_OVERFLOW:
            std::cerr << "An argument buffer overflowed.\n";
            break;

        case EAI_SYSTEM:
            std::cerr << "A system error occurred. The error code can be found in errno. \n";
            break;

        case EAI_SERVICE:
            std::cerr << "The requested service is not available for the requested socket type.\n";
            break;

        case EAI_SOCKTYPE:
            std::cerr << "The requested socket type is not supported at all.\n";
            break;
    }
    return error_code;
}


