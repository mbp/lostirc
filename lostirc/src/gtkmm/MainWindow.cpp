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

MainWindow::MainWindow(bool autoconnect)
    : Gtk::Window(), app(this)
{
    AppWin = this;
    set_title("LostIRC "VERSION);

    int width = app.options.window_width;
    int height = app.options.window_height;
    if (width && height)
          set_default_size(width, height);
    else
          set_default_size(600, 400);

    int x = app.options.window_x;
    int y = app.options.window_y;

    if (x && y)
          move(x, y);
    
    Gtk::VBox *vbox = manage(new Gtk::VBox());

    vbox->pack_start(notebook, Gtk::PACK_EXPAND_WIDGET);
    vbox->pack_start(statusbar, Gtk::PACK_SHRINK);

    add(*vbox);
    show_all();

    if (app.cfgservers.getServers().empty() || !autoconnect) {
        // Construct initial tab
        Tab *tab = newServer();
        tab->getText() << "\0037\nWelcome to LostIRC "VERSION"!\n\nYou use the client mainly by typing in commands and text in the entry-bar shown below.\n\nYou can connect to a server using:\n    \0038/SERVER <hostname>\n\n\0037Then join a channel:\n    \0038/JOIN <channel-name>\n\n\0037The rest of the commands is available with:\n    \0038/COMMANDS\0037.\n\n\0037Available keybindings:\n    \0038CTRL-[1-9] - switch tabs from 1-9.\n    CTRL-N - create new server tab.\n    CTRL-W - close current window(tab).\n    CTRL-P - open preferences.\n    Tab - nick-completion and command-completion.\n";
    } else {
        // Auto-connect to servers.
        app.start();
    }
}

MainWindow::~MainWindow()
{
    // Save the width and height of the windows
    int width, height;

    get_size(width, height);
    if (width && height) {
        app.options.window_width = width;
        app.options.window_height = height;
    }

    int x, y;
    get_window()->get_root_origin(x, y);
    if (x && y) {
        app.options.window_x = x;
        app.options.window_y = y;
    }

    AppWin = 0;
}

void MainWindow::displayMessage(const string& msg, FE::Destination d, bool shouldHighlight)
{

    if (d == FE::CURRENT) {
        Tab *tab = notebook.getCurrent();

        if (tab) {
            tab->getText() << msg;

            if (shouldHighlight)
                  notebook.highlightActivity(tab);
        }

    } else if (d == FE::ALL) {
        vector<Tab*> tabs;
        vector<Tab*>::const_iterator i;
        notebook.Tabs(tabs);

        for (i = tabs.begin(); i != tabs.end(); ++i) {
            (*i)->getText() << msg;
            if (shouldHighlight)
                  notebook.highlightActivity(*i);
        }
    
    }
}

void MainWindow::displayMessage(const string& msg, FE::Destination d, ServerConnection *conn, bool shouldHighlight)
{

    if (d == FE::CURRENT) {
        Tab *tab = notebook.getCurrent(conn);

        if (tab) {
            tab->getText() << msg;

            if (shouldHighlight)
                  notebook.highlightActivity(tab);
        }

    } else if (d == FE::ALL) {
        vector<Tab*> tabs;
        vector<Tab*>::const_iterator i;
        notebook.findTabs(conn, tabs);

        for (i = tabs.begin(); i != tabs.end(); ++i) {
            (*i)->getText() << msg;
            if (shouldHighlight)
                  notebook.highlightActivity(*i);
        }
    }
}

void MainWindow::displayMessage(const string& msg, ChannelBase& chan, ServerConnection *conn, bool shouldHighlight)
{
    Tab *tab = notebook.findTab(convert_to_utf8(chan.getName()), conn);

    // if the channel doesn't exist, it's probably a query. (the channel is
    // created on join) - there is also a hack here to ensure that it's not
    // a channel
    char p = chan.getName().at(0);
    if (!tab && (p != '#' && p != '&' && p != '!' && p != '+'))
        tab = notebook.addQueryTab(convert_to_utf8(chan.getName()), conn);

    if (tab) {
        tab->getText() << msg;

        if (shouldHighlight)
              notebook.highlightActivity(tab);
    }
}

void MainWindow::join(const string& nick, Channel& chan, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(convert_to_utf8(chan.getName()), conn);
    if (!tab) {
        tab = notebook.addChannelTab(convert_to_utf8(chan.getName()), conn);
        return;
    }
    tab->insertUser(convert_to_utf8(nick));
}

void MainWindow::part(const string& nick, Channel& chan, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(convert_to_utf8(chan.getName()), conn);
    if (tab) {
        if (nick == conn->Session.nick) {
            // It's us who's parting
            tab->setInActive();
        }
        tab->removeUser(convert_to_utf8(nick));
    }
}

void MainWindow::kick(const string& kicker, Channel& chan, const string& nick, const string& msg, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(convert_to_utf8(chan.getName()), conn);
    if (nick == conn->Session.nick) {
        // It's us who's been kicked
        tab->setInActive();
    }
    tab->removeUser(convert_to_utf8(nick));
}


void MainWindow::quit(const string& nick, vector<ChannelBase*> chans, ServerConnection *conn)
{
    vector<ChannelBase*>::const_iterator i;

    for (i = chans.begin(); i != chans.end(); ++i) {
        if (Tab *tab = notebook.findTab(convert_to_utf8((*i)->getName()), conn))
            tab->removeUser(convert_to_utf8(nick));
    }
}

void MainWindow::nick(const string& nick, const string& to, vector<ChannelBase*> chans, ServerConnection *conn)
{
    vector<ChannelBase*>::const_iterator i;

    for (i = chans.begin(); i != chans.end(); ++i) {
        if (Tab *tab = notebook.findTab(convert_to_utf8((*i)->getName()), conn))
              tab->renameUser(convert_to_utf8(nick), convert_to_utf8(to));
    }
    notebook.updateStatus();
}

void MainWindow::CUMode(const string& nick, Channel& chan, const std::vector<User>& users, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(convert_to_utf8(chan.getName()), conn);

    std::vector<User>::const_iterator i;
    for (i = users.begin(); i != users.end(); ++i) {
        tab->removeUser(convert_to_utf8(i->nick));
        tab->insertUser(convert_to_utf8(i->nick), i->getMode());
    }
}

void MainWindow::names(Channel& c, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(convert_to_utf8(c.getName()), conn);

    std::vector<User*> users = c.getUsers();
    std::vector<User*>::const_iterator i;

    for (i = users.begin(); i != users.end(); ++i) {
        tab->insertUser(convert_to_utf8((*i)->nick), (*i)->getMode());
    }
}

void MainWindow::highlight(ChannelBase& chan, ServerConnection* conn)
{
    Tab *tab = notebook.findTab(convert_to_utf8(chan.getName()), conn);

    if (tab)
          notebook.highlightNick(tab);
}

void MainWindow::away(bool away, ServerConnection* conn)
{
    notebook.updateStatus();
    notebook.updateTitle();
}

void MainWindow::connected(ServerConnection* conn)
{
    notebook.updateStatus();
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
    Tab *tab = notebook.addTab(convert_to_utf8(name), conn);
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
    Tab *tab = notebook.addTab(convert_to_utf8(name), conn);
    tab->setInActive();
    return tab;
}

bool MainWindow::on_key_press_event(GdkEventKey* e)
{
    // CTRL key.
    if (e->state & GDK_CONTROL_MASK) {
        if (e->keyval == GDK_0) {
            notebook.set_current_page(9);
        } else if (e->keyval == GDK_1) {
            notebook.set_current_page(0);
        } else if (e->keyval == GDK_2) {
            notebook.set_current_page(1);
        } else if (e->keyval == GDK_3) {
            notebook.set_current_page(2);
        } else if (e->keyval == GDK_4) {
            notebook.set_current_page(3);
        } else if (e->keyval == GDK_5) {
            notebook.set_current_page(4);
        } else if (e->keyval == GDK_6) {
            notebook.set_current_page(5);
        } else if (e->keyval == GDK_7) {
            notebook.set_current_page(6);
        } else if (e->keyval == GDK_8) {
            notebook.set_current_page(7);
        } else if (e->keyval == GDK_9) {
            notebook.set_current_page(8);
        } else if (e->keyval == GDK_w) {
            Tab *tab = notebook.getCurrent();
            if (tab->isChannel() && tab->getConn()->Session.isConnected && tab->isActive()) {
                // It's a channel, so we need to part it
                tab->getConn()->sendPart(Glib::locale_from_utf8(notebook.getLabel(tab)->get_text()), "");
            } else {
                // Query
                tab->getConn()->removeChannel(Glib::locale_from_utf8(notebook.getLabel(tab)->get_text()));
            }
            notebook.closeCurrent();
        } else if (e->keyval == GDK_p) {
            if (!notebook.getCurrent()->hasPrefs) {
                notebook.getCurrent()->startPrefs();
            } else {
                notebook.getCurrent()->endPrefs();
            }
        } else if (e->keyval == GDK_n) {
            newServer();
        } else if (e->keyval == GDK_q) {
            // hide() here will quit the application
            hide();
        }
    }
    Gtk::Window::on_key_press_event(e);
    return false;
}

Glib::ustring convert_to_utf8(const std::string& str)
{
    Glib::ustring str_utf8 (str);

    if (!str_utf8.validate()) { // invalid UTF-8?
        bool did_conversion = false;

        if (!Glib::get_charset()) {// locale charset is not UTF-8?
            try // ignore errors -- go on with the fallback if the conversion fails
            {
                str_utf8 = Glib::locale_to_utf8(str);
                did_conversion = true;
            }
            catch(const Glib::ConvertError&)
            {}
        }

        if (!did_conversion) {
            // Fallback conversion -- used either if the conversion from the
            // current locale's encoding failed, or if the user is running a
            // UTF-8 locale.
            str_utf8 = Glib::convert(str, "UTF-8", "ISO-8859-15");

            // ISO-8859-15 is the default fallback encoding. Might want to
            // make it configurable some day.
        }
    }

    return str_utf8;
}
