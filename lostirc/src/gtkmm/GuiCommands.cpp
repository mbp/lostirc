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

#include <set>
#include <Utils.h>
#include "GuiCommands.h"
#include "Tab.h"

using std::string;

const struct UserCommands guicmds[] = {
    { "QUERY",     GuiCommands::Query,    false },
    { "CLEAR",     GuiCommands::Clear,    false },
    { "CLEARALL",  GuiCommands::ClearAll, false },
    { "SETFONT",   GuiCommands::SetFont,  false },
    { "NEWSERVER", GuiCommands::NewServer, false },
    { "ME",        GuiCommands::Me,       true },
    { "PART",      GuiCommands::Part,     true },
    { "TOPIC",     GuiCommands::Topic,    true },
    { "KICK",      GuiCommands::Kick,     true },
    { "KICK",      GuiCommands::Kick,     true },
    { "BANLIST",   GuiCommands::Banlist,  true },
    { "OP",        GuiCommands::Op,       true },
    { "DEOP",      GuiCommands::Deop,     true },
    { "VOICE",     GuiCommands::Voice,    true },
    { "DEVOICE",   GuiCommands::Devoice,  true },
    { "EXIT",      GuiCommands::Exit,     true },
    { "COMMANDS",  GuiCommands::displayCommands, false },
    { 0,        0, false                        }
};

namespace GuiCommands {

void send(ServerConnection *conn, string cmd, const string& params)
{
    for (int i = 0; guicmds[i].cmd != 0; ++i) {
        if (guicmds[i].cmd == cmd) {
            if (!conn->Session.isConnected && guicmds[i].reqConnected) {
                throw CommandException("Must be connected.");
            }
            guicmds[i].function(conn, params);
            return;
        }
    }

    Commands::send(conn, cmd, params);
}

void Query(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/QUERY <nick>, start a query(tab) with a user");
    } else {
        AppWin->getNotebook().addQueryTab(params, conn);
    }
}

void Me(ServerConnection *conn, const string& params)
{
    string to = AppWin->getNotebook().getCurrent()->getLabel()->get_text();
    string param = to + " " + params;
    return Commands::Me(conn, param);
}

void SetFont(ServerConnection *conn, const string& params)
{
    // FIXME AppWin->getNotebook().setFont();
}

void Clear(ServerConnection *conn, const string& params)
{
    AppWin->getNotebook().getCurrent()->clearText();
}

void ClearAll(ServerConnection *conn, const string& params)
{
    AppWin->getNotebook().clearAll();
}

void NewServer(ServerConnection *conn, const string& params)
{
    AppWin->newServer();
}

void Part(ServerConnection *conn, const string& params)
{
    Commands::Part(conn, AppWin->getNotebook().getCurrent()->getLabel()->get_text() + " " + params);
}

void Topic(ServerConnection *conn, const string& params)
{
    Commands::Topic(conn, AppWin->getNotebook().getCurrent()->getLabel()->get_text() + " " + params);
}

void Kick(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/KICK <nick>, kick a user from a channel.");

    } else {
        Commands::Kick(conn, AppWin->getNotebook().getCurrent()->getLabel()->get_text() + " " + params);
    }
}

void Banlist(ServerConnection *conn, const string& params)
{
    Commands::Banlist(conn, AppWin->getNotebook().getCurrent()->getLabel()->get_text());
}

void Op(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/OP <nicks>, ops one or more users in the current channel.");

    } else {
        Commands::Op(conn, AppWin->getNotebook().getCurrent()->getLabel()->get_text() + " " + params);
    }
}

void Deop(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/DEOP <nicks>, deops one or more users in the current channel.");

    } else {
        Commands::Deop(conn, AppWin->getNotebook().getCurrent()->getLabel()->get_text() + " " + params);
    }
}

void Voice(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/VOICE <nicks>, voices one or more users in the current channel.");

    } else {
        Commands::Voice(conn, AppWin->getNotebook().getCurrent()->getLabel()->get_text() + " " + params);
    }
}

void Devoice(ServerConnection *conn, const string& params)
{
    if (params.empty()) {
        throw CommandException("/DEVOICE <nicks>, devoices one or more users in the current channel.");

    } else {
        Commands::Devoice(conn, AppWin->getNotebook().getCurrent()->getLabel()->get_text() + " " + params);
    }
}

void Exit(ServerConnection *conn, const string& params)
{
    Commands::Exit(conn, params);
    AppWin->hide();
}

void displayCommands(ServerConnection *conn, const string& params)
{
    std::vector<Glib::ustring> commands = getCommands();

    std::vector<Glib::ustring>::const_iterator i;

    Glib::ustring cmds;
    for (i = commands.begin(); i != commands.end(); ++i)
    {
        cmds += " \00311[\0030";
        cmds += *i;
        cmds += "\00311]";
    }

    FE::emit(FE::get(SERVMSG) << cmds, FE::CURRENT, conn);
}

std::vector<Glib::ustring> getCommands()
{
    std::set<string> commands;

    for (int i = 0; guicmds[i].cmd != 0; ++i) 
        commands.insert(guicmds[i].cmd);

    Commands::getCommands(commands);

    std::vector<Glib::ustring> cmds;
    for (std::set<string>::const_iterator i = commands.begin(); i != commands.end(); ++i)
          cmds.push_back(*i);

    return cmds;
}
}
