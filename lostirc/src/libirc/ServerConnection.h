/*
 * Copyright (C) 2002-2004 Morten Brix Pedersen <morten@wtf.dk>
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
#include <glibmm/main.h>
#include <glibmm/ustring.h>
#include "Socket.h"
#include "Parser.h"
#include "Channel.h"

class ServerConnection : public sigc::trackable
{

public:
    ServerConnection(const Glib::ustring& host, const Glib::ustring& nick, int port = 6667, bool doconnect = false);
    ~ServerConnection();

    void connect(const Glib::ustring& host, int port = 6667, const Glib::ustring& pass = "");
    void connect();
    void reconnect();
    void on_error(const char *msg);
    void on_host_resolved();
    void on_connected(Glib::IOCondition);
    void disconnect();
    void addConnectionTimerCheck();
    void addReconnectTimer();
    void removeReconnectTimer();
    bool sendPong(const Glib::ustring& crap);
    bool sendPing(const Glib::ustring& crap = "");
    bool sendUser(const Glib::ustring& nick, const Glib::ustring& localhost, const Glib::ustring& remotehost, const Glib::ustring& name);
    bool sendNick(const Glib::ustring& nick);
    bool sendPass(const Glib::ustring& pass);
    bool sendVersion(const Glib::ustring& to);
    bool sendMsg(const Glib::ustring& to, const Glib::ustring& msg, bool sendToGui = true);
    bool sendNotice(const Glib::ustring& to, const Glib::ustring& msg);
    bool sendJoin(const Glib::ustring& chan);
    bool sendPart(const Glib::ustring& chan, const Glib::ustring& msg);
    bool sendNames(const Glib::ustring& chan);
    bool sendKick(const Glib::ustring& chan, const Glib::ustring& nick, const Glib::ustring& msg);
    bool sendWhois(const Glib::ustring& params);
    bool sendQuit(const Glib::ustring& quitmsg = "");
    bool sendMode(const Glib::ustring& params);
    bool sendCtcp(const Glib::ustring& to, const Glib::ustring& params);
    bool sendCtcpNotice(const Glib::ustring& to, const Glib::ustring& params);
    bool sendTopic(const Glib::ustring& chan, const Glib::ustring& params);
    bool sendAway(const Glib::ustring& params);
    bool sendAdmin(const Glib::ustring& params);
    bool sendWhowas(const Glib::ustring& params);
    bool sendInvite(const Glib::ustring& to, const Glib::ustring& params);
    bool sendBanlist(const Glib::ustring& chan);
    bool sendMe(const Glib::ustring& to, const Glib::ustring& msg);
    bool sendWho(const Glib::ustring& mask);
    bool sendList(const Glib::ustring& params);
    bool sendOper(const Glib::ustring& login, const Glib::ustring& password);
    bool sendKill(const Glib::ustring& nick, const Glib::ustring& reason);
    bool sendWallops(const Glib::ustring& message);
    bool sendRaw(const Glib::ustring& text);

    void addChannel(const Glib::ustring& n);
    void addQuery(const Glib::ustring &n);
    void removeQuery(const Glib::ustring &n);
    bool removeChannel(const Glib::ustring& n);
    std::vector<ChannelBase*> findUser(const Glib::ustring& n);
    Channel* findChannel(const Glib::ustring& c);
    Query* findQuery(const Glib::ustring& c);
    void sendCmds();

    const char * getLocalIP() { return _socket.getLocalIP(); }

    void onReadData();
    bool autoReconnect();
    bool connectionCheck();

    // Session struct for all ServerConnections
    struct Sessioninfo {
        Glib::ustring nick;
        Glib::ustring realname;
        Glib::ustring password;
        Glib::ustring awaymsg;
        bool isConnected;
        bool isConnecting;
        bool hasRegistered;
        bool isAway;
        bool endOfMotd;
        int port;
        bool sentLagCheck;
        Glib::ustring servername;
        Glib::ustring host;
        std::vector<ChannelBase*> channels;
        std::vector<Glib::ustring> cmds;
    } Session;

    struct Supports {
        Supports() : chantypes("#&"), prefix("(ov)@+"),
                     chanmodes("b,k,l,imnpstru") { }
        Glib::ustring chantypes;
        Glib::ustring prefix;
        Glib::ustring chanmodes;
        Glib::ustring network;
    } supports;

private:
    Socket _socket;
    Parser _parser;
    char _tmpbuf[520];
    int _bufpos;

    void doCleanup();

    sigc::connection signal_connection;
    sigc::connection signal_autoreconnect;

    // Copy-ctr private to avoid copying.
    ServerConnection(const ServerConnection&);
    // operator= - ditto.
    ServerConnection& operator=(const ServerConnection&);
};

#endif
