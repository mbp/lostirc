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

#include <algorithm>
#include <functional>
#include "Channel.h"

using std::string;
using std::vector;

namespace algo
{
  struct isUser : public std::unary_function<User*, void>
  {
      isUser(const std::string& n) : nick(n) { }
      bool operator() (User* u) {
          if (u->nick == nick)
                return true;
          else
                return false;
      }
      std::string nick;
  };
}

IRC::UserMode User::getMode() const
{
    if (opped)
          return IRC::OP;
    else if (voiced)
          return IRC::VOICE;
    else
          return IRC::NONE;
}


void Channel::addUser(const string& n, IRC::UserMode i)
{
    User *u = new User();
    u->nick = n;
    if (i == IRC::OP)
          u->opped = true;
    else if (i == IRC::VOICE)
          u->voiced = true;

    _users.push_back(u);
}

void Channel::removeUser(const string& u)
{
    vector<User*>::iterator i = std::find_if(_users.begin(), _users.end(), algo::isUser(u));

    if (i != _users.end()) {
        delete (*i);
        _users.erase(i);
    }
}

bool Channel::findUser(const string& u)
{
    vector<User*>::iterator i = std::find_if(_users.begin(), _users.end(), algo::isUser(u));

    if (i != _users.end())
          return true;

    return false;
}

void Channel::renameUser(const string& from, const string& to)
{
    vector<User*>::iterator i = std::find_if(_users.begin(), _users.end(), algo::isUser(from));

    if (i != _users.end()) {
        (*i)->nick = to;
    }
}

User* Channel::getUser(const string& u)
{
    vector<User*>::iterator i = std::find_if(_users.begin(), _users.end(), algo::isUser(u));

    return *i;
}
