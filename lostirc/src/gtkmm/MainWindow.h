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

#include <gtk--/main.h>
#include <gtk--/window.h>
#include <gtk--/box.h>
#include <gdk/gdkkeysyms.h>
#include <vector>
#include <ServerConnection.h>
#include <LostIRCApp.h>
#include "MainNotebook.h"

class MainWindow : public Gtk::Window
{
    MainNotebook* _nb;

    /* Our LostIRCApp object, the backend to the client, controlling the
     * serverconnections and stuff.. but NOT the GUI. */
    LostIRCApp *_app;

public:
    MainWindow();
    ~MainWindow();

    gint delete_event_impl(GdkEventAny*) { Gtk::Main::quit(); return 0; }

    gint on_key_press_event(GdkEventKey* e);
    virtual gint on_key_press_event_impl(GdkEventKey* e) { }
    MainNotebook* getNotebook() { return _nb; }
    LostIRCApp* getApp() { return _app; }
    void newServer();

private:
    // Events
    void onDisplayMessage(const string& msg, const string& to, ServerConnection *conn);
    void onJoin(const string& nick, const string& chan, ServerConnection *conn);
    void onPart(const string& nick, const string& chan, ServerConnection *conn);
    void onQuit(const string& nick, const string& chan, ServerConnection *conn);
    void onNick(const string& from, const string& to, ServerConnection *conn);
    void onNotice(const string& from, const string& to, const string& msg, ServerConnection *conn);
    void onKick(const string& from, const string& chan, const string& kicker, const string& msg,  ServerConnection *conn);
    void onNames(Channel& c, ServerConnection *conn);
    void onMode(const string& nick, const string& chan, const string& topic, ServerConnection *conn);
    void onCMode(const string& nick, const string& chan, char, const string& modes, ServerConnection *conn);
    void onCUMode(const string& nick, const string& chan, const map<string, IRC::UserMode>& users, ServerConnection *conn);
    void onHighlight(const string& to, ServerConnection *conn);
    void onAway(bool away, ServerConnection *conn);
    void onNewTab(ServerConnection *conn);

};
#endif
