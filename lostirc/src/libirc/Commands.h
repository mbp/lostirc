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

#include <string>
#include <sstream>

#ifndef COMMANDS_H
#define COMMANDS_H

using namespace std;

class ServerConnection;

struct UserCommands {
    char *cmd;
    bool (*function)(ServerConnection *, const string&);
    int reqConnected;
};

class Commands
{
public:
    static bool send(ServerConnection *conn, string cmd, const string& params);

    static bool Join(ServerConnection *conn, const string& params);
    static bool Part(ServerConnection *conn, const string& params);
    static bool Quit(ServerConnection *conn, const string& params);
    static bool Kick(ServerConnection *conn, const string& params);
    static bool Server(ServerConnection *conn, const string& params);
    static bool Nick(ServerConnection *conn, const string& params);
    static bool Whois(ServerConnection *conn, const string& params);
    static bool Mode(ServerConnection *conn, const string& params);
    static bool Ctcp(ServerConnection *conn, const string& cmd);
    static bool Away(ServerConnection *conn, const string& params);
    static bool Invite(ServerConnection *conn, const string& cmd);
    static bool Topic(ServerConnection *conn, const string& cmd);
    static bool Banlist(ServerConnection *conn, const string& params);
    static bool Msg(ServerConnection *conn, const string& params);
    static bool Notice(ServerConnection *conn, const string& params);
    static bool Me(ServerConnection *conn, const string& params);

    static string error;

};
#endif
