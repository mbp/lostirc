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
    const signed int priority;
    const char *name;
    const signed int value;
} event_map[] = {
    { 2, "evt_privmsg", PRIVMSG },
    { 2, "evt_privmsg_highlight", PRIVMSG_HIGHLIGHT },
    { 2, "evt_action", ACTION },
    { 2, "evt_action_highlight", ACTION_HIGHLIGHT },
    { 2, "evt_dcc_receive", DCC_RECEIVE },
    { 2, "evt_servmsg", SERVMSG },
    { 2, "evt_servmsg2", SERVMSG2 },
    { 2, "evt_servmsg3", SERVMSG3 },
    { 2, "evt_ctcp", CTCP },
    { 2, "evt_ctcp_multi", CTCP_MULTI },
    { 2, "evt_topicchange", TOPICCHANGE },
    { 2, "evt_topicis", TOPICIS },
    { 2, "evt_topictime", TOPICTIME },
    { 2, "evt_noticepriv", NOTICEPRIV },
    { 2, "evt_noticepubl", NOTICEPUBL },
    { 2, "evt_error", ERROR },
    { 2, "evt_away", AWAY },
    { 2, "evt_banlist", BANLIST },
    { 1, "evt_unknown", UNKNOWN },
    { 1, "evt_join", JOIN },
    { 1, "evt_part", PART },
    { 1, "evt_quit", QUIT },
    { 1, "evt_nick", NICK },
    { 1, "evt_mode", MODE },
    { 1, "evt_cmode", CMODE },
    { 2, "evt_wallops", WALLOPS },
    { 1, "evt_kicked", KICKED },
    { 1, "evt_opped", OPPED },
    { 1, "evt_deopped", DEOPPED },
    { 1, "evt_voiced", VOICED },
    { 1, "evt_devoiced", DEVOICED },
    { 1, "evt_halfopped", HALFOPPED },
    { 1, "evt_halfdeopped", HALFDEOPPED },
    { 1, "evt_banned", BANNED },
    { 1, "evt_unbanned", UNBANNED },
    { 2, "evt_invited", INVITED },
    { 2, "evt_connecting", CONNECTING },
    { 2, "evt_names", NAMES },
    { 2, "evt_killed", KILLED },
    { 0, 0, 0 }
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
