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

#include <ServerConnection.h>
#include <Commands.h>
#include "GuiCommands.h"
#include "MainNotebook.h"
#include "Tab.h"

Tab::Tab(Gtk::Label *label, ServerConnection *conn)
    : Gtk::VBox(), _label(label), _conn(conn), is_highlighted(false)
{

    // Creating HBox; will contain 2 widgets, a scrollwindow and an entry
    _hbox = manage(new Gtk::HBox()); 
    _scrollwindow = manage(new Gtk::ScrolledWindow());
    _entry = manage(new Entry(this));

    // Attaching Gtk::Text to scollwindow
    _text = manage(new Gtk::Text());
    _text->set_word_wrap(true);
    _scrollwindow->set_policy(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    _scrollwindow->add(*_text);

    // Should go into ressource file!
    GdkColor col1;
    col1.red   = 0;
    col1.green = 0;
    col1.blue  = 0;

    GdkColor col2;
    col2.red   = 50000;
    col2.green = 50000;
    col2.blue  = 50000;
    
    Gtk::Style *style = Gtk::Style::create();
    style->set_font(Gdk_Font(
                "-b&h-lucidatypewriter-medium-r-normal-*-*-120-*-*-m-*-*-*"));
    style->set_base(GTK_STATE_NORMAL, col1);
    style->set_bg(GTK_STATE_NORMAL, col1);
    style->set_text(GTK_STATE_NORMAL, col2);
    style->set_fg(GTK_STATE_PRELIGHT, col2);
    _text->set_style(*style);

    _hbox->pack_start(*_scrollwindow);
    pack_start(*_hbox);
    pack_start(*_entry, 0, 1);

}

Gtk::Text* Tab::getText()
{
    return _text;
}

Gtk::HBox* Tab::getHBox()
{
    return _hbox;
}

Gtk::Label* Tab::getLabel()
{
    return _label;
}

Entry* Tab::getEntry()
{
    return _entry;
}

ServerConnection* Tab::getConn()
{
    return _conn;
}


TabQuery::TabQuery(Gtk::Label *label, ServerConnection *conn)
    : Tab(label, conn)
{

}

TabChannel::TabChannel(Gtk::Label *label, ServerConnection *conn)
    : Tab(label, conn)
{
    Gtk::ScrolledWindow *swin = manage(new Gtk::ScrolledWindow());
    swin->set_policy(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    _clist = manage(new Gtk::CList(2));
    _clist->set_column_width(0, 10);
    _clist->set_auto_sort(1);
    _clist->set_sort_type(GTK_SORT_DESCENDING);
    _clist->set_usize(100, 100);
    swin->add(*_clist);
    getHBox()->pack_start(*swin, 0, 0, 0);
}

void TabChannel::insertUser(const vector<string>& users)
{
    vector<string>::const_iterator i;

    _clist->rows().push_back(users);
}

void TabChannel::insertUser(const string& user)
{
    vector<string> users;
    users.push_back(" ");
    users.push_back(user);
    _clist->rows().push_back(users);
}

void TabChannel::removeUser(const string& nick)
{
    Gtk::CList_Helpers::RowIterator i = _clist->rows().begin();

    while(i != _clist->rows().end())
    {
        int row = i->get_row_num();
        string text = _clist->cell(row, 1).get_text();

        if (text == nick) {
            _clist->rows().remove(_clist->row(row));
            break;
        }
        i++;
    }
}

bool TabChannel::findUser(const string& nick)
{
    Gtk::CList_Helpers::RowIterator i = _clist->rows().begin();

    while(i != _clist->rows().end())
    {
        int row = i->get_row_num();
        string text = _clist->cell(row, 1).get_text();

        if (text == nick) {
            return true;
        }
        i++;
    }
    return false;
}

Gtk::CList* TabChannel::getCList()
{
    return _clist;
}

bool TabChannel::nickCompletion(const string& word, string& str)
{
    Gtk::CList_Helpers::RowIterator i = getCList()->rows().begin();

    int matches = 0;
    string nicks;
    // Convert it to lowercase so we can search ignoring the case
    string lcword = word;
    lcword = Utils::tolower(lcword);
    while(i != getCList()->rows().end())
    {
        int row = i->get_row_num();
        string nick = getCList()->cell(row, 1).get_text();

        // Lower case again
        string lcnick = nick;
        lcnick = Utils::tolower(lcnick);
        if (lcword == lcnick.substr(0, lcword.length())) {
            str = nick;
            nicks += nick + " ";
            matches++;
        }
        i++;
    }
    if (matches == 1) {
        return true;
    } else if (matches > 1) {
        str = nicks + "\n";
        return false;
    } else if (matches == 0) {
        str = "";
        return false;
    }
}


Entry::Entry(Tab* tab)
    : Gtk::Entry(510), _tab(tab)
{
    key_press_event.connect(slot(this, &Entry::on_key_press_event));
    activate.connect(slot(this, &Entry::onEntry));

}

void Entry::onEntry()
{
    if (get_text().length() == 0)
          return;

    string msg(get_text());
    if (msg.at(0) == '/') {

        string::size_type pos = msg.find_first_of(" ");

        string params;
        if (pos != string::npos) {
            params = msg.substr(pos + 1);
        }

        if(!GuiCommands::send(_tab->getConn(), msg.substr(1, pos - 1), params)) {
            _tab->getText()->insert(Commands::error + "\n");
        }

    } else {
        if (!_tab->getConn()->Session.isConnected && msg.size() > 0) {
            _tab->getText()->insert("Not connected to server.\n");
        } else if (msg.size() > 0) {
            _tab->getConn()->sendMsg(_tab->getLabel()->get_text(), msg);
            _tab->getText()->insert("<" + _tab->getConn()->Session.nick + "> " + msg + "\n");
        }
    }

    _entries.push_back(msg);
    set_text("");
}

gint Entry::on_key_press_event(GdkEventKey* e)
{
    if ((e->keyval == GDK_uparrow) || (e->keyval == GDK_Up)) {
        if (!_entries.empty()) {
            set_text(_entries.front());
        }
    }

    // Nick completion using Tab key
    if ((e->keyval == GDK_Tab)) {
        string line = get_text();
        if (line.length() > 0) {
            string str;
            string::size_type pos = line.find_last_of(" ");
            string word;
            if (pos == string::npos) {
                pos = 0;
                word = line.substr(pos);
            } else {
                word = line.substr(pos + 1);
            }
            if (_tab->nickCompletion(word, str)) {
                if (pos == 0) {
                    set_text("");
                    append_text(str + ", ");
                } else {
                    set_text(line.substr(0, pos + 1));
                    append_text(str);
                }
            } else {
                _tab->getText()->insert(str);
            }
        }
    }
}
