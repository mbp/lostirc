/*
 * Copyright (C) 2001 Morten Brix Pedersen <morten@wtf.dk>
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

#include <string>
#include <glib.h>
#include <unistd.h>
#include "Socket.h"
#include "Parser.h"
#include "InOut.h"

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

class ServerConnection
{

public:
    ServerConnection(InOut *inout, const string& host, int port, const string& nick);
    ServerConnection(InOut *inout, const string& nick, const string& realname);

    bool Connect(const string &host, int port = 6667);
    bool sendPong(const string& crap);
    bool sendUser(const string& nick, const string& localhost, const string& remotehost, const string& name);
    bool sendNick(const string& nick);
    bool sendVersion(const string& to);
    bool sendMsg(const string& to, const string& msg);
    bool sendNotice(const string& to, const string& msg);
    bool sendJoin(const string& chan);
    bool sendPart(const string& chan);
    bool sendKick(const string& nickandmsg);
    bool sendWhois(const string& params);
    bool sendQuit(const string& quitmsg);
    bool sendMode(const string& params);
    bool sendCtcp(const string& to, const string& params);
    bool sendTopic(const string& chan, const string& params);
    bool sendAway(const string& params);
    bool sendInvite(const string& to, const string& params);
    bool sendBanlist(const string& chan);
    bool sendMe(const string& to, const string& msg);

    static gboolean readdata(GIOChannel *, GIOCondition, gpointer);

    // Session struct for all ServerConnections
    struct {
        string nick;
        string realname;
        int isConnected;
        int hasRegistered;
        string servername;
    } Session;

    InOut *_io;

private:
    bool readsocket();
    Socket *_socket;
    Parser *_p;
};


#endif
