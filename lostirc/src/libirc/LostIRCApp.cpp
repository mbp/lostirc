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
#include "Events.h"
#include "Commands.h"
#include <pwd.h>

using std::string;
using std::vector;

LostIRCApp::LostIRCApp()
    : _cfg()
{
    Commands::app = this;
    uname(&uname_info);

    _evts = new Events(this);

    if (_cfg.getOpt("nick").empty()) {
        _cfg.setOpt("nick", getenv("USER"));
    }

    struct passwd *p = getpwnam(_cfg.getOpt("nick").c_str());
    if (p != NULL)
          realname = p->pw_gecos;
}

LostIRCApp::~LostIRCApp()
{
    delete _evts;
    vector<ServerConnection*>::iterator i;

    for (i = _servers.begin(); i != _servers.end();) {
        if ((*i)->Session.isConnected) {
            (*i)->sendQuit("");
        }
        delete (*i);
        i = _servers.erase(i);
    }
}

int LostIRCApp::start()
{
    vector<struct autoJoin*> servers = _cfg.getServers();
    vector<struct autoJoin*>::iterator i;

    for (i = servers.begin(); i != servers.end(); ++i) {
        ServerConnection *conn = newServer((*i)->hostname, (*i)->port);
        conn->Session.cmds = (*i)->cmds;
        if (!(*i)->password.empty())
              conn->Session.password = (*i)->password;
        if (!(*i)->nick.empty())
              conn->Session.nick = (*i)->nick;
        conn->Connect();
    }
    return servers.size();
}

ServerConnection* LostIRCApp::newServer(const string& host, int port)
{
    ServerConnection *conn = new ServerConnection(this, host, _cfg.getOpt("nick"), port);
    _servers.push_back(conn);
    return conn;
}

ServerConnection* LostIRCApp::newServer()
{
    ServerConnection *conn = new ServerConnection(this, "", _cfg.getOpt("nick"));
    conn->Session.realname = realname;
    _servers.push_back(conn);
    return conn;
}

struct utsname LostIRCApp::getsysinfo()
{
    return uname_info;
}
