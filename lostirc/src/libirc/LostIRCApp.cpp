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

#include "LostIRCApp.h"
#include "ServerConnection.h"
#include "Commands.h"

using Glib::ustring;
using std::vector;

LostIRCApp* App;

LostIRCApp::LostIRCApp(FrontEnd *f)
    : 
#ifdef DEBUG
    log(),
#endif
    init(this), fe(f),
    options("/.lostirc/options.conf"), events("/.lostirc/events.conf"),
    colors("/.lostirc/colors.conf"), cfgservers("/.lostirc/perform.conf")
{
#ifndef WIN32
    uname(&uname_info);
#endif

    ustring realname = Glib::get_real_name();

    // Only read until the first comma
    if (realname.find(",") != ustring::npos)
          realname = realname.substr(0, realname.find(","));

    if (options.nick->empty())
          options.nick = Glib::get_user_name();

    if (options.ircuser->empty())
          options.ircuser = Glib::get_user_name();

    if (options.realname->empty())
          options.realname = realname;
}

LostIRCApp::~LostIRCApp()
{
    vector<ServerConnection*>::iterator i;

    for (i = _servers.begin(); i != _servers.end();) {
        if ((*i)->Session.isConnected) {
            (*i)->sendQuit();
        }
        delete (*i);
        i = _servers.erase(i);
    }
}

void LostIRCApp::autoConnect()
{
    vector<Server*> servers = cfgservers.getServers();
    vector<Server*>::iterator i;

    for (i = servers.begin(); i != servers.end(); ++i) {
        if ((*i)->auto_connect) {
            ServerConnection *conn = newServer(*i);
            conn->connect();
        }
    }
}

ServerConnection* LostIRCApp::newServer(const ustring& host, int port)
{
    ServerConnection *conn = new ServerConnection(host, options.nick, port);
    _servers.push_back(conn);
    return conn;
}

ServerConnection* LostIRCApp::newServer(Server* server)
{
    if (server) {
        ServerConnection *conn = new ServerConnection(server->hostname, options.nick, server->port);
        _servers.push_back(conn);
        conn->Session.cmds = server->cmds;

        if (!server->password.empty())
              conn->Session.password = server->password;

        if (!server->nick.empty())
              conn->Session.nick = server->nick;
        return conn;
    }
    return 0;
}

ServerConnection* LostIRCApp::newServer()
{
    ServerConnection *conn = new ServerConnection("", options.nick);
    _servers.push_back(conn);
    return conn;
}

#ifndef WIN32
struct utsname LostIRCApp::uname_info;
#endif
Glib::ustring LostIRCApp::home;
Glib::ustring LostIRCApp::logdir;
