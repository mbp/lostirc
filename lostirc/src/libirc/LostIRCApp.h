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
#include <sigc++/signal_system.h>
#include <sys/utsname.h>
#include "ConfigHandler.h"
#include "Parser.h"
#include "Channel.h"

class ServerConnection;
class Events;

using namespace SigC;

class LostIRCApp
{
    std::string realname;

public:
    LostIRCApp();
    ~LostIRCApp();
    struct utsname getsysinfo();

    ServerConnection* newServer(const std::string& host, int port);
    ServerConnection* newServer();

    ConfigHandler& getCfg() { return _cfg; }
    Events* getEvts() { return _evts; }

    // Signals 
    
    // Emitted when a user joins a channel
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtJoin;

    // Emitted when a user parts a channel
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtPart;

    // Emitted when a user quits a channel
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtQuit;

    // Emitted when a user changes nick
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtNick;

    // Emitted when we receive all the names from the channel
    Signal2<void, Channel&, ServerConnection*> evtNames;
    Signal4<void, const std::string&, const std::string&, const std::map<std::string, IRC::UserMode>&, ServerConnection*> evtCUMode;
    Signal5<void, const std::string&, const std::string&, const std::string&, const std::string&, ServerConnection*> evtKick;

    // Emitted when the frontend needs to diplay a message
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtDisplayMessage;
    Signal3<void, const std::vector<std::string>&, const std::string&, ServerConnection*> evtDisplayMessageMultiple;

    // Emitted when a channel needs to be highlighted
    Signal2<void, const std::string&, ServerConnection*> evtHighlight;

    // Emitted when the user is going away
    Signal2<void, bool, ServerConnection*> evtAway;


private:
    std::vector<ServerConnection*> _servers;

    ConfigHandler _cfg;
    Events *_evts;
    struct utsname uname_info;

};

#endif
