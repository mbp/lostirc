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

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include "ConfigHandler.h"
#include "LostIRCApp.h"
#include "Events.h"

using std::string;
using std::cout;
using std::vector;
using std::map;

Servers::Servers()
{
    home = App->home;

    readServers(home + "/.lostirc/perform.conf");
}

Servers::~Servers()
{
    vector<struct autoJoin*>::iterator i;

    for (i = _servers.begin(); i != _servers.end();) {
        delete (*i);
        i = _servers.erase(i);
    }
}

bool Servers::readServers(const string& filename)
{
    std::ifstream in(filename.c_str());

    if (!in)
          return false;

    vector<string> tmpcmds;
    string server, tmp, nick, password;
    int port = 0;
    while (getline(in, tmp)) {
        string::size_type pos1 = tmp.find_first_of("=");
        string param;
        string value;

        if (pos1 != string::npos) {
            param = tmp.substr(0, pos1 - 1);
            value = tmp.substr(pos1 + 2);

            if (param == "hostname") {
                if (!server.empty()) {
                    struct autoJoin *j = new autoJoin;
                    j->hostname = server;
                    j->port = port ? port : 6667;
                    j->password = password;
                    j->cmds = tmpcmds;
                    j->nick = nick;

                    port = 0;
                    password = "";
                    nick = "";

                    _servers.push_back(j);
                    tmpcmds.clear();
                } 
                server = value;
            } else if (param == "cmd") {
                tmpcmds.push_back(value);
            } else if (param == "port") {
                port = Util::stoi(value);
            } else if (param == "password") {
                password = value;
            } else if (param == "nick") {
                nick = value;
            }
        }
    }
    if (!server.empty()) {
        struct autoJoin *j = new autoJoin;
        j->hostname = server;
        j->port = port ? port : 6667;
        j->password = password;
        j->cmds = tmpcmds;
        j->nick = nick;

        _servers.push_back(j);
    } 
    return true;
}

bool Servers::writeServers()
{
    std::ofstream out(string(home + "/.lostirc/perform.conf").c_str());

    if (!out)
          return false;

    vector<struct autoJoin*>::const_iterator i;
    vector<string>::const_iterator ivec;

    for (i = _servers.begin(); i != _servers.end(); ++i) {
        out << "hostname = " << (*i)->hostname << std::endl;
        out << "port = " << (*i)->port << std::endl;
        out << "password = " << (*i)->password << std::endl;
        out << "nick = " << (*i)->nick << std::endl;

        for (ivec = (*i)->cmds.begin(); ivec != (*i)->cmds.end(); ++ivec) {
            out << "cmd = " << *ivec << std::endl;
        }
        out << std::endl;
    }
    return true;
}
void Servers::removeServer(struct autoJoin *a)
{
    vector<struct autoJoin*>::iterator i = std::find(_servers.begin(), _servers.end(), a);
    _servers.erase(i);
    delete a;
}

baseConfig::baseConfig(const char *file)
{
    filename = App->home;
    filename += file;
}

void baseConfig::set(const string& key, const string& value)
{
    if (configvalues.find(key) != configvalues.end())
          *(configvalues[key]) = value;
    else
        std::cerr << "Not found key, `" << key << "'" << std::endl;
}

std::string baseConfig::get(const string& key)
{
    map<string, baseConfigValue*>::const_iterator i = configvalues.find(key);

    if (i != configvalues.end())
          return i->second->getString();

    std::cerr << "Not found key, `" << key << "'" << std::endl;
    return "";
}

bool baseConfig::readConfigFile()
{
    std::ifstream in(filename.c_str());

    if (!in)
          return false;

    string str;
    while (getline(in, str)) {
        string::size_type pos = str.find(" = ");
        string param = str.substr(0, pos);
        string value = str.substr(pos + 3);

        if (configvalues.find(param.c_str()) != configvalues.end())
              *(configvalues[param.c_str()]) = value;
        else 
            std::cerr << "Not recognized, `" << param.c_str() << "' => " << value << std::endl;
    }
    return true;
}

bool baseConfig::writeConfigFile()
{
    #ifdef DEBUG
    App->log << "Trying to write configfile: `" << filename << "'" << std::endl;
    #endif
    std::ofstream out(filename.c_str());

    if (!out)
          return false;

    #ifdef DEBUG
    App->log << "\twriting..." << std::endl;
    #endif
    map<string, baseConfigValue*>::const_iterator i;

    for (i = configvalues.begin(); i != configvalues.end(); ++i) {
        out << i->first << " = " << i->second->getString() << std::endl;
    }
    return true;

}


Options::Options(const char *filename)
    : baseConfig(filename),
    nickcompletion_char(this, "nickcompletion_character", ','),
    buffer_size(this, "buffer_size", 500),
    window_width(this, "window_width", 0),
    window_height(this, "window_height", 0),
    realname(this, "realname"),
    nick(this, "nick"),
    ircuser(this, "ircuser"),
    dccip(this, "dccip"),
    highlight_words(this, "highlight_words"),
    font(this, "font"),
    limited_highlighting(this, "limited_highlighting")

{
    readConfigFile();
    writeConfigFile();
}

Events::Events(const char *filename)
    : baseConfig(filename),
    privmsg(this, "privmsg", "$12<$0%1$12>$0 %2"),
    privmsg_highlight(this, "privmsg_highlight", "$2<$8%1$2>$0 %2"),
    action(this, "action", "$7* %1$0 %2"),
    action_highlight(this, "action_highlight", "$8* %1$0 %2"),
    dcc_receive(this, "dcc_receive", "$16-- DCC SEND from %1 (%2), use /DCC RECEIVE %3 to accept the file transfer."),
    servmsg(this, "servmsg", "$0-- : %1"),
    servmsg2(this, "servmsg2", "$0-- : %1 %2"),
    servmsg3(this, "servmsg3", "$0-- %1:$16 %2"),
    ctcp(this, "ctcp", "$16-- CTCP$9 %1$16 received from $0%2"),
    ctcp_multi(this, "ctcp_multi", "$16-- CTCP$9 %1$16 received from $0%2$16 (to$9 %3$16)"),
    topicchange(this, "topicchange", "$16-- $0%1$16 changes topic to:$15 %2"),
    topicis(this, "topicis", "$16-- Topic for $11%1$16 is:$0 %2"),
    topictime(this, "topictime", "$16-- Set by $0%1$16 on $9%2"),
    noticepriv(this, "noticepriv", "$7NOTICE $0%1$7 : %2"),
    noticepubl(this, "noticepubl", "$7NOTICE $0%1$7 (to %2): %3"),
    error(this, "error", "$16-- Error:$8 %1"),
    away(this, "away", "$3User $0%1$3 is away $15($3%2$15)"),
    banlist(this, "banlist", "$16-- Ban: $9%1$16 set by: $0%2"),
    unknown(this, "unknown", "$16-- Unknown message: $2%1"),
    join(this, "join", "$16-- $0%1$11 $15($9%3$15)$16 has joined $11%2"),
    part(this, "part", "$16-- $0%1$16 $15($9%3$15)$16 has parted $11%2 (%4)"),
    quit(this, "quit", "$16-- $0%1$16 has quit $11(%2)"),
    nick(this, "nick", "$16-- $0%1$16 changes nick to %2"),
    mode(this, "mode", "$16-- $0%1$16 sets mode$9 %2$16 %3"),
    cmode(this, "cmode", "$16-- $0%1$16 sets channel mode$9 %2$16 on %3"),
    wallops(this, "wallops", "$8WALLOPS/<$0%1$8>$9 %2"),
    kicked(this, "kicked", "$16-- $0%1$16 was kicked from $11%2$16 by %3 $15($9%4$15)"),
    opped(this, "opped", "$16-- $0%1$16 gives channel operator status to %2"),
    deopped(this, "deopped", "$16-- $0%1$16 removes channel operator status from %2"),
    voiced(this, "voiced", "$16-- $0%1$16 gives voice to %2"),
    devoiced(this, "devoiced", "$16-- $0%1$16 removes voice from %2"),
    halfopped(this, "halfopped", "$16-- $0%1$16 gives channel half-operator status to %2"),
    halfdeopped(this, "halfdeopped", "$16-- $0%1$16 removes channel half-operator status from %2"),
    banned(this, "banned", "$16-- $0%1$16 sets ban on %2"),
    unbanned(this, "unbanned", "$16-- $0%1$16 unbans %2"),
    invited(this, "invited", "$16-- $0%1$16 invites you to join %2"),
    connecting(this, "connecting", "$16-- Connecting to$8 %1$16 on port$8 %2$16..."),
    names(this, "names", "$0-- Names$11 %1$0:$16 %2"),
    killed(this, "killed", "$16-- You were killed by $0%1$16 $15($9%2$15)")
{
    map<string, baseConfigValue*>::iterator i = configvalues.begin();

    for (; i != configvalues.end(); ++i) {
        string msg = i->second->getString();
        std::replace(msg.begin(), msg.end(), '$', '\003');
        *(i->second) = msg;
    }

    readConfigFile();
    writeConfigFile();
}
