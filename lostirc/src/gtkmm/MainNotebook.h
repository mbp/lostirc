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

#ifndef MAINNOTEBOOK_H
#define MAINNOTEBOOK_H

#include <gtkmm/notebook.h>
#include <gtkmm/style.h>
#include <glibmm/ustring.h>
#include <ServerConnection.h>
#include <vector>
#include "Tab.h"

class MainNotebook : public Gtk::Notebook
{
public:
    MainNotebook();

    TabChannel* addChannelTab(const Glib::ustring& name, ServerConnection *conn);
    TabQuery* addQueryTab(const Glib::ustring& name, ServerConnection *conn);
    Tab* getCurrent(ServerConnection *conn);
    Tab* getCurrent();
    Tab* findTab(const Glib::ustring& name, ServerConnection *conn, bool findInActive = false);
    int findPage(const Glib::ustring& name, ServerConnection *conn, bool findInActive = false);

    void findTabs(const Glib::ustring& nick, ServerConnection *conn, std::vector<Tab*>& vec);
    void findTabs(ServerConnection *conn, std::vector<Tab*>& vec);
    void clearAll();
    void Tabs(std::vector<Tab*>& vec);
    void closeCurrent();
    void highlightNick(Tab *tab);
    void highlightActivity(Tab *tab);
    void setFont(const Glib::ustring& str);
    int countTabs(ServerConnection *conn);
    void updateTitle(Tab *tab = 0);
    void MainNotebook::updateStatus(Tab *tab = 0);

private:
    void onSwitchPage(GtkNotebookPage *p, unsigned int n);

    Pango::FontDescription fontdescription;
};
#endif
