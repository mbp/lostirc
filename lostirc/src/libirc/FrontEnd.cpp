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

#include "FrontEnd.h"
#include "ServerConnection.h"
#include "Parser.h"

struct {
    const signed int priority;
    const char *name;
    const signed int value;
} event_map[] = {
    { 2, "privmsg", PRIVMSG },
    { 2, "privmsg_highlight", PRIVMSG_HIGHLIGHT },
    { 2, "privmsg_self", PRIVMSG_SELF },
    { 2, "action", ACTION },
    { 2, "action_highlight", ACTION_HIGHLIGHT },
    { 2, "dcc_receive", DCC_RECEIVE },
    { 2, "servermsg1", SERVERMSG1 },
    { 2, "servermsg2", SERVERMSG2 },
    { 2, "clientmsg", CLIENTMSG },
    { 2, "ctcp", CTCP },
    { 2, "ctcp_multi", CTCP_MULTI },
    { 2, "ctcp_reply", CTCP_REPLY },
    { 2, "topicchange", TOPICCHANGE },
    { 2, "topicis", TOPICIS },
    { 2, "topictime", TOPICTIME },
    { 2, "noticepriv", NOTICEPRIV },
    { 2, "noticepubl", NOTICEPUBL },
    { 2, "error", ERROR },
    { 2, "away", AWAY },
    { 2, "banlist", BANLIST },
    { 1, "unknown", UNKNOWN },
    { 1, "join", JOIN },
    { 1, "part", PART },
    { 1, "part2", PART2 },
    { 1, "quit", QUIT },
    { 1, "quit2", QUIT2 },
    { 1, "nick", NICK },
    { 1, "mode", MODE },
    { 1, "cmode", CMODE },
    { 2, "wallops", WALLOPS },
    { 1, "kicked", KICKED },
    { 1, "opped", OPPED },
    { 1, "deopped", DEOPPED },
    { 1, "voiced", VOICED },
    { 1, "devoiced", DEVOICED },
    { 1, "halfopped", HALFOPPED },
    { 1, "halfdeopped", HALFDEOPPED },
    { 1, "banned", BANNED },
    { 1, "unbanned", UNBANNED },
    { 2, "invited", INVITED },
    { 2, "connecting", CONNECTING },
    { 2, "names", NAMES },
    { 2, "killed", KILLED },
    { 2, "whois_user", WHOIS_USER },
    { 2, "whois_channels", WHOIS_CHANNELS },
    { 2, "whois_server", WHOIS_SERVER },
    { 2, "whois_generic", WHOIS_GENERIC },
    { 0, 0, 0 }
};

namespace FE {

void emit(Tmpl& t, ChannelBase& chan, ServerConnection *conn)
{
    Glib::ustring msg = t.result();

    // If the user has limited_highligthing enabled, only highlight if the
    // priority is above 1
    if (App->options.limited_highlighting && t.priority < 2) {
        App->fe->displayMessage(msg, chan, conn, false);
    } else {
        App->fe->displayMessage(msg, chan, conn);
    }

    if (App->options.logging)
          logToFile(msg, chan, conn);
}

void emit(Tmpl& t, const std::vector<ChannelBase*>& to, ServerConnection *conn)
{
    std::vector<ChannelBase*>::const_iterator i;
    for (i = to.begin(); i != to.end(); ++i) {
        emit(t, *(*i), conn);
    }
}

void emit(Tmpl& t, Destination d, ServerConnection *conn)
{
    Glib::ustring msg = t.result();
    
    // If the user has limited_highligthing enabled, only highlight if the
    // priority is above 1
    if (App->options.limited_highlighting && t.priority < 2) {
        App->fe->displayMessage(msg, d, conn, false);
    } else {
        App->fe->displayMessage(msg, d, conn);
    }
}

void emit(Tmpl& t, Destination d)
{
    Glib::ustring msg = t.result();
    
    // If the user has limited_highligthing enabled, only highlight if the
    // priority is above 1
    if (App->options.limited_highlighting && t.priority < 2) {
        App->fe->displayMessage(msg, d, false);
    } else {
        App->fe->displayMessage(msg, d);
    }
}

Tmpl get(Event e)
{
    for (int i = 0; event_map[i].name != 0; ++i) {
        if (event_map[i].value == e)
            return Tmpl(App->events.get(event_map[i].name), event_map[i].priority);
    }
}

Glib::ustring Tmpl::result()
{
    // Add timestamp
    time_t timeval = time(0);
    char tim[11];
    strftime(tim, 10, "%H:%M:%S ", localtime(&timeval));

    Glib::ustring generated_str(tim);

    bool parsing_arg = false;
    Glib::ustring::iterator i;
    for (i = orig.begin(); i != orig.end(); ++i) {
        if (*i == '%') {
            parsing_arg = true;
        } else if (isdigit(*i) && parsing_arg) {
            unsigned int num = ((*i) - '0') - 1;
            if (tokens.size() > num)
                  generated_str += tokens[num];
            parsing_arg = false;
        } else {
            generated_str += *i; 
            parsing_arg = false;
        }
    }

    generated_str += '\n';

    return generated_str;
}

}

void logToFile(const Glib::ustring& msg, ChannelBase& chan, ServerConnection *conn)
{
    // log message to a channel-specific file.

    mkdir(App->logdir.c_str(), 0700);

    Glib::ustring filename = App->logdir + chan.getName() + "_" + conn->Session.servername + ".log";

    Glib::ustring stripped_msg = stripColors(msg, true);
    
    std::ofstream logfile(filename.c_str(), std::ios::app);
    if (logfile.good())
          logfile << stripped_msg;
}

