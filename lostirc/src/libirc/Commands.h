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

#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <exception>
#include "LostIRCApp.h"
#include "Events.h"

class ServerConnection;

struct UserCommands {
    char *cmd;
    void (*function)(ServerConnection *, const std::string&);
    int reqConnected;
};

class Commands
{
public:
    static void send(ServerConnection *conn, std::string cmd, const std::string& params);

    static void Join(ServerConnection *conn, const std::string& params);
    static void Part(ServerConnection *conn, const std::string& params);
    static void Quit(ServerConnection *conn, const std::string& params);
    static void Kick(ServerConnection *conn, const std::string& params);
    static void Server(ServerConnection *conn, const std::string& params);
    static void Nick(ServerConnection *conn, const std::string& params);
    static void Whois(ServerConnection *conn, const std::string& params);
    static void Mode(ServerConnection *conn, const std::string& params);
    static void Ctcp(ServerConnection *conn, const std::string& cmd);
    static void Away(ServerConnection *conn, const std::string& params);
    static void Names(ServerConnection *conn, const std::string& params);
    static void Invite(ServerConnection *conn, const std::string& cmd);
    static void Topic(ServerConnection *conn, const std::string& cmd);
    static void Banlist(ServerConnection *conn, const std::string& params);
    static void Msg(ServerConnection *conn, const std::string& params);
    static void Notice(ServerConnection *conn, const std::string& params);
    static void Me(ServerConnection *conn, const std::string& params);
    static void Who(ServerConnection *conn, const std::string& params);
    static void List(ServerConnection *conn, const std::string& params);
    static void Set(ServerConnection *conn, const std::string& params);
    static void Quote(ServerConnection *conn, const std::string& params);
    static void commands(ServerConnection *conn, const std::string& params);
//    static void Exec(ServerConnection *conn, const std::string& params);
    static bool commandCompletion(const std::string& word, std::string& str);

    static LostIRCApp *app;
};

class CommandException : public std::exception
{
    const char * error;
public:
    CommandException(const char *e) : error(e) { }
    CommandException(const std::string &e) : error(e.c_str()) { }
    const char * what() const throw() {
        return error;
    }

};
#endif
