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

#include <ctime>
#include <ServerConnection.h>
#include <Utils.h>
#include "Tab.h"

using std::vector;
using std::string;

Tab::Tab(Gtk::Label *label, ServerConnection *conn, Gdk_Font *font)
    : Gtk::VBox(), _label(label), _conn(conn), is_highlighted(false), _font(font)
{
    // To hold current context (colors) for Text widget
    _current_cx = new Gtk::Text::Context;

    // Creating HBox; will contain 2 widgets, a scrollwindow and an entry
    _hbox = manage(new Gtk::HBox()); 
    _scrollwindow = manage(new Gtk::ScrolledWindow());
    _entry = manage(new Entry(this));

    // Attaching Gtk::Text to scollwindow
    _text = manage(new Gtk::Text());
    _text->set_word_wrap(true);
    _scrollwindow->set_policy(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    _scrollwindow->add(*_text);

    setStyle();

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

void Tab::setFont(Gdk_Font *font)
{
    _font = font;
}

void Tab::setStyle() {
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
    style->set_font(*_font);
    style->set_base(GTK_STATE_NORMAL, col1);
    style->set_bg(GTK_STATE_NORMAL, col1);
    style->set_text(GTK_STATE_NORMAL, col2);
    style->set_fg(GTK_STATE_PRELIGHT, col2);
    _text->set_style(*style);
}

void Tab::parseAndInsert(const string& str)
{
    time_t timeval = time(0);
    char tim[16];
    strftime(tim, 15, "$1%H:%M:%S ", localtime(&timeval));

    string line(tim + str);
    string::size_type lastPos = line.find_first_not_of("$", 0);
    string::size_type pos = line.find_first_of("$", lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {   
        stringstream ss(line.substr(lastPos, 1));
        int color;
        ss >> color;
        if (ss.fail())
              color = 0;

        insertWithColor(color, line.substr(lastPos, pos - lastPos));
        lastPos = line.find_first_not_of("$", pos);
        pos = line.find_first_of("$", lastPos);
    }

}

void Tab::insertWithColor(int color, const string& str)
{   
    Gdk_Color colors[8];

    colors[0] = Gdk_Color("#C5C2C5");
    colors[1] = Gdk_Color("#FFFFFF");
    colors[2] = Gdk_Color("#FFABCF");
    colors[3] = Gdk_Color("#9AAB4F");
    colors[4] = Gdk_Color("#f9ef25");
    colors[5] = Gdk_Color("#ea6b6b");
    colors[6] = Gdk_Color("#6bdde5");
    colors[7] = Gdk_Color("#6b8ae5");
    colors[8] = Gdk_Color("#4aff4a");
    colors[9] = Gdk_Color("#5ea524");

    if (color == 0) {
        _text->insert(*_current_cx, "$" + str);
    } else {
        _current_cx->set_foreground(colors[color]);
        _text->insert(*_current_cx, str.substr(1));
    }
}

TabQuery::TabQuery(Gtk::Label *label, ServerConnection *conn, Gdk_Font *font)
    : Tab(label, conn, font)
{

}

TabChannel::TabChannel(Gtk::Label *label, ServerConnection *conn, Gdk_Font *font)
    : Tab(label, conn, font)
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


