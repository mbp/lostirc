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

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <string>
#include <glib.h>
#include "Socket.h"
#include "Parser.h"
#include "Channel.h"

class ServerConnection : public SigC::Object
{

public:
    ServerConnection(const std::string& host, const std::string& nick, int port = 6667, bool doconnect = false);
    ~ServerConnection();

    void connect(const std::string& host, int port = 6667, const std::string& pass = "");
    void connect();
    void on_error(const char *msg);
    void on_host_resolved();
    void disconnect();
    void addConnectionTimerCheck();
    void addReconnectTimer();
    bool removeReconnectTimer();
    bool sendPong(const std::string& crap);
    bool sendPing(const std::string& crap = "");
    bool sendUser(const std::string& nick, const std::string& localhost, const std::string& remotehost, const std::string& name);
    bool sendNick(const std::string& nick);
    bool sendPass(const std::string& pass);
    bool sendVersion(const std::string& to);
    bool sendMsg(const std::string& to, const std::string& msg);
    bool sendNotice(const std::string& to, const std::string& msg);
    bool sendJoin(const std::string& chan);
    bool sendPart(const std::string& chan, const std::string& msg);
    bool sendNames(const std::string& chan);
    bool sendKick(const std::string& chan, const std::string& nick, const std::string& msg);
    bool sendWhois(const std::string& params);
    bool sendQuit(const std::string& quitmsg = "");
    bool sendMode(const std::string& params);
    bool sendCtcp(const std::string& to, const std::string& params);
    bool sendCtcpNotice(const std::string& to, const std::string& params);
    bool sendTopic(const std::string& chan, const std::string& params);
    bool sendAway(const std::string& params);
    bool sendAdmin(const std::string& params);
    bool sendWhowas(const std::string& params);
    bool sendInvite(const std::string& to, const std::string& params);
    bool sendBanlist(const std::string& chan);
    bool sendMe(const std::string& to, const std::string& msg);
    bool sendWho(const std::string& mask);
    bool sendList(const std::string& params);
    bool sendOper(const std::string& login, const std::string& password);
    bool sendKill(const std::string& nick, const std::string& reason);
    bool sendWallops(const std::string& message);
    bool sendRaw(const std::string& text);

    void addChannel(const std::string& n);
    void addQuery(const std::string &n);
    void removeQuery(const std::string &n);
    bool removeChannel(const std::string& n);
    std::vector<ChannelBase*> findUser(const std::string& n);
    Channel* findChannel(const std::string& c);
    Query* findQuery(const std::string& c);
    void sendCmds();

    const char * getLocalIP() { return _socket->getLocalIP(); }

    static gboolean onReadData(GIOChannel *, GIOCondition, gpointer);
    static gboolean onConnect(GIOChannel *, GIOCondition, gpointer);
    static gboolean autoReconnect(gpointer);
    static gboolean connectionCheck(gpointer);

    // Session struct for all ServerConnections
    struct {
        std::string nick;
        std::string realname;
        std::string password;
        std::string awaymsg;
        bool isConnected;
        bool hasRegistered;
        bool isAway;
        bool endOfMotd;
        int port;
        bool sentLagCheck;
        std::string servername;
        std::string host;
        std::vector<ChannelBase*> channels;
        std::vector<std::string> cmds;
    } Session;

private:
    Socket *_socket;
    Parser *_parser;
    std::string tmpbuf;

    void doCleanup();

    guint _writeid;
    guint _watchid;
    guint _connectioncheckid;
    guint _autoreconnectid;
};

#endif
