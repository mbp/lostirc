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

#include <LostIRC.h>
#include "DCCList.h"

inline
unsigned int getProgress(DCC *dcc)
{
    return static_cast<unsigned int>(static_cast<float>((dcc->getPosition() * 100.00) / dcc->getSize()));
}

DCCList::DCCList()
    : _activeDccs(0), _columns(),
    _liststore(Gtk::ListStore::create(_columns))
{
    set_model(_liststore);

    append_column(_("Status"), _columns.status);
    append_column(_("Filename"), _columns.filename);
    append_column(_("Filesize"), _columns.filesize);
    append_column(_("Fileposition"), _columns.fileposition);
    append_column(_("Progress"), _columns.progress);
    append_column(_("From/To"), _columns.nick);

    show_all();
}

void DCCList::add(DCC *dcc)
{
    Gtk::TreeModel::Row row = *_liststore->append();
    row[_columns.status] = statusToStr(dcc->getStatus());
    row[_columns.filename] = dcc->getFilename();
    row[_columns.filesize] = dcc->getSize();
    row[_columns.fileposition] = dcc->getPosition();
    row[_columns.progress] = getProgress(dcc);
    row[_columns.nick] = dcc->getNick();

    row[_columns.dcc_ptr] = dcc;

    _activeDccs++;

    if (!signal_timeout.connected()) {
        signal_timeout = Glib::signal_timeout().connect(
                SigC::slot(*this, &DCCList::updateDccData),
                500);
    }
}

void DCCList::statusChange(DCC *dcc)
{
    // TODO: add code to mark dcc_ptr done.
    if (dcc->getStatus() == DCC::DONE)
          _activeDccs--;

    updateDccData();

    if (_activeDccs < 1)
        signal_timeout.disconnect();
}

bool DCCList::updateDccData()
{
    Gtk::TreeModel::Children::iterator iter;
    for(iter = get_model()->children().begin(); iter != get_model()->children().end(); ++iter)
    {
        Gtk::TreeModel::Row row = *iter;

        DCC *dcc = row[_columns.dcc_ptr];
        if (dcc) {
            row[_columns.status] = statusToStr(dcc->getStatus());
            row[_columns.filename] = dcc->getFilename();
            row[_columns.filesize] = dcc->getSize();
            row[_columns.fileposition] = dcc->getPosition();
            row[_columns.progress] = getProgress(dcc);
            row[_columns.nick] = dcc->getNick();
        }
    }

    return true;
}

Glib::ustring DCCList::statusToStr(DCC::Status status)
{
    if (status == DCC::DONE)
          return "DONE";
    else if (status == DCC::ONGOING)
          return "ONGOING";
    else if (status == DCC::WAITING)
          return "WAITING";
    else if (status == DCC::STOPPED)
          return "STOPPED";
    else if (status == DCC::FAIL)
          return "ERROR";
    else
          return "";
}


Tab* DCCList::currentTab = 0;
