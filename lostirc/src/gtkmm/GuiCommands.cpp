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

#include "GuiCommands.h"
#include "Tab.h"
#include <Utils.h>
#include <vector>

using std::string;

struct UserCommands guicmds[] = {
    { "QUERY",     GuiCommands::Query,    0 },
    { "ME",        GuiCommands::Me,       1 },
    { "SETFONT",   GuiCommands::SetFont,  0 },
    { "NEWSERVER", GuiCommands::NewServer, 0 },
    { "COMMANDS",  GuiCommands::commands, 0 },
    { 0,        0, 0                        }
};

void GuiCommands::send(ServerConnection *conn, string cmd, const string& params)
{
    for (int i = 0; guicmds[i].cmd != 0; ++i) {
        if (guicmds[i].cmd == Util::upper(cmd)) {
            if (!conn->Session.isConnected && guicmds[i].reqConnected) {
                throw CommandException("Must be connected.");
            }
            guicmds[i].function(conn, params);
            return;
        }
    }

    Commands::send(conn, cmd, params);
}

void GuiCommands::Query(ServerConnection *conn, const string& params)
{
    if (params.length() == 0) {
        throw CommandException("/QUERY <nick>, start a query(tab) with a user");
    } else {
        mw->getNotebook()->addQueryTab(params, conn);
    }
}

void GuiCommands::Me(ServerConnection *conn, const string& params)
{
    string to = mw->getNotebook()->getCurrent()->getLabel()->get_text();
    string param = to + " " + params;
    return Commands::Me(conn, param);
}

void GuiCommands::SetFont(ServerConnection *conn, const string& params)
{
    mw->getNotebook()->setFont();
}

void GuiCommands::NewServer(ServerConnection *conn, const string& params)
{
    mw->newServer();
}

void GuiCommands::commands(ServerConnection *conn, const string& params)
{
    string cmds;
    for (int i = 0; guicmds[i].cmd != 0; ++i) {
        cmds += " \00311[\0030";
        cmds += guicmds[i].cmd;
        cmds += "\00311]";
    }
    Commands::app->getEvts()->emit(Commands::app->getEvts()->get(SERVMSG) << cmds, conn);
    Commands::commands(conn, params);
}

MainWindow* GuiCommands::mw;
