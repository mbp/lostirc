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

class Options : public baseConfig
{
public:
    Options(const char *filename);

    Value<char> nickcompletion_char;
    Value<int> buffer_size;
    Value<int> window_width;
    Value<int> window_height;
    Value<std::string> realname;
    Value<std::string> nick;
    Value<std::string> ircuser;
    Value<std::string> dccip;
    Value<std::string> highlight_words;
    Value<std::string> font;
    Value<bool> limited_highlighting;
};

class Events : public baseConfig
{
public:
    Events(const char *filename);

    Value<std::string> privmsg;
    Value<std::string> privmsg_highlight;
    Value<std::string> action;
    Value<std::string> action_highlight;
    Value<std::string> dcc_receive;
    Value<std::string> servmsg;
    Value<std::string> servmsg2;
    Value<std::string> servmsg3;
    Value<std::string> ctcp;
    Value<std::string> ctcp_multi;
    Value<std::string> topicchange;
    Value<std::string> topicis;
    Value<std::string> topictime;
    Value<std::string> noticepriv;
    Value<std::string> noticepubl;
    Value<std::string> error;
    Value<std::string> away;
    Value<std::string> banlist;
    Value<std::string> unknown;
    Value<std::string> join;
    Value<std::string> part;
    Value<std::string> quit;
    Value<std::string> nick;
    Value<std::string> mode;
    Value<std::string> cmode;
    Value<std::string> wallops;
    Value<std::string> kicked;
    Value<std::string> opped;
    Value<std::string> deopped;
    Value<std::string> voiced;
    Value<std::string> devoiced;
    Value<std::string> halfopped;
    Value<std::string> halfdeopped;
    Value<std::string> banned;
    Value<std::string> unbanned;
    Value<std::string> invited;
    Value<std::string> connecting;
    Value<std::string> names;
    Value<std::string> killed;
};

struct autoJoin {
    std::string hostname;
    int port;
    std::string nick;
    std::string password;
    std::vector<std::string> cmds;
};

class Servers {
    std::map<std::string, std::string> _events;
    std::map<std::string, std::string> _options;
    std::vector<struct autoJoin*> _servers;

public:
    Servers();
    ~Servers();

    /* ~/.lostirc/perform.conf */
    void addServer(struct autoJoin* a) { _servers.push_back(a); }
    void removeServer(struct autoJoin* a);

    /* return "auto-join list" */
    const std::vector<struct autoJoin*>& getServers() { return _servers; }

    /* write server list */
    bool writeServers();

    std::string home;

private:
    bool readServers(const std::string& filename);

};
#endif
