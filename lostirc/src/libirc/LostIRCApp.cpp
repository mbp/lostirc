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
#include <pwd.h>

using std::string;
using std::vector;

LostIRCApp::LostIRCApp()
{
    _evts = new Events(this);
    uname(&uname_info);
    if (!_cfg.readConfig())
          std::cerr << "Failed reading config file ~/.lostircrc" << std::endl;

    nick = getenv("USER");
    struct passwd *p = getpwnam(nick.c_str());
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

ServerConnection* LostIRCApp::newServer(const string& host, int port)
{
    ServerConnection *conn = new ServerConnection(this, host, port, nick);
    _servers.push_back(conn);
    return conn;
}

ServerConnection* LostIRCApp::newServer()
{
    ServerConnection *conn = new ServerConnection(this, nick, realname);
    _servers.push_back(conn);
    return conn;
}

struct utsname LostIRCApp::getsysinfo()
{
    return uname_info;
}
