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

#ifndef PREFS_H
#define PREFS_H

#include <gtkmm/notebook.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include "Tab.h"

class Prefs : public Gtk::VBox
{
public:
    static Prefs* Instance() {
        static Prefs p;
        return &p;
    }

    // this is an important variable, the Tab that currently has the prefs
    // is defined here, so when we do endPrefs() we can call the right
    // endPrefs() member function in the Tab class
    static Tab* currentTab;
    void endPrefs();
private:
    Prefs();
    Prefs(const Prefs&);
    Prefs& operator=(const Prefs&);
    ~Prefs();

    void saveEntry();
    void saveSettings();
    void removeEntry();
    void addEntry();
    void onSelectRow(bool start_editing);
    void onUnSelectRow();
    void clearEntries();

    // General
    Gtk::Entry ircnickentry;
    Gtk::Entry realnameentry;
    Gtk::Entry ircuserentry;

    // Preferences
    Gtk::Entry nickcompletionentry;
    Gtk::Entry dccipentry;
    Gtk::Entry highlightentry;
    Gtk::Entry bufferentry;

    // Auto-join 
    Gtk::Entry nickentry;
    Gtk::Entry passentry;
    Gtk::Entry portentry;
    Gtk::Entry hostentry;
    Gtk::TextView cmdtext;
    Gtk::Notebook notebook;

    Gtk::Button *removebutton;
    Gtk::Button *addnewbutton;

    Gtk::HBox savehbox;

    // what our columned-list contains
    struct ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<Glib::ustring> servername;
        Gtk::TreeModelColumn<struct autoJoin*> autojoin;

        ModelColumns() { add(servername); add(autojoin); }
    };

    ModelColumns _columns;
    Glib::RefPtr<Gtk::ListStore> _liststore;
    Gtk::TreeView _treeview;
};

#endif
