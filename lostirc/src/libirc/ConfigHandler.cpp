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

#include "ConfigHandler.h"

using std::string;
using std::cout;
using std::vector;
using std::map;

bool ConfigHandler::readConfig()
{
    string home(getenv("HOME"));
    std::ifstream in(string(home + "/.lostircrc").c_str());

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
    std::cout << "trying to set '" + param + "' to: '" + value + "'" << std::endl;
    #endif
    string home(getenv("HOME"));
    std::ofstream out(string(home + "/.lostircrc").c_str(), std::ios::app);

    if (!out)
          return false;

    out << param << " = " << value << std::endl;

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
    _settings["evt_privmsg"] = "$12<$0%1$12>$0 %2";
    _settings["evt_privmsg_highlight"] = "$2<$8%1$2>$0 %2";
    _settings["evt_action"] = "$7* %1$0 %2";
    _settings["evt_action_highlight"] = "$8* %1$0 %2";
    _settings["evt_servmsg"] = "$0-- : %1";
    _settings["evt_servmsg2"] = "$0-- : %1 %2";
    _settings["evt_ctcp"] = "$16-- CTCP %1 received from $0%2";
    _settings["evt_topicchange"] = "$16-- $0%1$13 changes topic to: %2";
    _settings["evt_topicis"] = "$16-- Topic for $11%1$16 is:$0 %2";
    _settings["evt_topictime"] = "$16-- Set by $0%1$16 on $9%2";
    _settings["evt_noticepriv"] = "$7NOTICE $0%1$7 : %2";
    _settings["evt_noticepubl"] = "$7NOTICE $0%1$7 (to %2): %3";
    _settings["evt_error"] = "$16-- Error:$8 %1";
    _settings["evt_away"] = "$3User $0%1$3 is away $15($3%2$15)";
    _settings["evt_banlist"] = "$16-- Ban: $9%1$16 set by: $0%2";
    _settings["evt_unknown"] = "$16-- Unknown message: $2%1";
    _settings["evt_join"] = "$16-- $0%1$11 $15($9%3$15)$16 has joined $11%2";
    _settings["evt_part"] = "$16-- $0%1$16 $15($9%3$15)$16 has parted $11%2";
    _settings["evt_quit"] = "$16-- $0%1$16 has quit $11(%2)";
    _settings["evt_nick"] = "$16-- $0%1$16 changes nick to %2";
    _settings["evt_mode"] = "$16-- $0%1$16 sets mode $5%2$16 %3";
    _settings["evt_cmode"] = "$16-- $0%1$16 sets channel mode $5%2 %3$16 on %4";
    _settings["evt_wallops"] = "$2WALLOPS -: %1 :- %2";
    _settings["evt_kicked"] = "$16-- $0%1$16 was kicked from $11%2$16 by %3 $15($9%4$15)";

    return writeConfig();
}

bool ConfigHandler::writeConfig()
{
    string home(getenv("HOME"));
    std::ofstream out(string(home + "/.lostircrc").c_str());

    if (!out)
          return false;

    map<string, string>::const_iterator i;

    for (i = _settings.begin(); i != _settings.end(); ++i) {
        out << (*i).first << " = " << (*i).second << std::endl;
    }
    return true;
}
