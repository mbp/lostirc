/*
 * Copyright (C) 2002, 2003 Morten Brix Pedersen <morten@wtf.dk>
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

#ifndef LOSTIRCAPP_H
#define LOSTIRCAPP_H

#include <iostream>
#include <vector>
#include <cassert>
#ifndef WIN32
#include <sys/utsname.h>
#endif
#include <glibmm/miscutils.h>
#include <glibmm/ustring.h>
#include <unistd.h>
#include <sys/types.h>
#include "ConfigHandler.h"
#include "Channel.h"
#include "DCC.h"
#include "FrontEnd.h"
#include "LostIRC.h"

class ServerConnection;
class FrontEnd;

#ifdef DEBUG
#include <iostream>
class Log : public std::ofstream
{
    Glib::ustring _filename;
public:
    Log()
    {
        _filename = "lostirc-"VERSION".log";
        struct stat st;
        if (stat(_filename.c_str(), &st) == 0) {
            getUseableFilename(1);
        }
        std::cout << _("Logging to `") << _filename << "'." << std::endl;
        open(_filename.c_str());
        assert(good());

    }

private:
    void getUseableFilename(int i)
    {
        struct stat st;
        std::stringstream ss;
        Glib::ustring myint;
        ss << i;
        ss >> myint;
        Glib::ustring newfilename = _filename + "." + myint;
        if (stat(newfilename.c_str(), &st) == 0)
              getUseableFilename(++i);
        else
              _filename = newfilename;
    }

};
#endif


class LostIRCApp;
extern LostIRCApp* App;

class LostIRCApp
{
    std::vector<ServerConnection*> _servers;

    // This is a dummy class used for initialization
    class initobj
    {
    public:
        initobj(LostIRCApp* app) {
            App = app;
            App->home = Glib::get_home_dir();

            Glib::ustring configdir = Glib::ustring(App->home) + "/.lostirc/";
            App->logdir = Glib::ustring(App->home) + "/.lostirc/logs/";
            #ifndef WIN32
            mkdir(configdir.c_str(), 0700);
            #else
            mkdir(configdir.c_str());
            #endif
        }
    };

    initobj init;
    DCC_queue _dcc_queue;


public:
    LostIRCApp(FrontEnd *f);
    ~LostIRCApp();

    void autoConnect();
    ServerConnection* newServer(const Glib::ustring& host, int port);
    ServerConnection* newServer(Server* s);
    ServerConnection* newServer();

    DCC_queue& getDcc() { return _dcc_queue; }
    const std::vector<ServerConnection*>& getServers() { return _servers; }

#ifdef DEBUG
    Log log;
#endif

    FrontEnd* fe;

    Options options;
    Events events;
    Colors colors;
    Servers cfgservers;

    #ifndef WIN32
    static struct utsname uname_info;
    #endif
    static Glib::ustring home;
    static Glib::ustring logdir;

};

#endif
