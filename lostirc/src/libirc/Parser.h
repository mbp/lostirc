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

#ifndef PARSER_H
#define PARSER_H

#include <string>

// This class takes care of parsing incoming messages from the server

class ServerConnection;

class Parser
{
    ServerConnection *_conn;

public:
    Parser(ServerConnection *conn);

    void parseLine(std::string &data);

private:
    void Privmsg(const std::string& from, const std::string& param, const std::string& rest);
    void Notice(const std::string& from, const std::string& param, const std::string& rest);
    void Notice(const std::string& msg);
    void Topic(const std::string& param, const std::string& rest);
    void Topic(const std::string& from, const std::string& param, const std::string& rest);
    void TopicTime(const std::string& param);
    void Mode(const std::string& from, const std::string& param, const std::string& rest);
    void CMode(const std::string& from, const std::string& param);
    void Join(const std::string& nick, const std::string& param, const std::string& chan);
    void Part(const std::string& nick, const std::string& chan, std::string& rest);
    void Quit(const std::string& nick, const std::string& msg);
    void Nick(const std::string& from, const std::string& to);
    void Invite(const std::string& from, const std::string& to);
    void Kill(const std::string& from, const std::string& to);
    void Kick(const std::string& from, const std::string& chan, const std::string& nickandmsg);
    void Names(const std::string& chan, const std::string& names);
    void Ctcp(const std::string& from, const std::string& param, const std::string& rest);
    void Away(const std::string& param, const std::string& rest);
    void Wallops(const std::string& from, const std::string& rest);
    void Banlist(const std::string& param);
    void numeric(int n, const std::string& from, const std::string& param, const std::string& rest);

    inline void Ping(const std::string& rest);

    std::string findNick(const std::string& str) {
        return str.substr(0, str.find_first_of("!"));
    }

    std::string findHost(const std::string& str) {
        return str.substr(str.find_first_of("!") + 1);
    }

    bool shouldHighlight(const std::string& str);
};

std::string getWord(const std::string& str, int n);
std::string stripColors(const std::string& str);

#endif
