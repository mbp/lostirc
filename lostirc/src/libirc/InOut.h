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

class ServerConnection;

using namespace SigC;

class InOut
{
public:
    InOut();
    struct utsname getsysinfo();

    ServerConnection* newServer(const std::string& host, int port, const std::string& nick);
    ServerConnection* newServer(const std::string& nick, const std::string& realname);
    void quit();

    // Signals 
    Signal2<void, const std::string&, ServerConnection*> evtUnknownMessage;
    Signal2<void, const std::string&, ServerConnection*> evtGenericError;
    Signal2<void, const std::string&, ServerConnection*> evtSelfaway;
    
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtJoin;
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtPart;
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtQuit;
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtNick;
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtWallops;
    Signal3<void, const std::string&, const std::vector<std::vector<std::string> >&, ServerConnection*> evtNames;
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtCTCP;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtAway;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtNotice;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtNctcp;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtWhois;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtTopic;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtTopicTime;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtMode;
    Signal4<void, const std::string&, const std::string&, const vector<vector<std::string> >&, ServerConnection*> evtCUMode;
    Signal5<void, const std::string&, const std::string&, char, const std::string&, ServerConnection*> evtCMode;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtMsg;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtAction;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtErrhandler;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtBanlist;
    Signal4<void, const std::string&, const std::string&, const std::string&, ServerConnection*> evtServMsg;
    Signal5<void, int, const std::string&, const std::string&, const std::string&, ServerConnection*> evtServNumeric;
    Signal5<void, const std::string&, const std::string&, const std::string&, const std::string&, ServerConnection*> evtKick;
    Signal3<void, const std::string&, const std::string&, ServerConnection*> evtDisplayMessage;
    Signal3<void, const std::vector<std::string>&, const std::string&, ServerConnection*> evtDisplayMessageMultiple;

private:
    std::vector<ServerConnection*> _servers;

    struct utsname uname_info;

};
#endif
