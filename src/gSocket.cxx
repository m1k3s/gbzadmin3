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
    :	state(SocketError), sockfd(-1), netStats(true)
{
    prev_flow = 0.0;
    prefetch_token = false;
    token = "";
}

gSocket::~gSocket()
{
    disconnect();
}

void gSocket::preFetchToken(Glib::ustring callsign, Glib::ustring password)
{
	token = ""; // clear the token
	listServer.queryListServer(callsign, password, token);
}

void gSocket::disconnect()
{
    if (state != Okay) {
        return;
    }

    tcp_read.disconnect();

    ::shutdown(sockfd, 2);
    ::close(sockfd);
    sockfd = -1;

    state = SocketError;
}

bool gSocket::connect(Glib::ustring host, int p)
{
    if (state == Okay) { // already connected
        return false;
    }

    serverName = host;
    port = p;
    const char* const BanRefusalString = "REFUSED:";

    // initialize version string
	version = "BZFS0000";

    // resolve the host name address
    if (!resolve_host(host, Glib::ustring::compose("%1", port))) {
        state = ResolveFailure;
        rejectionMessage = errorMsg;
        return false;
    }
	// get the server IP string
	serverIP = get_ip_str(&server_info);

    // create a socket connection to server
    int sock_type = server_info.ai_socktype;// | SOCK_NONBLOCK;
    if ((sockfd = ::socket(server_info.ai_family, sock_type, server_info.ai_protocol)) == -1) {
        return false;
    }
    
    // try to connect
	if (::connect(sockfd, server_info.ai_addr, server_info.ai_addrlen) < 0) {
        if (errno != EINPROGRESS) {
            ::close(sockfd);
            return false;
        }
        if (!select(sockfd)) {
            ::close(sockfd);
            return false;
        }
        // check for connection errors
        int connectError;
        socklen_t errorLen = sizeof(int);
        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &connectError, &errorLen) < 0) {
            ::close(sockfd);
            return false;
        }
        if (connectError) {
            ::close(sockfd);
            return false;
        }
    }
    
    // send the bzflag connection header
    ::send(sockfd, BZ_CONNECT_HEADER, (int)strlen(BZ_CONNECT_HEADER), 0);

    bool got_version = false;
    int loopCount = 0;
    ssize_t recvd;
    while (!got_version) {
        loopCount++;
        // get server version and verify
        if (!select(sockfd)) {
            ::close(sockfd);
            return false;
        }
        recvd = ::recv(sockfd, (char*)version.c_str(), 8, 0);
        if (recvd == 8) {
            got_version = true;
        } else if (loopCount > 20) {
            ::close(sockfd);
            return false;
        }
        Glib::usleep(G_USEC_PER_SEC / 4); // 1/4 second
    }

//    if (recvd < 8) {
//        close(sockfd);
//        return false;
//    }

    if (setNonBlocking(sockfd) < 0) {
        ::close(sockfd);
        return false;
    }

	if (ServerVersion != version) {
        state = BadVersion;
		if (BanRefusalString == version) {
            state = Refused;
            char message[512];
            int len = ::recv(sockfd, (char*)message, 512, 0);
            if (len > 0) {
                message[len - 1] = 0;
            } else {
                message[0] = 0;
            }
            rejectionMessage = message;
        }
        ::close(sockfd);
        return false;
    }

    if (!select(sockfd)) {
        ::close(sockfd);
        return false;
    }
    // read local player's id
    recvd = ::recv(sockfd, (char *) &id, sizeof(id), 0);
    if (recvd < (int) sizeof(id)) {
        return false;
    }

    if (id == 0xff) {
        state = Rejected;
        ::close(sockfd);
        return false;
    }

    if (setBlocking(sockfd) < 0) {
        ::close(sockfd);
        return false;
    }

    // turn on TCP no delay
    setTcpNoDelay(sockfd);

    if (netStats) {
        resetNetStats();
    }

    state = Okay;
    return true;
}

bool gSocket::resolve_host(const Glib::ustring& host, const Glib::ustring& port)
{
    struct addrinfo hints, *res, *p;
    bool result = false;

    ::memset(&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC; // future support of ipv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;
    hints.ai_protocol = IPPROTO_TCP;

    int status = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
    if (check_status(status)) {
    	return false;
    }
	p = res;
    while (p) {
        if (p->ai_family == AF_INET) {
        	::memcpy(&server_info, p, sizeof(struct addrinfo));
        	result = true;
            break;
        } else if (p->ai_family == AF_INET6) {
        	// we don't support IPV6 at this time
            result = false;
        	break;
        }
        p = p->ai_next;
    }
    freeaddrinfo(res);
    
    return result;
}

bool gSocket::join(Glib::ustring callsign, Glib::ustring password, Glib::ustring motto)
{
    uint16_t code, rejCode;
    Glib::ustring reason;
//    Glib::ustring token("");

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
        tcp_read = Glib::signal_io().connect(sigc::mem_fun(*this, &gSocket::tcp_data_pending), sockfd, flags);

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

bool gSocket::select(int _sockfd)
{
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET((unsigned int)_sockfd, &set);
    timeout.tv_sec = long(5);
    timeout.tv_usec = 0;
    int nfound = ::select(_sockfd + 1, (fd_set*)&set, NULL, NULL, &timeout);
    if (nfound <= 0) {
        std::cerr << "gSocket::select(int _sockfd) Failed!\n";
        return false;
    }
    return true;
}

int gSocket::select(int _sockfd, int blockTime)
{
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET((unsigned int)_sockfd, &set);
    timeout.tv_sec = blockTime / 1000;
    timeout.tv_usec = blockTime - 1000 * timeout.tv_sec;
    return ::select(_sockfd + 1, (fd_set*)&set, NULL, NULL, (struct timeval*)(blockTime >= 0 ? &timeout : NULL));
}

int gSocket::send(guint16 code, guint16 len,	const void* msg)
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
    sent = ::send(sockfd, (const char*)msgbuf, len + 4, 0);
    if (netStats) {
        bytesSent += sent;
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

    // wait for data
    int nfound = select(sockfd, blockTime);

    if (nfound == 0) { // timed out or no data
        return 0;
    }
    if (nfound < 0) { // an error occurred
        return -1;
    }

    // get packet header
    char headerBuffer[4];
    int rlen = 0;
    rlen = ::recv(sockfd, (char*)headerBuffer, 4, 0);

    int tlen = rlen;
    while (rlen >= 1 && tlen < 4) {
        nfound = select(sockfd, -1);
        if (nfound == 0) {
            continue;
        }
        if (nfound < 0) {
            return -1;
        }
        rlen = ::recv(sockfd, (char*)headerBuffer + tlen, 4 - tlen, 0);
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
        rlen = ::recv(sockfd, (char*)msg, int(len), 0);
    } else {
        rlen = 0;
    }
    if (rlen == int(len)) {	// got the whole message, done
    	if (netStats) {
		    bytesReceived += rlen;
		}
    } else { // keep reading until we get the whole message
		tlen = rlen;
		while (rlen >= 1 && tlen < int(len)) {
		    nfound = select(sockfd, -1);
		    if (nfound == 0) {
		        continue;
		    }
		    if (nfound < 0) {
		        return -1;
		    }
		    rlen = ::recv(sockfd, (char*)msg + tlen, int(len) - tlen, 0);
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

int gSocket::setNonBlocking(int sockfd)
{
    int mode = fcntl(sockfd, F_GETFL, 0);
    if (mode == -1 || fcntl(sockfd, F_SETFL, mode | O_NDELAY) < 0) {
        return -1;
    }
    return 0;
}

int gSocket::setBlocking(int sockfd)
{
    int mode = fcntl(sockfd, F_GETFL, 0);
    if (mode == -1 || fcntl(sockfd, F_SETFL, mode & ~O_NDELAY) < 0) {
        return -1;
    }
    return 0;
}

void gSocket::setTcpNoDelay(int sockfd)
{
    gint off = 0;
    struct protoent *p = ::getprotobyname("tcp");

    if (p) {
        ::setsockopt(sockfd, p->p_proto, TCP_NODELAY, &off, sizeof(off));
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

    int status = getnameinfo((struct sockaddr*)&addr, sizeof(addr), host, 256, (char*)NULL, 0, 0);
    if (check_status(status)) {
	    result = errorMsg;
    } else {
    	result = callsign + "'s IP resolved to " + host + "\n";
    }
    return result;
}

int gSocket::check_status(int status_code)
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

Glib::ustring gSocket::get_ip_str(const struct addrinfo *ai)
{
	char addr_str[INET_ADDRSTRLEN];
	inet_ntop(ai->ai_family, get_in_addr((struct sockaddr*)ai->ai_addr), addr_str, INET_ADDRSTRLEN);
	return Glib::ustring(addr_str);
}

void* gSocket::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

