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

#include "ServerConnection.h"
#include "InOut.h"

ServerConnection::ServerConnection(InOut *inout, const string& host, int port, const string& nick)
    : _io(inout)
{
    Session.nick = nick;
    Connect(host, port);
}

ServerConnection::ServerConnection(InOut *inout, const string& nick, const string& realname)
    : _io(inout)
{
    Session.nick = nick;
    Session.realname = realname;
    Session.isConnected = 0;
    Session.hasRegistered = 0;

}

bool ServerConnection::Connect(const string &host, int port = 6667)
{
    Session.servername = host;
    _socket = new Socket();
    _p = new Parser(_io, this);

    if (_socket->connect(host, port)) {
        Session.isConnected = 1;

        /* Add a watch on our new server connection's file descriptor */
        g_io_add_watch(g_io_channel_unix_new(_socket->getfd()),
                       GIOCondition (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
                       &ServerConnection::readdata, this);

        char hostchar[256];
        gethostname(hostchar, sizeof(hostchar) - 1);
        string hostname(hostchar);
                       
        sendUser(Session.nick, hostname, Session.servername, Session.realname);
        sendNick(Session.nick);

        //_socket->setNonBlocking();

    } else {
        Session.isConnected = 0;
        _io->evtGenericError("Failed connecting: " + _socket->error, this);
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

bool ServerConnection::readsocket()
{
    string data;
    if (_socket->receive(data)) {
        if (!data.empty()) {
            _p->parseLine(data);
            return true;
        }
    } else {
        _io->evtGenericError(_socket->error, this);
        Session.isConnected = 0;
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
    string msg("USER " + nick + " " + localhost + " " + remotehost + " :" + name + "\r\n");

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
    struct utsname uname = _io->getsysinfo();
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
