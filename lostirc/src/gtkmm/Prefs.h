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

#ifndef PREFS_H
#define PREFS_H

#include <gtk--/button.h>
#include <gtk--/entry.h>
#include <gtk--/clist.h>
#include "Tab.h"

class Prefs : public Gtk::Notebook
{
public:
    static Prefs* Instance() {
        static Prefs p;
        return &p;
    }

    // this is an important variable, the Tab that currently has the prefs
    // is defined here, so when we do endPrefs() we can call the right
    // endPrefs() member function in the Tab class
    static Tab* currentTab;
    void endPrefs();
private:
    Prefs();
    Prefs(const Prefs&);
    Prefs& operator=(const Prefs&);
    ~Prefs();

    Gtk::CList *clist;
    Gtk::Entry *passentry;
    Gtk::Entry *portentry;
    Gtk::Entry *hostentry;
    Gtk::Entry *nickentry;
    Gtk::Text *cmdtext;
    Gtk::Button *closebutton;
    Gtk::Button *savebutton;
    Gtk::Button *removebutton;
    Gtk::Button *addnewbutton;
    Gtk::HBox *savehbox;
    Tab *tab;

    void saveEntry();
    void removeEntry();
    void addEntry();
    void onSelectRow(int row, int col, GdkEvent* e);
    void onUnSelectRow(int row, int col, GdkEvent* e);
    void clearEntries();

};

#endif
