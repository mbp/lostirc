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

using namespace std;
using namespace SigC;

class InOut
{

public:
    InOut();
    struct utsname getsysinfo();

    ServerConnection* newServer(const string& host, int port, const string& nick);
    ServerConnection* newServer(const string& nick, const string& realname);
    void quit();

    // Signals 
    Signal2<void, const string&, ServerConnection*> evtUnknownMessage;
    Signal2<void, const string&, ServerConnection*> evtGenericError;
    Signal2<void, const string&, ServerConnection*> evtSelfaway;
    
    Signal3<void, const string&, const string&, ServerConnection*> evtJoin;
    Signal3<void, const string&, const string&, ServerConnection*> evtPart;
    Signal3<void, const string&, const string&, ServerConnection*> evtQuit;
    Signal3<void, const string&, const string&, ServerConnection*> evtNick;
    Signal3<void, const string&, const string&, ServerConnection*> evtWallops;
    Signal3<void, const string&, const vector<vector<string> >&, ServerConnection*> evtNames;
    Signal3<void, const string&, const string&, ServerConnection*> evtCTCP;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtAway;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtNotice;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtNctcp;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtWhois;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtTopic;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtTopicTime;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtMode;
    Signal4<void, const string&, const string&, const vector<vector<string> >&, ServerConnection*> evtCUMode;
    Signal5<void, const string&, const string&, bool, const string&, ServerConnection*> evtCMode;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtMsg;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtAction;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtErrhandler;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtBanlist;
    Signal4<void, const string&, const string&, const string&, ServerConnection*> evtServMsg;
    Signal5<void, int, const string&, const string&, const string&, ServerConnection*> evtServNumeric;
    Signal5<void, const string&, const string&, const string&, const string&, ServerConnection*> evtKick;

private:
    vector<ServerConnection*> _servers;

    struct utsname uname_info;

};
#endif
