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

#ifndef TAB_H
#define TAB_H

#include <vector>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/style.h>
#include <gtkmm/paned.h>
#include <irc_defines.h>
#include "MainWindow.h"
#include "Entry.h"
#include "TextWidget.h"

class ServerConnection;

class Tab : public Gtk::VBox
{
public:
    Tab(ServerConnection *conn, Pango::FontDescription font);
    ~Tab();

    Entry&                      getEntry() { return _entry; }
    ServerConnection*           getConn() { return _conn; }

    void startPrefs();
    void endPrefs();

    virtual void insertUser(const Glib::ustring& user, IRC::UserMode m = IRC::NONE) = 0;
    virtual void removeUser(const Glib::ustring& nick) = 0;
    virtual void renameUser(const Glib::ustring& from, const Glib::ustring& to) = 0;
    virtual bool findUser(const Glib::ustring& nick) = 0;
    virtual std::vector<Glib::ustring> getNicks() = 0;
    TextWidget& getText() { return _textwidget; }
    void setInActive() {
        if (isActive()) {
            Gtk::Label *_label = AppWin->getNotebook().getLabel(this);
            _label->set_text("(" + _label->get_text() + ")");
            isOnChannel = false;
        }
    }
    virtual void setActive() {
        isOnChannel = true;
    }
    bool isActive() { return isOnChannel; }
    bool isHighlighted;
    bool hasPrefs;

private:
    bool isOnChannel;
    ServerConnection *_conn;
    Gtk::ScrolledWindow _swin;
    Gtk::HBox _hbox;

protected:
    Gtk::VBox _vbox;
    Gtk::HPaned *_hpaned;
    TextWidget _textwidget;
    Entry _entry;
};

class TabQuery : public Tab
{
public:
    TabQuery(ServerConnection *conn, Pango::FontDescription font)
            : Tab(conn, font) { }

    void insertUser(const Glib::ustring& user, IRC::UserMode i = IRC::NONE) {};
    void removeUser(const Glib::ustring& nick) {};
    void renameUser(const Glib::ustring& from, const Glib::ustring& to) {
        AppWin->getNotebook().getLabel(this)->set_text(to);
    }
    bool findUser(const Glib::ustring& nick) {
        if (nick == AppWin->getNotebook().getLabel(this)->get_text())
              return true;
        else
              return false;
    }
    std::vector<Glib::ustring> getNicks() {
        std::vector<Glib::ustring> vec; vec.push_back(AppWin->getNotebook().getLabel(this)->get_text()); return vec;
    }
};

class TabChannel : public Tab
{

public:
    TabChannel(ServerConnection *conn, Pango::FontDescription font);

    void insertUser(const Glib::ustring& user, IRC::UserMode i = IRC::NONE);
    void removeUser(const Glib::ustring& nick);
    void renameUser(const Glib::ustring& from, const Glib::ustring& to);
    bool findUser(const Glib::ustring& nick);
    std::vector<Glib::ustring> getNicks();

    void setActive() { _liststore->clear(); Tab::setActive(); }

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

    Gtk::Frame _users;

};

#endif
