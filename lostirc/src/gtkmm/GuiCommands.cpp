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

#include <vector>
#include <Utils.h>
#include "GuiCommands.h"
#include "Tab.h"

using std::string;

struct UserCommands guicmds[] = {
    { "QUERY",     GuiCommands::Query,    0 },
    { "ME",        GuiCommands::Me,       1 },
    { "PART",      GuiCommands::Part,     1 },
    { "TOPIC",     GuiCommands::Topic,    1 },
    { "SETFONT",   GuiCommands::SetFont,  0 },
    { "NEWSERVER", GuiCommands::NewServer, 0 },
    { "COMMANDS",  GuiCommands::commands, 0 },
    { 0,        0, 0                        }
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
    if (params.length() == 0) {
        throw CommandException("/QUERY <nick>, start a query(tab) with a user");
    } else {
        AppWin->getNotebook()->addQueryTab(params, conn);
    }
}

void Me(ServerConnection *conn, const string& params)
{
    string to = AppWin->getNotebook()->getCurrent()->getLabel()->get_text();
    string param = to + " " + params;
    return Commands::Me(conn, param);
}

void SetFont(ServerConnection *conn, const string& params)
{
    AppWin->getNotebook()->setFont();
}

void NewServer(ServerConnection *conn, const string& params)
{
    AppWin->newServer();
}

void Part(ServerConnection *conn, const string& params)
{
    Commands::Part(conn, AppWin->getNotebook()->getCurrent()->getLabel()->get_text() + " " + params);
}

void Topic(ServerConnection *conn, const string& params)
{
    Commands::Topic(conn, AppWin->getNotebook()->getCurrent()->getLabel()->get_text() + " " + params);
}

void commands(ServerConnection *conn, const string& params)
{
    string cmds;
    for (int i = 0; guicmds[i].cmd != 0; ++i) {
        cmds += " \00311[\0030";
        cmds += guicmds[i].cmd;
        cmds += "\00311]";
    }
    FE::emit(FE::get(SERVMSG) << cmds, FE::CURRENT, conn);
    Commands::commands(conn, params);
}

bool commandCompletion(const string& word, string& str)
{
    string lcword = Util::lower(word);
    for (int i = 0; guicmds[i].cmd != 0; ++i) {
        string lccmd = guicmds[i].cmd;
        lccmd = Util::lower(lccmd);
        if (lccmd.length() > lcword.length()) {
            if (lcword == lccmd.substr(0, lcword.length())) {
                str = guicmds[i].cmd;
                return true;
            }
        }
    }
    return Commands::commandCompletion(word, str);
}
}
