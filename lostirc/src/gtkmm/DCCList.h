/* 
 * Copyright (C) 2002-2004 Morten Brix Pedersen <morten@wtf.dk>
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
#include <gtkmm/stock.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include <DCC.h>

class DCCList : public Gtk::TreeView
{
public:
    static DCCList* Instance() {
        static DCCList p;
        return &p;
    }
    void add(DCC *dcc);
    void statusChange(DCC *dcc);
    void stopSelected();

private:
    DCCList();
    DCCList(const DCCList&);
    DCCList& operator=(const DCCList&);
    ~DCCList() { }

    bool updateDccData();

    // what our columned-list contains
    struct ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<unsigned long> progress;
        Gtk::TreeModelColumn<Glib::ustring> status;
        Gtk::TreeModelColumn<Glib::ustring> filename;
        Gtk::TreeModelColumn<unsigned long> filesize;
        Gtk::TreeModelColumn<Glib::ustring> nick;

        Gtk::TreeModelColumn<DCC*> dcc_ptr;

        ModelColumns() {
            add(progress); add(status); add(filename);
            add(filesize); add(nick); add(dcc_ptr);
        }
    };

    Glib::ustring statusToStr(DCC::Status s);

    int _activeDccs;
    sigc::connection signal_timeout;

    ModelColumns _columns;
    Glib::RefPtr<Gtk::ListStore> _liststore;
};

class DCCWindow : public Gtk::Dialog
{
    Gtk::ScrolledWindow _scrollwin;

public:
    DCCWindow(Gtk::Window& parent)
            : Gtk::Dialog("LostIRC DCC Transfers", parent)
    {
        add_button(Gtk::Stock::STOP, Gtk::RESPONSE_CANCEL);
        add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
        get_vbox()->set_border_width(12);
        set_border_width(5);
        _scrollwin.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
        _scrollwin.add(*DCCList::Instance());
        get_vbox()->pack_start(_scrollwin, Gtk::PACK_EXPAND_WIDGET);
        show_all();
    }
    virtual ~DCCWindow() { }

    virtual void on_response(int response)
    {
        if (response == Gtk::RESPONSE_CLOSE)
              hide();
        else if (response == Gtk::RESPONSE_CANCEL)
              DCCList::Instance()->stopSelected();
    }
};


#endif
