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

#include "GuiCommands.h"
#include "Tab.h"


struct UserCommands guicmds[] = {
    { "QUERY",     GuiCommands::Query,    0 },
    { "ME",        GuiCommands::Me,       1 },
    { "SETFONT",   GuiCommands::SetFont,  0 },
    { 0,        0, 0                        }
};


bool GuiCommands::send(ServerConnection *conn, string cmd, const string& params)
{
    for (int i = 0; guicmds[i].cmd != 0; ++i) {
        if (guicmds[i].cmd == Utils::toupper(cmd)) {
            if (!conn->Session.isConnected && guicmds[i].reqConnected) {
                Commands::error = "Must be connected.";
                return false;
            }
            return guicmds[i].function(conn, params);
        }
    }

    return Commands::send(conn, cmd, params);
}

bool GuiCommands::Query(ServerConnection *conn, const string& params)
{
    if (params.length() == 0) {
        Commands::error = "Missing name, syntax is /QUERY <nick>";
        return false;
    } else {
        nb->addQueryTab(params, conn);
    }

}

bool GuiCommands::Me(ServerConnection *conn, const string& params)
{
    string to = nb->getCurrent()->getLabel()->get_text();
    string param = to + " " + params;
    return Commands::Me(conn, param);

}

bool GuiCommands::SetFont(ServerConnection *conn, const string& params)
{
    nb->setFont();
}

MainNotebook* GuiCommands::nb;
