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

class InOut
{

public:
    InOut();
    struct utsname getsysinfo();

    ServerConnection * newServer(const string& host, int port, const string& nick);
    ServerConnection * newServer(const string& nick, const string& realname);
    void quit();

    // Signals 
    SigC::Signal2<void, const string&, ServerConnection*> evtUnknownMessage;
    SigC::Signal2<void, const string&, ServerConnection*> evtGenericError;
    SigC::Signal2<void, const string&, ServerConnection*> evtSelfaway;
    
    SigC::Signal3<void, const string&, const string&, ServerConnection*> evtJoin;
    SigC::Signal3<void, const string&, const string&, ServerConnection*> evtPart;
    SigC::Signal3<void, const string&, const string&, ServerConnection*> evtQuit;
    SigC::Signal3<void, const string&, const string&, ServerConnection*> evtNick;
    SigC::Signal3<void, const string&, const string&, ServerConnection*> evtWallops;
    SigC::Signal3<void, const string&, const vector<vector<string> >&, ServerConnection*> evtNames;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtAway;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtNotice;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtNctcp;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtWhois;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtTopic;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtTopicTime;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtMode;
    SigC::Signal4<void, const string&, const string&, const vector<vector<string> >&, ServerConnection*> evtCMode;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtMsg;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtAction;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtErrhandler;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtBanlist;
    SigC::Signal3<void, const string&, const string&, ServerConnection*> evtCTCP;
    SigC::Signal4<void, const string&, const string&, const string&, ServerConnection*> evtServMsg;
    SigC::Signal5<void, int, const string&, const string&, const string&, ServerConnection*> evtServNumeric;
    SigC::Signal5<void, const string&, const string&, const string&, const string&, ServerConnection*> evtKick;

private:
    vector<ServerConnection*> _servers;

    struct utsname uname_info;

};
#endif
