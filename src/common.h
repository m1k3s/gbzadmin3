#ifndef _common_h_
#define _common_h_
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
#include <cerrno>
#include <curl/curl.h>
#include <cstdlib>

#define APPNAME "gbzadmin3"

// notebook pages
enum {
    Page_Players = 0,
    Page_ServerVars,
    Page_ServerList
};

const int BlockTime = 100;	// milliseconds

const int PlayerIdPLen = sizeof(unsigned char);
const int CallSignLen = 32;
const int PasswordLen = 32;
const int MottoLen = 128;
const int TokenLen = 22;
const int VersionLen = 60;
const int MessageLen = 128;
const int MaxPacketLen = 1024;
const int IpLen = 32;
const int ServerNameLen = 64;
const int DescLen = 265;
const int ReferrerLen = 256;

const int TankPlayer = 0;
const int ComputerPlayer = 1;
//const int ChatPlayer = 2;

const int DefaultPort = 5154;

// types of text messages
enum MessageType {
    ChatMessage,
    ActionMessage
};

// types of updates we can receive
enum NetworkUpdates {
    NoUpdates       = 0, // receive no player data and no chats
    PlayerUpdates   = 1, // receive only player updates
    ChatUpdates     = 2, // receive only chats
    AllUpdates      = 3  // receive player data and chats
};

// Player allow attributes for MsgAllow
enum PlayerAllow {
    AllowNone = 0x00,
    AllowShoot = 0x01,
    AllowJump = 0x02,
    AllowTurnLeft = 0x04,
    AllowTurnRight = 0x08,
    AllowMoveForward = 0x10,
    AllowMoveBackward = 0x20,
    AllowAll = 0xFF
};

// pause codes
enum PauseCodes {
    PauseCodeDisable     = 0,
    PauseCodeEnable      = 1,
    PauseCodeCancel      = 2,
    PauseCodeAcknowledge = 3
};

// protocol version
const char* const ServerVersion	= "BZFS0221";

// null message code -- should never be sent/received
const guint16		MsgNull = 0x0000;

// server message codes
const guint16		MsgAccept = 0x6163;				// 'ac'
const guint16		MsgAdminInfo = 0x6169;			// 'ai'
const guint16		MsgAlive = 0x616c;				// 'al'
const guint16		MsgAllow = 0x696f; 				// 'ao'
const guint16		MsgAddPlayer = 0x6170;			// 'ap'
const guint16		MsgAutoPilot = 0x6175;			// 'au'
const guint16		MsgCapBits = 0x6362; 			// 'cb'
const guint16		MsgCaptureFlag = 0x6366;		// 'cf'
const guint16		MsgCustomSound = 0x6373;		// 'cs'
const guint16		MsgCacheURL = 0x6375;			// 'cu'
const guint16		MsgDropFlag = 0x6466;			// 'df'
const guint16		MsgEnter = 0x656e;				// 'en'
const guint16		MsgExit = 0x6578;				// 'ex'
const guint16		MsgFlagUpdate = 0x6675;			// 'fu'
const guint16		MsgFetchResources = 0x6672;		// 'fr'
const guint16		MsgGrabFlag = 0x6766;			// 'gf'
const guint16		MsgGMUpdate = 0x676d;			// 'gm'
const guint16		MsgGetWorld = 0x6777;			// 'gw'
const guint16		MsgGameSettings = 0x6773;		// 'gs'
const guint16		MsgGameTime = 0x6774;			// 'gt'
const guint16		MsgHit = 0x6869; 				// 'hi'
const guint16		MsgJoinServer = 0x6a73; 		// 'js'
const guint16		MsgKilled = 0x6b6c;				// 'kl'
const guint16		MsgKrbPrincipal = 0x6b70;		// 'kp'
const guint16		MsgKrbTicket    = 0x6b74;		// 'kt'
const guint16		MsgLagState = 0x6c73;			// 'ls'
const guint16		MsgLimboMessage = 0x6c6d; 		// 'lm'
const guint16		MsgMessage = 0x6d67;			// 'mg'
const guint16		MsgNewPlayer = 0x6e70; 			// 'np'
const guint16		MsgNearFlag = 0x4e66; 			// 'Nf'
const guint16		MsgNewRabbit = 0x6e52;			// 'nR'
const guint16		MsgNegotiateFlags = 0x6e66;		// 'nf'
const guint16		MsgPause = 0x7061;				// 'pa'
const guint16		MsgPlayerInfo = 0x7062;			// 'pb'
const guint16		MsgPlayerData = 0x7064; 		// 'pd'
const guint16		MsgPlayerUpdate = 0x7075;		// 'pu'
const guint16		MsgPlayerUpdateSmall = 0x7073;	// 'ps'
const guint16		MsgQueryGame = 0x7167;			// 'qg'
const guint16		MsgQueryPlayers = 0x7170;		// 'qp'
const guint16		MsgReject = 0x726a;				// 'rj'
const guint16		MsgRemovePlayer = 0x7270;		// 'rp'
const guint16		MsgReplayReset = 0x7272;		// 'rr'
const guint16		MsgShotBegin = 0x7362;			// 'sb'
const guint16		MsgScore = 0x7363;				// 'sc'
const guint16		MsgScoreOver = 0x736f;			// 'so'
const guint16		MsgShotEnd = 0x7365;			// 'se'
const guint16		MsgSuperKill = 0x736b;			// 'sk'
const guint16		MsgSetTeam = 0x7374; 			// 'st'
const guint16		MsgSetVar = 0x7376;				// 'sv'
const guint16		MsgTangibilityUpdate = 0x746e;	// 'tn'
const guint16		MsgTangibilityReset  = 0x7472;	// 'tr'
const guint16		MsgTimeUpdate = 0x746f;			// 'to'
const guint16		MsgTeleport = 0x7470;			// 'tp'
const guint16		MsgTransferFlag = 0x7466;		// 'tf'
const guint16		MsgTeamUpdate = 0x7475;			// 'tu'
const guint16		MsgWantWHash = 0x7768;			// 'wh'
const guint16		MsgWhatTimeIsIt = 0x7774;		// 'wt'
const guint16		MsgWantSettings = 0x7773;		// 'ws'
const guint16		MsgPortalAdd = 0x5061;			// 'Pa'
const guint16		MsgPortalRemove = 0x5072;		// 'Pr'
const guint16		MsgPortalUpdate = 0x5075;		// 'Pu'

// ping packet sizes, codes and structure
const guint16		MsgPingCodeReply = 0x0303;
const guint16		MsgPingCodeRequest = 0x0404;

const guint16		MsgEchoRequest  = 0x6572; // 'er'
const guint16		MsgEchoResponse = 0x6570; // 'ep'

// rejection codes
const guint16		RejectBadRequest = 0x0000;
const guint16		RejectBadTeam = 0x0001;
const guint16		RejectBadType = 0x0002;
const guint16		RejectBadEmail = 0x0003;
const guint16		RejectTeamFull = 0x0004;
const guint16		RejectServerFull = 0x0005;
const guint16		RejectBadCallsign = 0x0006;
const guint16		RejectRepeatCallsign = 0x0007;
const guint16		RejectRejoinWaitTime = 0x0008;
const guint16		RejectIPBanned = 0x0009;
const guint16		RejectHostBanned = 0x000A;
const guint16		RejectIDBanned = 0x000B;

// death by obstacle
const guint16		PhysicsDriverDeath = 0x7064;		// 'pd'

// request for additional UDP link
const guint16		MsgUDPLinkRequest = 0x6f66;			// 'of'
const guint16		MsgUDPLinkEstablished = 0x6f67;		// 'og'

// server control message
const guint16		MsgServerControl = 0x6f69;			// 'oi'

// lag ping sent by server to client and reply from client
const guint16		MsgLagPing = 0x7069;				// 'pi'

const uint32_t  PingPacketHexPackedSize	= (4 * 8 + 2 * 13);

typedef int ServerCode;

enum GameType {
    TeamFFA,    // normal teamed FFA
    ClassicCTF, // your normal CTF
    OpenFFA,    // teamless FFA
    RabbitChase // hunt the rabbit mode
};
// game styles
enum GameOptions {
    SuperFlagGameStyle =	 0x0002, // superflags allowed
    JumpingGameStyle =	 0x0008, // jumping allowed
    InertiaGameStyle =	 0x0010, // momentum for all
    RicochetGameStyle =	 0x0020, // all shots ricochet
    ShakableGameStyle =	 0x0040, // can drop bad flags
    AntidoteGameStyle =	 0x0080, // anti-bad flags
    HandicapGameStyle =	 0x0100, // handicap players based on score (eek! was TimeSyncGameStyle)
    NoTeamKillsGameStyle = 0x0400
};

enum {
    GotMessage = 0,
    NoMessage,
    Superkilled,
    CommError,
    Disconnect
};

const unsigned char IsRegistered = 1 << 0;
const unsigned char IsVerified   = 1 << 1;
const unsigned char IsAdmin      = 1 << 2;

enum TeamColor {
    AutomaticTeam = -2, NoTeam = -1, RogueTeam = 0, RedTeam = 1,
    GreenTeam = 2, BlueTeam = 3, PurpleTeam = 4, ObserverTeam = 5,
    RabbitTeam = 6, HunterTeam = 7
};

enum {
    LastRealPlayer = 243,
    FirstTeam = 251,
    AdminPlayers = 252,
    ServerPlayer = 253,
    AllPlayers = 254,
    NoPlayer = 255
};

const int MyTeam = (FirstTeam - ObserverTeam);

#endif // _common_h_

