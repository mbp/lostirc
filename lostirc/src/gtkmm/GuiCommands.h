/*
 * Copyright (C) 2002-2004 Morten Brix Pedersen <morten@wtf.dk>
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

#include <vector>
#include <Commands.h>
#include <glibmm/ustring.h>
#include "MainWindow.h"

namespace GuiCommands
{

    void send(ServerConnection *conn, Glib::ustring cmd, const Glib::ustring &params);
    void Query(ServerConnection *conn, const Glib::ustring& params);
    void Clear(ServerConnection *conn, const Glib::ustring& params);
    void ClearAll(ServerConnection *conn, const Glib::ustring& params);
    void Me(ServerConnection *conn, const Glib::ustring& params);
    void Part(ServerConnection *conn, const Glib::ustring& params);
    void Topic(ServerConnection *conn, const Glib::ustring& params);
    void Kick(ServerConnection *conn, const Glib::ustring& params);
    void Banlist(ServerConnection *conn, const Glib::ustring& params);
    void Op(ServerConnection *conn, const Glib::ustring& params);
    void Deop(ServerConnection *conn, const Glib::ustring& params);
    void Voice(ServerConnection *conn, const Glib::ustring& params);
    void Devoice(ServerConnection *conn, const Glib::ustring& params);
    void Exit(ServerConnection *conn, const Glib::ustring& params);
    void NewServer(ServerConnection *conn, const Glib::ustring& params);
    void displayCommands(ServerConnection *conn, const Glib::ustring& params);
    void displayKeybindings(ServerConnection *conn, const Glib::ustring& params);
    std::vector<Glib::ustring> getCommands();

}
#endif
