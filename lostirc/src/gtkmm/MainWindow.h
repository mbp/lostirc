/* 
 * Copyright (C) 2002-2004 Morten Brix Pedersen <morten@wtf.dk>
 * Copyright (C) 2007 Martin Braure de Calignon <braurede@free.fr>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>
#include <memory>
#include <gtkmm/main.h>
#include <glibmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/statusicon.h>
#include <gtk/gtk.h>
#include <ServerConnection.h>
#include <LostIRCApp.h>
#include <FrontEnd.h>
#include "Prefs.h"
#include "DCCList.h"
#include "ServerWindow.h"
#include "MainNotebook.h"
#include "StatusBar.h"

class MainWindow : public Gtk::Window, public FrontEnd
{
    virtual bool on_key_press_event(GdkEventKey* e);

    LostIRCApp _app;
    MainNotebook _notebook;

    Glib::RefPtr<Gtk::UIManager> _uimanager;

    std::auto_ptr<Prefs> _prefswin;
    std::auto_ptr<DCCWindow> _dccwin;
    std::auto_ptr<ServerWindow> _serverwin;
    std::auto_ptr<Gtk::Dialog> _helpwin;
    std::auto_ptr<Gtk::Dialog> _aboutwin;

#ifdef HAVE_STATUS_ICON
    Glib::RefPtr<Gtk::StatusIcon> _statusicon;
#endif

    void openPrefs();
    void openDccWindow();
    void openServerWindow();
    void openHelpIntro();
    void hideHelpIntro(int response);
    void openAboutWindow();
    void hideAboutWindow(int response);
    void setupMenus();
    void closeCurrentTab();
    void hideNickList();

    void initializeTagTable();
    void addToTable(Glib::ustring name, Glib::RefPtr<Gtk::TextTagTable> table, const Glib::ustring& colorname);
    #ifdef HAVE_STATUS_ICON
    void selfHide();
    static void on_tray_click( GtkStatusIcon* icon, gpointer data );
    #endif

public:
    MainWindow(bool autoconnect = 0);
    virtual ~MainWindow();

    MainNotebook& getNotebook() { return _notebook; }
    Glib::RefPtr<Gtk::UIManager> getUIManager() { return _uimanager; }
    Tab* newServerTab();
    void hideMenu();
    void hideStatusbar();

    StatusBar _statusbar;

    // Some of these should be private eventually.
    Glib::RefPtr<Gtk::TextTagTable> _tag_table1;
    Glib::RefPtr<Gtk::TextTagTable> _tag_table2;
    Glib::RefPtr<Gtk::TextTagTable> _current_tag_table;
    Glib::ustring background_color;

    // Methods implemented for the abstract base class 'FrontEnd' 
    void displayMessage(const Glib::ustring& msg, FE::Destination d, ServerConnection *conn = 0, bool shouldHighlight = true);
    void displayMessage(const Glib::ustring& msg, ChannelBase& to, ServerConnection *conn, bool shouldHighlight = true);
    void join(const Glib::ustring& nick, Channel& chan, ServerConnection *conn);
    void part(const Glib::ustring& nick, Channel& chan, ServerConnection *conn);
    void kick(const Glib::ustring& from, Channel& chan, const Glib::ustring& kicker, const Glib::ustring& msg,  ServerConnection *conn);
    void quit(const Glib::ustring& nick, std::vector<ChannelBase*> chans, ServerConnection *conn);
    void nick(const Glib::ustring& from, const Glib::ustring& to, std::vector<ChannelBase*> chans, ServerConnection *conn);
    void CUMode(const Glib::ustring& nick, Channel& chan, const std::vector<User>& users, ServerConnection *conn);
    void names(Channel& c, ServerConnection *conn);
    void highlight(ChannelBase& chan, ServerConnection *conn);
    void away(bool away, ServerConnection *conn);
    void connected(ServerConnection *conn);
    void newTab(ServerConnection *conn);
    void disconnected(ServerConnection *conn);
    void newDCC(DCC *dcc);
    void dccStatusChanged(DCC *dcc);
    void localeError(bool tried_custom_encoding);
};

extern MainWindow* AppWin;

#endif
