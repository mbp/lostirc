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

#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/combo.h>
#include <gtkmm/entry.h>
#include <gtkmm/table.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/textview.h>
#include "MainWindow.h"

class ServerWindow : public Gtk::Dialog
{
public:
    ServerWindow(Gtk::Window& parent);
    virtual ~ServerWindow() { }

private:
    void saveEntry();
    void removeEntry();
    void addEntry();
    void onChangeRow();
    void clearEntries();
    virtual void on_response(int) { hide(); }

    // Auto-join 
    Gtk::Entry nickentry;
    Gtk::Entry passentry;
    Gtk::Entry portentry;
    Gtk::Entry hostentry;
    Gtk::TextView cmdtext;
    Gtk::Notebook notebook;
    Gtk::VBox *serverinfobox;

    Gtk::CheckButton auto_connect_button;

    Gtk::HBox hboxserver;

    // what our columned-list contains
    struct ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<bool> auto_connect;
        Gtk::TreeModelColumn<Glib::ustring> servername;
        Gtk::TreeModelColumn<int> port;
        Gtk::TreeModelColumn<Server*> autojoin;

        ModelColumns() { add(servername); add(port); add(auto_connect); add(autojoin); }
    };

    ModelColumns _columns;
    Glib::RefPtr<Gtk::ListStore> _liststore;
    Gtk::TreeView _treeview;
    Gtk::Table _server_options_table;
};

#endif
