/*
 * Copyright (C) 2002-2004 Morten Brix Pedersen <morten@wtf.dk>
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

#include <glibmm/ustring.h>

// This class takes care of parsing incoming messages from the server

class ServerConnection;

class Parser
{
    ServerConnection *_conn;

public:
    Parser(ServerConnection *conn);

    void parseLine(Glib::ustring &data);

private:
    void Privmsg(const Glib::ustring& from, const Glib::ustring& param, const Glib::ustring& rest);
    void Notice(const Glib::ustring& from, const Glib::ustring& param, const Glib::ustring& rest);
    void Notice(const Glib::ustring& msg);
    void Topic(const Glib::ustring& param, const Glib::ustring& rest);
    void Topic(const Glib::ustring& from, const Glib::ustring& param, const Glib::ustring& rest);
    void TopicTime(const Glib::ustring& param);
    void Mode(const Glib::ustring& from, const Glib::ustring& param, const Glib::ustring& rest);
    void CMode(const Glib::ustring& from, const Glib::ustring& param);
    void Join(const Glib::ustring& nick, const Glib::ustring& param, const Glib::ustring& chan);
    void Part(const Glib::ustring& nick, const Glib::ustring& chan, Glib::ustring& rest);
    void Quit(const Glib::ustring& nick, const Glib::ustring& msg);
    void Nick(const Glib::ustring& from, const Glib::ustring& to);
    void Invite(const Glib::ustring& from, const Glib::ustring& to);
    void Kill(const Glib::ustring& from, const Glib::ustring& to);
    void Kick(const Glib::ustring& from, const Glib::ustring& chan, const Glib::ustring& nickandmsg);
    void Names(const Glib::ustring& chan, const Glib::ustring& names);
    void Ctcp(const Glib::ustring& from, const Glib::ustring& param, const Glib::ustring& rest);
    void Away(const Glib::ustring& param, const Glib::ustring& rest);
    void Wallops(const Glib::ustring& from, const Glib::ustring& rest);
    void Banlist(const Glib::ustring& param);
    void numeric(int n, const Glib::ustring& from, const Glib::ustring& param, const Glib::ustring& rest);
    void parseIRCSupports(const Glib::ustring& supports);

    inline void Ping(const Glib::ustring& rest);

    bool shouldHighlight(const Glib::ustring& str);
};

inline
Glib::ustring findNick(const Glib::ustring& str);

inline
Glib::ustring findHost(const Glib::ustring& str);

inline
Glib::ustring skipFirstWord(const Glib::ustring& str);

Glib::ustring getWord(const Glib::ustring& str, int n);
Glib::ustring stripColors(const Glib::ustring& str, const bool stripBoldAndUnderline);

#endif
