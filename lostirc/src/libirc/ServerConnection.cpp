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
    : _parser(this)
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
    Session.realname = App->options.realname;

    _socket.on_host_resolved.connect(SigC::slot(*this, &ServerConnection::on_host_resolved));
    _socket.on_error.connect(SigC::slot(*this, &ServerConnection::on_error));

    App->fe->newTab(this);

    if (doconnect)
          connect();
}

ServerConnection::~ServerConnection()
{
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

    if (signal_write.connected())
          signal_write.disconnect();

    if (signal_watch.connected())
          signal_watch.disconnect();

    if (signal_connection.connected())
          signal_connection.disconnect();

    for_each(Session.channels.begin(), Session.channels.end(), algo::deletePointer());
    Session.channels.clear();

    _socket.disconnect();
}

void ServerConnection::disconnect()
{
    #ifdef DEBUG
    App->log << "ServerConnection::disconnect()" << std::endl;
    #endif

    doCleanup();

    FE::emit(FE::get(CLIENTMSG) << "Disconnected.", FE::ALL, this);
    App->fe->disconnected(this);
}

void ServerConnection::connect()
{
    #ifdef DEBUG
    App->log << "ServerConnection::connect()" << std::endl;
    #endif

    doCleanup();

    FE::emit(FE::get(CONNECTING) << Session.host << Session.port, FE::CURRENT, this);

    _socket.resolvehost(Session.host);
}


void ServerConnection::on_error(const char *msg)
{
    FE::emit(FE::get(ERROR) << string("Failed connecting: ") + msg, FE::CURRENT, this);
    disconnect();
}

void ServerConnection::on_host_resolved()
{
    FE::emit(FE::get(CLIENTMSG) << "Resolved host. Connecting..", FE::CURRENT, this);
    try {

        _socket.connect(Session.port);

    } catch (SocketException &e) {
        FE::emit(FE::get(ERROR) << string("Failed connecting:") + e.what(), FE::CURRENT, this);
        disconnect();
        return;
    }

    // we got connected

    Session.isConnected = true;
    App->fe->connected(this);

    // This is a temporary watch to see when we can write, when we can
    // write - we are (hopefully) connected
    signal_write = Glib::signal_io().connect(
            SigC::slot(*this, &ServerConnection::onConnect),
            _socket.getfd(),
            Glib::IO_OUT | Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_PRI | Glib::IO_NVAL);
}

bool ServerConnection::autoReconnect()
{
    #ifdef DEBUG
    App->log << "ServerConnection::autoReconnect(): reconnecting." << std::endl;
    #endif
    connect();
    return false;
}

bool ServerConnection::connectionCheck()
{
    if (Session.sentLagCheck) {
        // disconnected! last lag check was never replied to
        #ifdef DEBUG
        App->log << "ServerConnection::connectionCheck(): disconnected." << std::endl;
        #endif
        disconnect();
        connect();

        return false;
    } else {
        #ifdef DEBUG
        App->log << "ServerConnection::connectionCheck(): still on" << std::endl;
        #endif
        sendPing();
        Session.sentLagCheck = true;
        return true;
    }
}

bool ServerConnection::onReadData(Glib::IOCondition)
{
    #ifdef DEBUG
    App->log << "Serverconnection::onReadData(): reading.." << std::endl;
    #endif

    try {

        char buf[4096];
        if (_socket.receive(buf, 4095)) {

            string str;
            if (!tmpbuf.empty()) {
                str = tmpbuf;
                tmpbuf = "";
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
                    tmpbuf = str.substr(lastPos);
                    return true;
                } else {
                    string tmp = str.substr(lastPos, pos - lastPos);
                    _parser.parseLine(tmp);
                }
                lastPos = str.find_first_not_of("\n", pos);
                pos = str.find_first_of("\n", lastPos);
            }
        }
        return true;

    } catch (SocketException &e) {
        FE::emit(FE::get(ERROR) << string("Failed to receive") + e.what(), FE::ALL, this);
        disconnect();
        addReconnectTimer();
        return false;
    } catch (SocketDisconnected &e) {
        disconnect();
        addReconnectTimer();
        return false;
    }
}

bool ServerConnection::onConnect(Glib::IOCondition cond)
{
    // The only purpose of this function is to register us to the server
    // when we are able to write
    FE::emit(FE::get(CLIENTMSG) << "Connected. Logging in...", FE::CURRENT, this);

    if (cond & Glib::IO_OUT) {
        char hostname[256];
        gethostname(hostname, sizeof(hostname) - 1);

        if (!Session.password.empty())
              sendPass(Session.password);

        sendNick(Session.nick);
        sendUser(App->options.ircuser, hostname, Session.host, Session.realname);
    }

    // Watch for incoming data from now on
    signal_watch = Glib::signal_io().connect(
            SigC::slot(*this, &ServerConnection::onReadData),
            _socket.getfd(),
            Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);

    return false;
}

void ServerConnection::addConnectionTimerCheck()
{
    signal_connection = Glib::signal_timeout().connect(
            SigC::slot(*this, &ServerConnection::connectionCheck),
            30000);
}

void ServerConnection::addReconnectTimer()
{
    if (!signal_autoreconnect.connected())
          signal_autoreconnect = Glib::signal_timeout().connect(
                  SigC::slot(*this, &ServerConnection::autoReconnect),
                  2000);
}

void ServerConnection::removeReconnectTimer()
{
    if (signal_autoreconnect.connected())
          signal_autoreconnect.disconnect();
}


bool ServerConnection::sendPong(const string& crap)
{
    string msg("PONG :" + crap + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendPing(const string& crap)
{
    string msg("PING LAG" + crap + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendUser(const string& nick, const string& localhost, const string& remotehost, const string& name)
{
    string msg("USER " + nick + " " + localhost + " " + remotehost + " :" + name + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendNick(const string& nick)
{
    if (Session.isConnected && Session.hasRegistered) {
        string msg("NICK " + nick + "\r\n");

        return _socket.send(msg);
    } else if (Session.isConnected && !Session.hasRegistered) {
        Session.nick = nick;
        string msg("NICK " + nick + "\r\n");

        return _socket.send(msg);
    } else {
        Session.nick = nick;
        return false;
    }
}

bool ServerConnection::sendPass(const string& pass)
{
    string msg("PASS " + pass + "\r\n");
    return _socket.send(msg);
}

bool ServerConnection::sendVersion(const string& to)
{
    string s(LostIRCApp::uname_info.sysname);
    string r(LostIRCApp::uname_info.release);
    string m(LostIRCApp::uname_info.machine);
    string vstring("LostIRC "VERSION" on " + s + " " + r + " [" + m + "]");
    string msg("NOTICE " + to + " :\001VERSION " + vstring + "\001\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendMsg(const string& to, const string& message)
{
    string msg("PRIVMSG " + to + " :" + message + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendNotice(const string& to, const string& message)
{
    string msg("NOTICE " + to + " :" + message + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendJoin(const string& chan)
{
    string msg("JOIN " + chan + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendPart(const string& chan, const string& message)
{
    string msg;
    if (!message.empty()) {
        msg = "PART " + chan + " :" + message + "\r\n";
    } else {
        msg = "PART " + chan + "\r\n";
    }

    return _socket.send(msg);
}

bool ServerConnection::sendKick(const string& chan, const string& nick, const string& kickmsg)
{
    string msg("KICK " + chan + " " + nick + " :" + kickmsg + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendWhois(const string& params)
{
    string msg("WHOIS " + params + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendList(const string& params)
{
    string msg("LIST " + params + "\r\n");

    return _socket.send(msg);
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

        return _socket.send(msg);
    }
    return true;
}

bool ServerConnection::sendMode(const string& params)
{
    string msg("MODE " + params + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendCtcp(const string& to, const string& params)
{
    string msg("PRIVMSG " + to + " :\001" + params + "\001\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendCtcpNotice(const string& to, const string& params)
{
    string msg("NOTICE " + to + " :\001" + params + "\001\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendAway(const string& params)
{
    string msg("AWAY :" + params + "\r\n");
    Session.awaymsg = params;

    return _socket.send(msg);
}

bool ServerConnection::sendAdmin(const string& params)
{
    string msg("ADMIN " + params + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendWhowas(const string& params)
{
    string msg("WHOWAS " + params + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendInvite(const string& to, const string& params)
{
    string msg("INVITE " + to + " " + params + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendTopic(const string& chan, const string& topic)
{
    string msg;
    if (topic.empty()) {
        msg = "TOPIC " + chan + "\r\n";
    } else {
        msg = "TOPIC " + chan + " :" + topic + "\r\n";
    }

    return _socket.send(msg);
}

bool ServerConnection::sendBanlist(const string& chan)
{
    string msg("MODE " + chan + " +b\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendMe(const string& to, const string& message)
{
    string msg("PRIVMSG " + to + " :\001ACTION " + message + "\001\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendWho(const string& mask)
{
    string msg("WHO " + mask + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendNames(const string& chan)
{
    string msg("NAMES " + chan + "\r\n");

    return _socket.send(msg);
}

bool ServerConnection::sendOper(const string& login, const string& password)
{
    string msg("OPER " + login + ' ' + password + "\r\n" );

    return _socket.send(msg);
}

bool ServerConnection::sendWallops(const string& message)
{
    string msg("WALLOPS :" + message + "\r\n" );

    return _socket.send(msg);
}

bool ServerConnection::sendKill(const string& nick, const string& reason)
{
    string msg("KILL " + nick + " :" + reason + "\r\n" );

    return _socket.send(msg);
}

bool ServerConnection::sendRaw(const string& text)
{
    string msg(text + "\r\n");

    return _socket.send(msg);
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

                FE::emit(FE::get(CLIENTMSG) << ce.what(), FE::CURRENT, this);
            }
        }
    }
}
