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

#include <sstream>
#include "Parser.h"
#include "Utils.h"
#include "Channel.h"
#include "Events.h"
#include "LostIRCApp.h"
#include "ServerConnection.h"

using std::vector;
using std::string;

Parser::Parser(LostIRCApp *app, ServerConnection *conn)
    : _conn(conn), _app(app)
{
    _evts = _app->getEvts();
}

void Parser::parseLine(string& data)
{
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
        std::cout << "\t[from '" << from << "']";
        std::cout << " [command '" << command << "']";
        std::cout << " [param '" << param << "']"; 
        std::cout << " [rest '" << rest << "']" << std::endl;
        #endif

        // Redirect to the right parsing function...
        
        int n = atoi(command.c_str());
        if (n)
              numeric(n, from, param, rest);
        else if (command == "PRIVMSG")
              Privmsg(from, param, rest);
        else if (command == "JOIN")
              Join(from, rest);
        else if (command == "QUIT")
              Quit(from, rest);
        else if (command == "PART")
              Part(from, param);
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
        else
              _evts->emit(_evts->get(UNKNOWN) << data, "", _conn);

    } else {
        // Parse string in form, eg. 'PING :23523525'
        string::size_type pos1 = data.find_first_of(" ", 0);
        string command = data.substr(0, pos1);

        // Check whether there is any params
        string::size_type pos2 = data.find_first_of(":", pos1 + 1);
        string param;

        if ((pos2 - 1) != pos1) {
            // We have params
            param = data.substr(pos1 + 1, (pos2 - 2) - pos1);
        }

        // Get rest (whats after the ':')
        string rest = data.substr(pos2 + 1);

        #ifdef DEBUG
        std::cout << "\t[command '" << command << "']";
        std::cout << " [param '" << param << "']";
        std::cout << " [rest '" << rest << "']" << std::endl;
        #endif

        // Redirect to the right parsing function... 
        if (command == "PING")
              Ping(rest);
        else if (command == "NOTICE")
              Notice(param + " :" + rest);
        else if (command == "ERROR")
              _evts->emit(_evts->get(ERROR) << param + " " + rest, "", _conn);
        else
              _evts->emit(_evts->get(SERVMSG) << data, "", _conn);
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
        string nick = param;
        if (param == _conn->Session.nick)
              nick = from;

        if (rest.find(_conn->Session.nick) != string::npos) {
            _evts->emit(_evts->get(PRIVMSG_HIGHLIGHT) << findNick(from) << rest, findNick(nick), _conn);
            _app->evtHighlight(findNick(nick), _conn);
        } else {
            _evts->emit(_evts->get(PRIVMSG) << findNick(from) << rest, findNick(nick), _conn);
        }
    }
}

void Parser::Ctcp(const string& from, const string& param, const string& rest)
{
    string::size_type pos = rest.find_first_of(" \001", 1);
    string command = rest.substr(1, pos - 1);

    if (command == "VERSION") {
        _conn->sendVersion(findNick(from));
        _evts->emit(_evts->get(CTCP) << command << findNick(from), "", _conn);
    } else if (command == "ACTION") {
        string rest_ = rest.substr(pos + 1, (rest.length() - pos) - 2);

        string nick = param;
        if (param == _conn->Session.nick)
              nick = from;

        if (rest_.find(_conn->Session.nick) != string::npos) {
            _evts->emit(_evts->get(ACTION_HIGHLIGHT) << findNick(from) << rest_, findNick(nick), _conn);
            _app->evtHighlight(findNick(nick), _conn);
        } else {
            _evts->emit(_evts->get(ACTION) << findNick(from) << rest_, findNick(nick), _conn);
        }
    } else {
        _evts->emit(_evts->get(CTCP) << command << findNick(from), "", _conn);
    }

}

void Parser::Notice(const string& from, const string& to, const string& rest)
{
    if (rest[0] == '\001') {
        // CTCP notice
        string tmp = rest;
        string::iterator i = remove(tmp.begin(), tmp.end(), '\001');
        string output(tmp.begin(), i);

        _evts->emit(_evts->get(SERVMSG) << findNick(from) + " " + output, "", _conn);
    } else {
        // Normal notice
        _evts->emit(_evts->get(NOTICEPUBL) << findNick(from) << to << rest, "", _conn);
    }
}

void Parser::Notice(const string& msg)
{
    string::size_type pos = msg.find_first_of(" ");
    string from = msg.substr(0, pos);
    string rest = msg.substr(pos + 1);

    _evts->emit(_evts->get(NOTICEPRIV) << from << rest, "", _conn);
}

void Parser::Kick(const string& from, const string& param, const string& msg)
{
    string chan, nick;
    std::istringstream ss(param);
    ss >> chan;
    ss >> nick;

    if (nick == _conn->Session.nick) // We got kicked
          _conn->removeChannel(chan);

    _evts->emit(_evts->get(KICKED) << nick << chan << findNick(from) << msg, chan, _conn);
    _app->evtKick(findNick(from), chan, nick, msg, _conn);
}

void Parser::Join(const string& nick, const string& chan)
{
    if (findNick(nick) == _conn->Session.nick)
          _conn->addChannel(chan);
    else
          _conn->findChannel(chan)->addUser(findNick(nick));

    _app->evtJoin(findNick(nick), chan, _conn); // Send join to frontend

    _evts->emit(_evts->get(JOIN) << findNick(nick) << chan << findHost(nick), chan, _conn);
}

void Parser::Part(const string& nick, const string& chan)
{
    _conn->findChannel(chan)->removeUser(findNick(nick));

    if (findNick(nick) == _conn->Session.nick) {
          _conn->removeChannel(chan); // Remove channel in ServerConn
          if (_conn->Session.channels.empty())
                _conn->sendQuit(""); // Quit the server if we are parting the last channel
    }

    _evts->emit(_evts->get(PART) << findNick(nick) << chan << findHost(nick), chan, _conn);
    _app->evtPart(findNick(nick), chan, _conn);
}

void Parser::Quit(const string& nick, const string& msg)
{
    vector<string> chans = _conn->findUser(findNick(nick));

    _evts->emit(_evts->get(QUIT) << findNick(nick) << msg, chans, _conn);
    _app->evtQuit(findNick(nick), msg, _conn);
}

void Parser::Nick(const string& from, const string& to)
{
    // Check whethers it's us who has changed nick
    if (findNick(from) == _conn->Session.nick) {
        _conn->Session.nick = to;
    }

    vector<string>::iterator i;
    vector<string> chans = _conn->findUser(findNick(from));

    for (i = chans.begin(); i != chans.end(); ++i) {
        _conn->findChannel(*i)->removeUser(findNick(from));
        _conn->findChannel(*i)->addUser(to);
    }

    _evts->emit(_evts->get(NICK) << findNick(from) << to, chans, _conn);
    _app->evtNick(findNick(from), to, _conn);
}

void Parser::Invite(const string& from, const string& params)
{
    _evts->emit(_evts->get(INVITED) << findNick(from) << params, "", _conn);
}

void Parser::Mode(const string& from, const string& param, const string& rest)
{
    if (rest.empty()) {
        // We encountered a channel mode message
        CMode(from, param);
    } else {
        // User mode message
        // We got line in the form: 'user +x'
        _evts->emit(_evts->get(MODE) << findNick(from) << param << rest, "", _conn);
    }
}

void Parser::CMode(const string& from, const string& param)
{
    // Parse line in the form: '#chan +ovo nick nick2 nick3'
    string::size_type pos1 = param.find_first_of(" ");
    string::size_type pos2 = param.find_first_of(" ", pos1 + 1);

    string chan = param.substr(0, pos1);
    string modes = param.substr(pos1 + 1, (pos2 - pos1) - 1);
    string args = param.substr(pos2 + 1);

    //vector<struct Mode> modesmap;
    std::map<string, IRC::UserMode> modesmap;

    // Get arguments
    vector<string> arguments;
    std::istringstream ss(args);
    string buf;
    while (ss >> buf)
          arguments.push_back(buf);

    if (arguments.empty()) {
        // Received a channel mode, like '#chan +n'
        _evts->emit(_evts->get(CMODE) << findNick(from) << modes.substr(0, 1) << modes << chan, chan, _conn);
        return;
    }

    bool sign = false; // Used to track whether we get a + or a -
    vector<string>::iterator arg_i = arguments.begin();
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
                IRC::UserMode u;
                sign ? (u = IRC::OP) : (u = IRC::NONE);
                sign ? (e = OPPED) : (e = DEOPPED);
                string nick = *arg_i++;

                modesmap.insert(make_pair(nick, u));
                _evts->emit(_evts->get(e) << findNick(from) << nick, chan, _conn);
                }
                break;
            case 'v':
                {
                Event e;
                IRC::UserMode u;
                sign ? (u = IRC::VOICE) : (u = IRC::NONE);
                sign ? (e = VOICED) : (e = DEVOICED);
                string nick = *arg_i++;

                modesmap.insert(make_pair(nick, u));
                _evts->emit(_evts->get(e) << findNick(from) << nick, chan, _conn);
                }
                break;
            case 'b':
                {
                Event e;
                sign ? (e = BANNED) : (e = UNBANNED);
                string nick = *arg_i++;
                _evts->emit(_evts->get(e) << findNick(from) << nick, chan, _conn);
                break;
                }
        }

    }

    // Channel user mode
    _app->evtCUMode(findNick(from), chan, modesmap, _conn);
}

void Parser::Topic(const string& from, const string& to, const string& rest)
{
    _evts->emit(_evts->get(TOPICCHANGE) << findNick(from) << rest, to, _conn);
}

void Parser::Topic(const string& param, const string& rest)
{
    string::size_type pos1 = param.find_first_of("#");
    string::size_type pos2 = param.find_first_of(" ", pos1);

    string chan = param.substr(pos1, pos2 - pos1);

    _evts->emit(_evts->get(TOPICIS) << chan << rest, chan, _conn);
}

void Parser::TopicTime(const string& param)
{
    // Find channel
    string::size_type pos1 = param.find_first_of("#");
    string::size_type pos2 = param.find_first_of(" ", pos1);
    string chan = param.substr(pos1, pos2 - pos1);

    // Find user who set the topic
    string::size_type pos3 = param.find_first_of(" ", pos2 + 1);
    string nick = param.substr(pos2 + 1, (pos3 - pos2) - 1);
    string time = param.substr(pos3 + 1);

    long date = std::atol(time.c_str());
    time = std::ctime(&date);

    _evts->emit(_evts->get(TOPICTIME) << nick << time.substr(0, time.size() - 1), chan, _conn);
}

void Parser::Away(const string& param, const string& rest)
{
    string param1, param2;
    std::istringstream ss(param);
    ss >> param1;
    ss >> param2;

    _evts->emit(_evts->get(AWAY) << param2 << rest, param2, _conn);
}

void Parser::Wallops(const string& from, const string& rest)
{
    _evts->emit(_evts->get(WALLOPS) << from << rest, "", _conn);
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

    _evts->emit(_evts->get(BANLIST) << banmask << owner, chan, _conn);
}
     
void Parser::numeric(int n, const string& from, const string& param, const string& rest)
{
    switch(n)
    {
        case 1:   // RPL_WELCOME
            _conn->Session.servername = from;
            _conn->Session.hasRegistered = 1;
            _conn->Session.nick = param;
        case 2:   // RPL_YOURHOST
        case 3:   // RPL_CREATED
        case 4:   // RPL_MYINFO
        case 5:   // RPL_MYINFO
        case 251: // RPL_LUSERCLIENT
        case 252: // RPL_LUSEROP
        case 253: // RPL_LUSERUNKNOWN
        case 254: // RPL_LUSERCHANNELS
        case 255: // RPL_LUSERME
            _evts->emit(_evts->get(SERVMSG) << rest, "", _conn);
            break;

        case 301: // RPL_AWAY
            Away(param, rest);
            break;

        case 305: // RPL_UNAWAY
            _app->evtAway(false, _conn);
            _conn->Session.isAway = false;
            _evts->emit(_evts->get(SERVMSG) << rest, "", _conn);
            break;

        case 306: // RPL_NOWAWAY
            _app->evtAway(true, _conn);
            _conn->Session.isAway = true;
            _evts->emit(_evts->get(SERVMSG) << rest, "", _conn);
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
            _evts->emit(_evts->get(SERVMSG) << rest, "", _conn);
            break;

        case 376: // RPL_ENDOFMOTD
            _evts->emit(_evts->get(SERVMSG) << rest, "", _conn);
            _conn->sendCmds();
            break;

        case 401: // ERR_NOSUCNICK
        case 403: // ERR_NOSUCHCHANNEL
        case 404: // ERR_CANNOTSENDTOCHAN
        case 405: // ERR_TOOMANYCHANNELS
            _evts->emit(_evts->get(ERROR) << param + " " + rest, "", _conn);
            break;

        case 412: // ERR_NOTEXTTOSEND (or something)
            _evts->emit(_evts->get(SERVMSG) << rest, "", _conn);
            break;

        case 422: // ERR_NOMOTD
            _conn->sendCmds();
            break;

        case 433: // ERR_NICKNAMEINUSE
            // Apply a _ to the nickname - XXX: also send msg to frontend?
            _conn->sendNick(_conn->Session.nick += "_");
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
            _evts->emit(_evts->get(ERROR) << param + " " + rest, "", _conn);
            break;

        case 353: // RPL_NAMREPLY
            Names(param, rest);
            break;

        case 366: // RPL_ENDOFNAMES
            {
                Channel *c = _conn->findChannel(getWord(param, 2));
                if (c && !c->endOfNames)
                      c->endOfNames = true;
                else
                      _evts->emit(_evts->get(SERVMSG) << param + " " + rest, "", _conn);
            }
            break; // Ignored.
        case 317: // RPL_WHOISIDLE
            {
                long idle = Util::stoi(getWord(param, 3));
                std::ostringstream ss;
                ss << idle / 3600 << ":" << (idle / 60) % 60 << ":" << idle % 60;
                long date = std::atol(getWord(param, 4).c_str());
                string time = std::ctime(&date);
                _evts->emit(_evts->get(SERVMSG) << ss.str() + ",  " + time.substr(0, time.size() - 1) + " " + rest, "", _conn);
            }
                break;
        case 311: // RPL_WHOISUSER
        case 312: // RPL_WHOISSERVER
        case 313: // RPL_WHOISOPERATOR
        case 318: // RPL_ENDOFWHOIS
        case 319: // RPL_WHOISCHANNELS
            // We need this find_first_of to omit the first word
            _evts->emit(_evts->get(SERVMSG2) << param.substr(param.find_first_of(" ") + 1) << rest, "", _conn);
            break;

        default:
            _evts->emit(_evts->get(SERVMSG) << param + " " + rest, "", _conn);
    }

}

void Parser::Names(const string& chan, const string& names)
{
    // Find channel from a string like 'nick = #chan'
    string::size_type pos = chan.find_last_of("#");
    string channel = chan.substr(pos);

    Channel *c = _conn->findChannel(channel);
    if (!c->endOfNames) {

        std::istringstream ss(names);
        string buf;

        while(ss >> buf)
        {
            if (buf[0] == '@') {
                if (c)
                      c->addUser(buf.substr(1), IRC::OP);
            } else if (buf[0] == '+') {
                if (c)
                      c->addUser(buf.substr(1), IRC::VOICE);
            } else {
                if (c)
                      c->addUser(buf, IRC::NONE);
            }
        }

        _app->evtNames(*c, _conn);
    } else {
        _evts->emit(_evts->get(NAMES) << channel << names, "", _conn);
    }
}

string Parser::getWord(const string& str, int n)
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
