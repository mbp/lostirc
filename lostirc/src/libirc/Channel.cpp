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

#include "Channel.h"
#include <algorithm>

using std::string;
using std::map;
using std::pair;
using std::vector;

typedef std::map<std::string, IRC::UserMode> uMap;

void Channel::addUser(const string& n, IRC::UserMode i = IRC::NONE)
{
    users.insert(make_pair(n, i));
}

void Channel::removeUser(const string& u)
{
    uMap::iterator i = users.find(u);

    if (i != users.end())
          users.erase(i);
}

bool Channel::findUser(const string& u)
{
    uMap::const_iterator i = users.find(u);

    if (i != users.end())
          return true;

    return false;
}
