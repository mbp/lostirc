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

// defined in Events.cpp
extern struct {
    const signed int priority;
    const char *name;
    const signed int value;
} event_map[];

namespace FE {

void emit(Tmpl& t, ChannelBase& chan, ServerConnection *conn)
{
    std::string msg = t.result();

    // If the user has limited_highligthing enabled, only highlight if the
    // priority is above 1
    if (App->options.limited_highlighting && t.priority < 2) {
        App->fe->displayMessage(msg, chan, conn, false);
    } else {
        App->fe->displayMessage(msg, chan, conn);
    }
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
    std::string msg = t.result();
    
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
    std::string msg = t.result();
    
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

}
