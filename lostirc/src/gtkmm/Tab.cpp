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

#include <pangomm/fontdescription.h>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <ServerConnection.h>
#include <Utils.h>
#include <gtkmm/image.h>
#include <gtkmm/stock.h>
#include "Tab.h"
#include "MainWindow.h"

using std::vector;
using Glib::ustring;

Tab::Tab(ServerConnection *conn, Pango::FontDescription font, Gtk::Label *label)
    : Gtk::VBox(), isHighlighted(false),
    _conn(conn), _textwidget(font), _isActive(true),
    _type(UNDEFINED), _entry(this), _label(label)
{
    _hpaned = new Gtk::HPaned();

    _hbox.pack_start(_entry);

    _vbox.pack_start(_textwidget);

    _vbox.pack_start(_hbox, Gtk::PACK_SHRINK);

    /* FIXME :
    Gtk::Button *prefs_button = manage(new Gtk::Button());
    Gtk::Image *prefsimage = manage(new Gtk::Image(Gtk::Stock::PREFERENCES, Gtk::ICON_SIZE_MENU));
    prefs_button->add(*prefsimage);
    prefs_button->signal_clicked().connect(slot(*this, &Tab::startPrefs));
    _hbox.pack_start(*prefs_button, Gtk::PACK_SHRINK);
    */

    _hpaned->pack1(_vbox, true, true);
    _hpaned->pack2(_nicklist, false, true);
    pack_start(*_hpaned);

    // Make the Entry the first in the focus chain.
    std::vector<Gtk::Widget *> focuses;
    focuses.push_back(&_entry);
    set_focus_chain(focuses);
}

Tab::~Tab()
{
    delete _hpaned;
}

void Tab::setInActive()
{
    _isActive = false;
    setLabelName();
}

void Tab::setActive()
{
    _isActive = true;
    setLabelName();

    _nicklist.setActive();
}

void Tab::setName(const Glib::ustring& str)
{
    _name = str;
    setLabelName();
}

void Tab::setLabelName()
{
    if (isActive())
          _label->set_text(_name);
    else
          _label->set_text("(" + _name + ")");
}

void Tab::highlightNick()
{
    if (this != AppWin->getNotebook().getCurrent()) {
        isHighlighted = true;
        _label->set_markup("<span foreground=\"blue\">" + _name + "</span>");
        _textwidget.setHighlightMark();
    }
}

void Tab::highlightActivity()
{
    if (this != AppWin->getNotebook().getCurrent() && !isHighlighted)
          _label->set_markup("<span foreground=\"red\">" + _name + "</span>");
}

void Tab::removeHighlight()
{
    isHighlighted = false;
    setLabelName();
}

void Tab::addOrRemoveNickList()
{
    if (isType(QUERY) || isType(SERVER) || !AppWin->hasNickList())
          _nicklist.hide();
}

void Tab::insertUser(const Glib::ustring& user, IRC::UserMode m)
{
    if (isType(CHANNEL))
          _nicklist.insertUser(user, m);
}

void Tab::removeUser(const Glib::ustring& nick)
{
    if (isType(CHANNEL))
          _nicklist.removeUser(nick);
}

void Tab::renameUser(const Glib::ustring& from, const Glib::ustring& to)
{
    if (isType(CHANNEL))
          _nicklist.renameUser(from, to);
    else if (isType(QUERY))
          setName(to);
}

bool Tab::findUser(const Glib::ustring& nick)
{
    if (isType(CHANNEL))
          return _nicklist.findUser(nick);
    else 
          return (nick == getName());
}

std::vector<Glib::ustring> Tab::getNicks()
{
    if (isType(CHANNEL))
          return _nicklist.getNicks();
    else {
        std::vector<Glib::ustring> vec;
        vec.push_back(getName());
        return vec;
    }
}
