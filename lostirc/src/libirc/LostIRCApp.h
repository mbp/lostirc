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

#ifndef INOUT_H
#define INOUT_H

#include <vector>
#include <string>
#include <sys/utsname.h>
#include "ConfigHandler.h"
#include "Channel.h"
#include "DCC.h"
#include "FrontEnd.h"

class ServerConnection;
class FrontEnd;

#ifdef DEBUG
#include <iostream>
class Log : public std::ofstream
{
    std::string _filename;
public:
    Log()
    {
        _filename = "lostirc-"VERSION".log";
        struct stat st;
        if (stat(_filename.c_str(), &st) == 0) {
            getUseableFilename(1);
        }
        std::cout << "Logging to `" << _filename << "'." << std::endl;
        open(_filename.c_str());
        assert(good());

    }

private:
    void getUseableFilename(int i)
    {
        struct stat st;
        std::stringstream ss;
        std::string myint;
        ss << i;
        ss >> myint;
        std::string newfilename = _filename + "." + myint;
        if (stat(newfilename.c_str(), &st) == 0)
              getUseableFilename(++i);
        else
              _filename = newfilename;
    }

};
#endif

class LostIRCApp
{

public:
    LostIRCApp(FrontEnd *f);
    ~LostIRCApp();

    // 'start' means 'auto-join servers' - it returns the number of servers
    // joined
    int start();
    ServerConnection* newServer(const std::string& host, int port);
    ServerConnection* newServer();

    ConfigHandler& getCfg() { return _cfg; }
    DCC_queue& getDcc() { return _dcc_queue; }
    const std::vector<ServerConnection*>& getServers() { return _servers; }

#ifdef DEBUG
    Log log;
#endif

    FrontEnd* fe;

    static struct utsname uname_info;

private:
    std::vector<ServerConnection*> _servers;

    ConfigHandler _cfg;
    DCC_queue _dcc_queue;
};

extern LostIRCApp* App;

#endif
