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

#ifndef PREFS_H
#define PREFS_H

#include <gtkmm/notebook.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/combo.h>
#include <gtkmm/entry.h>
#include <gtkmm/table.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/fontselection.h>
#include "MainWindow.h"

class Prefs : public Gtk::Window
{
public:
    Prefs();
    virtual ~Prefs() { }

private:
    void saveSettings();
    void saveEntry();
    void openFontWindow();
    void removeEntry();
    void addEntry();
    void onChangeRow();
    void clearEntries();
    void onClose() { saveSettings(); hide(); }

    Gtk::VBox* addPage(const Glib::ustring& str);

    // General
    Gtk::Entry ircuserentry;
    Gtk::Combo encodingcombo;
    Gtk::Entry fontentry;

    // Preferences
    Gtk::Entry nickcompletionentry;
    Gtk::Entry highlightentry;
    Gtk::Adjustment bufferadj;
    Gtk::SpinButton bufferspin;
    Gtk::CheckButton stripcolorsbutton;
    Gtk::CheckButton stripothersbutton;
    Gtk::CheckButton loggingbutton;

    // DCC
    Gtk::Entry dccipentry;
    Gtk::Entry dccportentry;

    Gtk::Table _general_table;
    Gtk::Table _prefs_table;
    Gtk::Table _dcc_table;

    Gtk::Notebook notebook;

    Gtk::VBox mainvbox;
};

#endif
