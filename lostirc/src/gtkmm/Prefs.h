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

#ifndef PREFS_H
#define PREFS_H

#include <gtkmm/notebook.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/combo.h>
#include <gtkmm/entry.h>
#include <gtkmm/table.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/textview.h>
#include <gtkmm/fontselection.h>
#include "MainWindow.h"

class Prefs : public Gtk::Dialog
{
public:
    Prefs(Gtk::Window& parent);
    virtual ~Prefs() { }

private:
    void applyGeneral();
    void applyPreferences();
    void applyFont();
    void cancelGeneral();
    void cancelPreferences();
    void cancelFont();

    void saveEntry();
    void removeEntry();
    void addEntry();
    void onChangeRow();
    void clearEntries();
    virtual void on_response(int) { hide(); }

    Gtk::VBox* addPage(const Glib::ustring& str);

    // General
    Gtk::Entry ircnickentry;
    Gtk::Entry realnameentry;
    Gtk::Entry ircuserentry;
    Gtk::Combo encodingcombo;

    // Preferences
    Gtk::Entry nickcompletionentry;
    Gtk::Entry dccipentry;
    Gtk::Entry dccportentry;
    Gtk::Entry highlightentry;
    Gtk::Entry bufferentry;
    Gtk::CheckButton highlightingbutton;
    Gtk::CheckButton stripcolorsbutton;
    Gtk::CheckButton stripothersbutton;
    Gtk::CheckButton loggingbutton;

    // Font selection
    Gtk::FontSelection fontsel;

    // Auto-join 
    Gtk::Entry nickentry;
    Gtk::Entry passentry;
    Gtk::Entry portentry;
    Gtk::Entry hostentry;
    Gtk::TextView cmdtext;
    Gtk::Notebook notebook;

    Gtk::Button *removebutton;
    Gtk::Button *addnewbutton;
    Gtk::CheckButton auto_connect_button;

    Gtk::HBox hboxserver;

    // what our columned-list contains
    struct ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<Glib::ustring> servername;
        Gtk::TreeModelColumn<bool> auto_connect;
        Gtk::TreeModelColumn<Server*> autojoin;

        ModelColumns() { add(servername); add(auto_connect); add(autojoin); }
    };

    ModelColumns _columns;
    Glib::RefPtr<Gtk::ListStore> _liststore;
    Gtk::TreeView _treeview;
    Gtk::Table _general_table;
    Gtk::Table _prefs_table;
    Gtk::Table _server_options_table;
};

#endif
