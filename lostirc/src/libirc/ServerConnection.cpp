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

#include "ServerConnection.h"
#include "LostIRCApp.h"
#include "Events.h"

using std::string;
using std::vector;

ServerConnection::ServerConnection(LostIRCApp *app, const string& host, int port, const string& nick)
    : _app(app), _socket(new Socket()), _p(new Parser(_app,this))
{
    Session.nick = nick;
    Connect(host, port);
}

ServerConnection::ServerConnection(LostIRCApp *app, const string& nick, const string& realname)
    : _app(app), _socket(new Socket()), _p(new Parser(_app,this))
{
    Session.nick = nick;
    Session.realname = realname;
    Session.isConnected = false;
    Session.hasRegistered = false;
    Session.isAway = false;
}

ServerConnection::~ServerConnection()
{
    delete _socket;
    delete _p;
}

bool ServerConnection::Connect(const string &host, int port = 6667)
{
    Session.servername = host;

    if (_socket->connect(host, port)) {
        Session.isConnected = true;

        // This is a temporary watch to see when we can write, when we can
        // write - we are (hopefull) connected
        g_io_add_watch(g_io_channel_unix_new(_socket->getfd()),
                       GIOCondition (G_IO_OUT),
                       &ServerConnection::write, this);

        // This watch makes sure we read data when it's available
        g_io_add_watch(g_io_channel_unix_new(_socket->getfd()),
                       GIOCondition (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
                       &ServerConnection::readdata, this);

        return true;
    } else {
        Session.isConnected = false;
        _app->getEvts()->emitEvent("servmsg", "Failed connecting: " + _socket->error, "", this);
        return false;
    }
}

gboolean ServerConnection::readdata(GIOChannel* io_channel, GIOCondition cond, gpointer data)
{
    ServerConnection& conn = *(static_cast<ServerConnection*>(data));

    if (conn.readsocket()) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

gboolean ServerConnection::write(GIOChannel* io_channel, GIOCondition cond, gpointer data)
{
    // The only purpose of this function is to register us to the server
    // when we are able to write
    ServerConnection& conn = *(static_cast<ServerConnection*>(data));

    char hostchar[256];
    gethostname(hostchar, sizeof(hostchar) - 1);
    string hostname(hostchar);

    conn.sendUser(conn.Session.nick, hostname, conn.Session.servername, conn.Session.realname);
    conn.sendNick(conn.Session.nick);

    std::cout << "write avail" << std::endl;
    return (FALSE);
}

bool ServerConnection::readsocket()
{
    string data = _socket->receive();

    if (_socket->isBlocking) {
        return true;
    } else if (!data.empty()) {
        _p->parseLine(data);
        return true;
    } else {
        _app->getEvts()->emitEvent("servmsg", _socket->error, "", this);
        Session.isConnected = false;
        return false;
    }

}

bool ServerConnection::sendPong(const string& crap)
{
    string msg("PONG :" + crap + "\r\n");

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

bool ServerConnection::sendVersion(const string& to)
{
    struct utsname uname = _app->getsysinfo();
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

bool ServerConnection::sendPart(const string& chan)
{
    string msg("PART " + chan + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendKick(const string& nickandmsg)
{
    string msg("KICK " + nickandmsg + "\r\n");

    return _socket->send(msg);
}

bool ServerConnection::sendWhois(const string& params)
{
    string msg("WHOIS " + params + "\r\n");

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

bool ServerConnection::sendAway(const string& params)
{
    string msg("AWAY :" + params + "\r\n");

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
        msg = "TOPIC " + chan + " " + topic + "\r\n";
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

bool ServerConnection::sendRaw(const string& text)
{
    string msg(text + "\r\n");

    return _socket->send(msg);
}

void ServerConnection::addChannel(const string& n)
{
    Channel *c = new Channel;
    c->setName(n);
    Session.channels.push_back(c);
}

void ServerConnection::removeChannel(const string& n)
{
    vector<Channel*>::iterator i = Session.channels.begin();

    for (;i != Session.channels.end(); ++i) {
        if (n == (*i)->getName()) {
            Session.channels.erase(i);
            return;
        }
    }
}

vector<string> ServerConnection::findUser(const string& n)
{
    vector<string> chans;
    vector<Channel*>::iterator i = Session.channels.begin();

    for (;i != Session.channels.end(); ++i) {
        if ((*i)->findUser(n)) {
            chans.push_back((*i)->getName());
        }
    }
    return chans;
}

Channel* ServerConnection::findChannel(const string& c)
{
    string chan = c;
    vector<Channel*>::iterator i = Session.channels.begin();
    for (;i != Session.channels.end(); ++i) {
        string name = (*i)->getName();
        if (Utils::tolower(name) == Utils::tolower(chan))
              return *i;
    }
    return 0;
}
