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

#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include "Utils.h"

struct autoJoin {
    std::string hostname;
    int port;
    std::string nick;
    std::string password;
    std::vector<std::string> cmds;
};

class ConfigHandler {
    std::map<std::string, std::string> _events;
    std::map<std::string, std::string> _options;
    std::vector<struct autoJoin*> _servers;

public:
    ConfigHandler();
    ~ConfigHandler();
    /* read all configuration files */
    bool readConfig();

    /* ~/.lostirc/events.conf */
    bool setEvt(const std::string& key, const std::string& value);
    std::string getEvt(const std::string& param);

    /* ~/.lostirc/options.conf */
    bool setOpt(const std::string& key, const std::string& value);
    std::string getOpt(const std::string& param);

    /* ~/.lostirc/perform.conf */
    void addServer(struct autoJoin* a) { _servers.push_back(a); }
    void removeServer(struct autoJoin* a);

    /* return "auto-join list" */
    std::vector<struct autoJoin*> getServers() { return _servers; }

    /* write server list */
    bool writeServers();

private:
    bool readOptions(const std::string& filename);
    bool readEvents(const std::string& filename);
    bool readServers(const std::string& filename);
    bool readIniFile(const std::string& filename, std::map<std::string, std::string> & themap);
    bool setEvtDefaults();
    void setEvtDefault(const std::string& key, const std::string& value);
    bool setOptDefaults();
    void setOptDefault(const std::string& key, const std::string& value);
    bool writeIniFile(const std::string& filename, std::map<std::string, std::string>& themap);
    bool writeEvents();
    bool writeOptions();

};
#endif
