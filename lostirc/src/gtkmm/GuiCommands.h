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

#ifndef GUICOMMANDS_H
#define GUICOMMANDS_H

#include <Commands.h>
#include <Utils.h>
#include <vector>
#include "MainWindow.h"

class GuiCommands : public Commands
{
public:
    static bool send(ServerConnection *conn, string cmd, const string &params);
    static bool Query(ServerConnection *conn, const string& params);
    static bool Me(ServerConnection *conn, const string& params);
    static bool SetFont(ServerConnection *conn, const string& params);
    static bool NewServer(ServerConnection *conn, const string& params);
    static bool commands(ServerConnection *conn, const string& params);

    static MainWindow *mw;

};
#endif
