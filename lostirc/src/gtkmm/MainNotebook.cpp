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

#include "MainNotebook.h"
#include "MainWindow.h"
#include "GuiCommands.h"

MainNotebook::MainNotebook(MainWindow *frontend)
    : Gtk::Notebook(), _fe(frontend)
{
    GuiCommands::nb = this;
    set_tab_pos(GTK_POS_BOTTOM);
    switch_page.connect(slot(this, &MainNotebook::switchPage));
    _font = Gdk_Font("-b&h-lucidatypewriter-medium-r-normal-*-*-120-*-*-m-*-*-*");
    show_all();
}

TabChannel * MainNotebook::addChannelTab(const string& name, ServerConnection *conn)
{
    Gtk::Notebook_Helpers::Page *p = findPage("<server>", conn);

    if (p) {
        TabChannel* tab = dynamic_cast<TabChannel*>(p->get_child());
        tab->getLabel()->set_text(name);
        show_all();
        return tab;
    } else {
        Gtk::Label *label = new Gtk::Label(name);
        TabChannel *tab = new TabChannel(label, conn, &_font);
        pages().push_back(Gtk::Notebook_Helpers::TabElem(*tab, *label));
        show_all();
        return tab;
    }
}

TabQuery * MainNotebook::addQueryTab(const string& name, ServerConnection *conn)
{
    Gtk::Label *label = new Gtk::Label(name);
    TabQuery *tab = new TabQuery(label, conn, &_font);
    pages().push_back(Gtk::Notebook_Helpers::TabElem(*tab, *label));
    show_all();
    return tab;
}

Tab* MainNotebook::getCurrent(ServerConnection *conn)
{
    Tab *tab = dynamic_cast<Tab*>(get_current()->get_child());
    if (tab->getConn() != conn) {
        tab = findTab("", conn);
    }
    return tab;
}

Tab* MainNotebook::getCurrent()
{
    Tab *tab = dynamic_cast<Tab*>(get_current()->get_child());
    return tab;
}

Tab * MainNotebook::findTab(const string& name, ServerConnection *conn)
{
    Gtk::Notebook_Helpers::Page *p = findPage(name, conn);

    if (p) {
        Tab* tab = dynamic_cast<Tab*>(p->get_child());
        return tab;
    }
    return 0;
}

Gtk::Notebook_Helpers::Page * MainNotebook::findPage(const string& name, ServerConnection *conn)
{
    string n = name;
    Gtk::Notebook_Helpers::PageList::iterator i;
            
    for (i = pages().begin(); i != pages().end(); ++i) {
        string tab_name = (*i)->get_tab_text();
        if (Utils::tolower(tab_name) == Utils::tolower(n)) {
            Tab *tab = dynamic_cast<Tab*>((*i)->get_child());
            if (tab->getConn() == conn) {
                return (*i);
            }
        }
    }
    return 0;
}

void MainNotebook::switchPage(Gtk::Notebook_Helpers::Page *p, unsigned int n)
{
    Tab *tab = dynamic_cast<Tab*>(p->get_child());
    if (tab) {
        string nick = tab->getConn()->Session.nick;
        Gdk_Color color("black");
        Gtk::Style *style = Gtk::Style::create();
        style->set_fg(GTK_STATE_NORMAL, color);
        tab->getLabel()->set_style(*style);
        tab->getEntry()->grab_focus();
        tab->is_highlighted = false;
        if (_fe->isAway) {
            _fe->set_title("LostIRC"VERSION" - " + nick + "[currently away] @ " + p->get_tab_text());
        } else {
            _fe->set_title("LostIRC"VERSION" - " + nick + " @ " + p->get_tab_text());
        }
    } else {
        _fe->set_title("LostIRC"VERSION" - " + p->get_tab_text());
    }
}

void MainNotebook::closeCurrent()
{
    Gtk::Notebook::Page *p = get_current();
    pages().remove(p);
    draw(NULL); // Needed for redrawing the widget
}

void MainNotebook::highlight(Tab *tab)
{
    if (tab != getCurrent()) {
        Gdk_Color color("blue");
        Gtk::Style *style = Gtk::Style::create();
        style->set_fg(GTK_STATE_NORMAL, color);
        tab->getLabel()->set_style(*style);
        tab->is_highlighted = true;
    }
}

void MainNotebook::insert(Tab *tab, const string& str)
{   
    if (tab != getCurrent() && !tab->is_highlighted) {
        Gdk_Color color("red");
        Gtk::Style *style = Gtk::Style::create();
        style->set_fg(GTK_STATE_NORMAL, color);
        tab->getLabel()->set_style(*style);
    }
    tab->parseAndInsert(str);
}

void MainNotebook::findTabsContaining(const string& nick, vector<Tab*>& vec)
{
    Gtk::Notebook_Helpers::PageList::iterator i;
            
    for (i = pages().begin(); i != pages().end(); ++i) {
        Tab *tab = dynamic_cast<Tab*>((*i)->get_child());
        if (tab->findUser(nick)) {
            vec.push_back(tab);
        }
    }
}

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
    Gtk::Style *style = Gtk::Style::create();
    _font = fontdialog->get_font();

    if (_font) {
        Gtk::Notebook_Helpers::PageList::iterator i;
                
        for (i = pages().begin(); i != pages().end(); ++i) {
            Tab *tab = dynamic_cast<Tab*>((*i)->get_child());
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
