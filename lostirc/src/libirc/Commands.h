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
#include "Events.h"

class ServerConnection;

struct UserCommands {
    char *cmd;
    void (*function)(ServerConnection *, const std::string&);
    int reqConnected;
};

namespace Commands
{

    void send(ServerConnection *conn, std::string cmd, const std::string& params);
    void Join(ServerConnection *conn, const std::string& params);
    void Part(ServerConnection *conn, const std::string& params);
    void Quit(ServerConnection *conn, const std::string& params);
    void Kick(ServerConnection *conn, const std::string& params);
    void Server(ServerConnection *conn, const std::string& params);
    void Disconnect(ServerConnection *conn, const std::string& params);
    void Nick(ServerConnection *conn, const std::string& params);
    void Whois(ServerConnection *conn, const std::string& params);
    void Mode(ServerConnection *conn, const std::string& params);
    void Ctcp(ServerConnection *conn, const std::string& cmd);
    void Away(ServerConnection *conn, const std::string& params);
    void Awayall(ServerConnection *conn, const std::string& params);
    void Names(ServerConnection *conn, const std::string& params);
    void Invite(ServerConnection *conn, const std::string& cmd);
    void Topic(ServerConnection *conn, const std::string& cmd);
    void Banlist(ServerConnection *conn, const std::string& params);
    void Msg(ServerConnection *conn, const std::string& params);
    void Notice(ServerConnection *conn, const std::string& params);
    void Me(ServerConnection *conn, const std::string& params);
    void Who(ServerConnection *conn, const std::string& params);
    void List(ServerConnection *conn, const std::string& params);
    void Set(ServerConnection *conn, const std::string& params);
    void Quote(ServerConnection *conn, const std::string& params);
    void commands(ServerConnection *conn, const std::string& params);
    //    void Exec(ServerConnection *conn, const std::string& params);
    void Oper(ServerConnection* conn, const std::string& params);
    void Kill(ServerConnection* conn, const std::string& params);
    void Wallops(ServerConnection* conn, const std::string& params);
    void DCC(ServerConnection* conn, const std::string& params);
    void Admin(ServerConnection* conn, const std::string& params);
    void Whowas(ServerConnection* conn, const std::string& params);
    void Op(ServerConnection* conn, const std::string& params);
    void Deop(ServerConnection* conn, const std::string& params);
    void Voice(ServerConnection* conn, const std::string& params);
    void Devoice(ServerConnection* conn, const std::string& params);
    bool commandCompletion(const std::string& word, std::string& str);

    std::string assignModes(char sign, char mode, istringstream& ss);

}

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
