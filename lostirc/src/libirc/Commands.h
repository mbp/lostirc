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

#include <glibmm/ustring.h>
#include <sstream>
#include <string>
#include <set>
#include <exception>

class ServerConnection;

struct UserCommands {
    const char *cmd;
    void (*function)(ServerConnection *, const Glib::ustring&);
    const bool reqConnected;
};

namespace Commands
{

    void send(ServerConnection *conn, Glib::ustring cmd, const Glib::ustring& params);
    void Join(ServerConnection *conn, const Glib::ustring& params);
    void Part(ServerConnection *conn, const Glib::ustring& params);
    void Quit(ServerConnection *conn, const Glib::ustring& params);
    void Kick(ServerConnection *conn, const Glib::ustring& params);
    void Server(ServerConnection *conn, const Glib::ustring& params);
    void Disconnect(ServerConnection *conn, const Glib::ustring& params);
    void Nick(ServerConnection *conn, const Glib::ustring& params);
    void Whois(ServerConnection *conn, const Glib::ustring& params);
    void Mode(ServerConnection *conn, const Glib::ustring& params);
    void Ctcp(ServerConnection *conn, const Glib::ustring& cmd);
    void Away(ServerConnection *conn, const Glib::ustring& params);
    void Awayall(ServerConnection *conn, const Glib::ustring& params);
    void Names(ServerConnection *conn, const Glib::ustring& params);
    void Invite(ServerConnection *conn, const Glib::ustring& cmd);
    void Topic(ServerConnection *conn, const Glib::ustring& cmd);
    void Banlist(ServerConnection *conn, const Glib::ustring& params);
    void Msg(ServerConnection *conn, const Glib::ustring& params);
    void Notice(ServerConnection *conn, const Glib::ustring& params);
    void Me(ServerConnection *conn, const Glib::ustring& params);
    void Who(ServerConnection *conn, const Glib::ustring& params);
    void List(ServerConnection *conn, const Glib::ustring& params);
    void Set(ServerConnection *conn, const Glib::ustring& params);
    void Quote(ServerConnection *conn, const Glib::ustring& params);
    void Exit(ServerConnection *conn, const Glib::ustring& params);
    //    void Exec(ServerConnection *conn, const Glib::ustring& params);
    void Oper(ServerConnection* conn, const Glib::ustring& params);
    void Kill(ServerConnection* conn, const Glib::ustring& params);
    void Wallops(ServerConnection* conn, const Glib::ustring& params);
    void DCC(ServerConnection* conn, const Glib::ustring& params);
    void Admin(ServerConnection* conn, const Glib::ustring& params);
    void Whowas(ServerConnection* conn, const Glib::ustring& params);
    void Op(ServerConnection* conn, const Glib::ustring& params);
    void Deop(ServerConnection* conn, const Glib::ustring& params);
    void Voice(ServerConnection* conn, const Glib::ustring& params);
    void Devoice(ServerConnection* conn, const Glib::ustring& params);
    void getCommands(std::set<Glib::ustring>& commands);

    Glib::ustring assignModes(char sign, char mode, std::istringstream& ss);

}

class CommandException : public std::exception
{
    const char * error;
public:
    CommandException(const char *e) : error(e) { }
    CommandException(const Glib::ustring &e) : error(e.c_str()) { }
    const char * what() const throw() {
        return error;
    }

};
#endif
