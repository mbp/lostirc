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

#include <algorithm>
#include <functional>
#include "Channel.h"

using Glib::ustring;
using std::vector;

namespace algo
{
  struct isUser : public std::unary_function<User*, void>
  {
      isUser(const Glib::ustring& n) : nick(n) { }
      bool operator() (User* u) {
          if (u->nick == nick)
                return true;
          else
                return false;
      }
      Glib::ustring nick;
  };
}

IRC::UserMode User::getMode() const
{
    if (state & IRC::OWNER)
          return IRC::OWNER;
    else if (state & IRC::ADMIN)
          return IRC::ADMIN;
    else if (state & IRC::OP)
          return IRC::OP;
    else if (state & IRC::HALFOP)
          return IRC::HALFOP;
    else if (state & IRC::VOICE)
          return IRC::VOICE;
    else
          return IRC::NONE;
}

void User::setMode(IRC::UserMode u)
{
    state |= u;
}

void User::removeMode(IRC::UserMode u)
{
    state &= ~u;
}

void Channel::addUser(const ustring& n, IRC::UserMode i)
{
    User *user = new User;
    user->nick = n;
    user->setMode(i);

    _users.push_back(user);
}

void Channel::removeUser(const ustring& u)
{
    vector<User*>::iterator i = std::find_if(_users.begin(), _users.end(), algo::isUser(u));

    if (i != _users.end()) {
        delete (*i);
        _users.erase(i);
    }
}

bool Channel::findUser(const ustring& u)
{
    vector<User*>::iterator i = std::find_if(_users.begin(), _users.end(), algo::isUser(u));

    if (i != _users.end())
          return true;

    return false;
}

void Channel::renameUser(const ustring& from, const ustring& to)
{
    vector<User*>::iterator i = std::find_if(_users.begin(), _users.end(), algo::isUser(from));

    if (i != _users.end()) {
        (*i)->nick = to;
    }
}

User* Channel::getUser(const ustring& u)
{
    vector<User*>::iterator i = std::find_if(_users.begin(), _users.end(), algo::isUser(u));

    return *i;
}
