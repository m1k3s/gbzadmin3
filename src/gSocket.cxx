// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2012 Michael Sheppard
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
    :	state( SocketError ),	fd( -1 ), netStats( true ), udpLength( 0 )
{
    prev_flow = 0.0;
}

gSocket::~gSocket()
{
    disconnect();
}

void gSocket::disconnect()
{
    if ( state != Okay )
        return;

    tcp_read.disconnect();

    ::shutdown( fd, 2 );
    ::close( fd );
    fd = -1;

    if ( urecvfd >= 0 ) {
        udp_read.disconnect();
        ::close( urecvfd );
    }

    udpLength = 0;
    urecvfd = -1;
    ulinkup = false;
    state = SocketError;
}

bool gSocket::connect( Glib::ustring host, int p )
{
    if ( state == Okay ) // already connected
        return false;

    int recvd;
    urecvfd = -1;
    serverName = host;
    port = p;
    const char* const BanRefusalString = "REFUSED:";

    ulinkup = false;

    // initialize version string
    ::strcpy( version, "BZFS0000" );

    // resolve the host name address
    if ( !resolveHost( host ) ) {
        state = ResolveFailure;
        rejectionMessage = "DNS: Failed to resolve host name\n";
        return false;
    }
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons( port );
    serverIP = inet_ntoa( sockaddr.sin_addr );

    // for UDP, used later
    ::memcpy( ( unsigned char * )&usendaddr, ( unsigned char * )&sockaddr, sizeof( sockaddr ) );

    // open connection to server.
    int query = socket( AF_INET, SOCK_STREAM, 0 );
    if ( query < 0 )
        return false;

    // this is where we actually connect. Hopefully.
    if ( ::connect( query, ( struct sockaddr * )&sockaddr, sizeof( sockaddr ) ) < 0 ) {
        if ( errno != EINPROGRESS ) {
            ::close( query );
            return false;
        }
        if ( !select( query ) ) {
            ::close( query );
            return false;
        }
        // check for connection errors
        int connectError;
        socklen_t errorLen = sizeof( int );
        if ( ::getsockopt( query, SOL_SOCKET, SO_ERROR, &connectError, &errorLen )	< 0 ) {
            ::close( query );
            return false;
        }
        if ( connectError != 0 ) {
            ::close( query );
            return false;
        }
    }
    // send the connection header
    ::send( query, BZ_CONNECT_HEADER, ( int )strlen( BZ_CONNECT_HEADER ), 0 );

    bool gotNetData = false;

    int loopCount = 0;
    while( !gotNetData ) {
        loopCount++;
        // get server version and verify
        if ( !select( query ) ) {
            ::close( query );
            return false;
        }
        recvd = ::recv( query, ( char* )version, 8, 0 );
        if ( recvd > 0 ) {
            gotNetData = true;
        } else if ( loopCount > 20 ) {
            ::close( query );
            return false;
        }
        Glib::usleep( G_USEC_PER_SEC / 4 ); // 1/4 second
    }

    if ( recvd < 8 ) {
        close( query );
        return false;
    }

    if ( setNonBlocking( query ) < 0 ) {
        close( query );
        return false;
    }

    if ( ::strcmp( version, ServerVersion ) != 0 ) {
        std::cerr << "BAD VERSION: " << version << std::endl;
        state = BadVersion;
        if ( ::strcmp( version, BanRefusalString ) == 0 ) {
            state = Refused;
            char message[512];
            int len = ::recv( query, ( char* )message, 512, 0 );
            if ( len > 0 ) {
                message[len - 1] = 0;
            } else {
                message[0] = 0;
            }
            rejectionMessage = message;
        }
        ::close( query );
        return false;
    }

    // read local player's id
    if ( !select( query ) ) {
        ::close( query );
        return false;
    }
    recvd = ::recv( query, ( char * ) &id, sizeof( id ), 0 );
    if ( recvd < ( int ) sizeof( id ) )
        return false;

    if ( id == 0xff ) {
        state = Rejected;
        ::close( query );
        return false;
    }

    if ( setBlocking( query ) < 0 ) {
        ::close( query );
        return false;
    }

    fd = query;

    // turn on TCP no delay
    setTcpNoDelay( fd );

    if ( netStats ) {
        resetNetStats();
    }

    state = Okay;
    return true;
}

bool gSocket::resolveHost( const Glib::ustring& host )
{
    struct addrinfo hints, *res;
    char addrstr[100];
    void *ptr = 0;

    ::memset( &hints, 0, sizeof ( hints ) );
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    int error = getaddrinfo( host.c_str(), NULL, &hints, &res );
    switch ( error ) {
        default: // no error (error == 0)
            break;

        case EAI_ADDRFAMILY:
            std::cerr << "The specified network host does not have any network addresses in the requested address family.\n";
            return false;

        case EAI_AGAIN:
            std::cerr << "The name could not be resolved at this time.\n";
            return false;

        case EAI_BADFLAGS:
            std::cerr << "The flags had an invalid value.\n";
            return false;

        case EAI_FAIL:
            std::cerr << "A non-recoverable error occurred.\n";
            return false;

        case EAI_FAMILY:
            std::cerr << "The address family was not recognized or the address length was invalid.\n";
            return false;

        case EAI_MEMORY:
            std::cerr << "There was a memory allocation failure.\n";
            return false;

        case EAI_NONAME:
            std::cerr << "The name does not resolve for the supplied parameters.\n";
            return false;

        case EAI_NODATA:
            std::cerr << "The specified network host exists, but does not have any network addresses defined.\n";
            return false;

        case EAI_OVERFLOW:
            std::cerr << "An argument buffer overflowed.\n";
            return false;

        case EAI_SYSTEM:
            std::cerr << "A system error occurred. The error code can be found in errno. \n";
            return false;

        case EAI_SERVICE:
            std::cerr << "The requested service is not available for the requested socket type.\n";
            return false;

        case EAI_SOCKTYPE:
            std::cerr << "The requested socket type is not supported at all.\n";
            return false;
    }

    while ( res ) {
        inet_ntop ( res->ai_family, res->ai_addr->sa_data, addrstr, 100 );

        switch ( res->ai_family ) {
            case AF_INET:
                ptr = &( ( struct sockaddr_in * ) res->ai_addr )->sin_addr;
                break;
            case AF_INET6:
                // we don't support IPV6 at this time
                //ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
                return false;
        }
        inet_ntop ( res->ai_family, ptr, addrstr, 100 );
        ::memcpy( &sockaddr.sin_addr, ptr, sizeof( struct in_addr ) );

        res = res->ai_next;
    }
    return true;
}

bool gSocket::join( Glib::ustring callsign, Glib::ustring password )
{
    uint16_t code, rejCode;
    Glib::ustring reason;
    Glib::ustring token( "" );

    // query the list server to get a token
    listServer.queryListServer( callsign, password, token );
    // request to enter the game -- sending callsign and token
    sendEnter( TankPlayer, ObserverTeam, callsign, "gbzadmin", token );

    // FIXME: need to make sure all this stuff goes before
    //        calling readEnter()

    // now see if we were accepted
    if ( readEnter( reason, code, rejCode ) ) {
        // we're in! connect the TCP signal only
        tcp_read = Glib::signal_io().connect( sigc::mem_fun( *this, &gSocket::tcp_data_pending ),
                                              fd, Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL );

        return true;
    }
    // we have failed to join
    rejectionMessage = reason;
    return false;
}

bool gSocket::tcp_data_pending( Glib::IOCondition )
{
    on_tcp_data_pending();
    return true;
}

bool gSocket::udp_data_pending( Glib::IOCondition )
{
    on_udp_data_pending();
    return true;
}

bool gSocket::select( int _fd )
{
    fd_set set;
    struct timeval timeout;

    FD_ZERO( &set );
    FD_SET( ( unsigned int )_fd, &set );
    timeout.tv_sec = long( 5 );
    timeout.tv_usec = 0;
    int nfound = ::select( _fd + 1, ( fd_set* )&set, NULL, NULL, &timeout );
    if ( nfound <= 0 ) {
        std::cerr << "gSocket::select(int _fd) Failed!\n";
        return false;
    }
    return true;
}

int gSocket::select( int _fd, int blockTime )
{
    fd_set set;
    struct timeval timeout;

    FD_ZERO( &set );
    FD_SET( ( unsigned int )_fd, &set );
    timeout.tv_sec = blockTime / 1000;
    timeout.tv_usec = blockTime - 1000 * timeout.tv_sec;
    return ::select( _fd + 1, ( fd_set* )&set, NULL, NULL, ( struct timeval* )( blockTime >= 0 ? &timeout : NULL ) );
}

int gSocket::send( guint16 code, guint16 len,	const void* msg )
{
    if ( state != Okay )
        return -1;
    //std::cerr << "Sending packet size: " << len << std::endl;

    bool useUDP = false;
    char msgbuf[MaxPacketLen];
    void* buf = msgbuf;
    ssize_t sent = 0, usent = 0;

    buf = parser.nboPackUShort( buf, len );
    buf = parser.nboPackUShort( buf, code );

    if ( msg && len != 0 )
        buf = parser.nboPackString( buf, msg, len );

    if ( ( urecvfd >= 0 ) && ulinkup ) {
        switch ( code ) {
            case MsgUDPLinkRequest:
            case MsgUDPLinkEstablished:
                useUDP = true;
                break;
        }
    }

    if ( useUDP ) {
        usent = ::sendto( urecvfd, ( const char * )msgbuf, ( char* )buf - msgbuf, 0, &usendaddr, sizeof( usendaddr ) );
        // we don't care about errors yet, but I'm sure we will...
        return 1;
    }
    sent = ::send( fd, ( const char* )msgbuf, len + 4, 0 );
    if ( netStats ) {
        bytesSent += sent;
        bytesSent += usent;
        packetsSent++;
    }
    return sent;
}

int gSocket::read( uint16_t& code, uint16_t& len, void* msg, int blockTime )
{
    code = MsgNull;
    len = 0;

    if ( state != Okay )
        return -1;

    if ( ( urecvfd >= 0 ) ) {
        int n;
        if ( !udpLength ) {
            socklen_t recvlen = sizeof( urecvaddr );
            n = ::recvfrom( urecvfd, ubuf, MaxPacketLen, 0, &urecvaddr, ( socklen_t* ) &recvlen );
            if ( n > 0 ) {
                udpLength = n;
                udpBufferPtr = ubuf;
            }
        }
        if ( udpLength ) {
            // unpack header and get message
            udpLength -= 4;
            if ( udpLength < 0 ) {
                udpLength = 0;
                return -1;
            }
            udpBufferPtr = ( char * )parser.nboUnpackUShort( udpBufferPtr, &len );
            udpBufferPtr = ( char * )parser.nboUnpackUShort( udpBufferPtr, &code );
            if ( len > udpLength ) {
                udpLength = 0;
                return -1;
            }
            ::memcpy( ( char * )msg, udpBufferPtr, len );
            udpBufferPtr += len;
            udpLength -= len;
            return 1;
        }
        len = 0;
        code = MsgNull;

        blockTime = 0;
    }

    /////////////////////////////////////////////////////////////////////
    // In this version the Gtkmm framework is emitting a signal when the
    // socket has pending data to be read. We know there is data when
    // we get here. No need to check to see if that is true. Just read
    // the data.
    /////////////////////////////////////////////////////////////////////

    // block for specified period. default is no blocking (polling)

    // only check server
    int nfound = select( fd, blockTime );

    if ( nfound == 0 )
        return 0;
    if ( nfound < 0 )
        return -1;

    // FIXME: don't really want to take the chance of waiting forever
    // on the remaining select() calls, but if the server and network
    // haven't been hosed then the data will get here soon. And if the
    // server or network is down then we don't really care anyway.

    // get packet header
    char headerBuffer[4];

    int rlen = 0;
    rlen = ::recv( fd, ( char* )headerBuffer, 4, 0 );

    int tlen = rlen;
    while ( rlen >= 1 && tlen < 4 ) {
        nfound = select( fd, -1 );
        if ( nfound == 0 )
            continue;
        if ( nfound < 0 )
            return -1;
        rlen = ::recv( fd, ( char* )headerBuffer + tlen, 4 - tlen, 0 );
        if ( rlen >= 0 )
            tlen += rlen;
    }
    if ( tlen < 4 ) {
        return -1;
    }

    if ( netStats ) {
        bytesReceived += 4;
        packetsReceived++;
    }
    // unpack header and get message
    void* buf = headerBuffer;
    buf = parser.nboUnpackUShort( buf, &len );
    buf = parser.nboUnpackUShort( buf, &code );

    if ( len > MaxPacketLen ) {
        return -1;
    }
    if ( len > 0 )
        rlen = ::recv( fd, ( char* )msg, int( len ), 0 );
    else
        rlen = 0;

    if ( rlen == int( len ) )	// got the whole thing, DONE!
        return 1;

    if ( netStats && rlen >= 0 ) {
        bytesReceived += rlen;
    }
    // keep reading until we get the whole message
    tlen = rlen;
    while ( rlen >= 1 && tlen < int( len ) ) {
        nfound = select( fd, -1 );
        if ( nfound == 0 )
            continue;
        if ( nfound < 0 )
            return -1;
        rlen = ::recv( fd, ( char* )msg + tlen, int( len ) - tlen, 0 );
        if ( rlen >= 0 )
            tlen += rlen;
    }

    if ( netStats && rlen >= 0 ) {
        bytesReceived += rlen;
    }

    if ( tlen < int( len ) ) {
        return -1;
    }

    return 1;
}

void gSocket::sendEnter( unsigned char type, unsigned int team, Glib::ustring callsign,
                         Glib::ustring motto, Glib::ustring token )
{
    if ( state != Okay )
        return;
    char msg[PlayerIdPLen + 4 + CallSignLen + MottoLen + TokenLen + VersionLen] = {0};
    void* buf = msg;

    buf = parser.nboPackUShort( buf, uint16_t( type ) );
    buf = parser.nboPackUShort( buf, uint16_t( team ) );
    ::memcpy( buf, callsign.c_str(), callsign.size() );
    buf = ( void* )( ( char* )buf + CallSignLen );
    ::memcpy( buf, motto.c_str(), motto.size() );
    buf = ( void* )( ( char* )buf + MottoLen );
    ::memcpy( buf, token.c_str(), token.size() );
    buf = ( void* )( ( char* )buf + TokenLen );
    ::memcpy( buf, getAppVersion(), ::strlen( getAppVersion() ) + 1 );
    buf = ( void* )( ( char* )buf + VersionLen );

    this->send( MsgEnter, sizeof( msg ), msg );
}

bool gSocket::readEnter ( Glib::ustring& reason, uint16_t& code, uint16_t& rejcode )
{
    // wait for response
    guint16 len;
    char msg[MaxPacketLen];

    while ( true ) {
        if ( this->read( code, len, msg, -1 ) < 0 ) {
            reason = "Communication error joining game [No immediate respose].";
            return false;
        }

        if ( code == MsgAccept ) {
            return true;
        } else if ( code == MsgSuperKill ) {
            reason = "Server forced disconnection.";
            return false;
        } else if ( code == MsgReject ) {
            void *buf;
            char buffer[MessageLen];
            buf = parser.nboUnpackUShort ( msg, &rejcode ); // filler for now
            buf = parser.nboUnpackString ( buf, buffer, MessageLen );
            buffer[MessageLen - 1] = '\0';
            reason = buffer;
            return false;
        }
    }

    return true;
}

void gSocket::sendUDPlinkRequest()
{
    char msg[1];
    unsigned short localPort;
    void* buf = msg;

    struct sockaddr_in serv_addr;

    if ( ( urecvfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 ) {
        return; // we cannot comply
    }

    socklen_t addr_len = sizeof( serv_addr );
    if ( ::getsockname( fd, ( struct sockaddr* )&serv_addr, ( socklen_t* ) &addr_len ) < 0 ) {
        std::cerr << "Error: getsockname() failed, cannot retrieve socket from fd" << std::endl;
        urecvfd = -1;
        return;
    }
    if ( ::bind( urecvfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr ) ) != 0 ) {
        std::cerr << "Error: bind() failed (UDP)" << std::endl;
        ulinkup = false;
        ::close( urecvfd );
        urecvfd = -1;
        return;  // we cannot get a UDP connection, bail out
    }

    localPort = ntohs( serv_addr.sin_port );
    ::memcpy( ( char * )&urecvaddr, ( char * )&serv_addr, sizeof( serv_addr ) );
    std::cerr << "Created local downlink port " << localPort << "\n";

    buf = parser.nboPackUByte( buf, id );

    if ( setNonBlocking( urecvfd ) < 0 ) {
        std::cerr << "Error: Unable to set NonBlocking for UDP socket" << std::endl;
    }
    // setup the UDP signaling channel
    udp_read = Glib::signal_io().connect( sigc::mem_fun( *this, &gSocket::udp_data_pending ),
                                          urecvfd, Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL );

    this->send( MsgUDPLinkRequest, sizeof( msg ), msg );
}

// heard back from server that we can send udp
void gSocket::enableOutboundUDP()
{
    if ( ulinkup == false )
        ulinkup = true;
}

// confirm that server can send us UDP
void gSocket::confirmIncomingUDP()
{
    // enableOutboundUDP will be setting this but frequently
    // the udp handshake will finish first so might as
    // well start with udp as soon as we can
    if ( ulinkup == false )
        ulinkup = true;

    this->send( MsgUDPLinkEstablished, 0, NULL );
}

int gSocket::setNonBlocking( int fd )
{
    int mode = fcntl( fd, F_GETFL, 0 );
    if ( mode == -1 || fcntl( fd, F_SETFL, mode | O_NDELAY ) < 0 )
        return -1;
    return 0;
}

int gSocket::setBlocking( int fd )
{
    int mode = fcntl( fd, F_GETFL, 0 );
    if ( mode == -1 || fcntl( fd, F_SETFL, mode & ~O_NDELAY ) < 0 )
        return -1;
    return 0;
}

void gSocket::setTcpNoDelay( int fd )
{
    gint off = 0;
    struct protoent *p = ::getprotobyname( "tcp" );

    if ( p )
        ::setsockopt( fd, p->p_proto, TCP_NODELAY, &off, sizeof( off ) );
}

void gSocket::sendCaps( bool downloads, bool sounds )
{
    char msg[3] = {0};
    void* buf = msg;

    buf = parser.nboPackUByte( buf, id );
    buf = parser.nboPackUByte( buf, downloads ? 1 : 0 );
    buf = parser.nboPackUByte( buf, sounds ? 1 : 0 );

    this->send( MsgCapBits, ( guint16 )( ( char* )buf - msg ), msg );
}

void gSocket::sendCustomData( const Glib::ustring key, const Glib::ustring value )
{
    if ( key.size() + value.size() >= ( guint )MaxPacketLen )
        return;

    char msg[MaxPacketLen];
    void* buf = msg;
    buf = parser.nboPackUByte( buf, id );
    buf = parser.nboPackStdString( buf, key );
    buf = parser.nboPackStdString( buf, value );

    this->send( MsgPlayerData, ( guint16 )( ( char* )buf - msg ), msg );
}

void gSocket::resetNetStats()
{
    startTime = time( 0 );
    bytesSent = 0;
    bytesReceived = 0;
    packetsSent = 0;
    packetsReceived = 0;
}

float gSocket::bitFlow()
{
    time_t now = time( 0 );
    double dt = difftime( now, startTime );

    // one minute moving average
    // average = ((prev_avg - bitspersec) * exp(-dt/secondsperminute)) + bitspersec
    float bitsTotal = ( float )( bytesReceived + bytesSent ) * 8.0;
    float bps = bitsTotal / 1000.0; // kilobits/sec
    float flow = ( ( prev_flow - bps ) * exp( -dt / 60.0 ) ) + bps;
    prev_flow = flow; // member variable

    resetNetStats();

    return flow;
}

void gSocket::sendLagPing( char pingRequest[2] )
{
    char msg[3];
    void* buf = msg;

    buf = parser.nboPackUByte( buf, guint8( getId() ) );
    buf = parser.nboPackString( buf, pingRequest, 2 );

    this->send( MsgLagPing, sizeof( msg ), msg );
}

void gSocket::sendExit()
{
    char msg[1];

    msg[0] = getId();

    this->send( MsgExit, sizeof( msg ), msg );
}

Glib::ustring gSocket::reverseResolve( Glib::ustring ip, Glib::ustring callsign )
{
    struct sockaddr_in addr;
    ::memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( ip.c_str() );
    char host[256];
    Glib::ustring result( "" );

    int error = getnameinfo( ( struct sockaddr* )&addr, sizeof( addr ), host, 256, ( char* )NULL, 0, 0 );
    switch ( error ) {
        default: // no error (error == 0)
            result = callsign + "'s IP resolved to " + host + "\n";
            break;

        case EAI_AGAIN:
            result = "The name could not be resolved at this time.\n";
            break;

        case EAI_BADFLAGS:
            result = "The flags had an invalid value.\n";
            break;

        case EAI_FAIL:
            result = "A non-recoverable error occurred.\n";
            break;

        case EAI_FAMILY:
            result = "The address family was not recognized or the address length was invalid.\n";
            break;

        case EAI_MEMORY:
            result = "There was a memory allocation failure.\n";
            break;

        case EAI_NONAME:
            result = "The name does not resolve for the supplied parameters.\n";
            break;

        case EAI_OVERFLOW:
            result = "An argument buffer overflowed.\n";
            break;

        case EAI_SYSTEM:
            result = "A system error occurred. The error code can be found in errno. \n";
            break;
    }
    return result;
}


