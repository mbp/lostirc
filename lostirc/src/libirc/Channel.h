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

#include <glibmm/ustring.h>
#include <vector>
#include "irc_defines.h"

class User {
    unsigned int state;

public:
    User() : state(0) { }
    Glib::ustring nick;
    IRC::UserMode getMode() const;
    void setMode(IRC::UserMode u);
    void removeMode(IRC::UserMode u);
};

class ChannelBase {
public:
    virtual ~ChannelBase() { }
    virtual Glib::ustring getName() = 0;
    virtual bool findUser(const Glib::ustring& n) = 0;
    virtual void renameUser(const Glib::ustring& from, const Glib::ustring& to) = 0;

};

class Channel : public ChannelBase {
    Glib::ustring _name;
    std::vector<User*> _users;

public:
    Channel(const Glib::ustring& n) : _name(n), endOfNames(false) { }
    Glib::ustring getName() { return _name; }
    bool findUser(const Glib::ustring& u);
    void renameUser(const Glib::ustring& from, const Glib::ustring& to);

    void addUser(const Glib::ustring& n, IRC::UserMode i = IRC::NONE);
    void removeUser(const Glib::ustring& u);
    const std::vector<User*>& getUsers() { return _users; }

    User* getUser(const Glib::ustring& n);

    bool endOfNames; // have we reached our 'ENDOFNAMES' on channel join?
};

class Query : public ChannelBase {
    Glib::ustring _name;

public:
    Query(const Glib::ustring& n) : _name(n) { }

    Glib::ustring getName() { return _name; }
    bool findUser(const Glib::ustring& n) { return (n == _name); }
    void renameUser(const Glib::ustring& from, const Glib::ustring& to) {
        if (_name == from) _name = to;
    }

};

#endif
