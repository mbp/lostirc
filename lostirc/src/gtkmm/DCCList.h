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

#ifndef DCCLIST_H
#define DCCLIST_H

#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <DCC.h>
#include "Tab.h"

class DCCList : public Gtk::TreeView
{
public:
    static DCCList* Instance() {
        static DCCList p;
        return &p;
    }

    // this is an important variable, the Tab that currently has the dcclist
    // is defined here, so when we do endDCCList() we can call the right
    // endDCCList() member function in the Tab class
    static Tab* currentTab;
    void closeDCCList() { currentTab->closeDCCList(); }

    void add(DCC *dcc);
    void statusChange(DCC *dcc);

private:
    DCCList();
    DCCList(const DCCList&);
    DCCList& operator=(const DCCList&);
    ~DCCList() { }

    bool updateDccData();

    // what our columned-list contains
    struct ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<Glib::ustring> status;
        Gtk::TreeModelColumn<Glib::ustring> filename;
        Gtk::TreeModelColumn<unsigned long> filesize;
        Gtk::TreeModelColumn<unsigned long> fileposition;
        Gtk::TreeModelColumn<Glib::ustring> nick;

        Gtk::TreeModelColumn<DCC*> dcc_ptr;

        ModelColumns() {
            add(status); add(filename); add(filesize);
            add(fileposition); add(nick); add(dcc_ptr);
        }
    };

    Glib::ustring statusToStr(DCC::Status s);

    int _activeDccs;
    SigC::Connection signal_timeout;

    ModelColumns _columns;
    Glib::RefPtr<Gtk::ListStore> _liststore;
};

#endif
