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

#include "Tab.h"
#include "MainWindow.h"
#include "GuiCommands.h"

using std::vector;
using std::string;

MainWindow::MainWindow()
: Gtk::Window(GTK_WINDOW_TOPLEVEL)
{
    GuiCommands::mw = this;
    set_policy(1, 1, 0); // Policy for main window: user resizeable
    set_usize(400, 200);
    key_press_event.connect(slot(this, &MainWindow::on_key_press_event));
    
    Gtk::VBox *_vbox1 = manage(new Gtk::VBox());

    _nb = manage(new MainNotebook(this));
    _vbox1->pack_start(*_nb, 1, 1);

    add(*_vbox1);
    set_title("LostIRC "VERSION);
    set_usize(600, 400);

    _app = new LostIRCApp();
    // Connect signals for all the backend events
    _app->evtDisplayMessage.connect(slot(this, &MainWindow::onDisplayMessage));
    _app->evtHighlight.connect(slot(this, &MainWindow::onHighlight));
    _app->evtJoin.connect(slot(this, &MainWindow::onJoin));
    _app->evtKick.connect(slot(this, &MainWindow::onKick));
    _app->evtPart.connect(slot(this, &MainWindow::onPart));
    _app->evtQuit.connect(slot(this, &MainWindow::onQuit));
    _app->evtNick.connect(slot(this, &MainWindow::onNick));
    _app->evtNames.connect(slot(this, &MainWindow::onNames));
    _app->evtCUMode.connect(slot(this, &MainWindow::onCUMode));
    _app->evtAway.connect(slot(this, &MainWindow::onAway));
    _app->evtNewTab.connect(slot(this, &MainWindow::onNewTab));

    _app->start();
    // Construct initial tab
    string name = "<server>";
    ServerConnection *conn = _app->newServer();
    conn->Session.servername = name;
    TabChannel *tab = _nb->addChannelTab(name, conn);
    tab->is_on_channel = false;
    show_all();
    _nb->insert(tab, "\00311Welcome to LostIRC!\n\nThis client is mainly keyboard oriented, so don't expect fancy menus and buttons for you to click on.

\0037Available commands:
\0038/SERVER <hostname> - connect to server.
/JOIN <channel> - join channel.
/PART <channel> - part channel.
/WHOIS <nick> - whois a user.
/NICK <nick> - change nick.
/CTCP <nick> <request> - send CTCP requests.
/AWAY <msg> - go away.
/QUIT <msg> - quit IRC with <msg>.

\0037Available GUI commands:
\0038/QUERY <nick> - start query with <nick>.

\0037Available keybindings:
\0038Alt + [1-9] - switch tabs from 1-9.
Alt + n - create new server tab.
Alt + c - close current tab.
Tab - nickcomplete.
");
}

MainWindow::~MainWindow()
{
    delete _app;
}

void MainWindow::onDisplayMessage(const string& msg, const string& to, ServerConnection *conn)
{
    Tab *tab;

    if (to.empty())
          tab = _nb->getCurrent(conn);
    else
          tab = _nb->findTab(to, conn);


    if (!tab)
          tab = _nb->addQueryTab(to, conn);

    _nb->insert(tab, msg);
}

void MainWindow::onJoin(const string& nick, const string& chan, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan, conn);
    if (!tab) {
        tab = _nb->addChannelTab(chan, conn);
    } else {
        tab->insertUser(nick);
    }
}

void MainWindow::onKick(const string& kicker, const string& chan, const string& nick, const string& msg, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan, conn);
    if (nick == conn->Session.nick) {
        // It's us who's been kicked
        tab->getLabel()->set_text("(" + chan + ")");
        tab->is_on_channel = false;
    }
    tab->removeUser(nick);
}

void MainWindow::onPart(const string& nick, const string& chan, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan, conn);
    if (tab) {
        if (nick == conn->Session.nick) {
            // It's us who's parting
            tab->getLabel()->set_text("(" + chan + ")");
            tab->is_on_channel = false;
        }
        tab->removeUser(nick);
    }
}

void MainWindow::onQuit(const string& nick, const string& msg, ServerConnection *conn)
{
    vector<Tab*> tabs;
    vector<Tab*>::const_iterator i;

    _nb->findTabs(nick, conn, tabs);

    for (i = tabs.begin(); i != tabs.end(); ++i) {
        (*i)->removeUser(nick);
    }
}

void MainWindow::onNick(const string& nick, const string& to, ServerConnection *conn)
{
    vector<Tab*> tabs;
    vector<Tab*>::const_iterator i;

    _nb->findTabs(nick, conn, tabs);

    for (i = tabs.begin(); i != tabs.end(); ++i) {
        (*i)->renameUser(nick, to);
    }
}

void MainWindow::onCUMode(const string& nick, const string& chan, const std::map<string, IRC::UserMode>& users, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan, conn);

    std::map<string, IRC::UserMode>::const_iterator i;
    for (i = users.begin(); i != users.end(); ++i) {
        tab->removeUser(i->first);
        tab->insertUser(i->first, i->second);
    }
}

void MainWindow::onNames(Channel& c, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(c.getName(), conn);

    std::map<string, IRC::UserMode> users = c.getUsers();
    std::map<string, IRC::UserMode>::const_iterator i;

    for (i = users.begin(); i != users.end(); ++i) {
        tab->insertUser(i->first, i->second);
    }
}

void MainWindow::onHighlight(const string& to, ServerConnection* conn)
{
    Tab *tab = _nb->findTab(to, conn);

    if (tab)
          _nb->highlight(tab);

}

void MainWindow::onAway(bool away, ServerConnection* conn)
{
    vector<Tab*>::iterator i;
    vector<Tab*> vec;

    _nb->Tabs(conn, vec);
    if (away) {
        for (i = vec.begin(); i != vec.end(); ++i) {
            (*i)->setAway();
        }
    } else {
        for (i = vec.begin(); i != vec.end(); ++i) {
            (*i)->setUnAway();
        }
    }
}

void MainWindow::onNewTab(ServerConnection *conn)
{
    string name = "<server>";
    conn->Session.servername = name;
    Tab *tab = _nb->addChannelTab(name, conn);
    _nb->show_all();
    _nb->set_page(0);
    tab->is_on_channel = false;
}

void MainWindow::newServer()
{
    string name = "<server>";
    ServerConnection *conn = _app->newServer();
    conn->Session.servername = name;
    Tab *tab = _nb->addChannelTab(name, conn);
    tab->is_on_channel = false;
}

gint MainWindow::on_key_press_event(GdkEventKey* e)
{
    // Default keybindings. Still needs work.
    if ((e->keyval == GDK_0) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(10);
    }
    if ((e->keyval == GDK_1) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(0);
    }
    if ((e->keyval == GDK_2) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(1);
    }
    if ((e->keyval == GDK_3) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(2);
    }
    if ((e->keyval == GDK_4) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(3);
    }
    if ((e->keyval == GDK_5) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(4);
    }
    if ((e->keyval == GDK_6) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(5);
    }
    if ((e->keyval == GDK_7) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(6);
    }
    if ((e->keyval == GDK_8) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(7);
    }
    if ((e->keyval == GDK_9) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(8);
    }
    if ((e->keyval == GDK_c) && (e->state & GDK_MOD1_MASK)) {
        TabChannel *tab = dynamic_cast<TabChannel*>(_nb->getCurrent());
        if (tab) {
            // It's a channel, so we need to part it
            if (tab->getConn()->Session.isConnected) {
                tab->getConn()->sendPart(tab->getLabel()->get_text());
            }
        }
        _nb->closeCurrent();
    }
    if ((e->keyval == GDK_p) && (e->state & GDK_MOD1_MASK)) {
        TabChannel *tab = dynamic_cast<TabChannel*>(_nb->getCurrent());
        if (tab) {
            tab->startPrefs();
        }
    }
    if ((e->keyval == GDK_n) && (e->state & GDK_MOD1_MASK)) {
        newServer();
    }
    if ((e->keyval == GDK_q) && (e->state & GDK_MOD1_MASK)) {
        Gtk::Main::quit();
    }
    if (e->keyval == GDK_Up || e->keyval == GDK_Tab || e->keyval == GDK_Down) {
        _nb->getCurrent()->getEntry()->on_key_press_event(e);
        gtk_signal_emit_stop_by_name(GTK_OBJECT(this->gtkobj()), "key_press_event");
    }
}
