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

#include <gtkmm/box.h>
#include <algorithm>
#include <functional>
#include "Tab.h"
#include "MainWindow.h"

using std::vector;
using std::string;

MainWindow* AppWin = 0;

MainWindow::MainWindow()
    : Gtk::Window(), app(this)
{
    AppWin = this;
    set_title("LostIRC "VERSION);

    int width = Util::stoi(app.getCfg().getOpt("window_width"));
    int height = Util::stoi(app.getCfg().getOpt("window_height"));
    if (width != 0 && height != 0) {
        set_default_size(width, height);
    } else {
        set_default_size(600, 400);
    }
    
    add(notebook);
    show_all();

    int num_of_servers = app.start();
    if (num_of_servers == 0) {
        // Construct initial tab
        Tab *tab = newServer();
        *tab << "\0037\nWelcome to LostIRC "VERSION"!\n\nYou use the client mainly by typing in commands and text in the entry-bar shown below.\n\nYou can connect to a server using:\n    \0038/SERVER <hostname>\n\n\0037Then join a channel:\n    \0038/JOIN <channel-name>\n\n\0037The rest of the commands is available with:\n    \0038/COMMANDS\0037.\n\n\0037Available keybindings:\n    \0038Alt + [1-9] - switch tabs from 1-9.\n    Alt + n - create new server tab.\n    Alt + c - close current tab.\n    Alt + p - open preferences.\n    Tab - nick-completion and command-completion.\n";
    }
}

MainWindow::~MainWindow()
{
    // Save the width and height of the windows
    int width, height;

    get_size(width, height);
    if (width && height) {
        app.getCfg().setOpt("window_width", width);
        app.getCfg().setOpt("window_height", height);
    }

    AppWin = 0;
}

void MainWindow::displayMessage(const string& msg, FE::Destination d, bool shouldHighlight)
{
    if (d == FE::CURRENT) {
        Tab *tab = notebook.getCurrent();

        *tab << msg;

        if (shouldHighlight)
              notebook.highlightActivity(tab);

    } else if (d == FE::ALL) {
        vector<Tab*> tabs;
        vector<Tab*>::const_iterator i;
        notebook.Tabs(tabs);

        for (i = tabs.begin(); i != tabs.end(); ++i) {
            *(*i) << msg;
            if (shouldHighlight)
                  notebook.highlightActivity(*i);
        }
    
    }
}

void MainWindow::displayMessage(const string& msg, FE::Destination d, ServerConnection *conn, bool shouldHighlight)
{
    if (d == FE::CURRENT) {
        Tab *tab = notebook.getCurrent(conn);

        *tab << msg;

        if (shouldHighlight)
              notebook.highlightActivity(tab);
    } else if (d == FE::ALL) {
        vector<Tab*> tabs;
        vector<Tab*>::const_iterator i;
        notebook.findTabs(conn, tabs);

        for (i = tabs.begin(); i != tabs.end(); ++i) {
            *(*i) << msg;
            if (shouldHighlight)
                  notebook.highlightActivity(*i);
        }
    }
}

void MainWindow::displayMessage(const string& msg, ChannelBase& chan, ServerConnection *conn, bool shouldHighlight)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);

    // if the channel doesn't exist, it's probably a query. (the channel is
    // created on join) - there is also a hack here to ensure that it's not
    // a channel
    if (!tab && chan.getName().at(0) != '#') {
        tab = notebook.addQueryTab(chan.getName(), conn);
    }

    if (tab) {
        *tab << msg;

        if (shouldHighlight)
              notebook.highlightActivity(tab);
    }
}

void MainWindow::join(const string& nick, Channel& chan, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);
    if (!tab) {
        tab = notebook.addChannelTab(chan.getName(), conn);
        return;
    }
    tab->insertUser(Glib::locale_to_utf8(nick));
}

void MainWindow::part(const string& nick, Channel& chan, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);
    if (tab) {
        if (nick == conn->Session.nick) {
            // It's us who's parting
            tab->setInActive();
        }
        tab->removeUser(Glib::locale_to_utf8(nick));
    }
}

void MainWindow::kick(const string& kicker, Channel& chan, const string& nick, const string& msg, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);
    if (nick == conn->Session.nick) {
        // It's us who's been kicked
        tab->setInActive();
    }
    tab->removeUser(Glib::locale_to_utf8(nick));
}


void MainWindow::quit(const string& nick, vector<ChannelBase*> chans, ServerConnection *conn)
{
    vector<ChannelBase*>::const_iterator i;

    for (i = chans.begin(); i != chans.end(); ++i) {
        if (Tab *tab = notebook.findTab((*i)->getName(), conn))
            tab->removeUser(Glib::locale_to_utf8(nick));
    }
}

void MainWindow::nick(const string& nick, const string& to, vector<ChannelBase*> chans, ServerConnection *conn)
{
    vector<ChannelBase*>::const_iterator i;

    for (i = chans.begin(); i != chans.end(); ++i) {
        if (Tab *tab = notebook.findTab((*i)->getName(), conn))
              tab->renameUser(Glib::locale_to_utf8(nick), Glib::locale_to_utf8(to));
    }
}

void MainWindow::CUMode(const string& nick, Channel& chan, const std::vector<User>& users, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);

    std::vector<User>::const_iterator i;
    for (i = users.begin(); i != users.end(); ++i) {
        tab->removeUser(Glib::locale_to_utf8(i->nick));
        tab->insertUser(Glib::locale_to_utf8(i->nick), i->getMode());
    }
}

void MainWindow::names(Channel& c, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(c.getName(), conn);

    std::vector<User*> users = c.getUsers();
    std::vector<User*>::const_iterator i;

    for (i = users.begin(); i != users.end(); ++i) {
        tab->insertUser(Glib::locale_to_utf8((*i)->nick), (*i)->getMode());
    }
}

void MainWindow::highlight(ChannelBase& chan, ServerConnection* conn)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);

    if (tab)
          notebook.highlightNick(tab);
}

void MainWindow::away(bool away, ServerConnection* conn)
{
    vector<Tab*> tabs;

    notebook.findTabs(conn, tabs);
    if (away) {
        std::for_each(tabs.begin(), tabs.end(), std::mem_fun(&Tab::setAway));
    } else {
        std::for_each(tabs.begin(), tabs.end(), std::mem_fun(&Tab::setUnAway));
    }
    notebook.updateTitle();
}

void MainWindow::disconnected(ServerConnection* conn)
{
    vector<Tab*> tabs;

    notebook.findTabs(conn, tabs);

    std::for_each(tabs.begin(), tabs.end(), std::mem_fun(&Tab::setInActive));
}

void MainWindow::newTab(ServerConnection *conn)
{
    string name = "server";
    conn->Session.servername = name;
    Tab *tab = notebook.addChannelTab(name, conn);
    notebook.show_all();

    // XXX: this is a hack for a "bug" in the gtkmm code which makes the
    // application crash in the start when no pages exists, even though we
    // added one above... doing set_current_page(0) will somehow add it fully.
    if (notebook.get_current_page() == -1) {
        notebook.set_current_page(0);
    }
    tab->setInActive();
}

Tab* MainWindow::newServer()
{
    string name = "server";
    ServerConnection *conn = app.newServer();
    conn->Session.servername = name;
    Tab *tab = notebook.addChannelTab(name, conn);
    tab->setInActive();
    return tab;
}

bool MainWindow::on_key_press_event(GdkEventKey* e)
{
    // Default keybindings. Still needs work.
    if ((e->keyval == GDK_0) && (e->state & GDK_MOD1_MASK)) {
        notebook.set_current_page(9);
    }
    else if ((e->keyval == GDK_1) && (e->state & GDK_MOD1_MASK)) {
        notebook.set_current_page(0);
    }
    else if ((e->keyval == GDK_2) && (e->state & GDK_MOD1_MASK)) {
        notebook.set_current_page(1);
    }
    else if ((e->keyval == GDK_3) && (e->state & GDK_MOD1_MASK)) {
        notebook.set_current_page(2);
    }
    else if ((e->keyval == GDK_4) && (e->state & GDK_MOD1_MASK)) {
        notebook.set_current_page(3);
    }
    else if ((e->keyval == GDK_5) && (e->state & GDK_MOD1_MASK)) {
        notebook.set_current_page(4);
    }
    else if ((e->keyval == GDK_6) && (e->state & GDK_MOD1_MASK)) {
        notebook.set_current_page(5);
    }
    else if ((e->keyval == GDK_7) && (e->state & GDK_MOD1_MASK)) {
        notebook.set_current_page(6);
    }
    else if ((e->keyval == GDK_8) && (e->state & GDK_MOD1_MASK)) {
        notebook.set_current_page(7);
    }
    else if ((e->keyval == GDK_9) && (e->state & GDK_MOD1_MASK)) {
        notebook.set_current_page(8);
    }
    else if ((e->keyval == GDK_c) && (e->state & GDK_MOD1_MASK)) {
        TabChannel *tab = dynamic_cast<TabChannel*>(notebook.getCurrent());
        if (tab && tab->getConn()->Session.isConnected && tab->isActive()) {
            // It's a channel, so we need to part it
            tab->getConn()->sendPart(tab->getLabel()->get_text(), "");
        } else {
            // Query
            notebook.getCurrent()->getConn()->removeChannel(notebook.getCurrent()->getLabel()->get_text());
        }
        notebook.closeCurrent();
    }
    else if ((e->keyval == GDK_p) && (e->state & GDK_MOD1_MASK)) {
        if (!notebook.getCurrent()->hasPrefs) {
            notebook.getCurrent()->startPrefs();
        } else {
            notebook.getCurrent()->endPrefs();
        }
    }
    else if ((e->keyval == GDK_n) && (e->state & GDK_MOD1_MASK)) {
        newServer();
    }
    else if ((e->keyval == GDK_q) && (e->state & GDK_MOD1_MASK)) {
        // hide() here will quit the application
        hide();
    }
    Gtk::Window::on_key_press_event(e);
    return false;
}
