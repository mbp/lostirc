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

#include <sstream>
#include "GuiCommands.h"
#include "Tab.h"
#include "Entry.h"

using std::vector;
using std::string;
using Glib::ustring;

Entry::Entry(Tab* tab)
    : Gtk::Entry(), _tab(tab), i(_entries.begin())
{
    signal_key_press_event().connect(slot(*this, &Entry::onKeyPress));
    signal_activate().connect(slot(*this, &Entry::onEntry));
}

void Entry::onEntry()
{
    if (get_text().length() == 0)
          return;

    ustring msg = get_text();

    // If the line is prefixed by two slashes, just send it as a msg.
    if (msg.length() > 1 && (msg.at(0) == '/' && msg.at(1) == '/')) {
        sendMsg(msg.substr(1));
    } else if (msg.at(0) == '/') {

        ustring::size_type pos = msg.find_first_of(" ");

        ustring params;
        if (pos != ustring::npos)
              params = msg.substr(pos + 1);

        try {

            GuiCommands::send(_tab->getConn(), Util::upper(Glib::locale_from_utf8(msg.substr(1, pos - 1))), Glib::locale_from_utf8(params));

        } catch (CommandException& ce) {

            *_tab << string(ce.what()) + string("\n");

        }

    } else {
        sendMsg(msg);
    }

    _entries.push_back(msg);
    i = _entries.begin();
    set_text("");
}

void Entry::sendMsg(const ustring& msg)
{
    if (!_tab->getConn()->Session.isConnected) {
        *_tab << "Not connected to server.\n";
    } else if (!_tab->isActive()) {
        *_tab << "Not on any channel.\n";
    } else {
        std::istringstream ss(Glib::locale_from_utf8(msg));

        if (ss.peek() == '\n')
              ss.ignore();

        // FIXME - ustring
        string line;
        while (getline(ss, line)) {
            _tab->getConn()->sendMsg(_tab->getLabel()->get_text(), line);
            *_tab << "\0037<\0030" + _tab->getConn()->Session.nick + "\0037>\0030 " + line + '\n';
        }
    }
}

bool Entry::onKeyPress(GdkEventKey* e)
{
    if ((e->keyval == GDK_uparrow) || (e->keyval == GDK_Up)) {
        if (!_entries.empty()) {
            // Use iterator to go to next element
            if (i == _entries.begin())
                  i = _entries.end() - 1;
            else
                  --i;

            set_text(*i);
            set_position(-1);

        }
    } else if ((e->keyval == GDK_downarrow) || (e->keyval == GDK_Down)) {
        if (!_entries.empty()) {
            // Use iterator to go to previous element
            i++;
            if (i == _entries.end())
                  i = _entries.begin();

            set_text(*i);
            set_position(-1);
        }
    } else if ((e->keyval == GDK_Tab)) {
        // Nick completion using Tab key
        ustring line = get_text();
        if (!line.empty()) {
            ustring str, word;
            ustring::size_type pos = line.find_last_of(" ");
            if (pos == ustring::npos) {
                pos = 0;
                word = line.substr(pos);
            } else {
                word = line.substr(pos + 1);
            }
            if (line.at(0) == '/' && !word.empty() && pos == 0) {
                // Command completion, could be prettier
                string newstr = Glib::locale_from_utf8(str);
                if (GuiCommands::commandCompletion(Glib::locale_from_utf8(word.substr(1)), newstr)) {
                    set_text("/" + Glib::locale_to_utf8(newstr) + " ");
                    set_position(-1);
                }

            } else if (_tab->nickCompletion(word, str)) {
                // Nick-completetion
                if (pos == 0) {
                    set_text(str + App->options.nickcompletion_char + " ");
                    set_position(-1);
                } else {
                    set_text(line.substr(0, pos + 1) + str);
                    set_position(-1);
                }
            } else {
                AppWin->statusbar.setText2("<span foreground=\"blue\">Matches:</span> " + str);
            }
        }
    }
    return true;
}
