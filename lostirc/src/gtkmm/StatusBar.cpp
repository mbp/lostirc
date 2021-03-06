/* 
 * Copyright (C) 2002-2004 Morten Brix Pedersen <morten@wtf.dk>
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

#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/stock.h>
#include "StatusBar.h"
#include <LostIRC.h>

StatusBar::StatusBar()
    : Gtk::HBox(false, 1),
    _label("", 0.5, 0.5)
{
    _frame.add(_label);
    _notifyframe.add(_notifylabel);
    //_statusbar.pack_end(_statuslabel);

    //_frame.set_shadow_type(Gtk::SHADOW_IN);
    _notifyframe.set_shadow_type(Gtk::SHADOW_IN);

    pack_start(_frame);
    pack_start(_notifyframe);

    setText1(_("Not connected."));

    show_all();
}
