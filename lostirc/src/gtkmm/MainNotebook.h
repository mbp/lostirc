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

#include <gtk--/notebook.h>
#include <gtk--/style.h>
#include <gtk--/fontselection.h>
#include <ServerConnection.h>
#include <vector>
#include "Tab.h"

class MainWindow;

class MainNotebook : public Gtk::Notebook
{
public:
    MainNotebook();
    ~MainNotebook();

    TabChannel* addChannelTab(const std::string& name, ServerConnection *conn);
    TabQuery* addQueryTab(const std::string& name, ServerConnection *conn);
    Tab* getCurrent(ServerConnection *conn);
    Tab* getCurrent();
    Tab* findTab(const std::string& name, ServerConnection *conn, bool findInActive = false);
    Gtk::Notebook_Helpers::Page* findPage(const std::string& name, ServerConnection *conn, bool findInActive = false);

    void findTabs(const std::string& nick, ServerConnection *conn, std::vector<Tab*>& vec);
    void findTabs(ServerConnection *conn, std::vector<Tab*>& vec);
    void Tabs(ServerConnection *conn, std::vector<Tab*>& vec);
    void closeCurrent();
    void highlight(Tab *tab);
    void insert(Tab *tab, const std::string& str);
    void setFont();
    int countTabs(ServerConnection *conn);

private:
    void switchPage(Gtk::Notebook_Helpers::Page *p, unsigned int n);
    void parseAndInsert(const std::string& str, Gtk::Text *text);
    void insertWithColor(int color, Gtk::Text *text, const std::string& str);
    void fontSelectionOk();
    void destroyFontSelection(Gtk::FontSelectionDialog *w);

    Gtk::FontSelectionDialog *fontdialog;
    Gdk_Font _font;

};
#endif
