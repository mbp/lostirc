/* 
 * Copyright (C) 2002, 2003 Morten Brix Pedersen <morten@wtf.dk>
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

#include <config.h>
#include <gtkmm/box.h>
#include <algorithm>
#include <gtkmm/messagedialog.h>
#include <functional>
#include <cstdlib>
#include "Tab.h"
#include "DCCList.h"
#include "MainWindow.h"

using std::vector;
using Glib::ustring;

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

    if (!app.cfgservers.hasAutoConnects() || !autoconnect) {
        // Construct initial tab
        Tab *tab = newServer();
        tab->getText() << "\0037\002Welcome to LostIRC "VERSION"!\002\n\nYou can now connect to a server using:\n    \0038/SERVER <hostname / ip>\n\n\0037...and then join a channel:\n    \0038/JOIN <channel-name>\n\n\0037A list of all commands are available with:\n    \0038/COMMANDS\0037\n\nAnd you should \002really\002 check out the list of key bindings:\n    \0038/KEYBINDINGS\n\n";
    } else {
        // Auto-connect to servers.
        app.autoConnect();
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

void MainWindow::displayMessage(const ustring& msg, FE::Destination d, bool shouldHighlight)
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

void MainWindow::displayMessage(const ustring& msg, FE::Destination d, ServerConnection *conn, bool shouldHighlight)
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

void MainWindow::displayMessage(const ustring& msg, ChannelBase& chan, ServerConnection *conn, bool shouldHighlight)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);

    // if the channel doesn't exist, it's probably a query. (the channel is
    // created on join) - there is also a hack here to ensure that it's not
    // a channel
    char p = chan.getName().at(0);
    if (!tab && (p != '#' && p != '&' && p != '!' && p != '+'))
        tab = notebook.addQueryTab(chan.getName(), conn);

    if (tab) {
        tab->getText() << msg;

        if (shouldHighlight)
              notebook.highlightActivity(tab);
    }
}

void MainWindow::join(const ustring& nick, Channel& chan, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);
    if (!tab) {
        tab = notebook.addChannelTab(chan.getName(), conn);
        notebook.updateTitle();
        return;
    }
    tab->insertUser(nick);
}

void MainWindow::part(const ustring& nick, Channel& chan, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);
    if (tab) {
        if (nick == conn->Session.nick) {
            // It's us who's parting
            tab->setInActive();
        }
        tab->removeUser(nick);
    }
}

void MainWindow::kick(const ustring& kicker, Channel& chan, const ustring& nick, const ustring& msg, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);
    if (nick == conn->Session.nick) {
        // It's us who's been kicked
        tab->setInActive();
    }
    tab->removeUser(nick);
}


void MainWindow::quit(const ustring& nick, vector<ChannelBase*> chans, ServerConnection *conn)
{
    vector<ChannelBase*>::const_iterator i;

    for (i = chans.begin(); i != chans.end(); ++i) {
        if (Tab *tab = notebook.findTab((*i)->getName(), conn))
            tab->removeUser(nick);
    }
}

void MainWindow::nick(const ustring& nick, const ustring& to, vector<ChannelBase*> chans, ServerConnection *conn)
{
    vector<ChannelBase*>::const_iterator i;

    for (i = chans.begin(); i != chans.end(); ++i) {
        if (Tab *tab = notebook.findTab((*i)->getName(), conn))
              tab->renameUser(nick, to);
    }
    notebook.updateStatus();
}

void MainWindow::CUMode(const ustring& nick, Channel& chan, const std::vector<User>& users, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);

    std::vector<User>::const_iterator i;
    for (i = users.begin(); i != users.end(); ++i) {
        tab->removeUser(i->nick);
        tab->insertUser(i->nick, i->getMode());
    }
}

void MainWindow::names(Channel& c, ServerConnection *conn)
{
    Tab *tab = notebook.findTab(c.getName(), conn);

    std::vector<User*> users = c.getUsers();
    std::vector<User*>::const_iterator i;

    for (i = users.begin(); i != users.end(); ++i) {
        tab->insertUser((*i)->nick, (*i)->getMode());
    }
}

void MainWindow::highlight(ChannelBase& chan, ServerConnection* conn)
{
    Tab *tab = notebook.findTab(chan.getName(), conn);

    if (tab) {
        notebook.highlightNick(tab);
        tab->getText().setHighlightMark();
    }
}

void MainWindow::away(bool away, ServerConnection* conn)
{
    notebook.updateStatus();
    notebook.updateTitle();
}

void MainWindow::connected(ServerConnection* conn)
{
    notebook.updateStatus();
    notebook.updateTitle();

    vector<Tab*> tabs;
    vector<Tab*>::const_iterator i;

    notebook.findTabs(conn, tabs);

    for (i = tabs.begin(); i != tabs.end(); ++i)
          if ((*i)->isQuery())
                (*i)->setActive();
}

void MainWindow::disconnected(ServerConnection* conn)
{
    vector<Tab*> tabs;

    notebook.findTabs(conn, tabs);

    std::for_each(tabs.begin(), tabs.end(), std::mem_fun(&Tab::setInActive));
}

void MainWindow::newTab(ServerConnection *conn)
{
    ustring name = "server";
    conn->Session.servername = name;
    Tab *tab = notebook.addTab(name, conn);
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
    ustring name = "server";
    ServerConnection *conn = app.newServer();
    conn->Session.servername = name;
    Tab *tab = notebook.addTab(name, conn);
    tab->setInActive();
    return tab;
}
void MainWindow::newDCC(DCC *dcc)
{
    DCCList *dcclist = DCCList::Instance();
    dcclist->add(dcc); 
}

void MainWindow::dccStatusChanged(DCC *dcc)
{
    DCCList *dcclist = DCCList::Instance();
    dcclist->statusChange(dcc); 
}

void MainWindow::localeError()
{
    Glib::ustring msg = "Locale conversion error. An error occured while converting text from UTF-8 to your current locale.\n\nThis is most likely because your locale is set to a value which doesn't support the character(s) converting to.\n\nIf you believe this is a bug, please report it to the application author.";

    char *locale = std::getenv("LANG");
    if (locale != NULL) {
        msg += "\n\nYour current locale (seems) to be: ";
        msg += locale;
    }

    msg += "\n\n(Note: You'll only see this warning once per LostIRC session)";


    Gtk::MessageDialog mdialog(*this, msg, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
    mdialog.run();
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
                tab->getConn()->sendPart(notebook.getLabel(tab)->get_text(), "");
            } else {
                // Query
                tab->getConn()->removeChannel(notebook.getLabel(tab)->get_text());
            }
            notebook.closeCurrent();
        } else if (e->keyval == GDK_p) {
            if (!notebook.getCurrent()->hasPrefs) 
                  notebook.getCurrent()->startPrefs();
            else 
                  notebook.getCurrent()->closePrefs();
        } else if (e->keyval == GDK_d) {
            if (!notebook.getCurrent()->hasDCCList)
                  notebook.getCurrent()->startDCCList();
            else
                  notebook.getCurrent()->closeDCCList();
        } else if (e->keyval == GDK_h) {
            // find highlight mark
            notebook.getCurrent()->getText().scrollToHighlightMark();
        } else if (e->keyval == GDK_n) {
            newServer();
        } else if (e->keyval == GDK_q) {
            // hide() here will quit the application
            hide();
        } else if (e->keyval == GDK_End) {
            notebook.getCurrent()->getText().scrollToBottom();
        } else if (e->keyval == GDK_Home) {
            notebook.getCurrent()->getText().scrollToTop();
        }
    } else if (e->state & GDK_MOD1_MASK) {
        // ALT key.
        if (e->keyval == GDK_Left)
              notebook.prev_page();
        else if (e->keyval == GDK_Right)
              notebook.next_page();
    }
    if (e->keyval == GDK_Page_Up) {
        // scroll up text widget
        notebook.getCurrent()->getText().scrollUpPage();

    } else if (e->keyval == GDK_Page_Down) {
        // scroll down text widget
        notebook.getCurrent()->getText().scrollDownPage();
    }

    Gtk::Window::on_key_press_event(e);
    return false;
}
