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
#include "Prefs.h"
#include "DCCList.h"
#include "MainWindow.h"

using std::vector;
using Glib::ustring;

Tab::Tab(ServerConnection *conn, Pango::FontDescription font)
    : Gtk::VBox(), isHighlighted(false), hasPrefs(false),
    _conn(conn), _nicklist(0), _textwidget(font), _isActive(true),
    _isChannel(false), _isQuery(false), _entry(this)
{
    _hpaned = new Gtk::HPaned();

    _hbox.pack_start(_entry);

    _vbox.pack_start(_textwidget);

    _vbox.pack_start(_hbox, Gtk::PACK_SHRINK);

    Gtk::Button *prefs_button = manage(new Gtk::Button());
    Gtk::Image *prefsimage = manage(new Gtk::Image(Gtk::Stock::PREFERENCES, Gtk::ICON_SIZE_MENU));
    prefs_button->add(*prefsimage);
    prefs_button->signal_clicked().connect(slot(*this, &Tab::startPrefs));
    _hbox.pack_start(*prefs_button, Gtk::PACK_SHRINK);

    _hpaned->pack1(_vbox, true, true);
    pack_start(*_hpaned);
}

Tab::~Tab()
{
    delete _hpaned;
    if (_nicklist)
          delete _nicklist;
}

void Tab::setInActive()
{
    if (isActive()) {
        Gtk::Label *label = AppWin->getNotebook().getLabel(this);
        label->set_text("(" + label->get_text() + ")");
        _isActive = false;
    }
}

void Tab::setActive()
{
    Gtk::Label *label = AppWin->getNotebook().getLabel(this);

    if (label->get_text().at(0) == '(') {
        // Remove the parentes.
        Glib::ustring text = label->get_text();
        label->set_text(text.substr(1, text.length() - 2));
    }

    _isActive = true;

    if (_nicklist)
          _nicklist->setActive();
}

void Tab::setQuery(bool value)
{
    _isQuery = value;
    _isChannel = !value;
    addOrRemoveNickList();
}

void Tab::setChannel(bool value)
{
    _isChannel = value;
    _isQuery = !value;
    addOrRemoveNickList();
}

void Tab::addOrRemoveNickList()
{
    if (isChannel() && !_nicklist) {
        _nicklist = new NickList;
        _hpaned->pack2(*_nicklist, false, true);
    } else if (isQuery() && _nicklist) {
        _hpaned->remove(*_nicklist);
        delete _nicklist;
        _nicklist = 0;
    }
}

void Tab::insertUser(const Glib::ustring& user, IRC::UserMode m)
{
    if (isChannel())
          _nicklist->insertUser(user, m);
}

void Tab::removeUser(const Glib::ustring& nick)
{
    if (isChannel())
          _nicklist->removeUser(nick);
}

void Tab::renameUser(const Glib::ustring& from, const Glib::ustring& to)
{
    if (isChannel())
          _nicklist->renameUser(from, to);
    else if (isQuery())
          AppWin->getNotebook().getLabel(this)->set_text(to);
}

bool Tab::findUser(const Glib::ustring& nick)
{
    if (isChannel())
          return _nicklist->findUser(nick);
    else 
          return (nick == AppWin->getNotebook().getLabel(this)->get_text());
    
}

std::vector<Glib::ustring> Tab::getNicks()
{
    if (isChannel())
          return _nicklist->getNicks();
    else {
        std::vector<Glib::ustring> vec;
        vec.push_back(AppWin->getNotebook().getLabel(this)->get_text());
        return vec;
    }
}

void Tab::startPrefs()
{
    remove(*_hpaned);
    Prefs *p = Prefs::Instance();
    if (Prefs::currentTab)
          p->closePrefs();

    Prefs::currentTab = this;
    pack_start(*p);
    hasPrefs = true;
}

void Tab::closePrefs()
{
    Prefs *p = Prefs::Instance();
    remove(*p);
    Prefs::currentTab = 0;
    pack_start(*_hpaned);
    _entry.grab_focus();
    hasPrefs = false;
}

void Tab::startDCCList()
{
    DCCList *dcc = DCCList::Instance();
    if (DCCList::currentTab)
          dcc->closeDCCList();

    DCCList::currentTab = this;
    _vbox.pack_end(*dcc);
    hasDCCList = true;
}

void Tab::closeDCCList()
{
    DCCList *dcc = DCCList::Instance();
    _vbox.remove(*dcc);
    DCCList::currentTab = 0;
    _entry.grab_focus();
    hasDCCList = false;

}
