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

#include "parser.h"

Parser::Parser()
    : Error(false), ErrorChecking(false), Length(0)
{

}

void Parser::nboUseErrorChecking(bool checking)
{
    ErrorChecking = checking;
}

bool Parser::nboGetBufferError()
{
    return Error;
}

void Parser::nboClearBufferError()
{
    Error = false;
}

void Parser::nboSetBufferLength(unsigned int length)
{
    Length = length;
}

unsigned int Parser::nboGetBufferLength()
{
    return Length;
}

//
// Packers
//
void *Parser::nboPackUByte(void *b, guint8 v)
{
    memcpy(b, &v, sizeof(guint8));
    return ADV(b, guint8);
}

void *Parser::nboPackShort(void *b, gint16 v)
{
    const gint16 x = (gint16) htons(v);
    memcpy(b, &x, sizeof(gint16));
    return ADV(b, gint16);
}

void *Parser::nboPackInt(void *b, gint32 v)
{
    const gint32 x = (gint32) htonl(v);
    memcpy(b, &x, sizeof(gint32));
    return ADV(b, gint32);
}

void *Parser::nboPackUShort(void *b, guint16 v)
{
    const guint16 x = (guint16) htons(v);
    memcpy(b, &x, sizeof(guint16));
    return ADV(b, guint16);
}

void *Parser::nboPackUInt(void *b, guint32 v)
{
    const guint32 x = (guint32) htonl(v);
    memcpy(b, &x, sizeof(guint32));
    return ADV(b, guint32);
}

void *Parser::nboPackFloat(void *b, float v)
{
    dtype t;
    t.a = htonl((guint32)v);
    memcpy(b, &t.b, sizeof(float));

    return ADV(b, float);
}

void *Parser::nboPackVector(void *b, const float *v)
{
    // hope that float is a 4 byte IEEE 754 standard encoding
    guint32 data[3];
    guint32 *pV = (guint32 *) v;
    guint32 *pB = (guint32 *) data;

    *(pB++) = htonl(*(pV++));
    *(pB++) = htonl(*(pV++));
    *pB = htonl(*pV);
    memcpy(b, data, 3 * sizeof(float));
    return (void *) (((char *) b) + 3 * sizeof(float));
}

void *Parser::nboPackString(void *b, const void *m, int len)
{
    if (!m || len == 0) {
        return b;
    }
    memcpy(b, m, len);
    return (void *) ((char *) b + len);
}

void *Parser::nboPackStdString(void* b, const Glib::ustring& str)
{
    guint32 strSize = (guint32)str.size();
    b = nboPackUInt(b, strSize);
    b = nboPackString(b, str.data(), strSize);
    return b;
}

//
// UnPackers
//
void *Parser::nboUnpackUByte(void *b, guint8 * v)
{
    if (ErrorChecking) {
        if (Length < sizeof(guint8)) {
            Error = true;
            *v = 0;
            return b;
        } else {
            Length -= sizeof(guint8);
        }
    }
    memcpy(v, b, sizeof(guint8));
    return ADV(b, guint8);
}

void *Parser::nboUnpackShort(void *b, gint16 * v)
{
    gint16 x;

    if (ErrorChecking) {
        if (Length < sizeof(gint16)) {
            Error = true;
            *v = 0;
            return b;
        } else {
            Length -= sizeof(gint16);
        }
    }

    memcpy(&x, b, sizeof(gint16));
    *v = (gint16) ntohs(x);
    return ADV(b, gint16);
}

void *Parser::nboUnpackInt(void *b, gint32 * v)
{
    gint32 x;

    if (ErrorChecking) {
        if (Length < sizeof(gint32)) {
            Error = true;
            *v = 0;
            return b;
        } else {
            Length -= sizeof(gint32);
        }
    }

    memcpy(&x, b, sizeof(gint32));
    *v = (gint32) ntohl(x);
    return ADV(b, guint32);
}

void *Parser::nboUnpackUShort(void *b, guint16 * v)
{
    guint16 x;

    if (ErrorChecking) {
        if (Length < sizeof(guint16)) {
            Error = true;
            *v = 0;
            return b;
        } else {
            Length -= sizeof(guint16);
        }
    }

    memcpy(&x, b, sizeof(guint16));
    *v = (guint16) ntohs(x);
    return ADV(b, guint16);
}

void *Parser::nboUnpackUInt(void *b, guint32 * v)
{
    guint32 x;
    if (ErrorChecking) {
        if (Length < sizeof(guint32)) {
            Error = true;
            *v = 0;
            return b;
        } else {
            Length -= sizeof(guint32);
        }
    }

    memcpy(&x, b, sizeof(guint32));
    *v = (guint32) ntohl(x);
    return ADV(b, guint32);
}

void *Parser::nboUnpackFloat(void *b, float *v)
{
    guint32 x;

    if (ErrorChecking) {
        if (Length < sizeof(float)) {
            Error = true;
            *v = 0.0f;
            return b;
        } else {
            Length -= sizeof(float);
        }
    }
    // hope that float is a 4 byte IEEE 754 standard encoding
    memcpy(&x, b, sizeof(guint32));
    dtype t;
    t.a = (guint32) ntohl(x);
    *v = t.b;

    return ADV(b, float);
}

void *Parser::nboUnpackVector(void *b, float *v)
{
    guint32 data[3];
    guint32 *pV;
    guint32 *pB;
    if (ErrorChecking) {
        if (Length < sizeof(float[3])) {
            Error = true;
            v[0] = v[1] = v[2] = 0.0f;
            return b;
        } else {
            Length -= sizeof(float[3]);
        }
    }
    // hope that float is a 4 byte IEEE 754 standard encoding
    memcpy(data, b, 3 * sizeof(float));
    pV = (guint32 *) v;
    pB = (guint32 *) data;

    *(pV++) = (guint32) ntohl(*(pB++));
    *(pV++) = (guint32) ntohl(*(pB++));
    *pV = (guint32) ntohl(*pB);
    return (void *) (((char *) b) + 3 * sizeof(float));
}

void *Parser::nboUnpackString(void *b, void *m, int len)
{
    if (!m || len == 0) {
        return b;
    }
    if (ErrorChecking) {
        if (Length < (unsigned int) len) {
            Error = true;
            ((char *) m)[0] = '\0';
            return b;
        } else {
            Length -= len;
        }
    }
    memcpy(m, b, len);
    return (void *) ((char *) b + len);
}

// abbv MUST be at least 3 bytes
void* Parser::unpack_flag(void* buf, guchar *abbv)
{
    abbv[0] = 0;
    abbv[1] = 0;
    abbv[2] = 0;

    buf = nboUnpackUByte(buf, &abbv[0]);
    buf = nboUnpackUByte(buf, &abbv[1]);

    return buf;
}

void* Parser::unpack_address(void* _buf, struct in_addr *addr)
{
    guchar* buf = (guchar*)_buf;
    guint32 hostaddr;

    // FIXME - should actually parse the first byte
    // to see if it's IPv4 or IPv6, But for now let's
    // see if we get anything other than IPv4.
    if (buf[0] != 0x04) {
        std::cout << "*** IP version: 0x detected." << std::hex << buf[0] << std::endl;
    }
    ++buf;
    // everything in in_addr should be stored in network byte order
    memcpy(&hostaddr, buf, sizeof(guint32));
    buf += sizeof(guint32);
    addr->s_addr = (gulong)hostaddr;

    return (void*)buf;
}

void *Parser::nboUnpackStdString(void* b, Glib::ustring& str)
{
    guint32 strSize;
    b = nboUnpackUInt(b, &strSize);
    char* buffer = new char[strSize + 1];
    b = nboUnpackString(b, buffer, strSize);
    buffer[strSize] = 0;
    str = buffer;
    delete[] buffer;
    if (ErrorChecking && Error) {
        str = "";
        return b;
    }
    return b;
}

gint Parser::hex2bin(gchar d)
{
    switch (d) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A':
        case 'a': return 10;
        case 'B':
        case 'b': return 11;
        case 'C':
        case 'c': return 12;
        case 'D':
        case 'd': return 13;
        case 'E':
        case 'e': return 14;
        case 'F':
        case 'f': return 15;
    }
    return 0;
}

gchar Parser::bin2hex(gint d)
{
    static const char* digit = "0123456789abcdef";
    return digit[d];
}

gchar* Parser::unpack_hex16(gchar* buf, guint16 *v)
{
    guint16 d = 0;
    d = (d << 4) | hex2bin(*buf++);
    d = (d << 4) | hex2bin(*buf++);
    d = (d << 4) | hex2bin(*buf++);
    d = (d << 4) | hex2bin(*buf++);
    *v = d;
    return buf;
}

gchar* Parser::unpack_hex8(gchar* buf, guint8 *v)
{
    guint16 d = 0;
    d = (d << 4) | hex2bin(*buf++);
    d = (d << 4) | hex2bin(*buf++);
    *v = (uint8_t)d;
    return buf;
}

gint64 Parser::unpack_game_time(void *buf)
{
    guint32 msb, lsb;
    buf = nboUnpackUInt(buf, &msb);
    buf = nboUnpackUInt(buf, &lsb);
    return ((gint64)msb << 32) + (gint64)lsb;
}

void* Parser::unpack_flag_info(void* buf, flag_info *fi)
{
    buf = unpack_flag(buf, (guchar*)fi->type);
    buf = nboUnpackUShort(buf, &fi->status);
    buf = nboUnpackUShort(buf, &fi->endurance);
    buf = nboUnpackUByte(buf, &fi->owner);
    buf = nboUnpackVector(buf, fi->position);
    buf = nboUnpackVector(buf, fi->launch_pos);
    buf = nboUnpackVector(buf, fi->landing_pos);
    buf = nboUnpackFloat(buf, &fi->flight_time);
    buf = nboUnpackFloat(buf, &fi->flight_end);
    buf = nboUnpackFloat(buf, &fi->initial_velocity);

    return buf;
}

void Parser::unpack_hex(gchar* buf, serverInfoHex *si)
{
    buf = unpack_hex16(buf, &si->gameType);
    buf = unpack_hex16(buf, &si->gameOptions);
    buf = unpack_hex16(buf, &si->maxShots);
    buf = unpack_hex16(buf, &si->shakeWins);
    buf = unpack_hex16(buf, &si->shakeTimeout);
    buf = unpack_hex16(buf, &si->maxPlayerScore);
    buf = unpack_hex16(buf, &si->maxTeamScore);
    buf = unpack_hex16(buf, &si->maxTime);
    buf = unpack_hex8(buf, &si->maxPlayers);
    buf = unpack_hex8(buf, &si->rogueCount);
    buf = unpack_hex8(buf, &si->rogueMax);
    buf = unpack_hex8(buf, &si->redCount);
    buf = unpack_hex8(buf, &si->redMax);
    buf = unpack_hex8(buf, &si->greenCount);
    buf = unpack_hex8(buf, &si->greenMax);
    buf = unpack_hex8(buf, &si->blueCount);
    buf = unpack_hex8(buf, &si->blueMax);
    buf = unpack_hex8(buf, &si->purpleCount);
    buf = unpack_hex8(buf, &si->purpleMax);
    buf = unpack_hex8(buf, &si->observerCount);
    buf = unpack_hex8(buf, &si->observerMax);
}


