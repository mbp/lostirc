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

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
//#include <gtkmm/statusbar.h>

class StatusBar : public Gtk::HBox
{
public:
    StatusBar();

    void setText1(const Glib::ustring& str) { _label.set_markup(str); }
    void clearText1() { _label.set_text(""); }

    void setText2(const Glib::ustring& str) {
        Glib::ustring realstr = str;
        if (realstr.length() > 100) {
            realstr = realstr.substr(0, 100);
            realstr += "...";
        }
        
        _notifylabel.set_markup(realstr);
        signal_timeout.disconnect();
        signal_timeout = Glib::signal_timeout().connect(
                sigc::mem_fun(*this, &StatusBar::onText2Timeout),
                5000);
    }
    void clearText2() { _notifylabel.set_text(""); }

    #if 0
    /* TODO: statusbar disabled for now since it doesn't support pango
     * markup.
    void setText2(const Glib::ustring& str) {
        // First remove any previous.
        signal_timeout.disconnect();
        _statusbar.pop();

        // Then add new one.
        _statusbar.push(str);
        signal_timeout = Glib::signal_timeout().connect(
                sigc::mem_fun(*this, &StatusBar::onText2Timeout),
                5000);
    }

    void clearText2() { _statusbar.pop(); }
    */
    #endif

private:
    bool onText2Timeout() { clearText2(); return false; }

    Gtk::Frame _frame;
    Gtk::Frame _notifyframe;
    // Gtk::Statusbar _statusbar; disabled for now

    Gtk::Label _label;
    Gtk::Label _notifylabel;

    sigc::connection signal_timeout;
};

#endif
