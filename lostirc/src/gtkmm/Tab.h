/* 
 * Copyright (C) 2001 Morten Brix Pedersen <morten@wtf.dk>
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

#include <gtk--/scrolledwindow.h>
#include <gtk--/entry.h>
#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/text.h>
#include <gtk--/clist.h>
#include <gtk--/style.h>
#include <vector>
#include <gdk/gdkkeysyms.h>
#include "Entry.h"

class MainNotebook;
class ServerConnection;

class Tab : public Gtk::VBox
{
public:
    Tab(Gtk::Label *label, ServerConnection *conn, Gdk_Font *font);
    ~Tab();

    Gtk::Text*                  getText();
    Gtk::HBox*                  getHBox();
    Gtk::Label*                 getLabel();
    virtual Gtk::CList*         getCList() { return 0; }
    Entry*                      getEntry();
    ServerConnection*           getConn();

    virtual void insertUser(const std::vector<std::string>& users) = 0;
    virtual void insertUser(const std::string& user) = 0;
    virtual void removeUser(const std::string& nick) = 0;
    virtual void renameUser(const std::string& from, const std::string& to) = 0;
    virtual bool findUser(const std::string& nick) = 0;
    virtual bool nickCompletion(const std::string& word, std::string& str) = 0;
    void parseAndInsert(const std::string& str);
    void insertWithColor(int color, const std::string& str);
    void setStyle();
    void setFont(Gdk_Font *font);
    bool is_highlighted;

private:
    Gtk::Label *_label;
    Entry *_entry;
    Gtk::Text *_text;
    Gtk::HBox *_hbox;
    ServerConnection *_conn;
    Gtk::ScrolledWindow *_scrollwindow;
    Gtk::Text::Context *_current_cx;
    Gdk_Font *_font;

};

class TabQuery : public Tab
{
public:
    TabQuery(Gtk::Label *label, ServerConnection *conn, Gdk_Font *font);

    Gtk::CList*         getCList() { return 0; }

    void insertUser(const std::vector<std::string>& users) {};
    void insertUser(const std::string& user) {};
    void removeUser(const std::string& nick) {};
    void renameUser(const std::string& from, const std::string& to) {
        getLabel()->set_text(to);
    }
    bool findUser(const std::string& nick) {
        if (nick == getLabel()->get_text())
              return true;
    }
    bool nickCompletion(const std::string& word, std::string& str) {
        str = getLabel()->get_text(); return true;
    }
};

class TabChannel : public Tab
{
public:
    TabChannel(Gtk::Label *label, ServerConnection *conn, Gdk_Font *font);

    Gtk::CList*         getCList();

    void insertUser(const std::vector<string>& users);
    void insertUser(const std::string& user);
    void removeUser(const std::string& nick);
    void renameUser(const std::string& from, const std::string& to) {
        removeUser(from); insertUser(to);
    }
    bool findUser(const std::string& nick);
    bool nickCompletion(const std::string& word, std::string& str);

private:
    Gtk::CList *_clist;

};

#endif
