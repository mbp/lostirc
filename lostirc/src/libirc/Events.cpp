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

#include <cctype>
#include "Events.h"

Events::Events(LostIRCApp *app)
    : _app(app)
{

}

void Events::emitEvent(const string& name, vector<string>& args, const string& to, ServerConnection *conn)
{
    string newmsg;
    string msg = _app->getCfg().getParam("evt_" + name);
    std::replace(msg.begin(), msg.end(), '$', '\003');

    bool parsing_arg;
    string::const_iterator i;
    for (i = msg.begin(); i != msg.end(); ++i) {
        switch (*i) {
            case '%':
                parsing_arg = true;
                break;
            default:
                if (isdigit(*i) && parsing_arg) {
                    int num = ((*i) - '0') - 1;
                    if (num >= args.size()) {
                        cerr << "Fatal error, too many args!" << "[ " << num << " compared to " << args.size() << " ]" << endl;
                    } else {
                        newmsg += args[num];
                    }
                    parsing_arg = false;
                    break;
                }
                newmsg += *i;
                parsing_arg = false;
        }
    }

    newmsg += '\n';

    _app->evtDisplayMessage(newmsg, to, conn);
}

void Events::emitEvent(const string& name, const string& arg, const string& to, ServerConnection *conn)
{
    vector<string> args;
    args.push_back(arg);

    emitEvent(name, args, to, conn);
}

void Events::emitEvent(const string& name, const string& arg, const vector<string>& to, ServerConnection *conn) {

    vector<string> args;
    args.push_back(arg);
    vector<string>::const_iterator i;

    for (i = to.begin(); i != to.end(); ++i) {
        string to = *i;
        emitEvent(name, args, to, conn);
    }
}

void Events::emitEvent(const string& name, vector<string>& arg, const vector<string>& to, ServerConnection *conn) {

    vector<string>::const_iterator i;

    for (i = to.begin(); i != to.end(); ++i) {
        string to = *i;
        emitEvent(name, arg, to, conn);
    }
}
