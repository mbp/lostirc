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
using std::string;

/* different functions used in for_each */
namespace algo
{

  struct removeUser : public std::unary_function<ChannelBase*, void>
  {
      removeUser(const std::string& n) : nick(n) { }
      void operator() (ChannelBase* x) {
          Channel *c = dynamic_cast<Channel*>(x);
          if (c)
                c->removeUser(nick);
      }
      std::string nick;
  };

  struct renameUser : public std::unary_function<ChannelBase*, void>
  {
      renameUser(const std::string& f, const std::string& t) : from(f), to(t) { }
      void operator() (ChannelBase* x) {
          x->renameUser(from, to);
      }
      std::string from;
      std::string to;
  };

}

Parser::Parser(ServerConnection *conn)
    : _conn(conn)
{
}

void Parser::parseLine(string& data)
{
    #ifdef DEBUG
    App->log << "<< " << data << std::endl;
    #endif
    // Erase \r and \n, we dont need them when parsing the messages.
    data.erase(data.find_last_not_of("\r\n") + 1);

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
        string::size_type pos1 = data.find_first_of(" ", 1);
        string from = data.substr(1, pos1 - 1);

        // Find command
        string::size_type pos2 = data.find_first_of(" ", pos1 + 1);
        string command = data.substr(pos1 + 1, (pos2 - 1) - pos1);

        // Check whether there is any params
        string::size_type pos3 = data.find_first_of(":", pos2 + 1);
        string param;

        if ((pos3 - 1) != pos2) {
            // We have params
            param = data.substr(pos2 + 1, (pos3 - 2) - pos2);
        }

        // Get rest (whats after the ':', if there were any ':')
        string rest;
        if (pos3 != string::npos) {
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
              Join(from, rest);
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
        // Parse string in form, eg. 'PING :23523525'
        string::size_type pos1 = data.find_first_of(" ", 0);
        string command = data.substr(0, pos1);

        // Check whether there are any params
        string::size_type pos2 = data.find_first_of(":", pos1 + 1);
        string param;

        if ((pos2 - 1) != pos1) {
            // We have params
            param = data.substr(pos1 + 1, (pos2 - 2) - pos1);
        }

        // Get rest (whats after the ':')
        string rest = data.substr(pos2 + 1);

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
        else if (command == "ERROR")
              FE::emit(FE::get(ERROR) << param + " " + rest, FE::CURRENT, _conn);
        else
              FE::emit(FE::get(SERVMSG) << data, FE::CURRENT, _conn);
    }

}

inline
void Parser::Ping(const string& rest)
{
    _conn->sendPong(rest);
}

void Parser::Privmsg(const string& from, const string& param, const string& rest)
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

void Parser::Ctcp(const string& from, const string& param, const string& rest)
{
    string::size_type pos = rest.find_first_of(" \001", 1);
    string command = rest.substr(1, pos - 1);

    if (command == "VERSION") {
        _conn->sendVersion(findNick(from));

    } else if (command == "ACTION") {
        string rest_ = rest.substr(pos + 1, (rest.length() - pos) - 2);

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
        string rest_ = rest.substr(pos + 1, (rest.length() - pos) - 2);
        _conn->sendCtcpNotice(findNick(from), "PING " + rest_);

    } else if (command == "DCC") {
        string dcc_type = getWord(rest, 2);
        string dcc_address = getWord(rest, 4);
        string dcc_port = getWord(rest, 5);

        if (dcc_type == "SEND") {
            string dcc_size = getWord(rest, 6);
            string dcc_filename = getWord(rest, 3);

            std::istringstream ss(dcc_address);
            unsigned long address;
            ss >> address;

            std::istringstream ss2(dcc_port);
            unsigned short port;
            ss2 >> port;

            std::istringstream ss3(dcc_size);
            unsigned long size;
            ss3 >> size;

            int n = App->getDcc().addDccSendIn(dcc_filename, address, port, size);
            FE::emit(FE::get(DCC_RECEIVE) << findNick(from) << dcc_filename << n, FE::CURRENT);

        } else if (dcc_type == "CHAT") {
            FE::emit(FE::get(SERVMSG2) << findNick(from) << rest, FE::CURRENT);
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

void Parser::Notice(const string& from, const string& to, const string& rest)
{
    if (rest[0] == '\001') {
        // CTCP notice
        string tmp = rest;
        string::iterator i = remove(tmp.begin(), tmp.end(), '\001');
        string output(tmp.begin(), i);

        FE::emit(FE::get(SERVMSG) << findNick(from) + " " + output, FE::CURRENT, _conn);
    } else {
        // Normal notice
        FE::emit(FE::get(NOTICEPUBL) << findNick(from) << to << rest, FE::CURRENT, _conn);
    }
}

void Parser::Notice(const string& msg)
{
    string::size_type pos = msg.find_first_of(" ");
    string from = msg.substr(0, pos);
    string rest = msg.substr(pos + 1);

    FE::emit(FE::get(NOTICEPRIV) << from << rest, FE::CURRENT, _conn);
}

void Parser::Kick(const string& from, const string& param, const string& msg)
{
    string chan, nick;
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

void Parser::Join(const string& nick, const string& chan)
{
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

void Parser::Part(const string& nick, const string& param, const string& rest)
{
    string chan = param;

    // Some clients/servers/bouncers might accidently send the channel name
    // in the 'rest' string, a bug there, but we would like to avoid a
    // segfault here. I noticed the same hack in the xchat sources.
    if (chan.empty() && !rest.empty())
          chan = getWord(rest, 1);

    Channel *c = _conn->findChannel(chan);

    if (c) {
        c->removeUser(findNick(nick));

        FE::emit(FE::get(PART) << findNick(nick) << chan << findHost(nick) << rest, *c, _conn);
        App->fe->part(findNick(nick), *c, _conn);

        if (findNick(nick) == _conn->Session.nick)
              _conn->removeChannel(chan);

    }
}

void Parser::Quit(const string& nick, const string& msg)
{
    vector<ChannelBase*> chans = _conn->findUser(findNick(nick));

    for_each(chans.begin(), chans.end(), algo::removeUser(nick));

    App->fe->quit(findNick(nick), chans, _conn);
    FE::emit(FE::get(QUIT) << findNick(nick) << msg, chans, _conn);
}

void Parser::Nick(const string& from, const string& to)
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

void Parser::Invite(const string& from, const string& rest)
{
    FE::emit(FE::get(INVITED) << findNick(from) << rest, FE::CURRENT, _conn);
}

void Parser::Kill(const string& from, const string& rest)
{
    FE::emit(FE::get(KILLED) << findNick(from) << rest, FE::CURRENT, _conn);
}

void Parser::Mode(const string& from, const string& param, const string& rest)
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

void Parser::CMode(const string& from, const string& param)
{
    // Parse line in the form: '#chan +ovo nick nick2 nick3'
    string::size_type pos1 = param.find_first_of(" ");
    string::size_type pos2 = param.find_first_of(" ", pos1 + 1);

    // No real modes? No reason to continue.
    if (pos1 == string::npos)
          return;

    string channel = param.substr(0, pos1);
    string modes = param.substr(pos1 + 1, (pos2 - pos1) - 1);
    string args = param.substr(pos2 + 1);

    Channel *chan = _conn->findChannel(channel);

    // Channel not found? Not sane to continue. 
    // This happened on a proxy/bouncer where no channel was mentioned in
    // MODE, just this:
    //   :nick!ident@host.com MODE nick +iw
    // Note the lack of ':' before +iw.
    if (!chan)
          return;

    // Get arguments
    vector<string> arguments;
    std::istringstream ss(args);
    string buf;
    while (ss >> buf)
          arguments.push_back(buf);

    if (arguments.empty()) {
        // There were no further arguments, eg. MODE #channel +n
        FE::emit(FE::get(CMODE) << findNick(from) << modes << channel, *chan, _conn);
        return;
    }

    std::vector<User> modesvec;
    bool sign = false; // Used to track whether we get a + or a -

    vector<string>::const_iterator arg_i = arguments.begin();

    for (string::const_iterator i = modes.begin(); i != modes.end(); ++i) {
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
                string nick = *arg_i++;

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
                string nick = *arg_i++;

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
                string nick = *arg_i++;

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

                string nick = *arg_i++;
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

                string nick = *arg_i++;
                FE::emit(FE::get(MODE) << findNick(from) << modebuf << nick, *chan, _conn);
                }
        }

    }

    // Channel user mode
    App->fe->CUMode(findNick(from), *chan, modesvec, _conn);
}

void Parser::Topic(const string& from, const string& chan, const string& rest)
{
    Channel *c = _conn->findChannel(chan);
    if (c)
          FE::emit(FE::get(TOPICCHANGE) << findNick(from) << rest, *c, _conn);
    else
          FE::emit(FE::get(TOPICCHANGE) << findNick(from) << rest, FE::CURRENT, _conn);

}

void Parser::Topic(const string& param, const string& rest)
{
    string::size_type pos1 = param.find_first_of("#&+!");
    string::size_type pos2 = param.find_first_of(" ", pos1);

    string chan = param.substr(pos1, pos2 - pos1);
    Channel *c = _conn->findChannel(chan);

    if (c)
          FE::emit(FE::get(TOPICIS) << chan << rest, *c, _conn);
    else
          FE::emit(FE::get(TOPICIS) << chan << rest, FE::CURRENT, _conn);
}

void Parser::TopicTime(const string& param)
{
    // Find channel
    string::size_type pos1 = param.find_first_of("#&+!");
    string::size_type pos2 = param.find_first_of(" ", pos1);
    string chan = param.substr(pos1, pos2 - pos1);

    // Find user who set the topic
    string::size_type pos3 = param.find_first_of(" ", pos2 + 1);
    string nick = param.substr(pos2 + 1, (pos3 - pos2) - 1);
    string time = param.substr(pos3 + 1);

    long date = std::atol(time.c_str());
    time = std::ctime(&date);

    Channel *c = _conn->findChannel(chan);

    if (c)
          FE::emit(FE::get(TOPICTIME) << nick << time.substr(0, time.size() - 1), *c, _conn);
    else
          FE::emit(FE::get(TOPICTIME) << nick << time.substr(0, time.size() - 1), FE::CURRENT, _conn);
}

void Parser::Away(const string& param, const string& rest)
{
    string param1, param2;
    std::istringstream ss(param);
    ss >> param1;
    ss >> param2;

    FE::emit(FE::get(AWAY) << param2 << rest, FE::CURRENT, _conn);
}

void Parser::Wallops(const string& from, const string& rest)
{
    FE::emit(FE::get(WALLOPS) << findNick(from) << rest, FE::CURRENT, _conn);
}

void Parser::Banlist(const string& param)
{
    string dummy, chan, banmask, owner, time;
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
     
void Parser::numeric(int n, const string& from, const string& param, const string& rest)
{
    switch(n)
    {
        case 1:   // RPL_WELCOME
            _conn->Session.servername = from;
            _conn->Session.hasRegistered = true;
            _conn->Session.nick = param;
            _conn->addConnectionTimerCheck();
        case 2:   // RPL_YOURHOST
        case 3:   // RPL_CREATED
        case 4:   // RPL_MYINFO
        case 5:   // RPL_MYINFO
        case 251: // RPL_LUSERCLIENT
        case 252: // RPL_LUSEROP
        case 253: // RPL_LUSERUNKNOWN
        case 254: // RPL_LUSERCHANNELS
        case 255: // RPL_LUSERME
            FE::emit(FE::get(SERVMSG) << rest, FE::CURRENT, _conn);
            break;

        case 301: // RPL_AWAY
            Away(param, rest);
            break;

        case 305: // RPL_UNAWAY
            _conn->Session.isAway = false;
            App->fe->away(false, _conn);
            FE::emit(FE::get(SERVMSG) << rest, FE::CURRENT, _conn);
            break;

        case 306: // RPL_NOWAWAY
            _conn->Session.isAway = true;
            App->fe->away(true, _conn);
            FE::emit(FE::get(SERVMSG) << rest, FE::CURRENT, _conn);
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
            FE::emit(FE::get(SERVMSG) << rest, FE::CURRENT, _conn);
            break;

        case 376: // RPL_ENDOFMOTD
            FE::emit(FE::get(SERVMSG) << rest, FE::CURRENT, _conn);
            _conn->Session.endOfMotd = true;
            _conn->sendCmds();
            break;

        case 401: // ERR_NOSUCNICK
        case 403: // ERR_NOSUCHCHANNEL
        case 404: // ERR_CANNOTSENDTOCHAN
        case 405: // ERR_TOOMANYCHANNELS
            FE::emit(FE::get(ERROR) << param + ": " + rest, FE::CURRENT, _conn);
            break;

        case 412: // ERR_NOTEXTTOSEND
            FE::emit(FE::get(SERVMSG) << rest, FE::CURRENT, _conn);
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
                FE::emit(FE::get(ERROR) << rest, FE::CURRENT, _conn);
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
            FE::emit(FE::get(ERROR) << param + " " + rest, FE::CURRENT, _conn);
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
                    FE::emit(FE::get(SERVMSG3) << getWord(param, 2) << rest, FE::CURRENT, _conn);
                }
            }
            break;
        case 317: // RPL_WHOISIDLE
            {
                long idle = Util::stoi(getWord(param, 3));
                std::ostringstream ss;
                ss << idle / 3600 << ":" << (idle / 60) % 60 << ":" << idle % 60;
                long date = std::atol(getWord(param, 4).c_str());
                string time = std::ctime(&date);
                FE::emit(FE::get(SERVMSG3) << ss.str() + ",  " + time.substr(0, time.size() - 1) << rest, FE::CURRENT, _conn);
            }
            break;
        case 311: // RPL_WHOISUSER
        case 312: // RPL_WHOISSERVER
        case 313: // RPL_WHOISOPERATOR
        case 318: // RPL_ENDOFWHOIS
        case 319: // RPL_WHOISCHANNELS
            // We need this find_first_of to omit the first word
            FE::emit(FE::get(SERVMSG2) << param.substr(param.find_first_of(" ") + 1) << rest, FE::CURRENT, _conn);
            break;

        default:
            FE::emit(FE::get(SERVMSG3) << param << rest, FE::CURRENT, _conn);
    }

}

void Parser::Names(const string& chan, const string& names)
{
    // Find channel from a string like 'nick = #chan'
    string::size_type pos = chan.find_first_of("#&+!");
    string channel = chan.substr(pos);

    Channel *c = _conn->findChannel(channel);
    if (c && !c->endOfNames) {

        std::istringstream ss(names);
        string buf;

        while (ss >> buf)
        {
            if (buf[0] == '@') {
                c->addUser(buf.substr(1), IRC::OP);
            } else if (buf[0] == '+') {
                c->addUser(buf.substr(1), IRC::VOICE);
            } else if (buf[0] == '%') {
                c->addUser(buf.substr(1), IRC::HALFOP);
            } else {
                c->addUser(buf, IRC::NONE);
            }
        }

    } else {
        FE::emit(FE::get(NAMES) << channel << names, FE::CURRENT, _conn);
    }
}

string getWord(const string& str, int n)
{
    int count = 0;
    string::size_type lastPos = str.find_first_not_of(" ", 0);
    string::size_type pos     = str.find_first_of(" ", lastPos);

    while (pos != string::npos || lastPos != string::npos)
    {
        if ((count + 1) == n)
              return str.substr(lastPos, pos - lastPos);

        lastPos = str.find_first_not_of(" ", pos);
        pos = str.find_first_of(" ", lastPos);
        count++;
    }
    return "";
}

bool Parser::shouldHighlight(const string& str)
{
    if (str.find(_conn->Session.nick) != string::npos)
          return true;

    std::istringstream ss(App->options.highlight_words);

    string tmp;
    while (ss >> tmp)
          if (str.find(tmp) != string::npos)
                return true;

    return false;
}
