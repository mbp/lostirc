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

#include <algorithm>
#include <functional>
#include <cstdlib>
#include <config.h>
#include <gtkmm/box.h>
#include <gdk/gdkkeysyms.h>
#include <sigc++/retype_return.h>
#include "DCCList.h"
#include "MainWindow.h"
#include "Tab.h"

using std::vector;
using Glib::ustring;

MainWindow* AppWin = 0;

MainWindow::MainWindow(bool autoconnect)
    : Gtk::Window(), _app(this)
{
    AppWin = this;
    set_title("LostIRC");

    int width = _app.options.window_width;
    int height = _app.options.window_height;
    if (width && height)
          set_default_size(width, height);
    else
          set_default_size(600, 400);

    int x = _app.options.window_x;
    int y = _app.options.window_y;

    if (x >= 0 && y >= 0)
          move(x, y);
    
    setupMenus();
    Gtk::VBox *vbox = manage(new Gtk::VBox());

    vbox->pack_start(_menubar, Gtk::PACK_SHRINK);
    vbox->pack_start(_notebook, Gtk::PACK_EXPAND_WIDGET);
    vbox->pack_start(_statusbar, Gtk::PACK_SHRINK);

    add(*vbox);
    show_all();

    if (_app.options.hidemenu)
          _menubar.hide();

    if (_app.options.hidestatusbar)
          _statusbar.hide();

    if (!_app.cfgservers.hasAutoConnects() || !autoconnect) {
        // Construct initial tab
        newServerTab();
    } else {
        // Auto-connect to servers.
        _app.autoConnect();
    }
}

MainWindow::~MainWindow()
{
    // Save the width and height of the windows
    int width, height;

    get_size(width, height);
    if (width && height) {
        _app.options.window_width = width;
        _app.options.window_height = height;
    }

    int x, y;
    get_window()->get_root_origin(x, y);
    if (x >= 0 && y >= 0) {
        _app.options.window_x = x;
        _app.options.window_y = y;
    }

    AppWin = 0;
}

void MainWindow::displayMessage(const ustring& msg, FE::Destination d, bool shouldHighlight)
{

    if (d == FE::CURRENT) {
        Tab *tab = _notebook.getCurrent();

        if (tab) {
            tab->getText() << msg;

            if (shouldHighlight)
                  tab->highlightActivity();
        }

    } else if (d == FE::ALL) {
        vector<Tab*> tabs;
        vector<Tab*>::const_iterator i;
        _notebook.Tabs(tabs);

        for (i = tabs.begin(); i != tabs.end(); ++i) {
            (*i)->getText() << msg;

            if (shouldHighlight)
                  (*i)->highlightActivity();
        }
    
    }
}

void MainWindow::displayMessage(const ustring& msg, FE::Destination d, ServerConnection *conn, bool shouldHighlight)
{

    if (d == FE::CURRENT) {
        Tab *tab = _notebook.getCurrent(conn);

        if (tab) {
            tab->getText() << msg;

            if (shouldHighlight)
                  tab->highlightActivity();
        }

    } else if (d == FE::ALL) {
        vector<Tab*> tabs;
        vector<Tab*>::const_iterator i;
        _notebook.findTabs(conn, tabs);

        for (i = tabs.begin(); i != tabs.end(); ++i) {
            (*i)->getText() << msg;

            if (shouldHighlight)
                  (*i)->highlightActivity();
        }
    }
}

void MainWindow::displayMessage(const ustring& msg, ChannelBase& chan, ServerConnection *conn, bool shouldHighlight)
{
    Tab *tab = _notebook.findTab(chan.getName(), conn);

    // if the channel doesn't exist, it's probably a query. (the channel is
    // created on join) - there is also a hack here to ensure that it's not
    // a channel
    char p = chan.getName().at(0);
    if (!tab && (p != '#' && p != '&' && p != '!' && p != '+'))
        tab = _notebook.addTab(Tab::QUERY, chan.getName(), conn);

    if (tab) {
        tab->getText() << msg;

        if (shouldHighlight)
              tab->highlightActivity();
    }
}

void MainWindow::join(const ustring& nick, Channel& chan, ServerConnection *conn)
{
    Tab *tab = _notebook.findTab(chan.getName(), conn);
    if (!tab) {
        tab = _notebook.addTab(Tab::CHANNEL, chan.getName(), conn);
        _notebook.updateTitle();
        return;
    }
    tab->insertUser(nick);
}

void MainWindow::part(const ustring& nick, Channel& chan, ServerConnection *conn)
{
    Tab *tab = _notebook.findTab(chan.getName(), conn);
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
    Tab *tab = _notebook.findTab(chan.getName(), conn);
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
        if (Tab *tab = _notebook.findTab((*i)->getName(), conn))
            tab->removeUser(nick);
    }
}

void MainWindow::nick(const ustring& nick, const ustring& to, vector<ChannelBase*> chans, ServerConnection *conn)
{
    vector<ChannelBase*>::const_iterator i;

    for (i = chans.begin(); i != chans.end(); ++i) {
        if (Tab *tab = _notebook.findTab((*i)->getName(), conn))
              tab->renameUser(nick, to);
    }
    _notebook.updateStatus();
}

void MainWindow::CUMode(const ustring& nick, Channel& chan, const std::vector<User>& users, ServerConnection *conn)
{
    Tab *tab = _notebook.findTab(chan.getName(), conn);

    std::vector<User>::const_iterator i;
    for (i = users.begin(); i != users.end(); ++i) {
        tab->removeUser(i->nick);
        tab->insertUser(i->nick, i->getMode());
    }
}

void MainWindow::names(Channel& c, ServerConnection *conn)
{
    Tab *tab = _notebook.findTab(c.getName(), conn);

    std::vector<User*> users = c.getUsers();
    std::vector<User*>::const_iterator i;

    for (i = users.begin(); i != users.end(); ++i)
          tab->insertUser((*i)->nick, (*i)->getMode());
}

void MainWindow::highlight(ChannelBase& chan, ServerConnection* conn)
{
    Tab *tab = _notebook.findTab(chan.getName(), conn);

    if (tab)
          tab->highlightNick();
}

void MainWindow::away(bool away, ServerConnection* conn)
{
    _notebook.updateStatus();
    _notebook.updateTitle();
}

void MainWindow::connected(ServerConnection* conn)
{
    _notebook.updateStatus();
    _notebook.updateTitle();

    vector<Tab*> tabs;
    vector<Tab*>::const_iterator i;

    _notebook.findTabs(conn, tabs);

    for (i = tabs.begin(); i != tabs.end(); ++i)
          if ((*i)->isType(Tab::QUERY))
                (*i)->setActive();
}

void MainWindow::disconnected(ServerConnection* conn)
{
    vector<Tab*> tabs;

    _notebook.findTabs(conn, tabs);

    std::for_each(tabs.begin(), tabs.end(), std::mem_fun(&Tab::setInActive));
}

void MainWindow::newTab(ServerConnection *conn)
{
    ustring name = _("server");
    conn->Session.servername = name;
    Tab *tab = _notebook.addTab(Tab::SERVER, name, conn);
    tab->setType(Tab::SERVER);

    // XXX: this is a hack for a "bug" in the gtkmm code which makes the
    // application crash in the start when no pages exists, even though we
    // added one above... doing set_current_page(0) will somehow add it fully.
    if (_notebook.get_current_page() == -1) {
        _notebook.set_current_page(0);
    }
    tab->setInActive();
}

Tab* MainWindow::newServerTab()
{
    ustring name = _("server");
    ServerConnection *conn = _app.newServer();
    conn->Session.servername = name;
    Tab *tab = _notebook.addTab(Tab::SERVER, name, conn);
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

void MainWindow::localeError(bool tried_custom_encoding)
{
    Glib::ustring msg;
    if (!tried_custom_encoding) {
        msg = _("Locale conversion error. An error occured while converting text from UTF-8 to your current locale.\n\nThis is most likely because your locale is set to a value which doesn't support the character(s) converting to.\n\nIf you believe this is a bug, please report it to the application author.");

        char *locale = std::getenv("LANG");
        if (locale != NULL) {
            msg += _("\n\nYour current locale (seems) to be: ");
            msg += locale;
        }
    } else {
        msg = _("Encoding conversion error. An error occured while converting text from UTF-8 to the user-defined encoding.\n\nThis is most likely because the encoding you have chosen doesn't support the character(s) converting to.\n\nIf you believe this is a bug, please report it to the application author.");

        msg += _("\n\nI was trying to convert to: ");
        msg += App->options.encoding;
    }

    msg += _("\n\n(Note: You'll only see this warning once per LostIRC session)");


    Gtk::MessageDialog mdialog(*this, msg, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
    mdialog.run();
}

void MainWindow::openPrefs()
{
    if (_prefswin.get()) {
          _prefswin->present();
    } else {
        std::auto_ptr<Prefs> dialog(new Prefs());

        dialog->show();

        _prefswin = dialog;
    }
}

void MainWindow::openDccWindow()
{
    if (_dccwin.get()) {
          _dccwin->present();
    } else {
        std::auto_ptr<DCCWindow> dialog(new DCCWindow(*this));

        dialog->show();

        _dccwin = dialog;
    }
}
void MainWindow::setupMenus()
{
    { // First menu.
        Gtk::Menu::MenuList& menulist = _firstmenu.items();

        menulist.push_back(Gtk::Menu_Helpers::MenuElem(
                    _("New Server Tab"),
                    Gtk::Menu::AccelKey("<control>n"),
                    SigC::hide_return(SigC::slot(*this, &MainWindow::newServerTab))));

        menulist.push_back(Gtk::Menu_Helpers::MenuElem(
                    _("Clear Window"),
                    SigC::slot(_notebook, &MainNotebook::clearWindow)));

        menulist.push_back(Gtk::Menu_Helpers::MenuElem(
                    _("Clear All Windows"),
                    SigC::slot(_notebook, &MainNotebook::clearAll)));

        menulist.push_back(Gtk::Menu_Helpers::MenuElem(
                    _("Close Current Tab"),
                    Gtk::Menu::AccelKey("<control>w"),
                    SigC::slot(*this, &MainWindow::closeCurrentTab)));


        menulist.push_back(Gtk::Menu_Helpers::SeparatorElem::SeparatorElem());
        menulist.push_back(
                Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::QUIT,
                    Gtk::Menu::AccelKey("<control>q"),
                    SigC::slot(*this, &Gtk::Window::hide)));
    }

    { // View menu.
        Gtk::Menu::MenuList& menulist = _viewmenu.items();


        menulist.push_back(Gtk::Menu_Helpers::MenuElem(
                    _("_Menubar"),
                    Gtk::Menu::AccelKey("<control>m"),
                    SigC::slot(*this, &MainWindow::hideMenu)));

        menulist.push_back(Gtk::Menu_Helpers::MenuElem(
                    _("Status_bar"),
                    Gtk::Menu::AccelKey("<control>b"),
                    SigC::slot(*this, &MainWindow::hideStatusbar)));

        menulist.push_back(Gtk::Menu_Helpers::SeparatorElem::SeparatorElem());

        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(
                    _("User _List"),
                    Gtk::Menu::AccelKey("<control>l"),
                    SigC::slot(*this, &MainWindow::hideNickList)));

        menulist.push_back(Gtk::Menu_Helpers::MenuElem(
                    _("_Server List"),
                    Gtk::Menu::AccelKey("<control>s"),
                    SigC::slot(*this, &MainWindow::openServerWindow)));
        menulist.push_back(Gtk::Menu_Helpers::MenuElem(
                    _("_DCC Transfers"),
                    Gtk::Menu::AccelKey("<control>d"),
                    SigC::slot(*this, &MainWindow::openDccWindow)));
        menulist.push_back(
                Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::PREFERENCES,
                    Gtk::Menu::AccelKey("<control>p"),
                    SigC::slot(*this, &MainWindow::openPrefs)));


    }

    { // Help menu.
        Gtk::Menu::MenuList& menulist = _helpmenu.items();

        menulist.push_back(Gtk::Menu_Helpers::MenuElem(
                    _("_Introduction"), SigC::slot(*this, &MainWindow::openHelpIntro)));

        menulist.push_back(Gtk::Menu_Helpers::SeparatorElem::SeparatorElem());

        menulist.push_back(Gtk::Menu_Helpers::MenuElem(
                    _("_About"), SigC::slot(*this, &MainWindow::openAboutWindow)));
    }

    _menubar.items().push_back(Gtk::Menu_Helpers::MenuElem(_("_LostIRC"), _firstmenu));
    _menubar.items().push_back(Gtk::Menu_Helpers::MenuElem(_("_View"), _viewmenu));
    _menubar.items().push_back(Gtk::Menu_Helpers::MenuElem(_("_Help"), _helpmenu));
}


void MainWindow::hideMenu()
{
    _app.options.hidemenu = !_app.options.hidemenu;
    if (_app.options.hidemenu)
          _menubar.hide();
    else
          _menubar.show();
}

void MainWindow::hideStatusbar()
{
    _app.options.hidestatusbar = !_app.options.hidestatusbar;
    if (_app.options.hidestatusbar)
          _statusbar.hide();
    else
          _statusbar.show();
}

void MainWindow::hideNickList()
{
    _app.options.hidenicklist = !_app.options.hidenicklist;
    vector<Tab*> tabs;

    _notebook.Tabs(tabs);
    std::for_each(tabs.begin(), tabs.end(), std::mem_fun(&Tab::toggleNickList));
}

void MainWindow::openServerWindow()
{
    if (_serverwin.get()) {
          _serverwin->present();
    } else {
        std::auto_ptr<ServerWindow> dialog(new ServerWindow(*this));

        dialog->show();

        _serverwin = dialog;
    }
}

void MainWindow::openHelpIntro()
{
    if (_helpwin.get()) {
          _helpwin->present();
    } else {
        std::auto_ptr<Gtk::MessageDialog> dialog(new Gtk::MessageDialog(_("LostIRC Quick Introduction\n\nThis help window is a quick guide to get you going with LostIRC.\nMove this window away from the LostIRC window, and use it as a quick reference window until you know the general idea.\n\nYou can connect to a server using:\n    /SERVER <hostname / ip>\n\n...and then join a channel:\n    /JOIN <channel-name>\n\nA list of all commands are available with:\n    /COMMANDS\n\nAnd you should really check out the list of key bindings:\n    /KEYBINDINGS"), Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, false));

        dialog->signal_response().connect(slot(*this, &MainWindow::hideHelpIntro));
        dialog->show();

        _helpwin = dialog;
    }
}

void MainWindow::openAboutWindow()
{
    if (_aboutwin.get()) {
          _aboutwin->present();
    } else {
        std::auto_ptr<Gtk::MessageDialog> dialog(new Gtk::MessageDialog(_("LostIRC "VERSION), Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, false));

        dialog->signal_response().connect(slot(*this, &MainWindow::hideAboutWindow));
        dialog->show();

        _aboutwin = dialog;
    }
}

void MainWindow::hideHelpIntro(int response)
{
    _helpwin->hide();
}

void MainWindow::hideAboutWindow(int response)
{
    _aboutwin->hide();
}

void MainWindow::closeCurrentTab()
{
    Tab *tab = _notebook.getCurrent();
    if (tab->isType(Tab::CHANNEL) && tab->getConn()->Session.isConnected && tab->isActive()) {
        // It's a channel, so we need to part it
        tab->getConn()->sendPart(tab->getName(), "");
    } else {
        // Query
        tab->getConn()->removeChannel(tab->getName());
    }
    _notebook.closeCurrent();
}

bool MainWindow::on_key_press_event(GdkEventKey* e)
{
    // CTRL key.
    if (e->state & GDK_CONTROL_MASK) {
        if (e->keyval == GDK_0) {
            _notebook.set_current_page(9);
        } else if (e->keyval == GDK_1) {
            _notebook.set_current_page(0);
        } else if (e->keyval == GDK_2) {
            _notebook.set_current_page(1);
        } else if (e->keyval == GDK_3) {
            _notebook.set_current_page(2);
        } else if (e->keyval == GDK_4) {
            _notebook.set_current_page(3);
        } else if (e->keyval == GDK_5) {
            _notebook.set_current_page(4);
        } else if (e->keyval == GDK_6) {
            _notebook.set_current_page(5);
        } else if (e->keyval == GDK_7) {
            _notebook.set_current_page(6);
        } else if (e->keyval == GDK_8) {
            _notebook.set_current_page(7);
        } else if (e->keyval == GDK_9) {
            _notebook.set_current_page(8);
        } else if (e->keyval == GDK_h) {
            _notebook.getCurrent()->getText().scrollToHighlightMark();
        } else if (e->keyval == GDK_End) {
            _notebook.getCurrent()->getText().scrollToBottom();
        } else if (e->keyval == GDK_Home) {
            _notebook.getCurrent()->getText().scrollToTop();
        }
        if (e->keyval == GDK_Page_Up) {
            _notebook.prev_page();

        } else if (e->keyval == GDK_Page_Down) {
            _notebook.next_page();
        }
    } else if (e->state & GDK_MOD1_MASK) {
        // ALT key.
        if (e->keyval == GDK_Left)
              _notebook.prev_page();
        else if (e->keyval == GDK_Right)
              _notebook.next_page();
    }
    if (e->keyval == GDK_Page_Up) {
        _notebook.getCurrent()->getText().scrollUpPage();

    } else if (e->keyval == GDK_Page_Down) {
        _notebook.getCurrent()->getText().scrollDownPage();
    }

    Gtk::Window::on_key_press_event(e);
    return false;
}
