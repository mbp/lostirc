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

#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include "irc_defines.h"

class Channel {
    std::string name;
    std::map<std::string, IRC::UserMode> users;
    //std::vector<std::string> users;

public:
    void setName(const std::string& n) { name = n; }
    std::string getName() { return name; }
    void addUser(const string& n, IRC::UserMode i = IRC::NONE);
    void removeUser(const std::string& u);
    bool findUser(const std::string& u);
    std::map<std::string, IRC::UserMode> getUsers() { return users; }
};

#endif
