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
    uname(&uname_info);

    struct passwd *pwentry = getpwuid(getuid());

    if (pwentry != NULL) {

        ustring realname = pwentry->pw_gecos;

        // Only read until the first comma
        if (realname.find(",") != ustring::npos) {
            realname = realname.substr(0, realname.find(","));
        }

        if (options.nick->empty())
              options.nick = pwentry->pw_name;

        if (options.ircuser->empty())
              options.ircuser = pwentry->pw_name;

        if (options.realname->empty())
              options.realname = realname;

    } else {
        if (options.nick->empty())
              options.nick = "Somebody";

        if (options.ircuser->empty())
              options.ircuser = "unknown";

        if (options.realname->empty())
              options.realname = "";

    }
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
            ServerConnection *conn = newServer((*i)->hostname, (*i)->port);
            conn->Session.cmds = (*i)->cmds;
            if (!(*i)->password.empty())
                  conn->Session.password = (*i)->password;
            if (!(*i)->nick.empty())
                  conn->Session.nick = (*i)->nick;
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

ServerConnection* LostIRCApp::newServer()
{
    ServerConnection *conn = new ServerConnection("", options.nick);
    _servers.push_back(conn);
    return conn;
}

struct utsname LostIRCApp::uname_info;
char * LostIRCApp::home;
