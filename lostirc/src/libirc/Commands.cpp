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

#include <cstdio>
#include <sstream>
#include "Commands.h"
#include "ServerConnection.h"
#include "Utils.h"
#include "LostIRCApp.h"

using Glib::ustring;
using std::stringstream;
using std::istringstream;

const struct UserCommands cmds[] = {
    { "SERVER",   Commands::Server,     false },
    { "DISCONNECT", Commands::Disconnect,     false },
    { "JOIN",     Commands::Join,       true },
    { "WHOIS",    Commands::Whois,      true },
    { "PART",     Commands::Part,       true },
    { "QUIT",     Commands::Quit,       true },
    { "NICK",     Commands::Nick,       false },
    { "KICK",     Commands::Kick,       true },
    { "NAMES",    Commands::Names,      true },
    { "MODE",     Commands::Mode,       true },
    { "CTCP",     Commands::Ctcp,       true },
    { "AWAY",     Commands::Away,       true },
    { "AWAYALL",  Commands::Awayall,    true },
    { "INVITE",   Commands::Invite,     true },
    { "TOPIC",    Commands::Topic,      true },
    { "NOTICE",   Commands::Notice,     true },
    { "BANLIST",  Commands::Banlist,    true },
    { "MSG",      Commands::Msg,        true },
    { "ME",       Commands::Me,         true },
    { "WHO",      Commands::Who,        true },
    { "LIST",     Commands::List,       true },
    { "SET",      Commands::Set,        false },
    { "QUOTE",    Commands::Quote,      true },
    //{ "EXEC",     Commands::Exec,       false },
    { "OPER",     Commands::Oper,       true },
    { "KILL",     Commands::Kill,       true },
    { "WALLOPS",  Commands::Wallops,    true },
    { "DCC",      Commands::DCC,        true },
    { "ADMIN",    Commands::Admin,      true },
    { "WHOWAS",   Commands::Whowas,     true },
    { "OP",       Commands::Op,         true },
    { "DEOP",     Commands::Deop,       true },
    { "VOICE",    Commands::Voice,      true },
    { "DEVOICE",  Commands::Devoice,    true },
    { "EXIT",     Commands::Exit,       false },
    { 0,        0,                      false }
};


namespace Commands {
void send(ServerConnection *conn, ustring cmd, const ustring& params) {

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

void Join(ServerConnection *conn, const ustring& params)
{
    if (params.length() == 0) {
        throw CommandException("/JOIN <channel>, join a channel");
    } else {
        conn->sendJoin(params);
    }
}

void Part(ServerConnection *conn, const ustring& params)
{
    if (params.length() == 0) {
        throw CommandException("/PART <channel> [msg], part a channel - optional with a part message");
    } else {
        ustring::size_type pos1 = params.find_first_of(" ");
        ustring chan = params.substr(0, pos1);
        ustring msg;
        if (pos1 != ustring::npos)
              msg = params.substr(pos1 + 1);

        conn->sendPart(chan, msg);
    }
}

void Quit(ServerConnection *conn, const ustring& params)
{
    conn->sendQuit(params);
    conn->disconnect();
}

void Kick(ServerConnection *conn, const ustring& params)
{
    ustring chan, nick, msg;
    ustring::size_type pos1 = params.find_first_of(" ");
    chan = params.substr(0, pos1);
    if (pos1 != ustring::npos) {
        ustring::size_type pos2 = params.find_first_of(" ", pos1 + 1);

        nick = params.substr(pos1 + 1, (pos2 - 1) - pos1);

        if (pos2 != ustring::npos) {
            msg = params.substr(pos2 + 1);
        }
    }

    if (params.empty() || chan.empty() || nick.empty()) {
        throw CommandException("/KICK <channel> <nick> [msg], kick a user from a channel.");
    } else {
        conn->sendKick(chan, nick, msg);
    }
}

void Server(ServerConnection *conn, const ustring& params)
{
    if (params.empty()) {
        throw CommandException("/SERVER <host/ip> [port] [password], connect to an IRC server");
    } else {
        ustring host, port, password;
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

void Disconnect(ServerConnection *conn, const ustring& params)
{
    conn->removeReconnectTimer();
    conn->disconnect();
}

void Nick(ServerConnection *conn, const ustring& params)
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

void Whois(ServerConnection *conn, const ustring& params)
{
    if (params.empty()) {
        throw CommandException("/WHOIS <nick>, whois nick.");
    } else {
        conn->sendWhois(params);
    }
}

void Mode(ServerConnection *conn, const ustring& params)
{
    if (params.empty()) {
        throw CommandException("/MODE <channel> <modes>, set modes for a channel.");
    } else {
        conn->sendMode(params);
    }
}

void Set(ServerConnection *conn, const ustring& params)
{
    ustring::size_type pos1 = params.find_first_of(" ");
    ustring key = params.substr(0, pos1);
    ustring value;
    if (pos1 != ustring::npos)
          value = params.substr(pos1 + 1);

    App->options.set(key, value);
}

void Ctcp(ServerConnection *conn, const ustring& params)
{
    ustring::size_type pos1 = params.find_first_of(" ");
    ustring to = params.substr(0, pos1);
    ustring action;
    if (pos1 != ustring::npos)
          action = params.substr(pos1 + 1);

    if (action.empty()) {
        throw CommandException("/CTCP <nick> <message>, sends a CTCP message to a user");
    } else {
        action = Util::upper(action);

        conn->sendCtcp(to, action);
    }
}

void Away(ServerConnection *conn, const ustring& params)
{
    conn->sendAway(params);
}

void Awayall(ServerConnection *conn, const ustring& params)
{
    std::vector<ServerConnection*> servers = App->getServers();
    std::vector<ServerConnection*>::iterator i;

    for (i = servers.begin(); i != servers.end(); ++i)
    {
        (*i)->sendAway(params);
    }
}

void Banlist(ServerConnection *conn, const ustring& chan)
{
    if (chan.empty()) {
        throw CommandException("/BANLIST <channel>, see banlist for channel.");
    } else {
        conn->sendBanlist(chan);
    }
}

void Invite(ServerConnection *conn, const ustring& params)
{
    ustring to, chan;
    stringstream ss(params);
    ss >> to;
    ss >> chan;

    if (chan.empty()) {
        throw CommandException("/INVITE <nick> <channel>, invites someone to a channel.");
    } else {
        conn->sendInvite(to, chan);
    }
}

void Topic(ServerConnection *conn, const ustring& params)
{
    ustring::size_type pos1 = params.find_first_of(" ");
    ustring chan = params.substr(0, pos1);
    ustring topic;
    if (pos1 != ustring::npos)
          topic = params.substr(pos1 + 1);

    if (chan.empty()) {
        throw CommandException("/TOPIC <channel> [topic], view or change topic for a channel.");
    } else {
        conn->sendTopic(chan, topic);
    }
}

void Msg(ServerConnection *conn, const ustring& params)
{
    ustring::size_type pos1 = params.find_first_of(" ");
    ustring to = params.substr(0, pos1);
    ustring msg;
    if (pos1 != ustring::npos)
          msg = params.substr(pos1 + 1);

    if (msg.empty()) {
        throw CommandException("/MSG <nick/channel> <message>, sends a normal message.");
    } else {
        conn->sendMsg(to, msg, false);
        ustring sendgui = "Message to " + to + ":";
        FE::emit(FE::get(CLIENTMSG) << sendgui << msg, FE::CURRENT, conn);
    }
}

void Notice(ServerConnection *conn, const ustring& params)
{
    ustring::size_type pos1 = params.find_first_of(" ");
    ustring to = params.substr(0, pos1);
    ustring msg;
    if (pos1 != ustring::npos)
          msg = params.substr(pos1 + 1);

    if (msg.empty()) {
        throw CommandException("/NOTICE <nick/channel> <message>, sends a notice.");
    } else {
        conn->sendNotice(to, msg);
        ustring sendgui = "Notice to " + to + ":";
        FE::emit(FE::get(CLIENTMSG) << sendgui << msg, FE::CURRENT, conn);
    }
}

void Me(ServerConnection *conn, const ustring& params)
{
    ustring::size_type pos1 = params.find_first_of(" ");
    ustring to = params.substr(0, pos1);
    ustring msg = params.substr(pos1 + 1);

    if (msg.empty()) {
        throw CommandException("/ME <message>, sends the action to the current channel.");
    } else {
        conn->sendMe(to, msg);
    }
}

void Who(ServerConnection *conn, const ustring& params)
{
    if (params.empty()) {
        throw CommandException("/WHO <mask> [o], search for mask on network, if o is supplied, only search for oppers.");
    } else {
        conn->sendWho(params);
    }
}

void List(ServerConnection *conn, const ustring& params)
{
    //throw CommandException("/LIST [channels] [server], list channels on a network, if a channel is supplied, only list that channel. If a server is supplied, forward the request to that IRC server.");
    conn->sendList(params);
}

void Quote(ServerConnection *conn, const ustring& params)
{
    if (params.empty()) {
        throw CommandException("/QUOTE <text>, send raw text to server.");
    } else {
        conn->sendRaw(params);
    }
}

void Names(ServerConnection *conn, const ustring& params)
{
    if (params.empty()) {
        throw CommandException("/NAMES <channel>, see who's on a channel.");
    } else {
        conn->sendNames(params);
    }
}

void Oper(ServerConnection* conn, const ustring& params)
{
    if (params.empty()) {
       throw CommandException("/OPER <login> <password>, oper up.");
    } else {
       ustring login, password;
       istringstream ss(params);
       ss >> login;
       ss >> password;

       if (login.empty() || password.empty())
           throw CommandException( "/OPER <login> <password>, oper up.");

       conn->sendOper(login, password);
    }
}

void Kill(ServerConnection* conn, const ustring& params)
{
    ustring::size_type pos1 = params.find_first_of(" ");
    ustring nick = params.substr(0, pos1);
    ustring reason;
    if (pos1 != ustring::npos)
          reason = params.substr(pos1 + 1);

    if (nick.empty()) {
       throw CommandException("/KILL <user> [reason], kill a user from the network.");
    } else {

       conn->sendKill(nick, reason);
    }
}

void Wallops(ServerConnection* conn, const ustring& params)
{
    if (params.empty()) {
       throw CommandException("/WALLOPS <message>, send wallop message.");
    } else {

       conn->sendWallops(params);
    }
}

void DCC(ServerConnection* conn, const ustring& params)
{
    if (params.empty()) {
       throw CommandException("/DCC <actions>, perform a DCC action.");
    } else {
       ustring action, secondparam;
       istringstream ss(params);
       ss >> action;
       ss >> secondparam;

       action = Util::upper(action);
       if (action == "RECEIVE") {
           if (!App->getDcc().do_dcc(Util::stoi(secondparam)))
                 throw CommandException("No DCC with that number");
       } else if (action == "SEND") {
           ustring filename;
           ss >> filename;

           App->getDcc().addDccSendOut(filename, secondparam, conn);
       }
    }
}

void Admin(ServerConnection* conn, const ustring& params)
{
    conn->sendAdmin(params);
}

void Whowas(ServerConnection* conn, const ustring& params)
{
    conn->sendWhowas(params);
}

void Op(ServerConnection* conn, const ustring& params)
{
    if (params.empty()) {
        throw CommandException("/OP <channel> <nicks>, give operator status to one or more nicks.");
    } else {

        ustring chan;
        istringstream ss(params);
        ss >> chan;

        Glib::ustring modeline = assignModes('+', 'o', ss);

        conn->sendMode(chan + " " + modeline);
    }
}

void Deop(ServerConnection* conn, const ustring& params)
{
    if (params.empty()) {
        throw CommandException("/DEOP <channel> <nicks>, remove operator status from one or more nicks.");
    } else {

        ustring chan;
        istringstream ss(params);
        ss >> chan;

        Glib::ustring modeline = assignModes('-', 'o', ss);

        conn->sendMode(chan + " " + modeline);
    }
}

void Voice(ServerConnection* conn, const ustring& params)
{
    if (params.empty()) {
        throw CommandException("/VOICE <channel> <nicks>, gives voice to one or more nicks.");
    } else {

        ustring chan;
        istringstream ss(params);
        ss >> chan;

        Glib::ustring modeline = assignModes('+', 'v', ss);

        conn->sendMode(chan + " " + modeline);
    }
}

void Devoice(ServerConnection* conn, const ustring& params)
{
    if (params.empty()) {
        throw CommandException("/DEVOICE <channel> <nicks>, removes voice from one or more nicks.");
    } else {

        ustring chan;
        istringstream ss(params);
        ss >> chan;

        Glib::ustring modeline = assignModes('-', 'v', ss);

        conn->sendMode(chan + " " + modeline);
    }
}

void Exit(ServerConnection* conn, const ustring& params)
{
    const std::vector<ServerConnection*> servers = App->getServers();

    std::vector<ServerConnection*>::const_iterator i;
    for (i = servers.begin(); i != servers.end(); ++i)
    {
        Quit(*i, params);
    }
}

/*
void Exec(ServerConnection *conn, const ustring& params)
{
    ustring::size_type pos1 = params.find_first_of(" ");
    ustring param = params.substr(0, pos1);
    ustring rest = params.substr(pos1 + 1);

    if (param == "-o") {
        FILE* f = popen(rest.c_str(), "r");

        char buf[4028];

        fread(buf, 1, 4028, f);

        std::cout << "output: \n" << buf << std::endl;
        FE::emit(FE::get(CLIENTMSG) << buf, FE::CURRENT, conn);

    } else if (!params.empty()) {
        FILE* f = popen(rest.c_str(), "r");

        char buf[4028];

        fread(buf, 1, 4028, f);

        std::cout << "output: \n" << buf << std::endl;
        FE::emit(FE::get(CLIENTMSG) << buf, FE::CURRENT, conn);

        ustring str(buf);
    } else {
       throw CommandException("/EXEC [-o] <command>, execute a command, if -o is used, output to channel.");
    }
}
*/

void getCommands(std::set<Glib::ustring>& commands)
{
    for (int i = 0; cmds[i].cmd != 0; ++i)
        commands.insert(cmds[i].cmd);
}

Glib::ustring assignModes(char sign, char mode, istringstream& ss)
{
    ustring modes;
    modes += sign;

    ustring nicks;

    ustring nick;
    while (ss >> nick)
    {
        modes += mode;
        nicks += nick + " ";
    }

    return modes + " " + nicks;
}

}
