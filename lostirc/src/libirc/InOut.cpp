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

#include "InOut.h"
#include "ServerConnection.h"
#include "Parser.h"

using std::string;
using std::vector;

InOut::InOut()
{
    uname(&uname_info);
}

ServerConnection* InOut::newServer(const string& host, int port, const string& nick)
{
    ServerConnection *conn = new ServerConnection(this, host, port, nick);
    _servers.push_back(conn);
    return conn;
}

ServerConnection* InOut::newServer(const string& nick, const string& realname)
{
    ServerConnection *conn = new ServerConnection(this, nick, realname);
    _servers.push_back(conn);
    return conn;
}

struct utsname InOut::getsysinfo()
{   
    return uname_info;
}   

void InOut::quit()
{
    vector<ServerConnection*>::const_iterator i;

    for (i = _servers.begin(); i != _servers.end(); ++i) {
        if ((*i)->Session.isConnected) {
            (*i)->sendQuit("");
        }
        delete (*i);
    }
}
