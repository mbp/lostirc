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

#ifndef FRONTEND_H
#define FRONTEND_H

#include <string>
#include <sstream>
#include <vector>
#include "Channel.h"
#include "LostIRCApp.h"

class ServerConnection;
class ChannelBase;

enum Event {
    PRIVMSG = 0, PRIVMSG_HIGHLIGHT, ACTION, ACTION_HIGHLIGHT, DCC_RECEIVE,
    SERVERMSG1, SERVERMSG2, CLIENTMSG, CTCP, CTCP_MULTI, CTCP_REPLY,
    TOPICCHANGE, TOPICIS, TOPICTIME, NOTICEPRIV, NOTICEPUBL, ERROR, AWAY,
    BANLIST, UNKNOWN, JOIN, PART, PART2, QUIT, QUIT2, NICK, MODE, CMODE,
    WALLOPS, KICKED, OPPED, DEOPPED, VOICED, DEVOICED, HALFOPPED,
    HALFDEOPPED, BANNED, UNBANNED, INVITED, CONNECTING, NAMES, KILLED,
    WHOIS_USER, WHOIS_CHANNELS, WHOIS_SERVER, WHOIS_GENERIC
};

namespace FE
{
    class Tmpl
    {
        std::string orig;
        std::vector<std::string> tokens;

    public:
        Tmpl(const std::string& str, const signed p) : orig(str), priority(p) { }

        Tmpl& operator<<(const std::string& str) { tokens.push_back(str); return *this; }
        Tmpl& operator<<(int i) { std::stringstream ss; ss << i; tokens.push_back(ss.str()); return *this; }

        const signed priority;
        std::string result();
    };

    enum Destination {
        CURRENT, ALL
    };

    /* when we want to send a message */
    void emit(Tmpl& t, ChannelBase& chan, ServerConnection *conn);

    /* when we want to send a message to multiple */
    void emit(Tmpl& t, const std::vector<ChannelBase*>& to, ServerConnection *conn);

    /* when we have no destination for our msg (frontend uses current tab) */
    void emit(Tmpl& t, Destination, ServerConnection *conn);

    /* when our message has no specific server (eg. DCC) */
    void emit(Tmpl& t, Destination);

    Tmpl get(Event i);
}

/* abstract base class for frontends. */
class FrontEnd
{
public:
    virtual void displayMessage(const std::string& msg, FE::Destination d, bool shouldHighlight = true) = 0;
    virtual void displayMessage(const std::string& msg, FE::Destination d, ServerConnection *conn, bool shouldHighlight = true) = 0;
    virtual void displayMessage(const std::string& msg, ChannelBase& to, ServerConnection *conn, bool shouldHighlight = true) = 0;
    virtual void highlight(ChannelBase& chan, ServerConnection *conn) = 0;
    virtual void join(const std::string& nick, Channel& chan, ServerConnection *conn) = 0;
    virtual void part(const std::string& nick, Channel& chan, ServerConnection *conn) = 0;
    virtual void kick(const std::string& from, Channel& chan, const std::string& kicker, const std::string& msg,  ServerConnection *conn) = 0;
    virtual void quit(const std::string& nick, std::vector<ChannelBase*> chans, ServerConnection *conn) = 0;
    virtual void nick(const std::string& from, const std::string& to, std::vector<ChannelBase*> chans, ServerConnection *conn) = 0;
    virtual void names(Channel& c, ServerConnection *conn) = 0;
    virtual void CUMode(const std::string& nick, Channel& chan, const std::vector<User>& users, ServerConnection *conn) = 0;
    virtual void away(bool away, ServerConnection *conn) = 0;
    virtual void connected(ServerConnection *conn) = 0;
    virtual void newTab(ServerConnection *conn) = 0;
    virtual void disconnected(ServerConnection *conn) = 0;
    virtual void newDCC(DCC *dcc) = 0;
    virtual void doneDCC(DCC *dcc) = 0;

};

#endif
