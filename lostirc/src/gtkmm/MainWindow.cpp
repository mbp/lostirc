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

MainWindow* AppWin;

MainWindow::MainWindow()
: Gtk::Window(GTK_WINDOW_TOPLEVEL)
{
    AppWin = this;
    set_policy(1, 1, 0); // Policy for main window: user resizeable
    set_usize(400, 200);
    key_press_event.connect(slot(this, &MainWindow::on_key_press_event));
    
    Gtk::VBox *_vbox1 = manage(new Gtk::VBox());

    _nb = new MainNotebook(this);
    _vbox1->pack_start(*_nb, 1, 1);

    add(*_vbox1);
    set_title("LostIRC "VERSION);
    set_usize(600, 400);

    _app = new LostIRCApp();
    // Connect signals for all the backend events
    _app->evtDisplayMessage.connect(slot(this, &MainWindow::onDisplayMessage));
    _app->evtDisplayMessageInChan.connect(slot(this, &MainWindow::onDisplayMessageInChan));
    _app->evtDisplayMessageInQuery.connect(slot(this, &MainWindow::onDisplayMessageInQuery));
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

    int num_of_servers = _app->start();
    if (num_of_servers == 0) {
        // Construct initial tab
        string name = "<server>";
        ServerConnection *conn = _app->newServer();
        conn->Session.servername = name;
        TabChannel *tab = _nb->addChannelTab(name, conn);
        tab->is_on_channel = false;
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
    show_all();
}

MainWindow::~MainWindow()
{
    delete _nb;
    delete _app;
}

void MainWindow::onDisplayMessage(const string& msg, FE::Dest d, ServerConnection *conn)
{
    if (d == FE::CURRENT) {
        Tab *tab = _nb->getCurrent(conn);

        _nb->insert(tab, msg);
    }
}

void MainWindow::onDisplayMessageInChan(const string& msg, Channel& chan, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan.getName(), conn);

    // does the channel exist? if not, we probably did a 'closeCurrent() and
    // parted it...
    if (tab)
          _nb->insert(tab, msg);
}

void MainWindow::onDisplayMessageInQuery(const string& msg, const string& to, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(to, conn);

    if (!tab)
          tab = _nb->addQueryTab(to, conn);

    _nb->insert(tab, msg);
}

void MainWindow::onJoin(const string& nick, Channel& chan, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan.getName(), conn);
    if (!tab) {
        tab = _nb->addChannelTab(chan.getName(), conn);
    } else {
        tab->insertUser(nick);
    }
}

void MainWindow::onKick(const string& kicker, Channel& chan, const string& nick, const string& msg, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan.getName(), conn);
    if (nick == conn->Session.nick) {
        // It's us who's been kicked
        tab->getLabel()->set_text("(" + chan.getName() + ")");
        tab->is_on_channel = false;
    }
    tab->removeUser(nick);
}

void MainWindow::onPart(const string& nick, Channel& chan, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan.getName(), conn);
    if (tab) {
        if (nick == conn->Session.nick) {
            // It's us who's parting
            tab->getLabel()->set_text("(" + chan.getName() + ")");
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

void MainWindow::onCUMode(const string& nick, Channel& chan, const std::map<string, IRC::UserMode>& users, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan.getName(), conn);

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
    // XXX: this is a hack for a "bug" in the gtkmm code which makes the
    // application crash in the start when no pages exists, even though we
    // added one above... doing set_page(0) will somehow add it fully.
    if (_nb->get_current_page_num() == -1) {
        _nb->set_page(0);
    }
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
        _nb->set_page(9);
    }
    else if ((e->keyval == GDK_1) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(0);
    }
    else if ((e->keyval == GDK_2) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(1);
    }
    else if ((e->keyval == GDK_3) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(2);
    }
    else if ((e->keyval == GDK_4) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(3);
    }
    else if ((e->keyval == GDK_5) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(4);
    }
    else if ((e->keyval == GDK_6) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(5);
    }
    else if ((e->keyval == GDK_7) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(6);
    }
    else if ((e->keyval == GDK_8) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(7);
    }
    else if ((e->keyval == GDK_9) && (e->state & GDK_MOD1_MASK)) {
        _nb->set_page(8);
    }
    else if ((e->keyval == GDK_c) && (e->state & GDK_MOD1_MASK)) {
        TabChannel *tab = dynamic_cast<TabChannel*>(_nb->getCurrent());
        if (tab && tab->getConn()->Session.isConnected) {
            // It's a channel, so we need to part it
            tab->getConn()->sendPart(tab->getLabel()->get_text(), "");
        }
        _nb->closeCurrent();
    }
    else if ((e->keyval == GDK_p) && (e->state & GDK_MOD1_MASK)) {
        if (!_nb->getCurrent()->hasPrefs) {
            _nb->getCurrent()->startPrefs();
        } else {
            _nb->getCurrent()->endPrefs();
        }
    }
    else if ((e->keyval == GDK_n) && (e->state & GDK_MOD1_MASK)) {
        newServer();
    }
    else if ((e->keyval == GDK_q) && (e->state & GDK_MOD1_MASK)) {
        Gtk::Main::quit();
    }
    else if (e->keyval == GDK_Up || e->keyval == GDK_Tab || e->keyval == GDK_Down) {
        if (!_nb->getCurrent()->hasPrefs) {
            _nb->getCurrent()->getEntry()->on_key_press_event(e);
            gtk_signal_emit_stop_by_name(GTK_OBJECT(this->gtkobj()), "key_press_event");
        }
    }
    else if ((e->keyval == GDK_f) && (e->state & GDK_MOD1_MASK)) {
        _nb->setFont();
    }
    return 0;
}
