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

#ifndef GUICOMMANDS_H
#define GUICOMMANDS_H

#include <Commands.h>
#include "MainWindow.h"

class GuiCommands : public Commands
{
public:
    static void send(ServerConnection *conn, std::string cmd, const std::string &params);
    static void Query(ServerConnection *conn, const std::string& params);
    static void Me(ServerConnection *conn, const std::string& params);
    static void SetFont(ServerConnection *conn, const std::string& params);
    static void NewServer(ServerConnection *conn, const std::string& params);
    static void commands(ServerConnection *conn, const std::string& params);
    static bool commandCompletion(const std::string& word, std::string& str);

    static MainWindow *mw;

};
#endif
