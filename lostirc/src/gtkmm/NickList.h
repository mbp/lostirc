/* 
 * Copyright (C) 2002, 2003 Morten Brix Pedersen <morten@wtf.dk>
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

#ifndef NICKLIST_H
#define NICKLIST_H

#include <vector>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/entry.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <irc_defines.h>

class NickList : public Gtk::Frame
{
public:
    NickList();

    void insertUser(const Glib::ustring& user, IRC::UserMode i = IRC::NONE);
    void removeUser(const Glib::ustring& nick);
    void renameUser(const Glib::ustring& from, const Glib::ustring& to);
    bool findUser(const Glib::ustring& nick);
    std::vector<Glib::ustring> getNicks();

    void setActive() { _liststore->clear(); }

private:
    void updateUserNumber();
    int sortFunc(const Gtk::TreeModel::iterator& i1, const Gtk::TreeModel::iterator& i2);

    /* what our columned-list contains */
    struct ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<Glib::ustring> status;
        Gtk::TreeModelColumn<Glib::ustring> nick;
        Gtk::TreeModelColumn<int> priority;

        ModelColumns() { add(status); add(nick); add(priority); }
    };


    ModelColumns _columns;
    Glib::RefPtr<Gtk::ListStore> _liststore;
    Gtk::TreeView _treeview;

};
    
#endif
