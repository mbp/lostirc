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

#include "Channel.h"
#include <algorithm>

using std::string;
using std::map;
using std::pair;

//typedef map<int, pair<Mode, string> >::iterator usersIter;

int Channel::addUser(const string& u)
{
    //usersIter i = users.begin();
    users.push_back(u);
    return 1;
}

void Channel::removeUser(const string& u)
{
    vector<string>::iterator i = std::find(users.begin(), users.end(), u);

    if (i != users.end())
          users.erase(i);
}

bool Channel::findUser(const string& n)
{
    vector<string>::iterator i = std::find(users.begin(), users.end(), n);

    if (i != users.end())
          return true;

    return false;
}
