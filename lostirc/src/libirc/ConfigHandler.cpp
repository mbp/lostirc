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

bool ConfigHandler::setParam(const string& key, const string& value)
{
    #ifdef DEBUG
    std::cout << "trying to set '" + key + "' to: '" + value + "'" << std::endl;
    #endif
    _settings[key] = value;

    return writeConfig();
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
    setDefault("evt_privmsg", "$12<$0%1$12>$0 %2");
    setDefault("evt_privmsg_highlight", "$2<$8%1$2>$0 %2");
    setDefault("evt_action", "$7* %1$0 %2");
    setDefault("evt_action_highlight", "$8* %1$0 %2");
    setDefault("evt_servmsg", "$0-- : %1");
    setDefault("evt_servmsg2", "$0-- : %1 %2");
    setDefault("evt_ctcp", "$16-- CTCP %1 received from $0%2");
    setDefault("evt_topicchange", "$16-- $0%1$16 changes topic to:$15 %2");
    setDefault("evt_topicis", "$16-- Topic for $11%1$16 is:$0 %2");
    setDefault("evt_topictime", "$16-- Set by $0%1$16 on $9%2");
    setDefault("evt_noticepriv", "$7NOTICE $0%1$7 : %2");
    setDefault("evt_noticepubl", "$7NOTICE $0%1$7 (to %2): %3");
    setDefault("evt_error", "$16-- Error:$8 %1");
    setDefault("evt_away", "$3User $0%1$3 is away $15($3%2$15)");
    setDefault("evt_banlist", "$16-- Ban: $9%1$16 set by: $0%2");
    setDefault("evt_unknown", "$16-- Unknown message: $2%1");
    setDefault("evt_join", "$16-- $0%1$11 $15($9%3$15)$16 has joined $11%2");
    setDefault("evt_part", "$16-- $0%1$16 $15($9%3$15)$16 has parted $11%2");
    setDefault("evt_quit", "$16-- $0%1$16 has quit $11(%2)");
    setDefault("evt_nick", "$16-- $0%1$16 changes nick to %2");
    setDefault("evt_mode", "$16-- $0%1$16 sets mode $5%2$16 %3");
    setDefault("evt_cmode", "$16-- $0%1$16 sets channel mode $5%2 %3$16 on %4");
    setDefault("evt_wallops", "$2WALLOPS -: %1 :- %2");
    setDefault("evt_kicked", "$16-- $0%1$16 was kicked from $11%2$16 by %3 $15($9%4$15)");

    return writeConfig();
}

void ConfigHandler::setDefault(const string& key, const string& value)
{
    map<string, string>::const_iterator i = _settings.find(key);

    if (i == _settings.end())
          _settings[key] = value;
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
