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
#include "NickList.h"

class ServerConnection;

class Tab : public Gtk::VBox
{
public:
    Tab(ServerConnection *conn, Pango::FontDescription font);
    ~Tab();

    Entry&                      getEntry() { return _entry; }
    TextWidget&                 getText() { return _textwidget; }
    ServerConnection*           getConn() { return _conn; }

    void startPrefs();
    void closePrefs();

    void startDCCList();
    void closeDCCList();

    void insertUser(const Glib::ustring& user, IRC::UserMode m = IRC::NONE);
    void removeUser(const Glib::ustring& nick);
    void renameUser(const Glib::ustring& from, const Glib::ustring& to);
    bool findUser(const Glib::ustring& nick);
    std::vector<Glib::ustring> getNicks();

    void setInActive();
    void setActive();

    void setQuery(bool value);
    void setChannel(bool value);

    bool isQuery() { return _isQuery; }
    bool isChannel() { return _isChannel; }
    bool isActive() { return _isActive; }

    bool isHighlighted;
    bool hasPrefs;
    bool hasDCCList;

private:
    void addOrRemoveNickList();
            
    ServerConnection *_conn;

    Gtk::VBox _vbox;
    Gtk::HBox _hbox;
    Gtk::HPaned *_hpaned;

    NickList *_nicklist;
    TextWidget _textwidget;

    bool _isActive;
    bool _isChannel;
    bool _isQuery;

    Entry _entry;
};

#endif
