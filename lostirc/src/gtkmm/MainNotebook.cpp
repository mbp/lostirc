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
    signal_switch_page().connect(slot(*this, &MainNotebook::switchPage));
    //_font = Gdk_Font("-b&h-lucidatypewriter-medium-r-normal-*-*-120-*-*-m-*-*-*");
    //_font = Gdk_Font("-*-fixed-medium-r-normal-*-14-*-*-*-c-*-*-*");
    show_all();
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
        TabChannel *tab = manage(new TabChannel(label, conn)); //, &_font));
        pages().push_back(Gtk::Notebook_Helpers::TabElem(*tab, *label));
        show_all();
        return tab;
    }
}

TabQuery * MainNotebook::addQueryTab(const string& name, ServerConnection *conn)
{
    Gtk::Label *label = manage(new Gtk::Label(name));
    TabQuery *tab = manage(new TabQuery(label, conn)); //, &_font));
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
            if ((Util::lower(tab_name) == Util::lower(n))) {
                return i->get_page_num();
            } else if (findInActive && Util::lower(tab_name) == string("(" + Util::lower(n) + ")")) {
                return i->get_page_num();
            }
        }
    }
    return -1;
}

//void MainNotebook::switchPage(Gtk::Notebook_Helpers::Page *p, unsigned int n)
void MainNotebook::switchPage(GtkNotebookPage *p, unsigned int n)
{
    Tab *tab = static_cast<Tab*>(get_nth_page(n));

    Gdk::Color color("black");
    tab->getLabel()->modify_fg(Gtk::STATE_NORMAL, color);
    tab->getEntry().grab_focus();
    tab->isHighlighted = false;

    if (tab->getConn()->Session.isAway) {
        AppWin->set_title("LostIRC "VERSION" - " + tab->getConn()->Session.nick + "[currently away]: " + tab->getLabel()->get_text());
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

void MainNotebook::highlight(Tab *tab)
{
    if (tab != getCurrent()) {
        Gdk::Color color("blue");
        tab->getLabel()->modify_fg(Gtk::STATE_NORMAL, color);
        tab->isHighlighted = true;
    }
}

void MainNotebook::onInserted(Tab *tab)
{   
    if (tab != getCurrent() && !tab->isHighlighted) {
        Gdk::Color color("red");
        tab->getLabel()->modify_fg(Gtk::STATE_NORMAL, color);
    }
}

void MainNotebook::findTabs(const string& nick, ServerConnection *conn, vector<Tab*>& vec)
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

void MainNotebook::clearAll()
{
    Gtk::Notebook_Helpers::PageList::iterator i;

    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = static_cast<Tab*>(i->get_child());
        tab->clearText();
    }
}

/* FIXME
void MainNotebook::setFont()
{
    fontdialog = manage(new Gtk::FontSelectionDialog("Font Selection Dialog"));
    fontdialog->get_ok_button()->clicked.connect(slot(this, &MainNotebook::fontSelectionOk));
    fontdialog->get_cancel_button()->clicked.connect(bind(slot(this, &MainNotebook::destroyFontSelection), fontdialog));
    fontdialog->destroy.connect(bind(slot(this, &MainNotebook::destroyFontSelection), fontdialog));
    fontdialog->set_preview_text("<John> Hello World!");
    fontdialog->show();
}

void MainNotebook::fontSelectionOk()
{
    _font = fontdialog->get_font();

    if (_font) {
        Gtk::Notebook_Helpers::PageList::iterator i;
                
        for (i = pages().begin(); i != pages().end(); ++i) {
            Tab *tab = static_cast<Tab*>(i->get_child());
            tab->setFont(&_font);
            tab->setStyle();
        }
    }
    destroyFontSelection(fontdialog);
}

void MainNotebook::destroyFontSelection(Gtk::FontSelectionDialog *w)
{
    // XXX: plain gtk+ code
    gtk_widget_destroy((GtkWidget*)w->gtkobj());
}
*/

