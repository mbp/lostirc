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

#include "GuiCommands.h"
#include "Entry.h"
#include <sstream>

using std::vector;
using std::string;

Entry::Entry(Tab* tab)
    : Gtk::Entry(), _tab(tab), i(_entries.rbegin())
{
    key_press_event.connect(slot(this, &Entry::on_key_press_event));
    activate.connect(slot(this, &Entry::onEntry));
}

void Entry::onEntry()
{
    if (get_text().length() == 0)
          return;

    string msg(get_text());
    if (msg.at(0) == '/') {

        string::size_type pos = msg.find_first_of(" ");

        string params;
        if (pos != string::npos) {
            params = msg.substr(pos + 1);
        }

        try {

            GuiCommands::send(_tab->getConn(), msg.substr(1, pos - 1), params);

        } catch (CommandException& ce) {

            _tab->parseAndInsert(string(ce.what()) + string("\n"));

        }

    } else {
        if (!_tab->getConn()->Session.isConnected) {
            _tab->parseAndInsert("Not connected to server.\n");
        } else if (!_tab->is_on_channel) {
            _tab->parseAndInsert("Not on any channel.\n");
        } else {
            printText(msg);
        }
    }

    _entries.push_back(msg);
    i = _entries.rbegin();
    set_text("");
}

void Entry::printText(const string& msg)
{
    std::istringstream ss(msg);

    if (ss.peek() == '\n')
          ss.ignore();

    string line;
    while (getline(ss, line)) {
        _tab->getConn()->sendMsg(_tab->getLabel()->get_text(), line);
        _tab->parseAndInsert("\0037<\0030" + _tab->getConn()->Session.nick + "\0037>\0030 " + line + '\n');
    }
}

gint Entry::on_key_press_event(GdkEventKey* e)
{
    if ((e->keyval == GDK_uparrow) || (e->keyval == GDK_Up)) {
        if (!_entries.empty()) {
            // Use reverse iterator to go to next element
            if (i == _entries.rend())
                  i = _entries.rbegin();

            set_text(*i);

            if (i != _entries.rend())
                  ++i;
        }
    } else if ((e->keyval == GDK_Tab)) {
        // Nick completion using Tab key
        string line = get_text();
        if (!line.empty()) {
            string str, word;
            string::size_type pos = line.find_last_of(" ");
            if (pos == string::npos) {
                pos = 0;
                word = line.substr(pos);
            } else {
                word = line.substr(pos + 1);
            }
            if (line.at(0) == '/' && !word.empty() && pos == 0) {
                // Command completion, could be prettier
                if (GuiCommands::commandCompletion(word.substr(1), str)) {
                    set_text("/" + str + " ");
                }

            } else if (_tab->nickCompletion(word, str)) {
                // Nick-completetion
                if (pos == 0) {
                    set_text("");
                    append_text(str + AppWin->getApp()->getCfg().getOpt("nickcompletion_character") + " ");
                } else {
                    set_text(line.substr(0, pos + 1));
                    append_text(str);
                }
            } else {
                _tab->getText()->insert(str);
            }
        }
    }
    return 0;
}
