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

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <string>
#include <glib.h>
#include <unistd.h>
#include "Socket.h"
#include "Parser.h"
#include "LostIRCApp.h"

class ServerConnection
{

public:
    ServerConnection(LostIRCApp *inout, const std::string& host, int port, const std::string& nick);
    ServerConnection(LostIRCApp *inout, const std::string& nick, const std::string& realname);
    ~ServerConnection();

    bool Connect(const std::string &host, int port = 6667);
    bool sendPong(const std::string& crap);
    bool sendUser(const std::string& nick, const std::string& localhost, const std::string& remotehost, const std::string& name);
    bool sendNick(const std::string& nick);
    bool sendVersion(const std::string& to);
    bool sendMsg(const std::string& to, const std::string& msg);
    bool sendNotice(const std::string& to, const std::string& msg);
    bool sendJoin(const std::string& chan);
    bool sendPart(const std::string& chan);
    bool sendKick(const std::string& nickandmsg);
    bool sendWhois(const std::string& params);
    bool sendQuit(const std::string& quitmsg);
    bool sendMode(const std::string& params);
    bool sendCtcp(const std::string& to, const std::string& params);
    bool sendTopic(const std::string& chan, const std::string& params);
    bool sendAway(const std::string& params);
    bool sendInvite(const std::string& to, const std::string& params);
    bool sendBanlist(const std::string& chan);
    bool sendMe(const std::string& to, const std::string& msg);
    bool sendWho(const std::string& mask);
    bool sendRaw(const std::string& text);

    static gboolean readdata(GIOChannel *, GIOCondition, gpointer);

    // Session struct for all ServerConnections
    struct {
        std::string nick;
        std::string realname;
        int isConnected;
        int hasRegistered;
        std::string servername;
    } Session;

private:
    LostIRCApp *_app;
    bool readsocket();
    Socket *_socket;
    Parser *_p;
};


#endif
