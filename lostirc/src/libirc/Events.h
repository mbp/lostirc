/*
 * Copyright (C) 2002 Morten Brix Pedersen <morten@wtf.dk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include <vector>
#include <sstream>
#include "ServerConnection.h"

class ServerConnection;

enum Event {
    PRIVMSG = 0, PRIVMSG_HIGHLIGHT, ACTION, ACTION_HIGHLIGHT, SERVMSG,
    SERVMSG2, CTCP, TOPICCHANGE, TOPICIS, TOPICTIME, NOTICEPRIV, NOTICEPUBL,
    ERROR, AWAY, BANLIST, UNKNOWN, JOIN, PART, QUIT, NICK, MODE, CMODE,
    WALLOPS, KICKED, OPPED, DEOPPED, VOICED, DEVOICED, BANNED, UNBANNED,
    INVITED, CONNECTING, NAMES
};

class Tmpl
{
    std::string orig;
    std::vector<std::string> tokens;

public:
    Tmpl(const std::string& str) : orig(str) { }

    Tmpl& operator<<(const std::string& str) { tokens.push_back(str); return *this; }
    Tmpl& operator<<(int i) { std::stringstream ss; ss << i; tokens.push_back(ss.str()); return *this; }

    std::string result();
};

namespace FE
{

    enum Dest {
        CURRENT, ALL
    };

    /* when we want to send a message to a channel */
    void emit(Tmpl& t, Channel& chan, ServerConnection *conn);

    /* when we want to send a message to multiple channels */
    void emit(Tmpl& t, const std::vector<Channel*>& to, ServerConnection *conn);

    /* when we want to send a message to a query */
    void emit(Tmpl& t, const std::string& nick, ServerConnection *conn);

    /* when we have no destination for our msg (frontend uses current tab) */
    void emit(Tmpl& t, Dest, ServerConnection *conn);

    Tmpl get(Event i);
}


#endif
