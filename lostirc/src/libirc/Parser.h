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

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include "InOut.h"

// This class takes care of parsing incoming messages from the server

class ServerConnection;

using namespace std;

class Parser
{
public:
    Parser(InOut *inout, ServerConnection *conn);

    void parseLine(string &data);

    void Ping(const string& rest);
    void Privmsg(const string& from, const string& param, const string& rest);
    void ServMsg(const string& from, const string& param, const string& rest);
    void Notice(const string& from, const string& param, const string& rest);
    void Notice(const string& msg);
    void Topic(const string& param, const string& rest);
    void Topic(const string& from, const string& param, const string& rest);
    void TopicTime(const string& param);
    void Mode(const string& from, const string& param, const string& rest);
    void CMode(const string& from, const string& param);
    void Join(const string& nick, const string& chan);
    void Part(const string& nick, const string& chan);
    void Quit(const string& nick, const string& msg);
    void Nick(const string& from, const string& to);
    void Kick(const string& from, const string& chan, const string& nickandmsg);
    void Whois(const string& from, const string& param, const string& rest);
    void Names(const string& chan, const string& names);
    void CTCP(const string& from, const string& param, const string& rest);
    void Away(const string& from, const string& param, const string& rest);
    void Errhandler(const string& from, const string& param, const string& rest);
    void Wallops(const string& from, const string& rest);
    void Selfaway(const string& rest);
    void Banlist(const string& param);
    void numeric(int n, const string& from, const string& param, const string& rest);

private:
    string findNick(const string& str);

    ServerConnection *_conn;
    InOut *_io;


};
#endif
