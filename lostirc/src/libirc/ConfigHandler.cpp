/*
 * Copyright (C) 2001 Morten Brix Pedersen <morten@wtf.dk>
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

#include "ConfigHandler.h"

using std::string;
using std::cout;
using std::vector;
using std::map;

bool ConfigHandler::readConfig()
{
    string home(getenv("HOME"));
    ifstream in(string(home + "/.lostircrc").c_str());

    if (in) {
        string str;
        while (getline(in, str)) {
            vector<string> vec;
            Utils::Tokenize(str, vec);
            vector<string>::const_iterator i = vec.begin();

            string _tmpparam;
            string _tmpvalue;
            /* parse a string in the form 'value = param param param' */
            while (i != vec.end())
            {
                if (*i == "=") {
                    ++i;
                    while (i != vec.end())
                    {
                        _tmpvalue += *i + " ";
                        ++i;
                    }
                    if (_tmpvalue.size() > 0) {
                        _tmpvalue = _tmpvalue.substr(0, _tmpvalue.size() - 1);
                    }
                    _settings.insert(make_pair(_tmpparam, _tmpvalue));
                    break;
                } else {
                    _tmpparam = *i;
                }

                ++i;
            }
            
        }
    }
    return setDefaults();
}

bool ConfigHandler::setParam(const string& param, const string& value)
{
    #ifdef DEBUG
    cout << "trying to set '" + param + "' to: '" + value + "'" << endl;
    #endif
    string home(getenv("HOME"));
    ofstream out(string(home + "/.lostircrc").c_str(), ios::app);

    if (!out)
          return false;

    out << param << " = " << value << endl;

    return true;
}

string ConfigHandler::getParam(const string& param)
{
    map<string, string>::const_iterator i = _settings.find(param);

    if (i != _settings.end())
          return (*i).second;

    return "";

}

bool ConfigHandler::setDefaults()
{
    _settings["evt_privmsg"] = "$1<%1>$2 %2";
    _settings["evt_privmsg_highlight"] = "$1<$4%1$1>$2 %2";
    _settings["evt_servmsg"] = "-- : %1";
    _settings["evt_servmsg2"] = "-- : %1 %2";
    _settings["evt_ctcp"] = "$8-- CTCP %1 received from %2";
    _settings["evt_topicchange"] = "$6-- %1 changes topic to: %2";
    _settings["evt_topicis"] = "$6-- Topic for %1 is: %2";
    _settings["evt_topictime"] = "$6-- Set by %1 on %2";
    _settings["evt_action"] = "$3* %1 $1%2";
    _settings["evt_noticepriv"] = "$7NOTICE %1 : %2";
    _settings["evt_noticepubl"] = "$7NOTICE %1 (to %2): %3";
    _settings["evt_error"] = "$4Error:$1 %1";
    _settings["evt_away"] = "$3User %1 is away (%2)";
    _settings["evt_banlist"] = "$2Ban: %1 set by: %2";
    _settings["evt_unknown"] = "$3Unknown message: $2%1";
    _settings["evt_join"] = "$8-- %1 (%3) has joined %2";
    _settings["evt_part"] = "$8-- %1 (%3) has parted %2";
    _settings["evt_wallops"] = "$2WALLOPS -: %1 :- %2";
    _settings["evt_kicked"] = "$8-- %1 was kicked from %2 by %3 (%4)";

    return writeConfig();

}

bool ConfigHandler::writeConfig()
{
    string home(getenv("HOME"));
    ofstream out(string(home + "/.lostircrc").c_str());

    if (!out)
          return false;

    map<string, string>::const_iterator i;

    for (i = _settings.begin(); i != _settings.end(); ++i) {
        out << (*i).first << " = " << (*i).second << endl;
    }
    return true;
}
