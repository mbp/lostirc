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

#include <pangomm/fontdescription.h>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <ServerConnection.h>
#include <Utils.h>
#include "Tab.h"
#include "Prefs.h"
#include "MainWindow.h"

using std::vector;
using Glib::ustring;

Tab::Tab(ServerConnection *conn, Pango::FontDescription font)
    : Gtk::VBox(), isHighlighted(false), hasPrefs(false),
    isOnChannel(true), _conn(conn), _textwidget(font), 
    _entry(this)
{
    _hpaned = new Gtk::HPaned();

    _hbox.pack_start(_entry);

    _vbox.pack_start(_textwidget);

    _vbox.pack_start(_hbox, Gtk::PACK_SHRINK);

    Gtk::Button *_button = manage(new Gtk::Button("Prefs"));
    _button->signal_clicked().connect(slot(*this, &Tab::startPrefs));
    _hbox.pack_start(*_button, Gtk::PACK_SHRINK);

    _hpaned->pack1(_vbox, true, true);
    pack_start(*_hpaned);
}

Tab::~Tab()
{
    delete _hpaned;
}

void Tab::startPrefs()
{
    remove(*_hpaned);
    Prefs *p = Prefs::Instance();
    if (Prefs::currentTab)
          p->endPrefs();

    Prefs::currentTab = this;
    pack_start(*p);
    hasPrefs = true;
}

void Tab::endPrefs()
{
    Prefs *p = Prefs::Instance();
    remove(*p);
    Prefs::currentTab = 0;
    pack_start(*_hpaned);
    _entry.grab_focus();
    hasPrefs = false;
}

TabChannel::TabChannel(ServerConnection *conn, Pango::FontDescription font)
    : Tab(conn, font),
    _columns(),
    _liststore(Gtk::ListStore::create(_columns)),
    _treeview(_liststore),
    _users("Not on channel")
{

    Gtk::ScrolledWindow *swin = manage(new Gtk::ScrolledWindow());
    swin->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    _treeview.append_column("", _columns.status);
    _treeview.append_column("", _columns.nick);
    _treeview.get_selection()->set_mode(Gtk::SELECTION_NONE);
    _liststore->set_default_sort_func(SigC::slot(*this, &TabChannel::sortFunc));
    _liststore->set_sort_column_id(Gtk::TreeSortable::DEFAULT_SORT_COLUMN_ID, Gtk::SORT_ASCENDING);

    /* FIXME: no set_column_width() like in the old days! */

    _treeview.set_headers_visible(false);
    //_treeview.set_default_size(100, 100);
    swin->add(_treeview);

    _users.add(*swin);
    _hpaned->pack2(_users, false, true);
}

void TabChannel::updateUserNumber()
{
    size_t size = _liststore->children().size();
    if (size > 0) {
        std::stringstream ss;
        ss << size;
        _users.set_label(ss.str() + " users");
    } else {
        _users.set_label("Not on channel");
    }
}

void TabChannel::insertUser(const ustring& nick, IRC::UserMode m)
{
    Gtk::TreeModel::Row row = *_liststore->append();
    row[_columns.nick] = nick;

    switch (m)
    {
        case IRC::OP:
            row[_columns.status] = "@";
            row[_columns.priority] = 3;
            break;
        case IRC::VOICE:
            row[_columns.status] = "+";
            row[_columns.priority] = 2;
            break;
        case IRC::HALFOP:
            row[_columns.status] = "%";
            row[_columns.priority] = 1;
            break;
        case IRC::NONE:
            row[_columns.status] = " ";
            row[_columns.priority] = 0;
    }

    updateUserNumber();
}

void TabChannel::removeUser(const ustring& nick)
{
    Gtk::ListStore::iterator i = _liststore->children().begin();

    while (i != _liststore->children().end())
    {
        if (i->get_value(_columns.nick) == nick)
              i = _liststore->erase(i);
        else
              ++i;
    }

    updateUserNumber();
}

void TabChannel::renameUser(const ustring& from, const ustring& to)
{
    Gtk::ListStore::iterator i = _liststore->children().begin();

    while (i != _liststore->children().end())
    {
        if (i->get_value(_columns.nick) == from) {
            i->set_value(_columns.nick, to);
            break;
        }
        i++;
    }
}

bool TabChannel::findUser(const ustring& nick)
{
    Gtk::ListStore::iterator i = _liststore->children().begin();

    while (i != _liststore->children().end())
    {
        if (i->get_value(_columns.nick) == nick)
              return true;
        i++;
    }
    return false;
}

vector<ustring> TabChannel::getNicks()
{
    vector<ustring> nicks;

    Gtk::ListStore::iterator i = _liststore->children().begin();

    for (i = _liststore->children().begin(); i != _liststore->children().end(); ++i)
    {
        nicks.push_back(i->get_value(_columns.nick));
    }

    return nicks;
}

int TabChannel::sortFunc(const Gtk::TreeModel::iterator& i1, const Gtk::TreeModel::iterator& i2)
{
    // Sort the nicklist. The status field has highest priority, nick has
    // second priority.

    // This is not very readable, but it works, and it has to be fast when
    // joining huge channels.

    if (i1->get_value(_columns.priority) == i2->get_value(_columns.priority))
        return i1->get_value(_columns.nick).compare(i2->get_value(_columns.nick));
    else if (i1->get_value(_columns.priority) > i2->get_value(_columns.priority))
          return -1;
    else
          return 1;

}
