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

#include "DCCList.h"

DCCList::DCCList()
    : _columns(),
    _liststore(Gtk::ListStore::create(_columns))
{
    set_model(_liststore);

    append_column("Status", _columns.status);
    append_column("Filename", _columns.filename);
    append_column("Filesize", _columns.filesize);
    append_column("Fileposition", _columns.fileposition);
    append_column("From/To", _columns.from);

    show_all();
}

void DCCList::add(DCC *dcc)
{
    Gtk::TreeModel::Row row = *_liststore->append();
    /*row[_columns.filename] = dcc->getFilename();
    //row[_columns.filesize] = dcc->getSize();
    //row[_columns.fileposition] = dcc->getPosition();
    //row[_columns.filefrom] = dcc->getNick();

    //row[_columns.dcc_ptr] = dcc;

    signal_connection = Glib::signal_timeout().connect(
            SigC::bind(SigC::slot(*this, &DCCList::updateDccData) dcc),
            1000);
            */
}

void DCCList::updateDccDate()
{
    // find dcc_ptr

    //row[_columns.filename] = dcc->getFilename();
    //row[_columns.filesize] = dcc->getSize();
    //row[_columns.fileposition] = dcc->getPosition();
    //row[_columns.filefrom] = dcc->getNick();
}



Tab* DCCList::currentTab = 0;

