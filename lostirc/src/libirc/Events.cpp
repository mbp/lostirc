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

struct {
    const char *s;
    signed int value;
} event_map[] = {
    { "evt_privmsg", PRIVMSG },
    { "evt_privmsg_highlight", PRIVMSG_HIGHLIGHT },
    { "evt_action", ACTION },
    { "evt_action_highlight", ACTION_HIGHLIGHT },
    { "evt_servmsg", SERVMSG },
    { "evt_servmsg2", SERVMSG2 },
    { "evt_ctcp", CTCP },
    { "evt_topicchange", TOPICCHANGE },
    { "evt_topicis", TOPICIS },
    { "evt_topictime", TOPICTIME },
    { "evt_noticepriv", NOTICEPRIV },
    { "evt_noticepubl", NOTICEPUBL },
    { "evt_error", ERROR },
    { "evt_away", AWAY },
    { "evt_banlist", BANLIST },
    { "evt_unknown", UNKNOWN },
    { "evt_join", JOIN },
    { "evt_part", PART },
    { "evt_quit", QUIT },
    { "evt_nick", NICK },
    { "evt_mode", MODE },
    { "evt_cmode", CMODE },
    { "evt_wallops", WALLOPS },
    { "evt_kicked", KICKED },
    { "evt_opped", OPPED },
    { "evt_deopped", DEOPPED },
    { "evt_voiced", VOICED },
    { "evt_devoiced", DEVOICED },
    { "evt_banned", BANNED },
    { "evt_unbanned", UNBANNED },
    { 0, 0 }
};

Events::Events(LostIRCApp *app)
    : _app(app)
{

}

void Events::emit(Tmpl& t, const string& chan, ServerConnection *conn)
{
    string msg = t.result(); // FIXME: we shouldn't be doing this operation here
    std::replace(msg.begin(), msg.end(), '$', '\003');
    _app->evtDisplayMessage(msg, chan, conn);
}

void Events::emit(Tmpl& t, const vector<string>& to, ServerConnection *conn)
{
    vector<string>::const_iterator i;
    for (i = to.begin(); i != to.end(); ++i) {
        string to = *i;
        emit(t, to, conn);
    }
}

Tmpl Events::get(Event e)
{
    string msg;
    for (int i = 0; event_map[i].s != 0; ++i) {
        if (event_map[i].value == e) {
            msg = _app->getCfg().getParam(event_map[i].s);
        }
    }
    Tmpl t(msg);
    return t;
}

string Tmpl::result()
{
    string newstr;
    bool parsing_arg = false;
    string::const_iterator i;
    for (i = orig.begin(); i != orig.end(); ++i) {
        switch (*i) {
            case '%':
                parsing_arg = true;
                break;
            default:
                if (isdigit(*i) && parsing_arg) {
                    unsigned int num = ((*i) - '0') - 1;
                    if (num >= tokens.size()) {
                        cerr << "Fatal error, too many tokens! [ " << num << " compared to " << tokens.size() << " ]" << endl;
                    } else {
                        newstr += tokens[num];
                    }
                    parsing_arg = false;
                    break;
                }
                newstr += *i;
                parsing_arg = false;
        }
    }

    newstr += '\n';

    return newstr;
}

