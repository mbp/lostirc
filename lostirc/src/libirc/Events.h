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

#ifndef EVENTS_H
#define EVENTS_H

#include <map>
#include <string>
#include <vector>
#include "InOut.h"

class Events
{
public:
    Events(InOut *inout);

    void emitEvent(const string& name, vector<string>& args, const string& chan, ServerConnection *conn);
    void emitEvent(const string& name, const string& arg, const string& chan, ServerConnection *conn);
private:
    std::map<std::string, std::string> _events;

    InOut *_io;
};
#endif