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

#include "Tab.h"
#include "MainWindow.h"
#include <pwd.h>
#include <sys/types.h>

using std::vector;
using std::string;

MainWindow::MainWindow()
: Gtk::Window(GTK_WINDOW_TOPLEVEL), isAway(false)
{
    set_policy(1, 1, 0); // Policy for main window, is user resizeable
    if (!_cfg.readConfig()) {
        cout << "Fatal! Couldn't read config." << endl;
    }

    set_usize(400, 200);
    key_press_event.connect(slot(this, &MainWindow::on_key_press_event));
    
    _io = new InOut();
    Gtk::VBox *_vbox1 = manage(new Gtk::VBox(false, 0));

    // Signals for all server events
    _io->evtDisplayMessage.connect(slot(this, &MainWindow::onDisplayMessage));
    _io->evtServNumeric.connect(slot(this, &MainWindow::onServNumeric));
    _io->evtJoin.connect(slot(this, &MainWindow::onJoin));
    _io->evtKick.connect(slot(this, &MainWindow::onKick));
    _io->evtPart.connect(slot(this, &MainWindow::onPart));
    _io->evtQuit.connect(slot(this, &MainWindow::onQuit));
    _io->evtNick.connect(slot(this, &MainWindow::onNick));
    _io->evtNames.connect(slot(this, &MainWindow::onNames));
    _io->evtMode.connect(slot(this, &MainWindow::onMode));
    _io->evtCUMode.connect(slot(this, &MainWindow::onCUMode));
    _io->evtCMode.connect(slot(this, &MainWindow::onCMode));

    _nb = manage(new MainNotebook(this));
    _vbox1->pack_start(*_nb, 1, 1);

    add(*_vbox1);
    set_title("LostIRC "VERSION);

    // Construct initial tab
    string nick = getenv("USER");
    string name = "<server>";
    struct passwd *p = getpwnam(nick.c_str());
    string realname(p->pw_gecos);
    ServerConnection *conn = _io->newServer(nick, realname);
    conn->Session.servername = name;
    TabChannel *tab = _nb->addChannelTab(name, conn);
    tab->is_on_channel = false;
    //tab->getText()->insert("Welcome to LostIRC!\n\nThis client is mainly keyboard oriented, so don't expect fancy menus and buttons for you to click on.\n\nTo list all available commands type /COMMANDS.\nTo see all available keybindings type /BINDS.\n\nType /SERVER <hostname> to connect to a server.\n");
    set_usize(600, 400);
    show_all();
    _nb->insert(tab, "Welcome to LostIRC!\n\nThis client is mainly keyboard oriented, so don't expect fancy menus and buttons for you to click on.\n\n$2Available commands:
$3/SERVER <hostname> - connect to server.
/JOIN <channel> - join channel.
/PART <channel> - part channel.
/WHOIS <nick> - whois a user.
/NICK <nick> - change nick.
/CTCP <nick> <request> - send CTCP requests.
/AWAY <msg> - go away.
/QUIT <msg> - quit IRC with <msg>.

$2Available GUI commands:
$3/QUERY <nick> - start query with <nick>.

$2Available keybindings:
$3Alt + [1-9] - switch tabs from 1-9.
Alt + n - create new server tab.
Alt + c - close current tab.
Tab - nickcomplete.
");


}

void MainWindow::onDisplayMessage(const string& msg, const string& to, ServerConnection *conn)
{
    Tab *tab;

    if (to.empty())
          tab = _nb->getCurrent(conn);
    else
          tab = _nb->findTab(to, conn);

    if (!tab) {
        tab = _nb->addQueryTab(to, conn);
    }

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
    }
    _nb->insert(tab, "-- " + nick + " was kicked from " + chan + " by " + kicker + " (" + msg + ")\n");
    tab->removeUser(nick);
}

void MainWindow::onPart(const string& nick, const string& chan, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan, conn);
    if (tab) {
        if (nick == conn->Session.nick) {
            // It's us who's parting
            tab->getLabel()->set_text("(" + chan + ")");
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
        _nb->insert(*i, "$5-- " + nick + " has quit (" + msg + ")\n");
        (*i)->removeUser(nick);
    }
}

void MainWindow::onNick(const string& nick, const string& to, ServerConnection *conn)
{
    // Check whethers it's us who has changed nick
    if (nick == conn->Session.nick) {
        conn->Session.nick = to;
    }
    vector<Tab*> tabs;
    vector<Tab*>::const_iterator i;

    _nb->findTabs(nick, conn, tabs);

    for (i = tabs.begin(); i != tabs.end(); ++i) {
        _nb->insert(*i, "$9-- " + nick + " changes nick to " + to + "\n");
        (*i)->renameUser(nick, to);
    }
}

void MainWindow::onMode(const string& nick, const string& param, const string& mode, ServerConnection *conn)
{
    Tab *tab = _nb->getCurrent(conn);
    _nb->insert(tab, "$7-- " + nick + " sets mode " + mode + " " + param + "\n");

}

void MainWindow::onCMode(const string& nick, const string& chan, char sign, const string& modes, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan, conn);

    string::const_iterator i;
    for (i = modes.begin(); i != modes.end(); ++i) {
        _nb->insert(tab, "$4-- "  + nick + " sets channel mode " + sign + *i + " on " + chan + "\n");
    }
}

void MainWindow::onCUMode(const string& nick, const string& chan, const vector<vector<string> >& users, ServerConnection *conn)
{
    Tab *tab = _nb->findTab(chan, conn);

    vector<vector<string> >::const_iterator i;
    for (i = users.begin(); i != users.end(); ++i) {
        vector<string> vec = *i;
        _nb->insert(tab, "$4-- "  + nick + " sets mode " + vec[0] + " to " + vec[1] + "\n");
        tab->removeUser(vec[1]);
        tab->insertUser(*i);
    }
}

void MainWindow::onServNumeric(int n, const string& from, const string& to, const string& msg, ServerConnection *conn)
{
    switch (n)
    {
        case 433: // ERR_NICKNAMEINUSE
            // Apply a _ to the nick
            conn->sendNick(conn->Session.nick += "_");
            break;
    }
}

void MainWindow::onNames(const string& chan, const vector<vector<string> >& users, ServerConnection *conn)
{
    vector<vector<string> >::const_iterator i;

    Tab *tab = _nb->findTab(chan, conn);

    for (i = users.begin(); i != users.end(); ++i) {
        tab->insertUser(*i);
    }
}

void MainWindow::newServer()
{
    string nick = getenv("USER");

    struct passwd *p = getpwnam(nick.c_str());
    string realname(p->pw_gecos);

    string name = "<server>";
    ServerConnection *conn = _io->newServer(nick, realname);
    conn->Session.servername = name;
    Tab *tab = _nb->addChannelTab(name, conn);
    tab->is_on_channel = false;
}

void MainWindow::quit()
{
    _io->quit();
    Gtk::Main::quit();
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
    if ((e->keyval == GDK_n) && (e->state & GDK_MOD1_MASK)) {
        newServer();
    }
    if ((e->keyval == GDK_q) && (e->state & GDK_MOD1_MASK)) {
        quit();
    }
    if (e->keyval == GDK_Up || e->keyval == GDK_Tab) {
        _nb->getCurrent()->getEntry()->on_key_press_event(e);
        gtk_signal_emit_stop_by_name(GTK_OBJECT(this->gtkobj()), "key_press_event");
    }
}
