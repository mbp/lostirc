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

#ifndef MAINNOTEBOOK_H
#define MAINNOTEBOOK_H

#include <vector>
#include <gtkmm/notebook.h>
#include <glibmm/ustring.h>
#include <ServerConnection.h>
#include "Tab.h"

class MainNotebook : public Gtk::Notebook
{
public:
    MainNotebook();

    Tab* addTab(Tab::Type type, const Glib::ustring& name, ServerConnection *conn);
    Tab* getCurrent(ServerConnection *conn = 0);
    Tab* findTab(const Glib::ustring& name, ServerConnection *conn, bool findInActive = false);
    Tab* findTab(Tab::Type type, ServerConnection *conn, bool findInActive = false);

    void findTabs(const Glib::ustring& nick, ServerConnection *conn, std::vector<Tab*>& vec);
    void findTabs(ServerConnection *conn, std::vector<Tab*>& vec);
    void clearWindow();
    void clearAll();
    void Tabs(std::vector<Tab*>& vec);
    void closeCurrent();
    void setFont(const Glib::ustring& str);
    int countTabs(ServerConnection *conn);
    void updateTitle(Tab *tab = 0);
    void updateStatus(Tab *tab = 0);

private:
    void onSwitchPage(GtkNotebookPage *p, unsigned int n);

    Pango::FontDescription _fontdesc;
};
#endif
