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

#include <cctype>
#include "Events.h"

Events::Events(InOut *inout)
    : _io(inout)
{
    _events["privmsg"] = "$1<%1>$2 %2";
    _events["privmsg_highlight"] = "$1<$4%1$1>$2 %2";
    _events["servmsg"] = "-- : %1";
    _events["servmsg2"] = "-- : %1 %2";
    _events["ctcp"] = "$8-- CTCP %1 received from %2";
    _events["topicchange"] = "$6-- %1 changes topic to: %2";
    _events["topicis"] = "$6-- Topic for %1 is: %2";
    _events["topictime"] = "$6-- Set by %1 on %2";
    _events["action"] = "$3* %1 $1%2";
    _events["noticepriv"] = "$7NOTICE %1 : %2";
    _events["noticepubl"] = "$7NOTICE %1 (to %3): %4";
    _events["error"] = "$4Error:$1 %1";
    _events["away"] = "$3User %1 is away %2";
    _events["banlist"] = "$2Ban: %1 set by: %2";
    _events["unknown"] = "$3Unknown message: $2%1";
    _events["join"] = "$8-- %1 has joined %2";
    _events["part"] = "$8-- %1 has parted %2";
    _events["wallops"] = "$2WALLOPS -: %1 :- %2";
}

void Events::emitEvent(const string& name, vector<string>& args, const string& to, ServerConnection *conn)
{
    string newmsg;
    string msg = _events[name];

    int total_args = args.size();
    int args_used = 0;
    bool parsing_arg;
    string::const_iterator i;
    for (i = msg.begin(); i != msg.end(); ++i) {
        switch (*i) {
            case '%':
                parsing_arg = true;
                break;
            default:
                if (isdigit(*i) && parsing_arg) {
                    if (args_used > total_args) {
                        cerr << "Fatal error, too many args!" << endl;
                    } else {
                        newmsg += args[args_used];
                    }
                    args_used++;
                    parsing_arg = false;
                    break;
                }
                newmsg += *i;
                parsing_arg = false;
                break;
        }
    }

    newmsg += '\n';

    _io->evtDisplayMessage(newmsg, to, conn);
}

void Events::emitEvent(const string& name, const string& arg, const string& to, ServerConnection *conn)
{
    vector<string> args;
    args.push_back(arg);

    emitEvent(name, args, to, conn);
}
