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

    if (in.fail())
          return false;

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
    return true;
}

bool ConfigHandler::setParam(const string& param, const string& value)
{
    #ifdef DEBUG
    cout << "trying to set '" + param + "' to: '" + value + "'" << endl;
    #endif
    string home(getenv("HOME"));
    ofstream out(string(home + "/.lostircrc").c_str(), ios::app);

    out << param + " = " + value << endl;

    return true;
}

string ConfigHandler::getParam(const string& param)
{
    map<string, string>::const_iterator i = _settings.find(param);

    if (i != _settings.end())
          return (*i).second;

    return "";

}
