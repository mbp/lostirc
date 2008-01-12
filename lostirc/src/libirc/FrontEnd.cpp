/*
 * Copyright (C) 2002-2004 Morten Brix Pedersen <morten@wtf.dk>
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

static struct {
    const char *name;
    const signed int value;
} event_map[] = {
    { "privmsg", PRIVMSG },
    { "privmsg_highlight", PRIVMSG_HIGHLIGHT },
    { "privmsg_self", PRIVMSG_SELF },
    { "action", ACTION },
    { "action_highlight", ACTION_HIGHLIGHT },
    { "dcc_receive", DCC_RECEIVE },
    { "servermsg1", SERVERMSG1 },
    { "servermsg2", SERVERMSG2 },
    { "clientmsg", CLIENTMSG },
    { "ctcp", CTCP },
    { "ctcp_multi", CTCP_MULTI },
    { "ctcp_reply", CTCP_REPLY },
    { "topicchange", TOPICCHANGE },
    { "topicis", TOPICIS },
    { "topictime", TOPICTIME },
    { "noticepriv", NOTICEPRIV },
    { "noticepubl", NOTICEPUBL },
    { "error", ERRORMSG },
    { "away", AWAY },
    { "banlist", BANLIST },
    { "unknown", UNKNOWN },
    { "join", JOIN },
    { "part", PART },
    { "part2", PART2 },
    { "quit", QUIT },
    { "quit2", QUIT2 },
    { "nick", NICK },
    { "mode", MODE },
    { "cmode", CMODE },
    { "wallops", WALLOPS },
    { "kicked", KICKED },
    { "ownered", OWNERED },
    { "deownered", DEOWNERED },
    { "admined", ADMINED },
    { "deadmined", DEADMINED },
    { "opped", OPPED },
    { "deopped", DEOPPED },
    { "voiced", VOICED },
    { "devoiced", DEVOICED },
    { "halfopped", HALFOPPED },
    { "halfdeopped", HALFDEOPPED },
    { "banned", BANNED },
    { "unbanned", UNBANNED },
    { "invited", INVITED },
    { "connecting", CONNECTING },
    { "names", NAMES },
    { "killed", KILLED },
    { "whois_user", WHOIS_USER },
    { "whois_channels", WHOIS_CHANNELS },
    { "whois_server", WHOIS_SERVER },
    { "whois_generic", WHOIS_GENERIC },
    { 0, 0 }
};

namespace FE {

void emit(Tmpl& t, ChannelBase& chan, ServerConnection *conn)
{
    Glib::ustring msg = t.result();

    App->fe->displayMessage(msg, chan, conn);

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
    
    App->fe->displayMessage(msg, d, conn);
}

Tmpl get(Event e)
{
    for (int i = 0; event_map[i].name != 0; ++i) {
        if (event_map[i].value == e)
            return Tmpl(App->events.get(event_map[i].name));
    }
    return Tmpl("");
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

    #ifndef WIN32
    mkdir(App->logdir.c_str(), 0700);
    #else
    mkdir(App->logdir.c_str());
    #endif

    Glib::ustring filename = App->logdir + chan.getName() + "_" + conn->Session.servername + ".log";

    Glib::ustring stripped_msg = stripColors(msg, true);
    
    std::ofstream logfile(filename.c_str(), std::ios::app);
    if (logfile.good())
          logfile << stripped_msg;
}
