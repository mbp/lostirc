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

#include <config.h>
#include "MainNotebook.h"
#include "MainWindow.h"

using Glib::ustring;
using std::vector;

MainNotebook::MainNotebook()
{
    set_tab_pos(Gtk::POS_BOTTOM);
    set_scrollable(true);
    _fontdesc = Pango::FontDescription(App->options.font);
    signal_switch_page().connect(sigc::mem_fun(*this, &MainNotebook::onSwitchPage));
}

Tab* MainNotebook::addTab(Tab::Type type, const ustring& name, ServerConnection *conn)
{
    Tab *tab = findTab(Tab::SERVER, conn);

    if (tab) {
        // If we have a server-tab, reuse it
        // no-op if statement
    } else if ((tab = findTab(name, conn, true))) {
        // If we find an *inactive* tab, lets reuse it.
        // no-op if statement
    } else {
        Gtk::Label *label = manage(new Gtk::Label());
        tab = manage(new Tab(conn, _fontdesc, label));
        pages().push_back(Gtk::Notebook_Helpers::TabElem(*tab, *label));
    }

    tab->setActive();
    tab->setType(type);
    tab->setName(name);

    return tab;
}

Tab* MainNotebook::getCurrent(ServerConnection *conn)
{
    Tab *tab = static_cast<Tab*>(get_nth_page(get_current_page()));

    if (conn && tab->getConn() != conn)
          tab = findTab("", conn);

    return tab;
}

Tab * MainNotebook::findTab(const ustring& name, ServerConnection *conn, bool findInActive)
{
    ustring n = name;
    Gtk::Notebook_Helpers::PageList::iterator i;
            
    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        if (tab->getConn() == conn) {
            ustring tab_name = tab->getName();
            if ((Util::lower(tab_name) == Util::lower(n)) || n.empty()) {
                if ((!tab->isActive() && findInActive) || tab->isActive())
                  return tab;
            }
        }
    }
    return 0;
}

Tab * MainNotebook::findTab(Tab::Type type, ServerConnection *conn, bool findInActive)
{
    Gtk::Notebook_Helpers::PageList::iterator i;
            
    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        if (tab->getConn() == conn && tab->isType(type))
              return static_cast<Tab*>(get_nth_page(i->get_page_num()));
    }
    return 0;
}

void MainNotebook::onSwitchPage(GtkNotebookPage *p, unsigned int n)
{
    Tab *tab = static_cast<Tab*>(get_nth_page(n));

    tab->removeHighlight();

    updateStatus(tab);
    updateTitle(tab);
}

void MainNotebook::updateStatus(Tab *tab)
{
    if (!tab)
          tab = getCurrent();

    if (tab->getConn()->Session.isAway)
          AppWin->_statusbar.setText1(tab->getConn()->Session.nick + _(" <span foreground=\"red\">(away: ") + tab->getConn()->Session.awaymsg + ")</span> - " + tab->getConn()->Session.servername);
    else
          AppWin->_statusbar.setText1(tab->getConn()->Session.nick + " - " + tab->getConn()->Session.servername);

}

void MainNotebook::updateTitle(Tab *tab)
{
    if (!tab)
          tab = getCurrent();

    if (tab->getConn()->Session.isAway)
          AppWin->set_title(tab->getName() +  _(" (currently away)") + " - LostIRC");
    else
          AppWin->set_title(tab->getName() + " - LostIRC");
}

void MainNotebook::closeCurrent()
{
    Tab *tab = getCurrent();

    if (countTabs(tab->getConn()) > 1) {
        pages().erase(get_current());
    } else {
        if (tab->getConn()->Session.isConnected) {
            tab->getConn()->sendQuit();
            tab->getConn()->disconnect();
            tab->setInActive();
        } else if (pages().size() > 1) {
            // Only delete the page if it's not the very last.
            pages().erase(get_current());
        }
    }
    queue_draw();
}

void MainNotebook::findTabs(const ustring& nick, ServerConnection *conn, vector<Tab*>& vec)
{
    Gtk::Notebook_Helpers::PageList::iterator i;
            
    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        if (tab->getConn() == conn && tab->findUser(nick)) {
            vec.push_back(tab);
        }
    }
}

void MainNotebook::findTabs(ServerConnection *conn, vector<Tab*>& vec)
{
    Gtk::Notebook_Helpers::PageList::iterator i;
            
    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        if (tab->getConn() == conn) {
            vec.push_back(tab);
        }
    }
}

void MainNotebook::Tabs(vector<Tab*>& vec)
{
    Gtk::Notebook_Helpers::PageList::iterator i;
            
    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        vec.push_back(tab);
    }
}

int MainNotebook::countTabs(ServerConnection *conn)
{
    int num = 0;

    Gtk::Notebook_Helpers::PageList::iterator i;

    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        if (tab->getConn() == conn)
              num++;
    }
    return num;
}

void MainNotebook::clearWindow()
{
    getCurrent()->getText().clearText();
}

void MainNotebook::clearAll()
{
    Gtk::Notebook_Helpers::PageList::iterator i;

    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        tab->getText().clearText();
    }
}

void MainNotebook::setFont(const Glib::ustring& str)
{
    _fontdesc = Pango::FontDescription(str);

    Gtk::Notebook_Helpers::PageList::iterator i;

    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        tab->getText().setFont(_fontdesc);
    }
}
