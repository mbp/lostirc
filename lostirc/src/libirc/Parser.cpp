/*
 * Copyright (C) 2002, 2003 Morten Brix Pedersen <morten@wtf.dk>
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

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "Parser.h"
#include "Utils.h"
#include "Channel.h"
#include "FrontEnd.h"
#include "LostIRCApp.h"
#include "ServerConnection.h"
#include "DCC.h"

using std::vector;
using Glib::ustring;

/* different functions used in for_each */
namespace algo
{

  struct removeUser : public std::unary_function<ChannelBase*, void>
  {
      removeUser(const Glib::ustring& n) : nick(n) { }
      void operator() (ChannelBase* x) {
          Channel *c = dynamic_cast<Channel*>(x);
          if (c)
                c->removeUser(nick);
      }
      Glib::ustring nick;
  };

  struct renameUser : public std::unary_function<ChannelBase*, void>
  {
      renameUser(const Glib::ustring& f, const Glib::ustring& t) : from(f), to(t) { }
      void operator() (ChannelBase* x) {
          x->renameUser(from, to);
      }
      Glib::ustring from;
      Glib::ustring to;
  };

}

Parser::Parser(ServerConnection *conn)
    : _conn(conn)
{
}

void Parser::parseLine(ustring& data)
{
    #ifdef DEBUG
    App->log << "<< " << data << std::endl;
    #endif
    if (App->options.strip_colors)
          data = stripColors(data, App->options.strip_boldandunderline);

    if (data[0] == ':') {
        /*
        *  Message in the form:
        *  message  =  [ ":" prefix SPACE ] command [ params ] crlf
        *  prefix    =  servername / ( nickname [ [ "!" user ] "@" host ] )
        *  command   =  1*letter / 3digit
        *  params    =  *14( SPACE middle ) [ SPACE ":" trailing ]
        *            =/ 14( SPACE middle ) [ SPACE [ ":" ] trailing ]
        */

        // Find prefix
        ustring::size_type pos1 = data.find_first_of(" ", 1);
        ustring from = data.substr(1, pos1 - 1);

        // Find command
        ustring::size_type pos2 = data.find_first_of(" ", pos1 + 1);
        ustring command = data.substr(pos1 + 1, (pos2 - 1) - pos1);

        // Check whether there is any params
        ustring::size_type pos3 = data.find_first_of(":", pos2 + 1);
        ustring param;

        if ((pos3 - 1) != pos2) {
            // We have params
            param = data.substr(pos2 + 1, (pos3 - 2) - pos2);
        }

        // Get rest (whats after the ':', if there were any ':')
        ustring rest;
        if (pos3 != ustring::npos) {
            rest = data.substr(pos3 + 1);
        }

        #ifdef DEBUG
        App->log << "\t[from '" << from << "']";
        App->log << " [command '" << command << "']";
        App->log << " [param '" << param << "']"; 
        App->log << " [rest '" << rest << "']" << std::endl;
        #endif

        // Redirect to the right parsing function...
        
        int n = Util::stoi(command);
        if (n)
              numeric(n, from, param, rest);
        else if (command == "PRIVMSG")
              Privmsg(from, param, rest);
        else if (command == "JOIN")
              Join(from, param, rest);
        else if (command == "PART")
              Part(from, param, rest);
        else if (command == "QUIT")
              Quit(from, rest);
        else if (command == "PONG")
              // we received our lag check..
              _conn->Session.sentLagCheck = false;
        else if (command == "MODE")
              Mode(from, param, rest);
        else if (command == "TOPIC")
              Topic(from, param, rest);
        else if (command == "NOTICE")
              Notice(from, param, rest);
        else if (command == "KICK")
              Kick(from, param, rest);
        else if (command == "NICK")
              Nick(from, rest);
        else if (command == "WALLOPS")
              Wallops(from, rest);
        else if (command == "INVITE")
              Invite(from, rest);
        else if (command == "KILL")
              Kill(from, rest);
        else
              FE::emit(FE::get(UNKNOWN) << data, FE::CURRENT, _conn);

    } else {
        // Parse ustring in form, eg. 'PING :23523525'
        ustring::size_type pos1 = data.find_first_of(" ", 0);
        ustring command = data.substr(0, pos1);

        // Check whether there are any params
        ustring::size_type pos2 = data.find_first_of(":", pos1 + 1);
        ustring param;

        if ((pos2 - 1) != pos1) {
            // We have params
            param = data.substr(pos1 + 1, (pos2 - 2) - pos1);
        }

        // Get rest (whats after the ':')
        ustring rest = data.substr(pos2 + 1);

        #ifdef DEBUG
        App->log << "\t[command '" << command << "']";
        App->log << " [param '" << param << "']";
        App->log << " [rest '" << rest << "']" << std::endl;
        #endif

        // Redirect to the right parsing function... 
        if (command == "PING")
              Ping(rest);
        else if (command == "NOTICE")
              Notice(param + " :" + rest);
        else if (command == "ERRORMSG")
              FE::emit(FE::get(ERRORMSG) << param + " " + rest, FE::CURRENT, _conn);
        else
              FE::emit(FE::get(SERVERMSG1) << data, FE::CURRENT, _conn);
    }

}

inline
void Parser::Ping(const ustring& rest)
{
    _conn->sendPong(rest);
}

void Parser::Privmsg(const ustring& from, const ustring& param, const ustring& rest)
{
    if (rest[0] == '\001') {
        // We got CTCP
        Ctcp(from, param, rest);
    } else {
        // Normal privmsg 

        ChannelBase *chan;

        if (param == _conn->Session.nick) {
            // The message was intended for *us*, so its a query.
            chan = _conn->findQuery(findNick(from));
            if (!chan) {
                chan = new Query(findNick(from));
                _conn->Session.channels.push_back(chan);
            }

        } else {
            chan = _conn->findChannel(param);

            // Even though this should never happen, it happens with some
            // bouncers.
            if (!chan)
                  return;
        }

        if (shouldHighlight(rest)) {
            FE::emit(FE::get(PRIVMSG_HIGHLIGHT) << findNick(from) << rest, *chan, _conn);
            App->fe->highlight(*chan, _conn);
        } else {
            FE::emit(FE::get(PRIVMSG) << findNick(from) << rest, *chan, _conn);
        }
    }
}

void Parser::Ctcp(const ustring& from, const ustring& param, const ustring& rest)
{
    ustring::size_type pos = rest.find_first_of(" \001", 1);
    ustring command = rest.substr(1, pos - 1);

    if (command == "VERSION") {
        _conn->sendVersion(findNick(from));

    } else if (command == "ACTION") {
        ustring rest_ = rest.substr(pos + 1, (rest.length() - pos) - 2);

        ChannelBase *chan;

        if (param == _conn->Session.nick) {
            // The message was intended for *us*, so its a query.
            chan = _conn->findQuery(findNick(from));
            if (!chan) {
                chan = new Query(findNick(from));
                _conn->Session.channels.push_back(chan);
            }

        } else {
            chan = _conn->findChannel(param);

            // Even though this should never happen, it happens with some
            // bouncers.
            if (!chan)
                  return;
        }

        if (shouldHighlight(rest)) {
            FE::emit(FE::get(ACTION_HIGHLIGHT) << findNick(from) << rest_, *chan, _conn);
            App->fe->highlight(*chan, _conn);
        } else {
            FE::emit(FE::get(ACTION) << findNick(from) << rest_, *chan, _conn);
        }
        return;

    } else if (command == "PING") {
        // Reply to the client with the same timestamp they sent us.
        ustring rest_ = rest.substr(pos + 1, (rest.length() - pos) - 2);
        _conn->sendCtcpNotice(findNick(from), "PING " + rest_);

    } else if (command == "DCC") {
        ustring dcc_type = getWord(rest, 2);
        ustring dcc_address = getWord(rest, 4);
        ustring dcc_port = getWord(rest, 5);

        if (dcc_type == "SEND") {
            ustring dcc_size = getWord(rest, 6);
            ustring dcc_filename = getWord(rest, 3);

            std::istringstream ss(dcc_address);
            unsigned long address;
            ss >> address;

            std::istringstream ss2(dcc_port);
            unsigned short port;
            ss2 >> port;

            std::istringstream ss3(dcc_size);
            unsigned long size;
            ss3 >> size;

            int n = App->getDcc().addDccSendIn(dcc_filename, findNick(from), address, port, size);
            if (n)
                  FE::emit(FE::get(DCC_RECEIVE) << findNick(from) << dcc_filename << n, FE::CURRENT);

        } else if (dcc_type == "CHAT") {
            FE::emit(FE::get(SERVERMSG1) << findNick(from) << rest, FE::CURRENT);
            std::istringstream ss(dcc_address);
            unsigned long address;
            ss >> address;

            std::istringstream ss2(dcc_port);
            unsigned short port;
            ss2 >> port;

            //DCC d(findNick(from), address, port);
        }
        return;

    }

    if (param == _conn->Session.nick)
          FE::emit(FE::get(CTCP) << command << findNick(from), FE::CURRENT, _conn);
    else
          FE::emit(FE::get(CTCP_MULTI) << command << findNick(from) << param, FE::CURRENT, _conn);

}

void Parser::Notice(const ustring& from, const ustring& to, const ustring& rest)
{
    if (rest[0] == '\001') {
        // CTCP notice
        std::string tmp = rest;
        std::string::iterator i = remove(tmp.begin(), tmp.end(), '\001');
        ustring output(tmp.begin(), i);

        FE::emit(FE::get(CTCP_REPLY) << getWord(output, 1) << findNick(from) << skipFirstWord(output), FE::CURRENT, _conn);

    } else {
        // Normal notice
        if (to == _conn->Session.nick) {
            FE::emit(FE::get(NOTICEPRIV) << findNick(from) << rest, FE::CURRENT, _conn);
        } else {
            Channel *c = _conn->findChannel(to);
            if (c)
                  FE::emit(FE::get(NOTICEPUBL) << findNick(from) << to << rest, *c, _conn);
            else
                  FE::emit(FE::get(NOTICEPUBL) << findNick(from) << to << rest, FE::CURRENT, _conn);

        }
    }
}

void Parser::Notice(const ustring& msg)
{
    ustring::size_type pos = msg.find_first_of(" ");
    ustring from = msg.substr(0, pos);
    ustring rest = msg.substr(pos + 1);

    FE::emit(FE::get(NOTICEPRIV) << from << rest, FE::CURRENT, _conn);
}

void Parser::Kick(const ustring& from, const ustring& param, const ustring& msg)
{
    ustring chan, nick;
    std::istringstream ss(param);
    ss >> chan;
    ss >> nick;

    Channel *c = _conn->findChannel(chan);
    assert(c);
    c->removeUser(findNick(nick));
    FE::emit(FE::get(KICKED) << nick << chan << findNick(from) << msg, *c, _conn);
    App->fe->kick(findNick(from), *c, nick, msg, _conn);

    if (nick == _conn->Session.nick) // We got kicked
          _conn->removeChannel(chan);

}

void Parser::Join(const ustring& nick, const ustring& param, const ustring& rest)
{
    ustring chan = param;

    // Some clients/servers/bouncers might accidently send the channel name
    // in the 'rest' string, a bug there, but we would like to avoid a
    // segfault here. I noticed the same hack in the xchat sources.
    if (chan.empty() && !rest.empty())
        chan = getWord(rest, 1);

    Channel *c;
    if (findNick(nick) == _conn->Session.nick) {
        c = new Channel(chan);
        _conn->Session.channels.push_back(c);
    } else { 
        c = _conn->findChannel(chan);
        assert(c);
        c->addUser(findNick(nick));
    }

    App->fe->join(findNick(nick), *c, _conn);

    FE::emit(FE::get(JOIN) << findNick(nick) << chan << findHost(nick), *c, _conn);
}

void Parser::Part(const ustring& nick, const ustring& param, ustring& rest)
{
    ustring chan = param;

    // Some clients/servers/bouncers might accidently send the channel name
    // in the 'rest' string, a bug there, but we would like to avoid a
    // segfault here. I noticed the same hack in the xchat sources.
    if (chan.empty() && !rest.empty()) {
        chan = getWord(rest, 1);
        rest.erase();
    }

    Channel *c = _conn->findChannel(chan);

    if (c) {
        c->removeUser(findNick(nick));

        if (rest.empty())
              FE::emit(FE::get(PART2) << findNick(nick) << chan << findHost(nick) << rest, *c, _conn);
        else
              FE::emit(FE::get(PART) << findNick(nick) << chan << findHost(nick) << rest, *c, _conn);


        App->fe->part(findNick(nick), *c, _conn);

        if (findNick(nick) == _conn->Session.nick)
              _conn->removeChannel(chan);

    }
}

void Parser::Quit(const ustring& nick, const ustring& msg)
{
    vector<ChannelBase*> chans = _conn->findUser(findNick(nick));

    for_each(chans.begin(), chans.end(), algo::removeUser(nick));

    App->fe->quit(findNick(nick), chans, _conn);

    if (msg.empty())
          FE::emit(FE::get(QUIT2) << findNick(nick), chans, _conn);
    else
          FE::emit(FE::get(QUIT) << findNick(nick) << msg, chans, _conn);
}

void Parser::Nick(const ustring& from, const ustring& to)
{
    // When we receive an error that "nick change was too fast", 'to' will
    // be empty. just return if it is.

    if (to.empty())
          return;

    // Check whethers it's us who has changed nick
    if (findNick(from) == _conn->Session.nick) {
        _conn->Session.nick = to;
    }

    vector<ChannelBase*> chans = _conn->findUser(findNick(from));

    App->fe->nick(findNick(from), to, chans, _conn);

    for_each(chans.begin(), chans.end(), algo::renameUser(findNick(from), to));

    FE::emit(FE::get(NICK) << findNick(from) << to, chans, _conn);

}

void Parser::Invite(const ustring& from, const ustring& rest)
{
    FE::emit(FE::get(INVITED) << findNick(from) << rest, FE::CURRENT, _conn);
}

void Parser::Kill(const ustring& from, const ustring& rest)
{
    FE::emit(FE::get(KILLED) << findNick(from) << rest, FE::CURRENT, _conn);
}

void Parser::Mode(const ustring& from, const ustring& param, const ustring& rest)
{
    if (rest.empty()) {
        // We encountered a channel mode message
        CMode(from, param);
    } else {
        // User mode message
        // We got line in the form: 'user +x'
        FE::emit(FE::get(MODE) << findNick(from) << param << rest, FE::CURRENT, _conn);
    }
}

void Parser::CMode(const ustring& from, const ustring& param)
{
    // Parse line in the form: '#chan +ovo nick nick2 nick3'
    ustring::size_type pos1 = param.find_first_of(" ");
    ustring::size_type pos2 = param.find_first_of(" ", pos1 + 1);

    // No real modes? No reason to continue.
    if (pos1 == ustring::npos)
          return;

    ustring channel = param.substr(0, pos1);
    ustring modes = param.substr(pos1 + 1, (pos2 - pos1) - 1);
    ustring args = param.substr(pos2 + 1);

    Channel *chan = _conn->findChannel(channel);

    // Channel not found? Not sane to continue. 
    // This happened on a proxy/bouncer where no channel was mentioned in
    // MODE, just this:
    //   :nick!ident@host.com MODE nick +iw
    // Note the lack of ':' before +iw.
    if (!chan)
          return;

    // Get arguments
    vector<ustring> arguments;
    Util::tokenizeWords(args, arguments);

    if (arguments.empty()) {
        // There were no further arguments, eg. MODE #channel +n
        FE::emit(FE::get(CMODE) << findNick(from) << modes << channel, *chan, _conn);
        return;
    }

    std::vector<User> modesvec;
    bool sign = false; // Used to track whether we get a + or a -

    vector<ustring>::const_iterator arg_i = arguments.begin();

    for (ustring::iterator i = modes.begin(); i != modes.end(); ++i) {
        switch (*i)
        {
            case '+':
                sign = true;
                break;
            case '-':
                sign = false;
                break;
            case 'o':
                {
                Event e;
                IRC::UserMode mode = IRC::OP;
                sign ? (e = OPPED) : (e = DEOPPED);
                ustring nick = *arg_i++;

                User *user = chan->getUser(nick);
                sign ? (user->setMode(mode)) : (user->removeMode(mode));

                modesvec.push_back(*user);
                FE::emit(FE::get(e) << findNick(from) << nick, *chan, _conn);
                }
                break;
            case 'v':
                {
                Event e;
                IRC::UserMode mode = IRC::VOICE;
                sign ? (e = VOICED) : (e = DEVOICED);
                ustring nick = *arg_i++;

                User *user = chan->getUser(nick);

                sign ? (user->setMode(mode)) : (user->removeMode(mode));

                modesvec.push_back(*user);
                FE::emit(FE::get(e) << findNick(from) << nick, *chan, _conn);
                }
                break;
            case 'h':
                {
                Event e;
                IRC::UserMode mode = IRC::HALFOP;
                sign ? (e = HALFOPPED) : (e = HALFDEOPPED);
                ustring nick = *arg_i++;

                User *user = chan->getUser(nick);
                sign ? (user->setMode(mode)) : (user->removeMode(mode));

                modesvec.push_back(*user);
                FE::emit(FE::get(e) << findNick(from) << nick, *chan, _conn);
                }
                break;
            case 'b':
                {
                Event e;
                sign ? (e = BANNED) : (e = UNBANNED);

                ustring nick = *arg_i++;
                FE::emit(FE::get(e) << findNick(from) << nick, *chan, _conn);
                break;
                }
            default:
                {
                // none of the above modes, just display a default
                // "foo set mode +X user" message. or alternatively
                // MODE #chan +l 500
                char modebuf[3];
                if (sign)
                      modebuf[0] = '+';
                else
                      modebuf[0] = '-';

                modebuf[1] = (*i);
                modebuf[2] = '\0';

                ustring nick = *arg_i++;
                FE::emit(FE::get(MODE) << findNick(from) << modebuf << nick, *chan, _conn);
                }
        }

    }

    // Channel user mode
    App->fe->CUMode(findNick(from), *chan, modesvec, _conn);
}

void Parser::Topic(const ustring& from, const ustring& chan, const ustring& rest)
{
    Channel *c = _conn->findChannel(chan);
    if (c)
          FE::emit(FE::get(TOPICCHANGE) << findNick(from) << rest, *c, _conn);
    else
          FE::emit(FE::get(TOPICCHANGE) << findNick(from) << rest, FE::CURRENT, _conn);

}

void Parser::Topic(const ustring& param, const ustring& rest)
{
    ustring::size_type pos1 = param.find_first_of("#&+!");
    ustring::size_type pos2 = param.find_first_of(" ", pos1);

    ustring chan = param.substr(pos1, pos2 - pos1);
    Channel *c = _conn->findChannel(chan);

    if (c)
          FE::emit(FE::get(TOPICIS) << chan << rest, *c, _conn);
    else
          FE::emit(FE::get(TOPICIS) << chan << rest, FE::CURRENT, _conn);
}

void Parser::TopicTime(const ustring& param)
{
    // Find channel
    ustring::size_type pos1 = param.find_first_of("#&+!");
    ustring::size_type pos2 = param.find_first_of(" ", pos1);
    ustring chan = param.substr(pos1, pos2 - pos1);

    // Find user who set the topic
    ustring::size_type pos3 = param.find_first_of(" ", pos2 + 1);
    ustring nick = param.substr(pos2 + 1, (pos3 - pos2) - 1);
    ustring time = param.substr(pos3 + 1);

    long date = std::atol(time.c_str());
    time = std::ctime(&date);

    Channel *c = _conn->findChannel(chan);

    if (c)
          FE::emit(FE::get(TOPICTIME) << nick << time.substr(0, time.size() - 1), *c, _conn);
    else
          FE::emit(FE::get(TOPICTIME) << nick << time.substr(0, time.size() - 1), FE::CURRENT, _conn);
}

void Parser::Away(const ustring& param, const ustring& rest)
{
    ustring param1, param2;
    std::istringstream ss(param);
    ss >> param1;
    ss >> param2;

    FE::emit(FE::get(AWAY) << param2 << rest, FE::CURRENT, _conn);
}

void Parser::Wallops(const ustring& from, const ustring& rest)
{
    FE::emit(FE::get(WALLOPS) << findNick(from) << rest, FE::CURRENT, _conn);
}

void Parser::Banlist(const ustring& param)
{
    ustring dummy, chan, banmask, owner, time;
    std::istringstream ss(param);
    ss >> dummy;
    ss >> chan;
    ss >> banmask;
    ss >> owner;
    ss >> time;

    long date = std::atol(time.c_str());
    time = std::ctime(&date);

    Channel *c = _conn->findChannel(chan);
    assert(c);

    FE::emit(FE::get(BANLIST) << banmask << owner, *c, _conn);
}
     
void Parser::numeric(int n, const ustring& from, const ustring& param, const ustring& rest)
{
    switch(n)
    {
        case 1:   // RPL_WELCOME
            _conn->Session.servername = from;
            _conn->Session.hasRegistered = true;
            _conn->Session.nick = param;
            _conn->addConnectionTimerCheck();
            App->fe->connected(_conn);
        case 2:   // RPL_YOURHOST
        case 3:   // RPL_CREATED
        case 4:   // RPL_MYINFO
        case 5:   // RPL_MYINFO
        case 251: // RPL_LUSERCLIENT
        case 252: // RPL_LUSEROP
        case 253: // RPL_LUSERUNKNOWN
        case 254: // RPL_LUSERCHANNELS
        case 255: // RPL_LUSERME
            FE::emit(FE::get(SERVERMSG1) << rest, FE::CURRENT, _conn);
            break;

        case 301: // RPL_AWAY
            Away(param, rest);
            break;

        case 305: // RPL_UNAWAY
            _conn->Session.isAway = false;
            App->fe->away(false, _conn);
            FE::emit(FE::get(SERVERMSG1) << rest, FE::CURRENT, _conn);
            break;

        case 306: // RPL_NOWAWAY
            _conn->Session.isAway = true;
            App->fe->away(true, _conn);
            FE::emit(FE::get(SERVERMSG1) << rest, FE::CURRENT, _conn);
            break;

        case 332: // RPL_TOPIC
            Topic(param, rest);
            break;

        case 333:
            TopicTime(param);
            break;

        case 367: // RPL_BANLIST
            Banlist(param);
            break;

        case 368: // RPL_END_OF_BANLIST
        case 372: // RPL_MOTD
        case 375: // RPL_MOTDSTART
            FE::emit(FE::get(SERVERMSG1) << rest, FE::CURRENT, _conn);
            break;

        case 376: // RPL_ENDOFMOTD
            FE::emit(FE::get(SERVERMSG1) << rest, FE::CURRENT, _conn);
            _conn->Session.endOfMotd = true;
            _conn->sendCmds();
            break;

        case 401: // ERR_NOSUCNICK
        case 403: // ERR_NOSUCHCHANNEL
        case 404: // ERR_CANNOTSENDTOCHAN
        case 405: // ERR_TOOMANYCHANNELS
            FE::emit(FE::get(ERRORMSG) << skipFirstWord(param) + ": " + rest, FE::CURRENT, _conn);
            break;

        case 412: // ERR_NOTEXTTOSEND
            FE::emit(FE::get(SERVERMSG1) << rest, FE::CURRENT, _conn);
            break;

        case 422: // ERR_NOMOTD
            _conn->sendCmds();
            break;

        case 433: // ERR_NICKNAMEINUSE
            // Apply a _ to the nickname if we havn't received MOTD
            // XXX: also send msg to frontend?
            if (!_conn->Session.endOfMotd)
                _conn->sendNick(_conn->Session.nick += "_");
            else
                FE::emit(FE::get(ERRORMSG) << rest, FE::CURRENT, _conn);
            break;

        case 438: // Nick change to fast
        case 442: // ERR_NOTONCHANNEL
        case 443: // ERR_USERONCHANNEL
        case 451: // ERR_NOTREGISTERED
        case 461: // ERR_NEEDMOREPARAMS
        case 462: // ERR_ALLREADYREGISTERED
        case 464: // ERR_PASSWDMISMATCH
        case 465: // ERR_YOUREBANNEDCREEP
        case 467: // ERR_KEYSET
        case 471: // ERR_CHANNELISFULL
        case 472: // ERR_UNKNOWMODE
        case 473: // ERR_INVITEONLYCHAN
        case 474: // ERR_BANNEDFROMCHAN
        case 475: // ERR_BADCHANNELKEY
        case 481: // ERR_NOPRIVILEGES
        case 482: // ERR_CHANOPRIVSNEEDED
        case 491: // ERR_NOOPERHOST
        case 501: // ERR_UMODEUNKNOWNFLAG
        case 502: // ERR_USERSDONTMATCH
            FE::emit(FE::get(ERRORMSG) << skipFirstWord(param) + " " + rest, FE::CURRENT, _conn);
            break;

        case 353: // RPL_NAMREPLY
            Names(param, rest);
            break;

        case 366: // RPL_ENDOFNAMES
            {
                Channel *c = _conn->findChannel(getWord(param, 2));
                if (c && !c->endOfNames) {
                    c->endOfNames = true;
                    App->fe->names(*c, _conn);
                } else {
                    FE::emit(FE::get(SERVERMSG2) << getWord(param, 2) << rest, FE::CURRENT, _conn);
                }
            }
            break;
        case 317: // RPL_WHOISIDLE
            {
                long idle = Util::stoi(getWord(param, 3));
                std::ostringstream ss;
                ss << _("idle: ");
                ss << idle / 3600 << ":" << (idle / 60) % 60 << ":" << idle % 60;
                long date = std::atol(getWord(param, 4).c_str());
                ustring time = std::ctime(&date);
                ss << _(", signon time: ") << time.substr(0, time.size() - 1);
                FE::emit(FE::get(WHOIS_GENERIC) << getWord(param, 2) << ss.str(), FE::CURRENT, _conn);
            }
            break;
        case 314: // RPL_WHOWASUSER
        case 311: // RPL_WHOISUSER
            FE::emit(FE::get(WHOIS_USER) << getWord(param, 2) << getWord(param, 3) << getWord(param, 4) << rest, FE::CURRENT, _conn);
            break;
        case 312: // RPL_WHOISSERVER
            FE::emit(FE::get(WHOIS_SERVER) << getWord(param, 2) << getWord(param, 3) << rest, FE::CURRENT, _conn);
            break;
        case 313: // RPL_WHOISOPERATOR
            // We need this find_first_of to omit the first word
            FE::emit(FE::get(WHOIS_GENERIC) << getWord(param, 2) << rest, FE::CURRENT, _conn);
            break;
        case 318: // RPL_ENDOFWHOIS
            break;
        case 319: // RPL_WHOISCHANNELS
            FE::emit(FE::get(WHOIS_CHANNELS) << getWord(param, 2) << rest, FE::CURRENT, _conn);
            break;
        case 320: // NickServ (freenode.net only?) "foo is an identified user" reply.
            FE::emit(FE::get(WHOIS_GENERIC) << getWord(param, 2) << rest, FE::CURRENT, _conn);
            break;
        case 330: // QuakeNet "foo is authed as" reply.
            FE::emit(FE::get(WHOIS_GENERIC) << getWord(param, 2) << rest + " " + getWord(param, 3), FE::CURRENT, _conn);
            break;

        case 421: // ERR_UNKNOWNCOMMAND
            FE::emit(FE::get(SERVERMSG2) << getWord(param, 2) << rest, FE::CURRENT, _conn);
            break;

        default:
            FE::emit(FE::get(SERVERMSG2) << skipFirstWord(param) << rest, FE::CURRENT, _conn);
    }

}

void Parser::Names(const ustring& chan, const ustring& names)
{
    // Find channel from a ustring like 'nick = #chan'
    ustring::size_type pos = chan.find_first_of("#&+!");
    ustring channel = chan.substr(pos);

    Channel *c = _conn->findChannel(channel);
    if (c && !c->endOfNames) {

        vector<ustring> nicks;
        Util::tokenizeWords(names, nicks);

        vector<ustring>::const_iterator i;

        for (i = nicks.begin(); i != nicks.end(); ++i)
        {
            if ((*i)[0] == '@')
                  c->addUser(i->substr(1), IRC::OP);
            else if ((*i)[0] == '+')
                  c->addUser(i->substr(1), IRC::VOICE);
            else if ((*i)[0] == '%')
                  c->addUser(i->substr(1), IRC::HALFOP);
            else
                  c->addUser(*i, IRC::NONE);
        }

    } else {
        FE::emit(FE::get(NAMES) << channel << names, FE::CURRENT, _conn);
    }
}

bool Parser::shouldHighlight(const ustring& str)
{
    if (str.find(_conn->Session.nick) != ustring::npos)
          return true;

    std::istringstream ss(App->options.highlight_words->raw());

    ustring tmp;
    while (ss >> tmp)
          if (str.find(tmp) != ustring::npos)
                return true;

    return false;
}

ustring getWord(const ustring& str, int n)
{
    int count = 0;
    ustring::size_type lastPos = str.find_first_not_of(" ", 0);
    ustring::size_type pos     = str.find_first_of(" ", lastPos);

    while (pos != ustring::npos || lastPos != ustring::npos)
    {
        if ((count + 1) == n)
              return str.substr(lastPos, pos - lastPos);

        lastPos = str.find_first_not_of(" ", pos);
        pos = str.find_first_of(" ", lastPos);
        count++;
    }
    return "";
}

// Strip mIRC colors. Spec at: http://www.mirc.co.uk/help/color.txt
// Only strips color-strings. Not bold and underline.
ustring stripColors(const ustring& str, const bool stripBoldAndUnderline)
{
    ustring newstr;
    bool color = false;
    int numbercount = 0;
    for (ustring::size_type i = 0; i < str.length(); ++i)
    {
        if (str[i] == '\017') { // RESET
            color = false;
        } else if (stripBoldAndUnderline && str[i] == '\002') {
            // BOLD
            // No-op.
        } else if (stripBoldAndUnderline && str[i] == '\037') {
            // UNDERLINE
            // No-op.
        } else if (str[i] == '\003') { // COLOR
            color = true;
        } else if (color && isdigit(str[i]) && numbercount < 2) {

            numbercount++;
        } else if (color && str[i] == ',' && numbercount < 3) {
            numbercount = 0;
        } else {
            numbercount = 0;
            color = false;
            newstr += str[i];
        }
    }
    return newstr;
}

Glib::ustring findNick(const Glib::ustring& str)
{
    return str.substr(0, str.find_first_of("!"));
}

Glib::ustring findHost(const Glib::ustring& str)
{
    return str.substr(str.find_first_of("!") + 1);
}

Glib::ustring skipFirstWord(const Glib::ustring& str)
{
    return str.substr(str.find_first_of(" ") + 1);
}
