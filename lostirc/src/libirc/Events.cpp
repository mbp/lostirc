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

#include <iostream>
#include <cctype>
#include "Events.h"

using std::string;

struct {
    const char *s;
    signed int value;
} event_map[] = {
    { "evt_privmsg", PRIVMSG },
    { "evt_privmsg_highlight", PRIVMSG_HIGHLIGHT },
    { "evt_action", ACTION },
    { "evt_action_highlight", ACTION_HIGHLIGHT },
    { "evt_dcc_receive", DCC_RECEIVE },
    { "evt_servmsg", SERVMSG },
    { "evt_servmsg2", SERVMSG2 },
    { "evt_servmsg3", SERVMSG3 },
    { "evt_ctcp", CTCP },
    { "evt_ctcp_multi", CTCP_MULTI },
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
    { "evt_invited", INVITED },
    { "evt_connecting", CONNECTING },
    { "evt_names", NAMES },
    { 0, 0 }
};

string Tmpl::result()
{
    string newstr;
    bool parsing_arg = false;
    string::const_iterator i;
    for (i = orig.begin(); i != orig.end(); ++i) {
        if (*i == '%') {
            parsing_arg = true;
        } else if (isdigit(*i) && parsing_arg) {
            unsigned int num = ((*i) - '0') - 1;
            if (num >= tokens.size()) {
                std::cerr << "Fatal error, too many tokens! (" << num << " compared to " << tokens.size() << ")" << std::endl;
            } else {
                newstr += tokens[num];
            }
            parsing_arg = false;
        } else {
            newstr += *i; 
            parsing_arg = false;
        }
    }

    newstr += '\n';

    return newstr;
}
