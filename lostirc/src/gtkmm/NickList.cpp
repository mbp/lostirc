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

#include <sstream>
#include "NickList.h"

using std::vector;
using Glib::ustring;

NickList::NickList()
    : Gtk::Frame("Not on channel"),
    _columns(),
    _liststore(Gtk::ListStore::create(_columns)),
    _treeview(_liststore)
{

    Gtk::ScrolledWindow *swin = manage(new Gtk::ScrolledWindow());
    swin->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    _treeview.append_column("", _columns.status);
    _treeview.append_column("", _columns.nick);
    _treeview.get_selection()->set_mode(Gtk::SELECTION_NONE);
    _liststore->set_default_sort_func(SigC::slot(*this, &NickList::sortFunc));
    _liststore->set_sort_column_id(Gtk::TreeSortable::DEFAULT_SORT_COLUMN_ID, Gtk::SORT_ASCENDING);

    /* FIXME: no set_column_width() like in the old days! */

    _treeview.set_headers_visible(false);
    //_treeview.set_default_size(100, 100);
    swin->add(_treeview);

    add(*swin);
}

void NickList::updateUserNumber()
{
    size_t size = _liststore->children().size();
    if (size > 0) {
        std::stringstream ss;
        ss << size << (size == 1 ? " user" : " users");
        set_label(ss.str());
    } else {
        set_label("Not on channel");
    }
}

void NickList::insertUser(const ustring& nick, IRC::UserMode m)
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

void NickList::removeUser(const ustring& nick)
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

void NickList::renameUser(const ustring& from, const ustring& to)
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

bool NickList::findUser(const ustring& nick)
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

vector<ustring> NickList::getNicks()
{
    vector<ustring> nicks;

    Gtk::ListStore::iterator i = _liststore->children().begin();

    for (i = _liststore->children().begin(); i != _liststore->children().end(); ++i)
    {
        nicks.push_back(i->get_value(_columns.nick));
    }

    return nicks;
}

int NickList::sortFunc(const Gtk::TreeModel::iterator& i1, const Gtk::TreeModel::iterator& i2)
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
