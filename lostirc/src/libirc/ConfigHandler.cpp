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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "ConfigHandler.h"
#include "Events.h"

using std::string;
using std::cout;
using std::vector;
using std::map;

ConfigHandler::ConfigHandler()
{
    string home(getenv("HOME"));
    string configdir = home + "/.lostirc/";
    mkdir(configdir.c_str(), 0700);

    readConfig();
}

ConfigHandler::~ConfigHandler()
{
    vector<struct autoJoin*>::iterator i;

    for (i = _servers.begin(); i != _servers.end();) {
        delete (*i);
        i = _servers.erase(i);
    }
}

bool ConfigHandler::readConfig()
{
    string home(getenv("HOME"));
    readOptions(home + "/.lostirc/options.conf");
    readEvents(home + "/.lostirc/events.conf");
    readServers(home + "/.lostirc/perform.conf");
    writeEvents();
    return true; // FIXME
}

bool ConfigHandler::readEvents(const string& filename)
{
    if (!readIniFile(filename, _events))
          return setEvtDefaults();

    return true;
}

bool ConfigHandler::readOptions(const string& filename)
{
    if (!readIniFile(filename, _options))
          return setOptDefaults();

    return true;
}

bool ConfigHandler::readIniFile(const string& filename, map<string, string> & themap)
{

    std::ifstream in(filename.c_str());

    if (!in) {
        return false;
    }

    /* FIXME: too many nested while loops below */
    string str;
    while (getline(in, str)) {
        string::size_type pos = str.find(" = ");
        string param = str.substr(0, pos);
        string value = str.substr(pos + 3);

        themap.insert(make_pair(param, value));
    }
    return false;
}

bool ConfigHandler::writeEvents()
{
    return writeIniFile(string(string(getenv("HOME")) + "/.lostirc/events.conf"), _events);
}

bool ConfigHandler::writeOptions()
{
    return writeIniFile(string(string(getenv("HOME")) + "/.lostirc/options.conf"), _options);
}

bool ConfigHandler::writeIniFile(const string& filename, map<string, string>& themap)
{
    std::ofstream out(filename.c_str());

    if (!out)
          return false;

    map<string, string>::const_iterator i;

    for (i = themap.begin(); i != themap.end(); ++i) {
        out << i->first << " = " << i->second << std::endl;
    }
    return true;

}

bool ConfigHandler::readServers(const string& filename)
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

bool ConfigHandler::writeServers()
{
    string home(getenv("HOME"));
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

bool ConfigHandler::setEvt(const string& key, const string& value)
{
    #ifdef DEBUG
    std::cout << "ConfigHandler::setEvt(): Setting '" + key + "' to: '" + value + "'" << std::endl;
    #endif
    _events[key] = value;

    //return writeEvents();
    return true;
}

string ConfigHandler::getEvt(const string& param)
{
    map<string, string>::const_iterator i = _events.find(param);

    if (i != _events.end())
          return (*i).second;

    return "";
}

bool ConfigHandler::setOpt(const string& key, const string& value)
{
    #ifdef DEBUG
    std::cout << "ConfigHandler::setOpt(): Setting '" + key + "' to: '" + value + "'" << std::endl;
    #endif
    _options[key] = value;

    return writeOptions();
    return true;
}

string ConfigHandler::getOpt(const string& param)
{
    map<string, string>::const_iterator i = _options.find(param);

    if (i != _options.end())
          return (*i).second;

    return "";
}

bool ConfigHandler::setEvtDefaults()
{
    setEvtDefault("evt_privmsg", "$12<$0%1$12>$0 %2");
    setEvtDefault("evt_privmsg_highlight", "$2<$8%1$2>$0 %2");
    setEvtDefault("evt_action", "$7* %1$0 %2");
    setEvtDefault("evt_action_highlight", "$8* %1$0 %2");
    setEvtDefault("evt_dcc_receive", "$16-- DCC SEND from %1 (%2), use /DCC RECEIVE %3 to accept the file transfer.");
    setEvtDefault("evt_servmsg", "$0-- : %1");
    setEvtDefault("evt_servmsg2", "$0-- : %1 %2");
    setEvtDefault("evt_servmsg3", "$0-- %1:$16 %2");
    setEvtDefault("evt_ctcp", "$16-- CTCP$9 %1$16 received from $0%2");
    setEvtDefault("evt_ctcp_multi", "$16-- CTCP$9 %1$16 received from $0%2$16 (to$9 %3$16)");
    setEvtDefault("evt_topicchange", "$16-- $0%1$16 changes topic to:$15 %2");
    setEvtDefault("evt_topicis", "$16-- Topic for $11%1$16 is:$0 %2");
    setEvtDefault("evt_topictime", "$16-- Set by $0%1$16 on $9%2");
    setEvtDefault("evt_noticepriv", "$7NOTICE $0%1$7 : %2");
    setEvtDefault("evt_noticepubl", "$7NOTICE $0%1$7 (to %2): %3");
    setEvtDefault("evt_error", "$16-- Error:$8 %1");
    setEvtDefault("evt_away", "$3User $0%1$3 is away $15($3%2$15)");
    setEvtDefault("evt_banlist", "$16-- Ban: $9%1$16 set by: $0%2");
    setEvtDefault("evt_unknown", "$16-- Unknown message: $2%1");
    setEvtDefault("evt_join", "$16-- $0%1$11 $15($9%3$15)$16 has joined $11%2");
    setEvtDefault("evt_part", "$16-- $0%1$16 $15($9%3$15)$16 has parted $11%2 (%4)");
    setEvtDefault("evt_quit", "$16-- $0%1$16 has quit $11(%2)");
    setEvtDefault("evt_nick", "$16-- $0%1$16 changes nick to %2");
    setEvtDefault("evt_mode", "$16-- $0%1$16 sets mode $5%2$16 %3");
    setEvtDefault("evt_cmode", "$16-- $0%1$16 sets channel mode $9%2$16 on %3");
    setEvtDefault("evt_wallops", "$8WALLOPS/<$0%1$8>$9 %2");
    setEvtDefault("evt_kicked", "$16-- $0%1$16 was kicked from $11%2$16 by %3 $15($9%4$15)");
    setEvtDefault("evt_opped", "$16-- $0%1$16 gives channel operator status to %2");
    setEvtDefault("evt_deopped", "$16-- $0%1$16 removes channel operator status from %2");
    setEvtDefault("evt_voiced", "$16-- $0%1$16 gives voice to %2");
    setEvtDefault("evt_devoiced", "$16-- $0%1$16 removes voice from %2");
    setEvtDefault("evt_banned", "$16-- $0%1$16 sets ban on %2");
    setEvtDefault("evt_unbanned", "$16-- $0%1$16 unbans %2");
    setEvtDefault("evt_invited", "$16-- $0%1$16 invites you to join %2");
    setEvtDefault("evt_connecting", "$16-- Connecting to$8 %1$16 on port$8 %2$16...");
    setEvtDefault("evt_names", "$0-- Names$11 %1$0:$16 %2");

    map<string, string>::iterator i = _events.begin();

    for (; i != _events.end(); ++i) {
        string msg = i->second;
        std::replace(msg.begin(), msg.end(), '$', '\003');
        i->second = msg;
    }

    return writeEvents();
}

void ConfigHandler::setEvtDefault(const string& key, const string& value)
{
    map<string, string>::const_iterator i = _events.find(key);

    if (i == _events.end())
          _events[key] = value;
}

bool ConfigHandler::setOptDefaults()
{
    setOptDefault("nickcompletion_character", ",");

    return writeOptions();
}

void ConfigHandler::setOptDefault(const string& key, const string& value)
{
    map<string, string>::const_iterator i = _options.find(key);

    if (i == _options.end())
          _options[key] = value;
}

void ConfigHandler::removeServer(struct autoJoin *a)
{
    vector<struct autoJoin*>::iterator i = std::find(_servers.begin(), _servers.end(), a);
    _servers.erase(i);
    delete a;
}
