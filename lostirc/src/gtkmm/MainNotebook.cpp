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

#include "MainNotebook.h"
#include "MainWindow.h"

using std::string;
using std::vector;

MainNotebook::MainNotebook()
    : Gtk::Notebook()
{
    set_tab_pos(Gtk::POS_BOTTOM);
    fontdescription = Pango::FontDescription(App->getCfg().getOpt("font"));
    signal_switch_page().connect(SigC::slot(*this, &MainNotebook::onSwitchPage));
}

TabChannel * MainNotebook::addChannelTab(const string& name, ServerConnection *conn)
{
    // First try to find out whether we have a "server"-tab for this
    // ServerConnection.
    int pagenum = findPage("(server)", conn);

    if (pagenum != -1) {
        // If we have a "server"-tab, reuse it as a channel-tab.
        TabChannel* tab = dynamic_cast<TabChannel*>(get_nth_page(pagenum));
        tab->getLabel()->set_text(name);
        tab->setActive();
        show_all();
        return tab;
    } else if (Tab *tab = findTab(name, conn, true)) {
        // If we find an *inactive* channel-tab, lets reuse it.
        tab->setActive();
        tab->getLabel()->set_text(name);
        return dynamic_cast<TabChannel*>(tab);
    } else {
        // If not, create a new channel-tab.
        Gtk::Label *label = manage(new Gtk::Label(name));
        TabChannel *tab = manage(new TabChannel(label, conn, fontdescription));
        pages().push_back(Gtk::Notebook_Helpers::TabElem(*tab, *label));
        show_all();
        return tab;
    }
}

TabQuery * MainNotebook::addQueryTab(const string& name, ServerConnection *conn)
{
    Gtk::Label *label = manage(new Gtk::Label(name));
    TabQuery *tab = manage(new TabQuery(label, conn, fontdescription));
    pages().push_back(Gtk::Notebook_Helpers::TabElem(*tab, *label));
    show_all();
    return tab;
}

Tab* MainNotebook::getCurrent(ServerConnection *conn)
{
    Tab *tab = getCurrent();
    if (tab->getConn() != conn) {
        tab = findTab("", conn);
    }
    return tab;
}

Tab* MainNotebook::getCurrent()
{
    return static_cast<Tab*>(get_nth_page(get_current_page()));
}

Tab * MainNotebook::findTab(const string& name, ServerConnection *conn, bool findInActive)
{
    int pagenum = findPage(name, conn, findInActive);

    if (pagenum != -1) {
        return static_cast<Tab*>(get_nth_page(pagenum));
    }
    return 0;
}

int MainNotebook::findPage(const string& name, ServerConnection *conn, bool findInActive)
{
    string n = name;
    Gtk::Notebook_Helpers::PageList::iterator i;
            
    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        if (tab->getConn() == conn) {
            string tab_name = i->get_tab_label_text();
            if ((Util::lower(tab_name) == Util::lower(n)) || n.empty()) {
                return i->get_page_num();
            } else if (findInActive && Util::lower(tab_name) == string("(" + Util::lower(n) + ")")) {
                return i->get_page_num();
            }
        }
    }
    return -1;
}

void MainNotebook::onSwitchPage(GtkNotebookPage *p, unsigned int n)
{
    Tab *tab = static_cast<Tab*>(get_nth_page(n));

    tab->getLabel()->modify_fg(Gtk::STATE_NORMAL, Gdk::Color("black"));
    tab->getEntry().grab_focus();
    tab->isHighlighted = false;
    updateTitle(tab);
}

void MainNotebook::updateTitle(Tab *tab)
{
    if (!tab)
          tab = getCurrent();

    if (tab->getConn()->Session.isAway) {
        AppWin->set_title("LostIRC "VERSION" - " + tab->getConn()->Session.nick + " (currently away): " + tab->getLabel()->get_text());
    } else {
        AppWin->set_title("LostIRC "VERSION" - " + tab->getConn()->Session.nick + ": " + tab->getLabel()->get_text());
    }
}

void MainNotebook::closeCurrent()
{
    Tab *tab = getCurrent();
    if (countTabs(tab->getConn()) > 1) {
        pages().remove(*get_nth_page(get_current_page()));
    } else {
        if (tab->getConn()->Session.isConnected) {
            tab->getConn()->sendQuit();
            tab->getConn()->disconnect();
            tab->setInActive();
        } else if (pages().size() > 1) {
            // Only delete the page if it's not the very last.
            pages().remove(*get_nth_page(get_current_page()));
        }
    }
    queue_draw();
}

void MainNotebook::highlightNick(Tab *tab)
{
    if (tab != getCurrent()) {
        tab->getLabel()->modify_fg(Gtk::STATE_NORMAL, Gdk::Color("blue"));
        tab->isHighlighted = true;
    }
}

void MainNotebook::highlightActivity(Tab *tab)
{   
    if (tab != getCurrent() && !tab->isHighlighted) {
        tab->getLabel()->modify_fg(Gtk::STATE_NORMAL, Gdk::Color("red"));
    }
}

void MainNotebook::findTabs(const string& nick, ServerConnection *conn, vector<Tab*>& vec)
{
    Gtk::Notebook_Helpers::PageList::iterator i;
            
    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        if (tab->getConn() == conn && tab->findUser(Glib::locale_to_utf8(nick))) {
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

void MainNotebook::clearAll()
{
    Gtk::Notebook_Helpers::PageList::iterator i;

    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        tab->clearText();
    }
}

void MainNotebook::setFont(const Glib::ustring& str)
{
    fontdescription = Pango::FontDescription(str);

    Gtk::Notebook_Helpers::PageList::iterator i;

    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        tab->setFont(fontdescription);
    }
}
