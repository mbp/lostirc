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
      _fallback_encoding("ISO-8859-15"),
      isOnChannel(true), _label(label), _conn(conn), _entry(this)
{
    _vbox = new Gtk::VBox();
    _hbox = new Gtk::HBox();
    _hpaned = new Gtk::HPaned();

    _hbox->pack_start(_entry);

    // Attaching Gtk::TextView to scollwindow
    
    _textview.set_wrap_mode(Gtk::WRAP_CHAR);
    _textview.unset_flags(Gtk::CAN_FOCUS);
    _textview.set_editable(false);
    _swin.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    _swin.set_size_request(0, -1);
    _swin.add(_textview);

    _vbox->pack_start(_swin);

    _vbox->pack_start(*_hbox, Gtk::PACK_SHRINK);

    Gtk::Button *_button = manage(new Gtk::Button("Prefs"));
    _button->signal_clicked().connect(slot(*this, &Tab::startPrefs));
    _hbox->pack_start(*_button, Gtk::PACK_SHRINK);

    initializeColorMap();
    setStyle();

    _textview.modify_font(font);
    _hpaned->pack1(*_vbox, true, true);
    pack_start(*_hpaned);
}

Tab::~Tab()
{
    delete _hbox;
}

void Tab::setFont(const Pango::FontDescription& font)
{
    _textview.modify_font(font);
}

void Tab::setStyle() {
    // TODO: Should this go into a ressource file?
    Gdk::Color col1(Glib::locale_to_utf8(App->colors.bgcolor));

    _textview.modify_base(Gtk::STATE_NORMAL, col1);
}

Tab& Tab::operator<<(const char * str)
{
    return operator<<(std::string(str));
}

Tab& Tab::operator<<(const std::string& str)
{
    Glib::ustring str_utf8 (str);

    if (!str_utf8.validate()) // invalid UTF-8?
    {
        bool did_conversion = false;

        if (!Glib::get_charset()) // locale charset is not UTF-8?
        {
            try // ignore errors -- go on with the fallback if the conversion fails
            {
                str_utf8 = Glib::locale_to_utf8(str);
                did_conversion = true;
            }
            catch(const Glib::ConvertError&)
            {}
        }

        if (!did_conversion)
        {
            // Fallback conversion -- used either if the conversion from the
            // current locale's encoding failed, or if the user is running a
            // UTF-8 locale.
            str_utf8 = Glib::convert(str, "UTF-8", _fallback_encoding);
        }
    }

    // Pass on to the real operator <<
    return operator<<(str_utf8);
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

    // FIXME: temp hack.
    if (color > colorMap.size())
        color = colorMap.size() - 1;

    // Insert the text
    realInsert(color, str);
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();

    if (scroll)
          _textview.scroll_to_mark(buffer->create_mark("e", buffer->end()), 0.0);

    // FIXME: possible performance critical
    int buffer_size = App->options.buffer_size;
    if (buffer_size && buffer->get_line_count() > buffer_size)
          buffer->erase(buffer->begin(), buffer->get_iter_at_line(buffer->get_line_count() - buffer_size));
}

void Tab::realInsert(int color, const ustring& line)
{
    // This function has the purpose to insert the line - but first check to
    // see whether we have an URL.
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();

    ustring::size_type pos1;

    pos1 = line.find("http:");

    if (pos1 == ustring::npos)
          pos1 = line.find("www.");

    if (pos1 == ustring::npos)
          pos1 = line.find("ftp.");

    if (pos1 == ustring::npos)
          pos1 = line.find("ftp:");

    if (pos1 != ustring::npos) {
        // Found an URL - insert the front and end of the line, and insert
        // the URL with special markup.
        ustring::size_type pos2 = line.find(" ", pos1 + 1);

        // What's before the URL
        buffer->insert_with_tag(buffer->end(), line.substr(0, pos1), colorMap[color]);

        // The URL
        buffer->insert_with_tag(buffer->end(), line.substr(pos1, pos2 - pos1), underlinetag);

        // After the URL
        if (pos2 != ustring::npos)
              buffer->insert_with_tag(buffer->end(), line.substr(pos2), colorMap[color]);
    } else {
        // Just insert the line, no URLs were found
        buffer->insert_with_tag(buffer->end(), line, colorMap[color]);
    }
}

void Tab::insertText(const ustring& str)
{
    insertWithColor(0, str);
}

void Tab::clearText()
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
    buffer->erase(buffer->begin(), buffer->end());
}

void Tab::startPrefs()
{
    remove(*_hpaned);
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
    pack_start(*_hpaned);
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
    _liststore->set_default_sort_func(SigC::slot(*this, &TabChannel::sortFunc));
    _liststore->set_sort_column_id(Gtk::TreeSortable::DEFAULT_SORT_COLUMN_ID, Gtk::SORT_ASCENDING);

    /* FIXME: no set_column_width() like in the old days! */

    _treeview.set_headers_visible(false);
    //_treeview.set_default_size(100, 100);
    swin->add(_treeview);

    _users->add(*swin);
    _hpaned->pack2(*_users, false, true);
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
    Gtk::TreeModel::Row row = *_liststore->append();
    row[_columns.nick] = nick;

    switch (m)
    {
        case IRC::OP:
            row[_columns.status] = "@";
            row[_columns.priority] = 3;
            break;
        case IRC::VOICE:
            row[_columns.status] = "+";
            row[_columns.priority] = 2;
            break;
        case IRC::HALFOP:
            row[_columns.status] = "%";
            row[_columns.priority] = 1;
            break;
        case IRC::NONE:
            row[_columns.status] = " ";
            row[_columns.priority] = 0;
    }

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

vector<ustring> TabChannel::getNicks()
{
    vector<ustring> nicks;

    Gtk::ListStore::iterator i = _liststore->children().begin();

    for (i = _liststore->children().begin(); i != _liststore->children().end(); ++i)
    {
        nicks.push_back(i->get_value(_columns.nick));
    }

    return nicks;
}

int TabChannel::sortFunc(const Gtk::TreeModel::iterator& i1, const Gtk::TreeModel::iterator& i2)
{
    // Sort the nicklist. The status field has highest priority, nick has
    // second priority.

    // This is not very readable, but it works, and it has to be fast when
    // joining huge channels.

    if (i1->get_value(_columns.priority) == i2->get_value(_columns.priority))
        return i1->get_value(_columns.nick).compare(i2->get_value(_columns.nick));
    else if (i1->get_value(_columns.priority) > i2->get_value(_columns.priority))
          return -1;
    else
          return 1;

}

void Tab::helperInitializer(int i, const Glib::ustring& colorname)
{
    Glib::RefPtr<Gtk::TextTag> texttag = Gtk::TextTag::create();
    Glib::PropertyProxy_WriteOnly<Glib::ustring> fg = texttag->property_foreground();
    fg.set_value(colorname);
    _textview.get_buffer()->get_tag_table()->add(texttag);
    colorMap[i] = texttag;
}

void Tab::initializeColorMap()
{
    helperInitializer(0, Glib::locale_to_utf8(App->colors.color0));
    helperInitializer(1, Glib::locale_to_utf8(App->colors.color1));
    helperInitializer(2, Glib::locale_to_utf8(App->colors.color2));
    helperInitializer(3, Glib::locale_to_utf8(App->colors.color3));
    helperInitializer(4, Glib::locale_to_utf8(App->colors.color4));
    helperInitializer(5, Glib::locale_to_utf8(App->colors.color5));
    helperInitializer(6, Glib::locale_to_utf8(App->colors.color6));
    helperInitializer(7, Glib::locale_to_utf8(App->colors.color7));
    helperInitializer(8, Glib::locale_to_utf8(App->colors.color8));
    helperInitializer(9, Glib::locale_to_utf8(App->colors.color9));
    helperInitializer(10, Glib::locale_to_utf8(App->colors.color10));
    helperInitializer(11, Glib::locale_to_utf8(App->colors.color11));
    helperInitializer(12, Glib::locale_to_utf8(App->colors.color12));
    helperInitializer(13, Glib::locale_to_utf8(App->colors.color13));
    helperInitializer(14, Glib::locale_to_utf8(App->colors.color14));
    helperInitializer(15, Glib::locale_to_utf8(App->colors.color15));
    helperInitializer(16, Glib::locale_to_utf8(App->colors.color16));
    helperInitializer(17, Glib::locale_to_utf8(App->colors.color17));
    helperInitializer(18, Glib::locale_to_utf8(App->colors.color18));
    helperInitializer(19, Glib::locale_to_utf8(App->colors.color19));

    // Create a underlined-tag.
    underlinetag = Gtk::TextTag::create();
    underlinetag->property_underline() = Pango::UNDERLINE_SINGLE;
    underlinetag->property_foreground().set_value(Glib::locale_to_utf8(App->colors.color0));
    _textview.get_buffer()->get_tag_table()->add(underlinetag);
}
