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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>
#include <gtkmm/main.h>
#include <glibmm/main.h>
#include <gtkmm/window.h>
#include <gtk/gtksignal.h>
#include <gdk/gdkkeysyms.h>
#include <ServerConnection.h>
#include <LostIRCApp.h>
#include <FrontEnd.h>
#include "MainNotebook.h"
#include "StatusBar.h"

Glib::ustring convert_to_utf8(const std::string& str);


class MainWindow : public Gtk::Window, public FrontEnd
{
    virtual bool on_key_press_event(GdkEventKey* e);

    LostIRCApp app;
    MainNotebook notebook;

public:
    MainWindow(bool autoconnect = 0);
    virtual ~MainWindow();

    MainNotebook& getNotebook() { return notebook; }
    Tab* newServer();

    // Methods implemented for the abstract base class 'FrontEnd' 
    void displayMessage(const std::string& msg, FE::Destination d, bool shouldHighlight = true);
    void displayMessage(const std::string& msg, FE::Destination d, ServerConnection *conn, bool shouldHighlight = true);
    void displayMessage(const std::string& msg, ChannelBase& to, ServerConnection *conn, bool shouldHighlight = true);
    void join(const std::string& nick, Channel& chan, ServerConnection *conn);
    void part(const std::string& nick, Channel& chan, ServerConnection *conn);
    void kick(const std::string& from, Channel& chan, const std::string& kicker, const std::string& msg,  ServerConnection *conn);
    void quit(const std::string& nick, std::vector<ChannelBase*> chans, ServerConnection *conn);
    void nick(const std::string& from, const std::string& to, std::vector<ChannelBase*> chans, ServerConnection *conn);
    void CUMode(const std::string& nick, Channel& chan, const std::vector<User>& users, ServerConnection *conn);
    void names(Channel& c, ServerConnection *conn);
    void highlight(ChannelBase& chan, ServerConnection *conn);
    void away(bool away, ServerConnection *conn);
    void connected(ServerConnection *conn);
    void newTab(ServerConnection *conn);
    void disconnected(ServerConnection *conn);

    StatusBar statusbar;
};

extern MainWindow* AppWin;

#endif
