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

#ifndef CONFIGHANDLER_H
#define CONFIGHANDLER_H

#include <string>
#include <map>
#include <vector>
#include "Utils.h"
#include "ConfigValue.h"

// General LostIRC options.
class Options : public baseConfig
{
public:
    Options(const char *filename);

    Value<char> nickcompletion_char;
    Value<int> buffer_size;
    Value<int> window_width;
    Value<int> window_height;
    Value<int> window_x;
    Value<int> window_y;
    Value<Glib::ustring> realname;
    Value<Glib::ustring> nick;
    Value<Glib::ustring> ircuser;
    Value<Glib::ustring> dccip;
    Value<int> dccport;
    Value<Glib::ustring> highlight_words;
    Value<Glib::ustring> font;
    Value<bool> limited_highlighting;
    Value<bool> strip_colors;
    Value<bool> strip_boldandunderline;
};

// Color definitions used by frontend.
class Colors : public baseConfig
{
public:
    Colors(const char *filename);

    Value<Glib::ustring> bgcolor;
    Value<Glib::ustring> color0;
    Value<Glib::ustring> color1;
    Value<Glib::ustring> color2;
    Value<Glib::ustring> color3;
    Value<Glib::ustring> color4;
    Value<Glib::ustring> color5;
    Value<Glib::ustring> color6;
    Value<Glib::ustring> color7;
    Value<Glib::ustring> color8;
    Value<Glib::ustring> color9;
    Value<Glib::ustring> color10;
    Value<Glib::ustring> color11;
    Value<Glib::ustring> color12;
    Value<Glib::ustring> color13;
    Value<Glib::ustring> color14;
    Value<Glib::ustring> color15;
    Value<Glib::ustring> color16;
    Value<Glib::ustring> color17;
    Value<Glib::ustring> color18;
    Value<Glib::ustring> color19;
};

// The different events which can occur.
class Events : public baseConfig
{
public:
    Events(const char *filename);

    Value<Glib::ustring> privmsg;
    Value<Glib::ustring> privmsg_highlight;
    Value<Glib::ustring> action;
    Value<Glib::ustring> action_highlight;
    Value<Glib::ustring> dcc_receive;
    Value<Glib::ustring> servermsg1;
    Value<Glib::ustring> servermsg2;
    Value<Glib::ustring> clientmsg;
    Value<Glib::ustring> ctcp;
    Value<Glib::ustring> ctcp_multi;
    Value<Glib::ustring> ctcp_reply;
    Value<Glib::ustring> topicchange;
    Value<Glib::ustring> topicis;
    Value<Glib::ustring> topictime;
    Value<Glib::ustring> noticepriv;
    Value<Glib::ustring> noticepubl;
    Value<Glib::ustring> error;
    Value<Glib::ustring> away;
    Value<Glib::ustring> banlist;
    Value<Glib::ustring> unknown;
    Value<Glib::ustring> join;
    Value<Glib::ustring> part;
    Value<Glib::ustring> part2;
    Value<Glib::ustring> quit;
    Value<Glib::ustring> quit2;
    Value<Glib::ustring> nick;
    Value<Glib::ustring> mode;
    Value<Glib::ustring> cmode;
    Value<Glib::ustring> wallops;
    Value<Glib::ustring> kicked;
    Value<Glib::ustring> opped;
    Value<Glib::ustring> deopped;
    Value<Glib::ustring> voiced;
    Value<Glib::ustring> devoiced;
    Value<Glib::ustring> halfopped;
    Value<Glib::ustring> halfdeopped;
    Value<Glib::ustring> banned;
    Value<Glib::ustring> unbanned;
    Value<Glib::ustring> invited;
    Value<Glib::ustring> connecting;
    Value<Glib::ustring> names;
    Value<Glib::ustring> killed;
    Value<Glib::ustring> whois_user;
    Value<Glib::ustring> whois_channels;
    Value<Glib::ustring> whois_server;
    Value<Glib::ustring> whois_generic;
};

struct Server {
    Glib::ustring hostname;
    int port;
    Glib::ustring nick;
    Glib::ustring password;
    std::vector<Glib::ustring> cmds;
    bool auto_connect;
};

// The "auto-join servers" configuration interface.
class Servers {
    std::vector<Server*> _servers;

public:
    Servers(const char *file);
    ~Servers();

    void addServer(Server* a) { _servers.push_back(a); }
    void removeServer(Server* a);

    // return "auto-join list"
    const std::vector<Server*>& getServers() { return _servers; }

    bool hasAutoConnects();

    // write server list
    bool writeServersFile();

private:
    bool readServersFile();

    Glib::ustring filename;

};
#endif
