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

int autoCompletion(ustring& search, ustring& matches_str, vector<ustring> full_list);

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

            GuiCommands::send(_tab->getConn(), Util::upper(msg.substr(1, pos - 1)), params);

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
            ustring word;
            ustring::size_type pos = line.find_last_of(" ");
            if (pos == ustring::npos) {
                pos = 0;
                word = line.substr(pos);
            } else {
                word = line.substr(pos + 1);
            }
            if (line.at(0) == '/' && !word.empty() && pos == 0) {

                // Auto complete the command
                ustring matches_str;
                word = word.substr(1);
                int matches = autoCompletion(word, matches_str, GuiCommands::getCommands());

                if (matches == 1) {
                    set_text("/" + word + " ");
                    set_position(-1);

                } else if (matches > 1) {
                    set_text("/" + word);
                    set_position(-1);
                    AppWin->statusbar.setText2("<span foreground=\"blue\">Matches:</span> " + matches_str);

                } else {
                    AppWin->statusbar.setText2("<span foreground=\"blue\">No matches.</span>");

                }

            } else if (!word.empty()) {

                ustring matches_str;
                int matches = autoCompletion(word, matches_str, _tab->getNicks());

                if (matches == 1) {

                    if (pos == 0) {
                        set_text(word + App->options.nickcompletion_char + " ");
                        set_position(-1);
                    } else {
                        set_text(line.substr(0, pos + 1) + word);
                        set_position(-1);
                    }


                } else if (matches > 1) {
                    if (pos == 0) {
                        set_text(word);
                        set_position(-1);
                    } else {
                        set_text(line.substr(0, pos + 1) + word);
                        set_position(-1);
                    }
                    AppWin->statusbar.setText2("<span foreground=\"blue\">Matches:</span> " + matches_str);
                } else {
                    AppWin->statusbar.setText2("<span foreground=\"blue\">No matches.</span>");

                }

            }
        }
    }
    return true;
}

struct notPrefixedBy : public std::binary_function<ustring,ustring,bool> 
{
    bool operator() (const ustring& str1, const ustring& str2) const {
        if (str1.length() >= str2.length() && str2.lowercase() == str1.substr(0, str2.length()).lowercase())
              return false;
        else
              return true;
    }
};


void findCommon(vector<ustring>& vec, const ustring& search, unsigned int& atchar)
{
    if (atchar > search.length())
          return;

    for (vector<ustring>::const_iterator i = vec.begin(); i != vec.end(); ++i)
    {
        if (atchar > i->length() ||
                search.substr(0, atchar).lowercase() != i->substr(0, atchar).lowercase()) {
            atchar--;
            return;
        }
    }
    findCommon(vec, search, ++atchar);

}

int autoCompletion(ustring& search, ustring& matches_str, vector<ustring> full_list)
{
    full_list.erase(remove_if(full_list.begin(), full_list.end(), std::bind2nd(notPrefixedBy(), search)), full_list.end());;

    if (full_list.size() == 1) {
        // Match!
        search = full_list[0];

    } else if (full_list.size() > 1) {

        unsigned int atchar = 1;
        findCommon(full_list, full_list[0], atchar);

        search = full_list[0].substr(0, atchar);

        // More than one candidate.
        for (vector<ustring>::const_iterator i = full_list.begin(); i != full_list.end(); ++i)
              matches_str += *i + " ";

    }

    return full_list.size();
}

