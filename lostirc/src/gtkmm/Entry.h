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

#ifndef ENTRY_H
#define ENTRY_H

#include <vector>

class Tab;

class Entry : public Gtk::Entry
{

public:
    Entry(Tab *tab);

    bool onKeyPress(GdkEventKey* e);
private:
    void onEntry();
    void sendMsg(const Glib::ustring& msg);
    std::vector<Glib::ustring> _entries;
    Tab* _tab;
    std::vector<Glib::ustring>::iterator i;

};
#endif
