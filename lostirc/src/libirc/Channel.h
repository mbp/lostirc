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
#include <vector>
#include "irc_defines.h"

class User {
    unsigned int state;

public:
    User() { }
    std::string nick;
    IRC::UserMode getMode() const;
    void setMode(IRC::UserMode u);
    void removeMode(IRC::UserMode u);
};

class ChannelBase {
public:
    virtual ~ChannelBase() { }
    virtual std::string getName() = 0;
    virtual bool findUser(const std::string& n) = 0;
    virtual void renameUser(const std::string& from, const std::string& to) = 0;

};

class Channel : public ChannelBase {
    std::string _name;
    std::vector<User*> _users;

public:
    Channel(const std::string& n) : _name(n), endOfNames(false) { }
    std::string getName() { return _name; }
    bool findUser(const std::string& u);
    void renameUser(const std::string& from, const std::string& to);

    void addUser(const std::string& n, IRC::UserMode i = IRC::NONE);
    void removeUser(const std::string& u);
    const std::vector<User*>& getUsers() { return _users; }

    User* getUser(const std::string& n);

    bool endOfNames; // have we reached our 'ENDOFNAMES' on channel join?
};

class Query : public ChannelBase {
    std::string _name;

public:
    Query(const std::string& n) : _name(n) { }

    std::string getName() { return _name; }
    bool findUser(const std::string& n) { return (n == _name); }
    void renameUser(const std::string& from, const std::string& to) {
        if (_name == from) _name = to;
    }

};

#endif
