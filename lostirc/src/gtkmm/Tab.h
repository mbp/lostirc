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
#include <map>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/style.h>
#include <gtkmm/paned.h>
#include <gdk/gdkkeysyms.h>
#include <irc_defines.h>
#include "Entry.h"

class ServerConnection;

class Tab : public Gtk::VBox
{
public:
    Tab(Gtk::Label *label, ServerConnection *conn, Pango::FontDescription font);
    ~Tab();

    Gtk::Label*                 getLabel() { return _label; }
    Entry&                      getEntry() { return _entry; }
    ServerConnection*           getConn() { return _conn; }

    void startPrefs();
    void endPrefs();

    virtual void insertUser(const Glib::ustring& user, IRC::UserMode m = IRC::NONE) = 0;
    virtual void removeUser(const Glib::ustring& nick) = 0;
    virtual void renameUser(const Glib::ustring& from, const Glib::ustring& to) = 0;
    virtual bool findUser(const Glib::ustring& nick) = 0;
    virtual bool nickCompletion(const Glib::ustring& word, Glib::ustring& str) = 0;
    Tab& operator<<(const char * str);
    Tab& operator<<(const std::string& str);
    Tab& operator<<(const Glib::ustring& str);
    void insertText(const Glib::ustring& str);
    void setStyle();
    void setFont(const Pango::FontDescription& font);
    void setInActive() {
        if (isActive()) {
            _label->set_text("(" + _label->get_text() + ")");
            isOnChannel = false;
        }
    }
    virtual void setActive() {
        isOnChannel = true;
    }
    bool isActive() { return isOnChannel; }
    void clearText();
    bool isHighlighted;
    bool hasPrefs;

private:
    void insertWithColor(int color, const Glib::ustring& str);
    void realInsert(int color, const Glib::ustring& str);

    bool isOnChannel;
    Gtk::Label *_label;
    ServerConnection *_conn;
    Entry _entry;
    Gtk::ScrolledWindow _swin;
    Gtk::Label *_away;
    Gtk::HBox *_hbox;

    std::map<int, Glib::RefPtr<Gtk::TextTag> > colorMap;
    Glib::RefPtr<Gtk::TextTag> underlinetag;

    void initializeColorMap();
    void helperInitializer(int i, const Glib::ustring& colorname);

protected:
    Gtk::VBox *_vbox;
    Gtk::TextView _textview;
    Gtk::HPaned *_hpaned;
};

class TabQuery : public Tab
{
public:
    TabQuery(Gtk::Label *label, ServerConnection *conn, Pango::FontDescription font)
            : Tab(label, conn, font) { }

    void insertUser(const Glib::ustring& user, IRC::UserMode i = IRC::NONE) {};
    void removeUser(const Glib::ustring& nick) {};
    void renameUser(const Glib::ustring& from, const Glib::ustring& to) {
        getLabel()->set_text(to);
    }
    bool findUser(const Glib::ustring& nick) {
        if (nick == getLabel()->get_text())
              return true;
        else
              return false;
    }
    bool nickCompletion(const Glib::ustring& word, Glib::ustring& str) {
        str = getLabel()->get_text(); return true;
    }
};

class TabChannel : public Tab
{
    Gtk::Frame *_users;

public:
    TabChannel(Gtk::Label *label, ServerConnection *conn, Pango::FontDescription font);

    void insertUser(const Glib::ustring& user, IRC::UserMode i = IRC::NONE);
    void removeUser(const Glib::ustring& nick);
    void renameUser(const Glib::ustring& from, const Glib::ustring& to);
    bool findUser(const Glib::ustring& nick);
    bool nickCompletion(const Glib::ustring& word, Glib::ustring& str);

    void setActive() { _liststore->clear(); Tab::setActive(); }

private:
    void updateUserNumber();
    gint sortFunc(const Gtk::TreeModel::iterator& i1, const Gtk::TreeModel::iterator& i2);

    /* what our columned-list contains */
    struct ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<Glib::ustring> status;
        Gtk::TreeModelColumn<Glib::ustring> nick;

        ModelColumns() { add(status); add(nick); }
    };


    ModelColumns _columns;
    Glib::RefPtr<Gtk::ListStore> _liststore;
    Gtk::TreeView _treeview;

};

#endif
