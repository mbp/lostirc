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

#ifndef INOUT_H
#define INOUT_H

#include <vector>
#include <string>
#include <sigc++/signal_system.h>
#include <sys/utsname.h>
#include "ConfigHandler.h"

class ServerConnection;
class Events;

using namespace SigC;

class LostIRCApp
{
    string nick;
    string realname;

public:
    LostIRCApp();
    struct utsname getsysinfo();

    ServerConnection* newServer(const std::string& host, int port);
    ServerConnection* newServer();
    void quit();

    ConfigHandler& getCfg() { return _cfg; }
    Events* getEvts() { return _evts; }

    // Signals 
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtJoin;
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtPart;
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtQuit;
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtNick;
    Signal3<void, const std::string&, const std::vector<std::vector<std::string> >&, ServerConnection*> evtNames;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtMode;
    Signal4<void, const std::string&, const std::string&, const vector<vector<std::string> >&, ServerConnection*> evtCUMode;
    Signal5<void, const std::string&, const std::string&, char, const std::string&, ServerConnection*> evtCMode;
    Signal5<void, const std::string&, const std::string&, const std::string&, const std::string&, ServerConnection*> evtKick;

    // Emitted when the frontend needs to diplay a message
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtDisplayMessage;
    Signal3<void, const std::vector<std::string>&, const std::string&, ServerConnection*> evtDisplayMessageMultiple;

    // Emitted when a channel needs to be highlighted
    Signal2<void, const std::string&, ServerConnection*> evtHighlight;


private:
    std::vector<ServerConnection*> _servers;

    ConfigHandler _cfg;
    Events *_evts;
    struct utsname uname_info;

};
#endif
