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

#include <pangomm/fontdescription.h>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <ServerConnection.h>
#include <Utils.h>
#include "Tab.h"
#include "Prefs.h"
#include "MainWindow.h"

using std::vector;
using Glib::ustring;

Tab::Tab(Gtk::Label *label, ServerConnection *conn, Pango::FontDescription font)
    : Gtk::VBox(), isHighlighted(false), hasPrefs(false),
      isOnChannel(true), _label(label), _conn(conn), _entry(this)
{
    // Creating HBox; will contain 2 widgets, a scrollwindow and an entry
    _hbox = new Gtk::HBox(false, 3); 

    // Attaching Gtk::TextView to scollwindow
    
    _textview.set_wrap_mode(Gtk::WRAP_WORD);
    _textview.unset_flags(Gtk::CAN_FOCUS);
    _textview.set_editable(false);
    _swin.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    _swin.set_size_request(0, -1);
    _swin.add(_textview);

    _hbox->pack_start(_swin);
    pack_start(*_hbox);
    _hbox2 = new Gtk::HBox();
    pack_start(*_hbox2, Gtk::FILL);
    _hbox2->pack_start(_entry);

    Gtk::Button *_button = manage(new Gtk::Button("Prefs"));
    _button->signal_clicked().connect(slot(*this, &Tab::startPrefs));
    _hbox2->pack_start(*_button, Gtk::SHRINK);

    if (_conn->Session.isAway)
          setAway();

    initializeColorMap();
    setStyle();

    _textview.modify_font(font);
}

Tab::~Tab()
{
    delete _hbox2;
    delete _hbox;
}

void Tab::setFont(const Pango::FontDescription& font)
{
    _textview.modify_font(font);
}

void Tab::setStyle() {
    // TODO: Should this go into a ressource file?
    Gdk::Color col1;
    col1.set_rgb(0, 0, 0);

    _textview.modify_base(Gtk::STATE_NORMAL, col1);
}

Tab& Tab::operator<<(const char * str)
{
    // Convert the string to utf8 and pass it on to the real operator <<
    return operator<<(Glib::locale_to_utf8(str));
}

Tab& Tab::operator<<(const std::string& str)
{
    // Convert the string to utf8 and pass it on to the real operator <<
    return operator<<(Glib::locale_to_utf8(str));
}

Tab& Tab::operator<<(const ustring& line)
{
    // Add timestamp 
    time_t timeval = time(0);
    char tim[11];
    strftime(tim, 10, "%H:%M:%S ", localtime(&timeval));

    insertWithColor(0, ustring(tim));

    // FIXME: can be done prettier and better with TextBuffer marks

    ustring::size_type lastPos = line.find_first_not_of("\003", 0);
    ustring::size_type pos = line.find_first_of("\003", lastPos);

    while (ustring::npos != pos || ustring::npos != lastPos)
    {   
        // Check for digits
        if (Util::isDigit(line.substr(lastPos, 2))) {
            int color = Util::stoi(line.substr(lastPos, 2));
            insertWithColor(color, line.substr(lastPos + 2, (pos - lastPos) - 2));
        } else if (Util::isDigit(line.substr(lastPos, 1))) {
            int color = Util::stoi(line.substr(lastPos, 1));
            insertWithColor(color, line.substr(lastPos + 1, (pos - lastPos) - 1));
        } else {
            insertWithColor(0, line.substr(lastPos, pos - lastPos));
        }

        lastPos = line.find_first_not_of("\003", pos);
        pos = line.find_first_of("\003", lastPos);
    }
    return *this;
}

void Tab::insertWithColor(int color, const ustring& str)
{   
    // see if the scrollbar is located in the bottom, then we need to scroll
    // after insert 
    bool scroll = false;
    if (_swin.get_vadjustment()->get_value() >= (_swin.get_vadjustment()->get_upper() - _swin.get_vadjustment()->get_page_size() - 1e-12))                                                  
          scroll = true;

    // Insert the text
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
    buffer->insert_with_tag(buffer->end(), str, colorMap[color]);

    if (scroll)
          _textview.scroll_to_mark(buffer->create_mark("e", buffer->end()), 0.0);

    // FIXME: possible performance critical
    int buffer_size = Util::stoi(App->getCfg().getOpt("buffer_size"));
    if (buffer_size && buffer->get_line_count() > buffer_size) {
        Gtk::TextBuffer::iterator start = buffer->get_iter_at_line(0);
        Gtk::TextBuffer::iterator end = buffer->get_iter_at_line(buffer->get_line_count() - buffer_size);
        buffer->delete_text(start, end);
    }
}

void Tab::insertText(const ustring& str)
{
    insertWithColor(0, str);
}

void Tab::clearText()
{
    Glib::RefPtr<Gtk::TextBuffer> buf = _textview.get_buffer();
    Gtk::TextBuffer::iterator begin = buf->begin();
    Gtk::TextBuffer::iterator end = buf->end();
    buf->delete_text(begin, end);
}

void Tab::setAway()
{
    setUnAway();

    bool away = false;

    Gtk::Box_Helpers::BoxList::iterator i;

    for (i = _hbox2->children().begin(); i != _hbox2->children().end(); ++i) {
        Gtk::Label *a = dynamic_cast<Gtk::Label*>(i->get_widget());
        if (a)
              away = true;
    }

    if (!away) {
        Gtk::Label *a = manage(new Gtk::Label("Away (" + _conn->Session.awaymsg + ")"));
        _hbox2->pack_start(*a);
        _hbox2->show_all();
    }
}

void Tab::setUnAway()
{
    Gtk::Box_Helpers::BoxList::iterator i;

    for (i = _hbox2->children().begin(); i != _hbox2->children().end();) {
        Gtk::Label *a = dynamic_cast<Gtk::Label*>(i->get_widget());
        if (a)
              i = _hbox2->children().erase(i);
        else
              ++i;
    }
    _hbox2->show_all();
}

void Tab::startPrefs()
{
    remove(*_hbox2);
    remove(*_hbox);
    Prefs *p = Prefs::Instance();
    if (Prefs::currentTab)
          p->endPrefs();

    Prefs::currentTab = this;
    pack_start(*p);
    hasPrefs = true;
}

void Tab::endPrefs()
{
    Prefs *p = Prefs::Instance();
    remove(*p);
    Prefs::currentTab = 0;
    pack_start(*_hbox);
    pack_start(*_hbox2, Gtk::FILL);
    _entry.grab_focus();
    hasPrefs = false;
}

TabChannel::TabChannel(Gtk::Label *label, ServerConnection *conn, Pango::FontDescription font)
    : Tab(label, conn, font),
    _columns(),
    _liststore(Gtk::ListStore::create(_columns)),
    _treeview(_liststore)
{

    Gtk::ScrolledWindow *swin = manage(new Gtk::ScrolledWindow());
    swin->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    _users = manage(new Gtk::Frame("Not on channel"));
    _treeview.append_column("", _columns.status);
    _treeview.append_column("", _columns.nick);
    _treeview.get_selection()->set_mode(Gtk::SELECTION_NONE);
    _liststore->set_sort_column_id(0, Gtk::SORT_ASCENDING);
    _liststore->set_sort_func(0, SigC::slot(*this, &TabChannel::sortFunc));

    /* FIXME: no set_column_width() like in the old days! */

    _treeview.set_headers_visible(false);
    //_treeview.set_default_size(100, 100);
    swin->add(_treeview);

    _hbox->pack_start(*_users, Gtk::SHRINK);
    _users->add(*swin);
}

void TabChannel::updateUserNumber()
{
    size_t size = _liststore->children().size();
    if (size > 0) {
        std::stringstream ss;
        ss << size;
        _users->set_label(ss.str() + " users");
    } else {
        _users->set_label("Not on channel");
    }
}

void TabChannel::insertUser(const ustring& nick, IRC::UserMode m)
{
    Glib::ustring sign;
    switch (m)
    {
        case IRC::OP:
            sign = "@";
            break;
        case IRC::VOICE:
            sign = "+";
            break;
        case IRC::HALFOP:
            sign = "%";
            break;
        case IRC::NONE:
            sign = " ";
    }

    Gtk::TreeModel::Row row = *_liststore->append();
    row[_columns.status] = sign;
    row[_columns.nick] = nick;
    updateUserNumber();
}

void TabChannel::removeUser(const ustring& nick)
{
    Gtk::ListStore::iterator i = _liststore->children().begin();

    while (i != _liststore->children().end())
    {
        if (i->get_value(_columns.nick) == nick)
              i = _liststore->erase(i);
        else
              ++i;
    }

    updateUserNumber();
}

void TabChannel::renameUser(const ustring& from, const ustring& to)
{
    Gtk::ListStore::iterator i = _liststore->children().begin();

    while (i != _liststore->children().end())
    {
        if (i->get_value(_columns.nick) == from) {
            i->set_value(_columns.nick, to);
            break;
        }
        i++;
    }
}

bool TabChannel::findUser(const ustring& nick)
{
    Gtk::ListStore::iterator i = _liststore->children().begin();

    while (i != _liststore->children().end())
    {
        if (i->get_value(_columns.nick) == nick)
              return true;
        i++;
    }
    return false;
}

bool TabChannel::nickCompletion(const ustring& word, ustring& str)
{
    Gtk::ListStore::iterator i = _liststore->children().begin();

    int matches = 0;
    ustring nicks;
    // Convert it to lowercase so we can search ignoring the case
    ustring lcword = word.casefold();
    while (i != _liststore->children().end())
    {
        ustring nick = i->get_value(_columns.nick);

        ustring lcnick = nick.casefold();
        if (lcword == lcnick.substr(0, lcword.length())) {
            str = nick;
            nicks += nick + " ";
            matches++;
        }
        i++;
    }

    if (matches == 1) {
        return true;
    } else if (matches > 1) {
        str = nicks + '\n';
        return false;
    } else {
        str = "";
        return false;
    }
}

gint TabChannel::sortFunc(const Gtk::TreeModel::iterator& i1, const Gtk::TreeModel::iterator& i2)
{
    // Sort the nicklist. The status field has highest priority, nick has second priority.

    // This is not very readable, but it works, and it has to be fast when
    // joining huge channels.

    return strcmp(i1->get_value(_columns.status).c_str(), i2->get_value(_columns.status).c_str())
            <
            strcmp(i1->get_value(_columns.nick).c_str(), i2->get_value(_columns.nick).c_str());

}

void Tab::helperInitializer(int i, const char * colorname)
{
    Glib::RefPtr<Gtk::TextTag> texttag = Gtk::TextTag::create();
    Glib::PropertyProxy_WriteOnly<Glib::ustring> fg = texttag->property_foreground();
    fg.set_value(colorname);
    _textview.get_buffer()->get_tag_table()->add(texttag);
    colorMap[i] = texttag;
}

void Tab::initializeColorMap()
{
    helperInitializer(0, "#FFFFFF");
    helperInitializer(1, "#000000");
    helperInitializer(2, "#0000CC");
    helperInitializer(3, "#00CC00");
    helperInitializer(4, "#DD0000");
    helperInitializer(5, "#AA0000");
    helperInitializer(6, "#BB00BB");
    helperInitializer(7, "#FFAA00");
    helperInitializer(8, "#EEDD22");
    helperInitializer(9, "#33DE55");
    helperInitializer(10, "#00CCCC");
    helperInitializer(11, "#33DDEE");
    helperInitializer(12, "#0000FF");
    helperInitializer(13, "#EE22EE");
    helperInitializer(14, "#777777");
    helperInitializer(15, "#999999");
    helperInitializer(16, "#BEBEBE");
    helperInitializer(17, "#000000");
    helperInitializer(18, "#FFFFFF");
    helperInitializer(19, "#000000");
}
