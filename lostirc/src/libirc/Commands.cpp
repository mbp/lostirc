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

#include "Commands.h"
#include "ServerConnection.h"
#include "Utils.h"
#include <cstdio>

using std::string;
using std::stringstream;

struct UserCommands cmds[] = {
    { "SERVER",   Commands::Server,     0 },
    { "JOIN",     Commands::Join,       1 },
    { "WHOIS",    Commands::Whois,      1 },
    { "PART",     Commands::Part,       1 },
    { "QUIT",     Commands::Quit,       1 },
    { "NICK",     Commands::Nick,       1 },
    { "MODE",     Commands::Mode,       1 },
    { "CTCP",     Commands::Ctcp,       1 },
    { "AWAY",     Commands::Away,       1 },
    { "INVITE",   Commands::Invite,     1 },
    { "TOPIC",    Commands::Topic,      1 },
    { "NOTICE",   Commands::Notice,     1 },
    { "BANLIST",  Commands::Banlist,    1 },
    { "MSG",      Commands::Msg,        1 },
    { "ME",       Commands::Me,         1 },
    { "WHO",      Commands::Who,        1 },
    { "QUOTE",    Commands::Quote,      1 },
    { "COMMANDS", Commands::commands,   0 },
    { "EXEC",     Commands::Exec,       0 },
    { 0,        0,                      0 }
};


bool Commands::send(ServerConnection *conn, string cmd, const string& params) {

    for (int i = 0; cmds[i].cmd != 0; ++i) {
        if (cmds[i].cmd == Utils::toupper(cmd)) {
            if (!conn->Session.isConnected && cmds[i].reqConnected) {
                error = "Must be connected";
                return false;
            }

            return cmds[i].function(conn, params);
        }
    }

    // If we did not call any of the functions, then we assume that the
    // command isn't implemented.
    error = "Command not implemented: '" + cmd + "'";
    return false;
}

bool Commands::Join(ServerConnection *conn, const string& params)
{
    conn->sendJoin(params);
    return true;
}

bool Commands::Part(ServerConnection *conn, const string& params)
{
    conn->sendPart(params);
    return true;
}

bool Commands::Quit(ServerConnection *conn, const string& params)
{
    conn->sendQuit(params);
    return true;
}

bool Commands::Kick(ServerConnection *conn, const string& params)
{
    conn->sendKick(params);
    return true;
}

bool Commands::Server(ServerConnection *conn, const string& params)
{
    conn->Connect(params);
    return true;
}

bool Commands::Nick(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        error = "/NICK <nick>, change nick.";
        return false;
    } else {
        conn->sendNick(params);
        return true;
    }
}

bool Commands::Whois(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        error = "/WHOIS <nick>, whois nick.";
        return false;
    } else {
        conn->sendWhois(params);
        return true;
    }
}

bool Commands::Mode(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        error = "/MODE <channel> <modes>, set modes for a channel.";
        return false;
    } else {
        conn->sendMode(params);
        return true;
    }
}

bool Commands::Ctcp(ServerConnection *conn, const string& msg)
{
    string to, action;
    stringstream ss(msg);
    ss >> to;
    ss >> action;

    if (action.empty()) {
        error = "/CTCP <nick> <message>, sends a CTCP message to a user";
        return false;
    } else {
        action = Utils::toupper(action);

        conn->sendCtcp(to, action);
        return true;
    }
}

bool Commands::Away(ServerConnection *conn, const string& params)
{
    conn->sendAway(params);
    return true;
}

bool Commands::Banlist(ServerConnection *conn, const string& chan)
{
    if (chan.empty()) {
        error = "/BANLIST <channel>, see banlist for channel.";
        return false;
    } else {
        conn->sendBanlist(chan);
        return true;
    }
}

bool Commands::Invite(ServerConnection *conn, const string& params)
{
    string to, chan;
    stringstream ss(params);
    ss >> to;
    ss >> chan;

    if (chan.empty()) {
        error = "/INVITE <nick> <channel>, invites someone to a channel.";
        return false;
    } else {
        conn->sendInvite(to, chan);
        return true;
    }
}

bool Commands::Topic(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string chan = params.substr(0, pos1);
    string topic = params.substr(pos1 + 1);

    if (chan.empty()) {
        error = "/TOPIC <channel> [topic], view or change topic for a channel.";
        return false;
    } else {
        conn->sendTopic(chan, topic);
        return true;
    }
}

bool Commands::Msg(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string to = params.substr(0, pos1);
    string msg = params.substr(pos1 + 1);

    if (msg.empty()) {
       error = "/MSG <nick/channel> <message>, sends a normal message.";
       return false;
    } else {
       conn->sendMsg(to, msg);
       return true;
    }
}

bool Commands::Notice(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string to = params.substr(0, pos1);
    string msg = params.substr(pos1 + 1);

    if (msg.empty()) {
       error = "/NOTICE <nick/channel> <message>, sends a notice.";
       return false;
    } else {
       conn->sendNotice(to, msg);
       error = "-- " + conn->Session.nick + " -> " + to + " : " + msg;
       return false;
    }
}

bool Commands::Me(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string to = params.substr(0, pos1);
    string msg = params.substr(pos1 + 1);

    if (msg.empty()) {
       error = "/ME <message>, sends the action to the current channel.";
       return false;
    } else {
       conn->sendMe(to, msg);
       error = "* " + conn->Session.nick + " " + msg;
       return false;
    }
}

bool Commands::Who(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
       error = "/WHO <mask> [o], search for mask on network, if o is supplied, only search for oppers.";
       return false;
    } else {
       conn->sendWho(params);
       return true;
    }
}

bool Commands::Quote(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
       error = "/QUOTE <text>, send raw text to server.";
       return false;
    } else {
       conn->sendRaw(params);
       return true;
    }
}

bool Commands::commands(ServerConnection *conn, const string& params)
{
    for (int i = 0; cmds[i].cmd != 0; ++i) {
        Commands::error += " \00311[\0030";
        Commands::error += cmds[i].cmd;
        Commands::error += "\00311]";
    }
    return false;
}

bool Commands::Exec(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string param = params.substr(0, pos1);
    string rest = params.substr(pos1 + 1);

    if (param == "-o") {
        FILE* f = popen("/bin/sh -c ls", "r");

        char buf[4028];

        fread(buf, 1, 4028, f);

        cout << buf << endl;

        return true;
    } else if (!params.empty()) {
        FILE* f = popen("/bin/sh -c ls", "r");

        char buf[4028];

        fread(buf, 1, 4028, f);

        string str(buf);
        Commands::error = str;
        return false;
    } else {
       error = "/EXEC [-o] <command>, execute a command, if -o is used, output to channel.";
       return false;
    }
}

string Commands::error;
