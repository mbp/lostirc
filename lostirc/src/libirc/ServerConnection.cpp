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
#include "Commands.h"

using std::string;
using std::vector;

ServerConnection::ServerConnection(LostIRCApp *app, const string& host, const string& nick, int port = 6667, bool connect = false)
    : _app(app), _socket(new Socket()), _p(new Parser(_app,this))
{
    Session.nick = nick;
    Session.servername = host;
    Session.host = host;
    Session.port = port;
    Session.isConnected = false;
    Session.hasRegistered = false;
    Session.isAway = false;

    _app->evtNewTab(this);

    if (connect)
          Connect();
}

ServerConnection::~ServerConnection()
{
    delete _socket;
    delete _p;
}

bool ServerConnection::Connect(const string &host, int port = 6667, const string& pass = "")
{
    Session.servername = host;
    Session.host = host;
    Session.password = pass;
    Session.port = port;

    return Connect();
}

bool ServerConnection::Connect()
{

    _app->getEvts()->emit(_app->getEvts()->get(CONNECTING) << Session.host << Session.port, this);
    
    try {

        _socket->connect(Session.host, Session.port);

    } catch (SocketException &e) {
        Session.isConnected = false;
        _app->getEvts()->emit(_app->getEvts()->get(SERVMSG2) << "Failed connecting:" << e.what(), this);
        return false;
    }

    // we got connected

    Session.isConnected = true;

    // This is a temporary watch to see when we can write, when we can
    // write - we are (hopefully) connected
    g_io_add_watch(g_io_channel_unix_new(_socket->getfd()),
                   GIOCondition (G_IO_OUT),
                   &ServerConnection::write, this);

    // This watch makes sure we read data when it's available
    // FIXME: should this be moved to ServerConnection::write?
    g_io_add_watch(g_io_channel_unix_new(_socket->getfd()),
                   GIOCondition (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
                   &ServerConnection::readdata, this);

    return true;
}

gboolean ServerConnection::auto_reconnect(gpointer data)
{
    ServerConnection& conn = *(static_cast<ServerConnection*>(data));
    conn.Connect();
    return (FALSE);
}

gboolean ServerConnection::readdata(GIOChannel* io_channel, GIOCondition cond, gpointer data)
{
    ServerConnection& conn = *(static_cast<ServerConnection*>(data));

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
                    return (TRUE);
                } else {
                    string tmp = str.substr(lastPos, pos - lastPos);
                    conn._p->parseLine(tmp);
                }
                lastPos = str.find_first_not_of("\n", pos);
                pos = str.find_first_of("\n", lastPos);
            }
        }
        return (TRUE);

    } catch (SocketException &e) {
        conn._app->getEvts()->emit(conn._app->getEvts()->get(SERVMSG) << e.what(), &conn);
        conn.Session.isConnected = false;
        g_timeout_add(2000, &ServerConnection::auto_reconnect, &conn);
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

    if (!conn.Session.password.empty())
          conn.sendPass(conn.Session.password);

    conn.sendNick(conn.Session.nick);
    conn.sendUser(conn.Session.nick, hostname, conn.Session.host, conn.Session.realname);

    return (FALSE);
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

bool ServerConnection::sendPass(const string& pass)
{
    string msg("PASS " + pass + "\r\n");
    return _socket->send(msg);
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

bool ServerConnection::sendRaw(const string& text)
{
    string msg(text + "\r\n");

    return _socket->send(msg);
}

Channel* ServerConnection::addChannel(const string& n)
{
    Channel *c = new Channel(n);
    Session.channels.push_back(c);
    return c;
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

vector<Channel*> ServerConnection::findUser(const string& n)
{
    vector<Channel*> chans;
    vector<Channel*>::iterator i = Session.channels.begin();

    for (;i != Session.channels.end(); ++i) {
        if ((*i)->findUser(n)) {
            chans.push_back(*i);
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
        if (Util::lower(name) == Util::lower(chan))
              return *i;
    }
    return 0;
}

void ServerConnection::sendCmds()
{
    vector<string>::iterator i;
    for (i = Session.cmds.begin(); i != Session.cmds.end(); ++i) {
        string::size_type pos = i->find_first_of(" ");
        string params = i->substr(pos + 1);
        Commands::send(this, i->substr(1, pos - 1), params);
    }
}
