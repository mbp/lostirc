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

#include <cstdio>
#include <sstream>
#include "Commands.h"
#include "ServerConnection.h"
#include "Utils.h"
#include "LostIRCApp.h"

using std::string;
using std::stringstream;
using std::istringstream;

const struct UserCommands cmds[] = {
    { "SERVER",   Commands::Server,     0 },
    { "DISCONNECT", Commands::Disconnect,     0 },
    { "JOIN",     Commands::Join,       1 },
    { "WHOIS",    Commands::Whois,      1 },
    { "PART",     Commands::Part,       1 },
    { "QUIT",     Commands::Quit,       1 },
    { "NICK",     Commands::Nick,       0 },
    { "KICK",     Commands::Kick,       1 },
    { "NAMES",    Commands::Names,      1 },
    { "MODE",     Commands::Mode,       1 },
    { "CTCP",     Commands::Ctcp,       1 },
    { "AWAY",     Commands::Away,       1 },
    { "AWAYALL",  Commands::Awayall,    1 },
    { "INVITE",   Commands::Invite,     1 },
    { "TOPIC",    Commands::Topic,      1 },
    { "NOTICE",   Commands::Notice,     1 },
    { "BANLIST",  Commands::Banlist,    1 },
    { "MSG",      Commands::Msg,        1 },
    { "ME",       Commands::Me,         1 },
    { "WHO",      Commands::Who,        1 },
    { "LIST",     Commands::List,       1 },
    { "SET",      Commands::Set,        0 },
    { "QUOTE",    Commands::Quote,      1 },
    { "COMMANDS", Commands::commands,   0 },
    //{ "EXEC",     Commands::Exec,       0 },
    { "OPER",     Commands::Oper,       1 },
    { "KILL",     Commands::Kill,       1 },
    { "WALLOPS",  Commands::Wallops,    1 },
    { "DCC",      Commands::DCC,        1 },
    { "ADMIN",    Commands::Admin,      1 },
    { "WHOWAS",   Commands::Whowas,     1 },
    { "OP",       Commands::Op,         1 },
    { "DEOP",     Commands::Deop,       1 },
    { "VOICE",    Commands::Voice,      1 },
    { "DEVOICE",  Commands::Devoice,    1 },
    { 0,        0,                      0 }
};


namespace Commands {
void send(ServerConnection *conn, string cmd, const string& params) {

    for (int i = 0; cmds[i].cmd != 0; ++i) {
        if (cmds[i].cmd == cmd) {
            if (!conn->Session.isConnected && cmds[i].reqConnected) {
                throw CommandException("Must be connected.");
            }
            cmds[i].function(conn, params);
            return;
        }
    }

    // If no matching functions were found, just try to send the command raw
    // to the server.
    
    if (conn->Session.isConnected)
          Quote(conn, cmd + " " + params);
}

void Join(ServerConnection *conn, const string& params)
{
    if (params.length() == 0) {
        throw CommandException("/JOIN <channel>, join a channel");
    } else {
        conn->sendJoin(params);
    }
}

void Part(ServerConnection *conn, const string& params)
{
    if (params.length() == 0) {
        throw CommandException("/PART <channel> [msg], part a channel - optional with a part message");
    } else {
        string::size_type pos1 = params.find_first_of(" ");
        string chan = params.substr(0, pos1);
        string msg;
        if (pos1 != string::npos)
              msg = params.substr(pos1 + 1);

        conn->sendPart(chan, msg);
    }
}

void Quit(ServerConnection *conn, const string& params)
{
    conn->sendQuit(params);
    conn->disconnect();
}

void Kick(ServerConnection *conn, const string& params)
{
    string chan, nick, msg;
    string::size_type pos1 = params.find_first_of(" ");
    chan = params.substr(0, pos1);
    if (pos1 != string::npos) {
        string::size_type pos2 = params.find_first_of(" ", pos1 + 1);

        nick = params.substr(pos1 + 1, (pos2 - 1) - pos1);

        if (pos2 != string::npos) {
            msg = params.substr(pos2 + 1);
        }
    }

    if (params.empty() || chan.empty() || nick.empty()) {
        throw CommandException("/KICK <channel> <nick> [msg], kick a user from a channel.");
    } else {
        conn->sendKick(chan, nick, msg);
    }
}

void Server(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/SERVER <host/ip> [port] [password], connect to an IRC server");
    } else {
        string host, port, password;
        istringstream ss(params);
        ss >> host;
        ss >> port;
        ss >> password;

        if (conn->Session.isConnected) {
              conn->sendQuit();
              conn->Session.isConnected = false;
        }
        if (!port.empty()) {
            int p = Util::stoi(port);

            if (!password.empty()) {
                conn->connect(host, p, password);
            } else {
                conn->connect(host, p);
            }
        } else {
            conn->connect(host);
        }
    }
}

void Disconnect(ServerConnection *conn, const string& params)
{
    conn->removeReconnectTimer();
    conn->disconnect();
}

void Nick(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/NICK <nick>, change nick.");
    } else {
        if (conn->Session.isConnected) {
            conn->sendNick(params);
        } else {
            conn->Session.nick = params;
        }
    }
}

void Whois(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/WHOIS <nick>, whois nick.");
    } else {
        conn->sendWhois(params);
    }
}

void Mode(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/MODE <channel> <modes>, set modes for a channel.");
    } else {
        conn->sendMode(params);
    }
}

void Set(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string key = params.substr(0, pos1);
    string value;
    if (pos1 != string::npos)
          value = params.substr(pos1 + 1);

    App->getCfg().setOpt(key, value);
}

void Ctcp(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string to = params.substr(0, pos1);
    string action;
    if (pos1 != string::npos)
          action = params.substr(pos1 + 1);

    if (action.empty()) {
        throw CommandException("/CTCP <nick> <message>, sends a CTCP message to a user");
    } else {
        action = Util::upper(action);

        conn->sendCtcp(to, action);
    }
}

void Away(ServerConnection *conn, const string& params)
{
    conn->sendAway(params);
}

void Awayall(ServerConnection *conn, const string& params)
{
    std::vector<ServerConnection*> servers = App->getServers();
    std::vector<ServerConnection*>::iterator i;

    for (i = servers.begin(); i != servers.end(); ++i)
    {
        (*i)->sendAway(params);
    }
}

void Banlist(ServerConnection *conn, const string& chan)
{
    if (chan.empty()) {
        throw CommandException("/BANLIST <channel>, see banlist for channel.");
    } else {
        conn->sendBanlist(chan);
    }
}

void Invite(ServerConnection *conn, const string& params)
{
    string to, chan;
    stringstream ss(params);
    ss >> to;
    ss >> chan;

    if (chan.empty()) {
        throw CommandException("/INVITE <nick> <channel>, invites someone to a channel.");
    } else {
        conn->sendInvite(to, chan);
    }
}

void Topic(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string chan = params.substr(0, pos1);
    string topic;
    if (pos1 != string::npos)
          topic = params.substr(pos1 + 1);

    if (chan.empty()) {
        throw CommandException("/TOPIC <channel> [topic], view or change topic for a channel.");
    } else {
        conn->sendTopic(chan, topic);
    }
}

void Msg(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string to = params.substr(0, pos1);
    string msg;
    if (pos1 != string::npos)
          msg = params.substr(pos1 + 1);

    if (msg.empty()) {
        throw CommandException("/MSG <nick/channel> <message>, sends a normal message.");
    } else {
        conn->sendMsg(to, msg);
        string sendgui = "Message to " + to + ": " + msg;
        FE::emit(FE::get(SERVMSG) << sendgui, FE::CURRENT, conn);
    }
}

void Notice(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string to = params.substr(0, pos1);
    string msg;
    if (pos1 != string::npos)
          msg = params.substr(pos1 + 1);

    if (msg.empty()) {
        throw CommandException("/NOTICE <nick/channel> <message>, sends a notice.");
    } else {
        conn->sendNotice(to, msg);
        string sendgui = "Notice to " + to + ": " + msg;
        FE::emit(FE::get(SERVMSG) << sendgui, FE::CURRENT, conn);
    }
}

void Me(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string to = params.substr(0, pos1);
    string msg = params.substr(pos1 + 1);

    if (msg.empty()) {
        throw CommandException("/ME <message>, sends the action to the current channel.");
    } else {
        conn->sendMe(to, msg);
        FE::emit(FE::get(ACTION) << conn->Session.nick << msg, FE::CURRENT, conn);
    }
}

void Who(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/WHO <mask> [o], search for mask on network, if o is supplied, only search for oppers.");
    } else {
        conn->sendWho(params);
    }
}

void List(ServerConnection *conn, const string& params)
{
    //throw CommandException("/LIST [channels] [server], list channels on a network, if a channel is supplied, only list that channel. If a server is supplied, forward the request to that IRC server.");
    conn->sendList(params);
}

void Quote(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/QUOTE <text>, send raw text to server.");
    } else {
        conn->sendRaw(params);
    }
}

void Names(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/NAMES <channel>, see who's on a channel.");
    } else {
        conn->sendNames(params);
    }
}

void Oper(ServerConnection* conn, const string& params)
{
    if (params.empty()) {
       throw CommandException("/OPER <login> <password>, oper up.");
    } else {
       string login, password;
       istringstream ss(params);
       ss >> login;
       ss >> password;

       if (login.empty() || password.empty())
           throw CommandException( "/OPER <login> <password>, oper up.");

       conn->sendOper(login, password);
    }
}

void Kill(ServerConnection* conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string nick = params.substr(0, pos1);
    string reason;
    if (pos1 != string::npos)
          reason = params.substr(pos1 + 1);

    if (nick.empty()) {
       throw CommandException("/KILL <user> [reason], kill a user from the network.");
    } else {

       conn->sendKill(nick, reason);
    }
}

void Wallops(ServerConnection* conn, const string& params)
{
    if (params.empty()) {
       throw CommandException("/WALLOPS <message>, send wallop message.");
    } else {

       conn->sendWallops(params);
    }
}

void DCC(ServerConnection* conn, const string& params)
{
    if (params.empty()) {
       throw CommandException("/DCC <actions>, perform a DCC action.");
    } else {
       string action, secondparam;
       istringstream ss(params);
       ss >> action;
       ss >> secondparam;

       action = Util::upper(action);
       if (action == "RECEIVE") {
           if (!App->getDcc().do_dcc(Util::stoi(secondparam)))
                 throw CommandException("No DCC with that number");
       } else if (action == "SEND") {
           string filename;
           ss >> filename;

           App->getDcc().addDccSendOut(filename, secondparam, conn);
       }
    }
}

void Admin(ServerConnection* conn, const string& params)
{
    conn->sendAdmin(params);
}

void Whowas(ServerConnection* conn, const string& params)
{
    conn->sendWhowas(params);
}

void Op(ServerConnection* conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/OP <channel> <nicks>, give operator status to one or more nicks.");
    } else {

        string chan;
        istringstream ss(params);
        ss >> chan;

        std::string modeline = assignModes('+', 'o', ss);

        conn->sendMode(chan + " " + modeline);
    }
}

void Deop(ServerConnection* conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/DEOP <channel> <nicks>, remove operator status from one or more nicks.");
    } else {

        string chan;
        istringstream ss(params);
        ss >> chan;

        std::string modeline = assignModes('-', 'o', ss);

        conn->sendMode(chan + " " + modeline);
    }
}

void Voice(ServerConnection* conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/VOICE <channel> <nicks>, gives voice to one or more nicks.");
    } else {

        string chan;
        istringstream ss(params);
        ss >> chan;

        std::string modeline = assignModes('+', 'v', ss);

        conn->sendMode(chan + " " + modeline);
    }
}

void Devoice(ServerConnection* conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/DEVOICE <channel> <nicks>, removes voice from one or more nicks.");
    } else {

        string chan;
        istringstream ss(params);
        ss >> chan;

        std::string modeline = assignModes('-', 'v', ss);

        conn->sendMode(chan + " " + modeline);
    }
}

/*
void Exec(ServerConnection *conn, const string& params)
{
    string::size_type pos1 = params.find_first_of(" ");
    string param = params.substr(0, pos1);
    string rest = params.substr(pos1 + 1);

    if (param == "-o") {
        FILE* f = popen(rest.c_str(), "r");

        char buf[4028];

        fread(buf, 1, 4028, f);

        std::cout << "output: \n" << buf << std::endl;
        FE::emit(FE::get(SERVMSG) << buf, FE::CURRENT, conn);

    } else if (!params.empty()) {
        FILE* f = popen(rest.c_str(), "r");

        char buf[4028];

        fread(buf, 1, 4028, f);

        std::cout << "output: \n" << buf << std::endl;
        FE::emit(FE::get(SERVMSG) << buf, FE::CURRENT, conn);

        string str(buf);
    } else {
       throw CommandException("/EXEC [-o] <command>, execute a command, if -o is used, output to channel.");
    }
}
*/

void commands(ServerConnection *conn, const string& params)
{
    string cmdss;
    for (int i = 0; cmds[i].cmd != 0; ++i) {
        cmdss += " \00311[\0030";
        cmdss += cmds[i].cmd;
        cmdss += "\00311]";
    }
    FE::emit(FE::get(SERVMSG) << cmdss, FE::CURRENT, conn);
}

bool commandCompletion(const string& word, string& str)
{
    string lcword = word;
    lcword = Util::lower(lcword);
    for (int i = 0; cmds[i].cmd != 0; ++i) {
        string lccmd = cmds[i].cmd;
        lccmd = Util::lower(lccmd);
        if (lccmd.length() > lcword.length()) {
            if (lcword == lccmd.substr(0, lcword.length())) {
                str = cmds[i].cmd;
                return true;
            }
        }
    }
    return false;
}

std::string assignModes(char sign, char mode, istringstream& ss)
{
    string modes;
    modes += sign;

    string nicks;

    string nick;
    while (ss >> nick)
    {
        modes += mode;
        nicks += nick + " ";
    }

    return modes + " " + nicks;
}

}
