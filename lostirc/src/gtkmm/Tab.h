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
#include <gdk/gdkkeysyms.h>
#include <irc_defines.h>
#include "Entry.h"

class ServerConnection;

class Tab : public Gtk::VBox
{
public:
    Tab(Gtk::Label *label, ServerConnection *conn);//, Gdk_Font *font);
    ~Tab();

    Gtk::Label*                 getLabel() { return _label; }
    Entry&                      getEntry() { return _entry; }
    ServerConnection*           getConn() { return _conn; }

    void setAway();
    void setUnAway();
    void startPrefs();
    void endPrefs();

    virtual void insertUser(const std::string& user, IRC::UserMode m = IRC::NONE) = 0;
    virtual void removeUser(const std::string& nick) = 0;
    virtual void renameUser(const std::string& from, const std::string& to) = 0;
    virtual bool findUser(const std::string& nick) = 0;
    virtual bool nickCompletion(const Glib::ustring& word, Glib::ustring& str) = 0;
    Tab& operator<<(const char * str);
    Tab& operator<<(const std::string& str);
    Tab& operator<<(const Glib::ustring& str);
    void insertText(const Glib::ustring& str);
    void setStyle();
    //void setFont(Gdk_Font *font);
    void setInActive() {
        if (isActive()) {
            _label->set_text("(" + _label->get_text() + ")");
            isOnChannel = false;
        }
    }
    void setActive() {
        isOnChannel = true;
    }
    bool isActive() { return isOnChannel; }
    bool isHighlighted;
    bool hasPrefs;

private:
    void insertWithColor(int color, const Glib::ustring& str);

    bool isOnChannel;
    Gtk::Label *_label;
    ServerConnection *_conn;
    Entry _entry;
    Gtk::ScrolledWindow _swin;
    //Gdk_Font *_font;
    Gtk::Label *_away;
    Gtk::HBox *_hbox2;

    std::map<int, Glib::RefPtr<Gtk::TextTag> > colorMap;

    void initializeColorMap();
    void helperInitializer(int i, const char *colorname);

protected:
    Gtk::HBox *_hbox;
    Gtk::TextView _textview;
};

class TabQuery : public Tab
{
public:
    TabQuery(Gtk::Label *label, ServerConnection *conn); //, Gdk_Font *font);

    void insertUser(const std::string& user, IRC::UserMode i = IRC::NONE) {};
    void removeUser(const std::string& nick) {};
    void renameUser(const std::string& from, const std::string& to) {
        getLabel()->set_text(to);
    }
    bool findUser(const std::string& nick) {
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
    TabChannel(Gtk::Label *label, ServerConnection *conn); //, Gdk_Font *font);

    void insertUser(const std::string& user, IRC::UserMode i = IRC::NONE);
    void removeUser(const std::string& nick);
    void renameUser(const std::string& from, const std::string& to);
    bool findUser(const std::string& nick);
    bool nickCompletion(const Glib::ustring& word, Glib::ustring& str);

private:
    void updateUserNumber();

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

//gint sortFunc(GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);

#endif
