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
    MainNotebook(MainWindow *frontend);

    TabChannel* addChannelTab(const string& name, ServerConnection *conn);
    TabQuery* addQueryTab(const string& name, ServerConnection *conn);
    Tab* getCurrent(ServerConnection *conn);
    Tab* getCurrent();
    Tab* findTab(const string& name, ServerConnection *conn);
    Gtk::Notebook_Helpers::Page * findPage(const string& name, ServerConnection *conn);

    void findTabsContaining(const string& nick, vector<Tab*>& vec);
    void closeCurrent();
    void highlight(Tab *tab);
    void insert(Tab *tab, const string& str);
    void setFont();

private:
    void switchPage(Gtk::Notebook_Helpers::Page *p, unsigned int n);
    void parseAndInsert(const string& str, Gtk::Text *text);
    void insertWithColor(int color, Gtk::Text *text, const string& str);
    void fontSelectionOk();
    void destroyFontSelection(Gtk::FontSelectionDialog *w);

    Gtk::FontSelectionDialog *fontdialog;
    Gdk_Font _font;

    MainWindow *_fe;

};
#endif
