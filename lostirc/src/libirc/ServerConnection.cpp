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

#include <iostream>
#include <functional>
#include <algorithm>
#include <unistd.h>
#include "ServerConnection.h"
#include "LostIRCApp.h"
#include "Events.h"
#include "Commands.h"

using std::string;
using std::vector;

namespace algo
{

  struct deletePointer : public std::unary_function<ChannelBase*, void>
  {
      deletePointer() { }
      void operator() (ChannelBase* x) {
          delete x;
      }
  };

}

ServerConnection::ServerConnection(const string& host, const string& nick, int port, bool doconnect)
    : _socket(new Socket()), _parser(this), _writeid(0),
    _watchid(0), _connectioncheckid(0), _autoreconnectid(0)
{
    Session.nick = nick;
    Session.servername = host;
    Session.host = host;
    Session.port = port;
    Session.isConnected = false;
    Session.hasRegistered = false;
    Session.isAway = false;
    Session.endOfMotd = false;
    Session.sentLagCheck = false;
    Session.realname = App->getCfg().getOpt("realname");

    _socket->on_host_resolved.connect(SigC::slot(this, &ServerConnection::on_host_resolved));
    _socket->on_error.connect(SigC::slot(this, &ServerConnection::on_error));

    App->fe->newTab(this);

    if (doconnect)
          connect();
}

ServerConnection::~ServerConnection()
{
    delete _socket;
    
    for_each(Session.channels.begin(), Session.channels.end(), algo::deletePointer());
}

void ServerConnection::connect(const string &host, int port, const string& pass)
{
    Session.servername = host;
    Session.host = host;
    Session.password = pass;
    Session.port = port;

    connect();
}

void ServerConnection::doCleanup()
{
    // called when we get disconnected or connect to a new server, need to
    // clean various things up.
    Session.isConnected = false;
    Session.hasRegistered = false;
    Session.isAway = false;
    Session.endOfMotd = false;
    Session.sentLagCheck = false;

    if (_writeid > 0) {
        g_source_remove(_writeid);
        _writeid = 0;
    }
    if (_watchid > 0) {
        g_source_remove(_watchid);
        _watchid = 0;
    }
    if (_connectioncheckid > 0) {
        g_source_remove(_connectioncheckid);
        _connectioncheckid = 0;
    }
    for_each(Session.channels.begin(), Session.channels.end(), algo::deletePointer());
    Session.channels.clear();

    _socket->disconnect();
}

void ServerConnection::disconnect()
{
    doCleanup();

    FE::emit(FE::get(SERVMSG) << "Disconnected.", FE::ALL, this);
    App->fe->disconnected(this);
}

void ServerConnection::connect()
{
    doCleanup();

    FE::emit(FE::get(CONNECTING) << Session.host << Session.port, FE::CURRENT, this);

    _socket->resolvehost(Session.host);
}


void ServerConnection::on_error(const char *msg)
{
    FE::emit(FE::get(SERVMSG2) << "Failed connecting:" << msg, FE::CURRENT, this);
    disconnect();
}

void ServerConnection::on_host_resolved()
{
    FE::emit(FE::get(SERVMSG) << "Resolved host. Connecting..", FE::CURRENT, this);
    try {

        _socket->connect(Session.port);

    } catch (SocketException &e) {
        FE::emit(FE::get(SERVMSG2) << "Failed connecting:" << e.what(), FE::CURRENT, this);
        disconnect();
        return;
    }

    // we got connected

    Session.isConnected = true;

    // This is a temporary watch to see when we can write, when we can
    // write - we are (hopefully) connected
    _writeid = g_io_add_watch(g_io_channel_unix_new(_socket->getfd()),
                   GIOCondition (G_IO_OUT | G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_PRI | G_IO_NVAL),
                   &ServerConnection::onConnect, this);

}

gboolean ServerConnection::autoReconnect(gpointer data)
{
    #ifdef DEBUG
    App->log << "ServerConnection::autoReconnect(): reconnecting." << std::endl;
    #endif
    ServerConnection& conn = *(static_cast<ServerConnection*>(data));
    conn.connect();
    return FALSE;
}

gboolean ServerConnection::connectionCheck(gpointer data)
{
    ServerConnection& conn = *(static_cast<ServerConnection*>(data));

    if (conn.Session.sentLagCheck) {
        // disconnected! last lag check was never replied to
        #ifdef DEBUG
        App->log << "ServerConnection::connectionCheck(): disconnected." << std::endl;
        #endif
        conn.disconnect();
        conn.connect();

        return FALSE;
    } else {
        #ifdef DEBUG
        App->log << "ServerConnection::connectionCheck(): still on" << std::endl;
        #endif
        conn.sendPing();
        conn.Session.sentLagCheck = true;
        return TRUE;
    }
}

gboolean ServerConnection::onReadData(GIOChannel* io_channel, GIOCondition cond, gpointer data)
{
    ServerConnection& conn = *(static_cast<ServerConnection*>(data));
    #ifdef DEBUG
    App->log << "ServerConnection::onReadData(): reading.." << std::endl;
    #endif

    try {

        char buf[4096];
        if (conn._socket->receive(buf, 4095)) {

            string str;
            if (!conn.tmpbuf.empty()) {
                str = conn.tmpbuf;
                conn.tmpbuf = "";
            }
            str += buf;

            // run through the string we got, take it line by line and pass
            // each line to parseLine() - if we did not reach the end, save
            // the rest in a tmp buffer.

            string::size_type lastPos = str.find_first_not_of("\n", 0);
            string::size_type pos     = str.find_first_of("\n", lastPos);

            while (string::npos != pos || string::npos != lastPos)
            {
                if (pos == string::npos && lastPos != str.length()) {
                    // we reached the last line, but it was not ended with
                    // \n, lets buffer this
                    conn.tmpbuf = str.substr(lastPos);
                    return TRUE;
                } else {
                    string tmp = str.substr(lastPos, pos - lastPos);
                    conn._parser.parseLine(tmp);
                }
                lastPos = str.find_first_not_of("\n", pos);
                pos = str.find_first_of("\n", lastPos);
            }
        }
        return TRUE;

    } catch (SocketException &e) {
        FE::emit(FE::get(SERVMSG3) << "Failed to receive" << e.what(), FE::ALL, &conn);
        conn.disconnect();
        conn.addReconnectTimer();
        return FALSE;
    } catch (SocketDisconnected &e) {
        conn.disconnect();
        conn.addReconnectTimer();
        return FALSE;
    }
}

gboolean ServerConnection::onConnect(GIOChannel* io_channel, GIOCondition cond, gpointer data)
{
    // The only purpose of this function is to register us to the server
    // when we are able to write
    ServerConnection& conn = *(static_cast<ServerConnection*>(data));
    FE::emit(FE::get(SERVMSG) << "Connected. Logging in...", FE::CURRENT, &conn);

    if (cond & G_IO_OUT) {
        char hostname[256];
        gethostname(hostname, sizeof(hostname) - 1);

        if (!conn.Session.password.empty())
              conn.sendPass(conn.Session.password);

        conn.sendNick(conn.Session.nick);
        conn.sendUser(App->getCfg().getOpt("ircuser"), hostname, conn.Session.host, conn.Session.realname);
    }

    // Watch for incoming data from now on
    conn._watchid = g_io_add_watch(g_io_channel_unix_new(conn._socket->getfd()),
                   GIOCondition (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
                   &ServerConnection::onReadData, &conn);

    return FALSE;
}

void ServerConnection::addConnectionTimerCheck()
{
    _connectioncheckid = g_timeout_add(30000, &ServerConnection::connectionCheck, this);
}

void ServerConnection::addReconnectTimer()
{
    if (_autoreconnectid == 0) {
        _autoreconnectid = g_timeout_add(2000, &ServerConnection::autoReconnect, this);
    }
}

bool ServerConnection::removeReconnectTimer()
{
    if (_autoreconnectid > 0) {
        g_source_remove(_autoreconnectid);
        _autoreconnectid = 0;
        return true;
    }
    return false;
}


bool ServerConnection::sendPong(const string& crap)
{
    string msg("PONG :" + crap + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendPing(const string& crap)
{
    string msg("PING LAG" + crap + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendUser(const string& nick, const string& localhost, const string& remotehost, const string& name)
{
    string msg("USER " + nick + " " + localhost + " " + remotehost + " : " + name + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendNick(const string& nick)
{
    if (Session.isConnected && Session.hasRegistered) {
        string msg("NICK " + nick + "\r\n");

        return _socket->send(msg);
    } else if (Session.isConnected && !Session.hasRegistered) {
        Session.nick = nick;
        string msg("NICK " + nick + "\r\n");

        return _socket->send(msg);
    } else {
        Session.nick = nick;
        return false;
    }
}

bool ServerConnection::sendPass(const string& pass)
{
    string msg("PASS " + pass + "\r\n");
    return _socket->send(msg);
}

bool ServerConnection::sendVersion(const string& to)
{
    struct utsname uname = App->getsysinfo();
    string s(uname.sysname);
    string r(uname.release);
    string m(uname.machine);
    string vstring("LostIRC "VERSION" on " + s + " " + r + " [" + m + "]");
    string msg("NOTICE " + to + " :\001VERSION " + vstring + "\001\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendMsg(const string& to, const string& message)
{
    string msg("PRIVMSG " + to + " :" + message + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendNotice(const string& to, const string& message)
{
    string msg("NOTICE " + to + " :" + message + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendJoin(const string& chan)
{
    string msg("JOIN " + chan + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendPart(const string& chan, const string& message)
{
    string msg;
    if (!message.empty()) {
        msg = "PART " + chan + " :" + message + "\r\n";
    } else {
        msg = "PART " + chan + "\r\n";
    }

    return _socket->send(msg);
}

bool ServerConnection::sendKick(const string& chan, const string& nick, const string& kickmsg)
{
    string msg("KICK " + chan + " " + nick + " :" + kickmsg + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendWhois(const string& params)
{
    string msg("WHOIS " + params + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendList(const string& params)
{
    string msg("LIST " + params + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendQuit(const string& quitmsg)
{
    if (Session.isConnected) {
        string msg;
        if (quitmsg.size() < 1) {
            msg = "QUIT\r\n";
        } else {
            msg = "QUIT :" + quitmsg + "\r\n";
        }

        return _socket->send(msg);
    }
    return true;
}

bool ServerConnection::sendMode(const string& params)
{
    string msg("MODE " + params + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendCtcp(const string& to, const string& params)
{
    string msg("PRIVMSG " + to + " :\001" + params + "\001\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendCtcpNotice(const string& to, const string& params)
{
    string msg("NOTICE " + to + " :\001" + params + "\001\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendAway(const string& params)
{
    string msg("AWAY :" + params + "\r\n");
    Session.awaymsg = params;

    return _socket->send(msg);
}

bool ServerConnection::sendAdmin(const string& params)
{
    string msg("ADMIN " + params + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendWhowas(const string& params)
{
    string msg("WHOWAS " + params + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendInvite(const string& to, const string& params)
{
    string msg("INVITE " + to + " " + params + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendTopic(const string& chan, const string& topic)
{
    string msg;
    if (topic.empty()) {
        msg = "TOPIC " + chan + "\r\n";
    } else {
        msg = "TOPIC " + chan + " :" + topic + "\r\n";
    }

    return _socket->send(msg);
}

bool ServerConnection::sendBanlist(const string& chan)
{
    string msg("MODE " + chan + " +b\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendMe(const string& to, const string& message)
{
    string msg("PRIVMSG " + to + " :\001ACTION " + message + "\001\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendWho(const string& mask)
{
    string msg("WHO " + mask + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendNames(const string& chan)
{
    string msg("NAMES " + chan + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendOper(const string& login, const string& password)
{
    string msg("OPER " + login + ' ' + password + "\r\n" );

    return _socket->send(msg);
}

bool ServerConnection::sendWallops(const string& message)
{
    string msg("WALLOPS :" + message + "\r\n" );

    return _socket->send(msg);
}

bool ServerConnection::sendKill(const string& nick, const string& reason)
{
    string msg("KILL " + nick + " :" + reason + "\r\n" );

    return _socket->send(msg);
}

bool ServerConnection::sendRaw(const string& text)
{
    string msg(text + "\r\n");

    return _socket->send(msg);
}

void ServerConnection::addChannel(const string& n)
{
    Channel *c = new Channel(n);
    Session.channels.push_back(c);
}

void ServerConnection::addQuery(const string& n)
{
    Query *c = new Query(n);
    Session.channels.push_back(c);
}

bool ServerConnection::removeChannel(const string& n)
{
    vector<ChannelBase*>::iterator i = Session.channels.begin();

    string chan = Util::lower(n);
    for (;i != Session.channels.end(); ++i) {
        string name = Util::lower((*i)->getName());
        if (name == chan) {
            Session.channels.erase(i);
            return true;
        }
    }
    return false;
}


vector<ChannelBase*> ServerConnection::findUser(const string& n)
{
    vector<ChannelBase*> chans;
    vector<ChannelBase*>::iterator i = Session.channels.begin();

    for (;i != Session.channels.end(); ++i) {
        if ((*i)->findUser(n)) {
            chans.push_back(*i);
        }
    }
    return chans;
}

Channel* ServerConnection::findChannel(const string& c)
{
    string chan = Util::lower(c);
    vector<ChannelBase*>::iterator i = Session.channels.begin();
    for (;i != Session.channels.end(); ++i) {
        string name = Util::lower((*i)->getName());
        if (name == chan)
              return dynamic_cast<Channel*>(*i);
    }
    return 0;
}

Query* ServerConnection::findQuery(const string& c)
{
    string chan = Util::lower(c);
    vector<ChannelBase*>::iterator i = Session.channels.begin();
    for (;i != Session.channels.end(); ++i) {
        string name = Util::lower((*i)->getName());
        if (name == chan)
              return dynamic_cast<Query*>(*i);
    }
    return 0;
}

void ServerConnection::sendCmds()
{
    vector<string>::iterator i;
    for (i = Session.cmds.begin(); i != Session.cmds.end(); ++i) {
        if (i->at(0) == '/') {

            string::size_type pos = i->find_first_of(" ");

            string params;
            if (pos != string::npos) {
                params = i->substr(pos + 1);
            }

            try {

                Commands::send(this, Util::upper(i->substr(1, pos - 1)), params);

            } catch (CommandException& ce) {

                FE::emit(FE::get(SERVMSG) << ce.what(), FE::CURRENT, this);
            }
        }
    }
}
