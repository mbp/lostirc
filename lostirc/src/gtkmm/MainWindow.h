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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtk--/main.h>
#include <gtk--/window.h>
#include <gtk--/box.h>
#include <gdk/gdkkeysyms.h>
#include <vector>
#include <ConfigHandler.h>
#include <ServerConnection.h>
#include <InOut.h>
#include "MainNotebook.h"

class MainWindow : public Gtk::Window
{
public:
    MainWindow();

    gint delete_event_impl(GdkEventAny*) { quit(); }

    gint on_key_press_event(GdkEventKey* e);
    virtual gint on_key_press_event_impl(GdkEventKey* e) { }
    bool isAway;

private:
    // Events
    void onMsg(const string& to, const string& from, const string& msg, ServerConnection *conn);
    void onCTCP(const string& command, const string& nick, ServerConnection *conn);
    void onAction(const string& to, const string& from, const string& msg, ServerConnection *conn);
    void onServMsg(const string& to, const string& from, const string& msg, ServerConnection *conn);
    void onGenericError(const string& error, ServerConnection *conn);
    void onUnknownMessage(const string& line, ServerConnection *conn);
    void onServNumeric(int n, const string& to, const string& from, const string& msg, ServerConnection *conn);
    void onJoin(const string& nick, const string& chan, ServerConnection *conn);
    void onPart(const string& nick, const string& chan, ServerConnection *conn);
    void onQuit(const string& nick, const string& chan, ServerConnection *conn);
    void onWhois(const string& from, const string& to, const string& rest, ServerConnection *conn);
    void onNick(const string& from, const string& to, ServerConnection *conn);
    void onNotice(const string& from, const string& to, const string& msg, ServerConnection *conn);
    void onKick(const string& from, const string& chan, const string& kicker, const string& msg,  ServerConnection *conn);
    void onNames(const string& chan, const vector<vector<string> >& users, ServerConnection *conn);
    void onTopic(const string& nick, const string& chan, const string& topic , ServerConnection *conn);
    void onTopicTime(const string& nick, const string& chan, const string& time , ServerConnection *conn);
    void onMode(const string& nick, const string& chan, const string& topic, ServerConnection *conn);
    void onCMode(const string& nick, const string& chan, bool, const string& modes, ServerConnection *conn);
    void onCUMode(const string& nick, const string& chan, const vector<vector<string> >& users, ServerConnection *conn);
    void onAway(const string& from, const string& param, const string& rest , ServerConnection *conn);
    void onSelfaway(const string& rest , ServerConnection *conn);
    void onNctcp(const string& from, const string& to, const string& msg, ServerConnection *conn);
    void onWallops(const string& from, const string& rest, ServerConnection *conn);
    void onErrhandler(const string& from, const string& to, const string& rest, ServerConnection *conn);
    void onBanlist(const string& chan, const string& banmask, const string& owner, ServerConnection *conn);

    void newServer();
    void quit();
    Gtk::Text::Context setColor(const string& col, Gtk::Text *t);

    MainNotebook* _nb;
    Gtk::VBox* _vbox1;

    /* Our 'In and Out' object, the backend to the client, controlling the
     * serverconnections and stuff.. but NOT the GUI. */
    InOut *_io;
    ConfigHandler _cfg;

};
#endif
