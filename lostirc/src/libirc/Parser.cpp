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

#include "ServerConnection.h"
#include "Utils.h"
#include "Parser.h"
#include "Events.h"

using std::vector;
using std::string;
using std::stringstream;
using std::cout;

Parser::Parser(LostIRCApp *app, ServerConnection *conn)
    : _conn(conn), _app(app)
{
    _evts = _app->getEvts();
}

void Parser::parseLine(string& data)
{
    #ifdef DEBUG
    cout << "<< " + data;
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
        cout << "\t[from '" << from << "']";
        cout << " [command '" << command << "']";
        cout << " [param '" << param << "']"; 
        cout << " [rest '" << rest << "']" << std::endl;
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
        cout << "\t[command '" << command << "']";
        cout << " [param '" << param << "']";
        cout << " [rest '" << rest << "']" << std::endl;
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
        vector<string> args;
        args.push_back(findNick(from));
        args.push_back(rest_);

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

        _evts->emit(_evts->get(SERVMSG) << output, "", _conn);
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
    stringstream ss(param);
    ss >> chan;
    ss >> nick;

    _evts->emit(_evts->get(KICKED) << nick << chan << findNick(from) << msg, chan, _conn);
    _app->evtKick(findNick(from), chan, nick, msg, _conn);
}

void Parser::Join(const string& nick, const string& chan)
{
    if (findNick(nick) == _conn->Session.nick)
          _conn->addChannel(chan); // Add channel to ServerConn
          
    _conn->findChannel(chan)->addUser(findNick(nick));
    _app->evtJoin(findNick(nick), chan, _conn); // Send join to frontend

    _evts->emit(_evts->get(JOIN) << findNick(nick) << chan << findHost(nick), chan, _conn);
}

void Parser::Whois(const string& from, const string& param, const string& rest)
{
    _evts->emit(_evts->get(SERVMSG2) << param << rest, "", _conn);
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

void Parser::Topic(const string& from, const string& to, const string& rest)
{
    _evts->emit(_evts->get(TOPICCHANGE) << findNick(from) << rest, to, _conn);
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

    vector<struct Mode> modesvec;

    // Get arguments
    vector<string> arguments;
    stringstream ss(args);
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
                struct Mode m;
                sign ? (m.mode = IRC::OP) : (m.mode = IRC::DEOP);
                m.nick = *arg_i++;

                modesvec.push_back(m);
                }
                break;
            case 'v':
                {
                struct Mode m;
                sign ? (m.mode = IRC::VOICE) : (m.mode = IRC::DEVOICE);
                m.nick = *arg_i++;

                modesvec.push_back(m);
                }
                break;
            case 'b':
                {
                struct Mode m;
                sign ? (m.mode = IRC::BAN) : (m.mode = IRC::UNBAN);
                m.nick = *arg_i++;

                modesvec.push_back(m);
                break;
                }
        }

    }

    // Go through our modes and send the proper msg to the client
    vector<struct Mode>::iterator i;
    for (i = modesvec.begin(); i != modesvec.end(); ++i) {
        Event e;
        switch (i->mode)
        {
            case IRC::OP:
                e = OPPED;
                break;
            case IRC::DEOP:
                e = DEOPPED;
                break;
            case IRC::VOICE:
                e = VOICED;
                break;
            case IRC::DEVOICE:
                e = DEVOICED;
                break;
            case IRC::BAN:
                e = BANNED;
                break;
            case IRC::UNBAN:
                e = UNBANNED;
                break;
        }
        _evts->emit(_evts->get(e) << findNick(from) << i->nick, chan, _conn);
    }

    // Channel user mode
    _app->evtCUMode(findNick(from), chan, modesvec, _conn);
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


void Parser::ServMsg(const string& from, const string& param, const string& msg)
{
    _evts->emit(_evts->get(SERVMSG) << msg, "", _conn);
}

void Parser::Away(const string& from, const string& param, const string& rest)
{
    string param1, param2;
    stringstream ss(param);
    ss >> param1;
    ss >> param2;

    _evts->emit(_evts->get(AWAY) << param2 << rest, param2, _conn);
}

void Parser::Selfaway(const string& rest)
{
    _evts->emit(_evts->get(SERVMSG) << rest, "", _conn);
}

void Parser::Wallops(const string& from, const string& rest)
{
    _evts->emit(_evts->get(WALLOPS) << from << rest, "", _conn);
}

void Parser::Banlist(const string& param)
{
    string dummy, chan, banmask, owner, time;
    stringstream ss(param);
    ss >> dummy;
    ss >> chan;
    ss >> banmask;
    ss >> owner;
    ss >> time;

    long date = std::atol(time.c_str());
    time = std::ctime(&date);

    _evts->emit(_evts->get(BANLIST) << banmask << owner, chan, _conn);
}
     
void Parser::Errhandler(const string& from, const string& param, const string& rest)
{
    _evts->emit(_evts->get(ERROR) << param + " " + rest, "", _conn);
}

void Parser::numeric(int n, const string& from, const string& param, const string& rest)
{
    switch(n)
    {
    case 1: // RPL_WELCOME
        _conn->Session.servername = from;
        _conn->Session.hasRegistered = 1;
        ServMsg(from, param, rest);
        break;

    case 2: // RPL_YOURHOST
        ServMsg(from, param, rest);
        break;

    case 3: // RPL_CREATED
        ServMsg(from, param, rest);
        break;

    case 4: // RPL_MYINFO
        ServMsg(from, param, rest);
        break;
    
    case 5: // RPL_MYINFO
        ServMsg(from, param, rest);
        break;

    case 251: // RPL_LUSERCLIENT
        ServMsg(from, param, rest);
        break;

    case 252: // RPL_LUSEROP
        ServMsg(from, param, rest);
        break;
    
    case 253: // RPL_LUSERUNKNOWN
        ServMsg(from, param, rest);
        break;
    
    case 254: // RPL_LUSERCHANNELS
        ServMsg(from, param.substr(param.find_last_of(" ")), rest);
        break;

    case 255: // RPL_LUSERME
        ServMsg(from, param, rest);
        break;

    case 301: // RPL_AWAY
        Away(from, param, rest);
        break;

    case 305: // RPL_UNAWAY
        _app->evtAway(false, _conn);
        _conn->Session.isAway = false;
        Selfaway(rest);
        break;

    case 306: // RPL_NOWAWAY
        _app->evtAway(true, _conn);
        _conn->Session.isAway = true;
        Selfaway(rest);
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
        _evts->emit(_evts->get(SERVMSG) << rest, "", _conn);
        break;
    
    case 372: // RPL_MOTD
        ServMsg(from, param, rest);
        break;

    case 375: // RPL_MOTDSTART
        ServMsg(from, param, rest);
        break;

    case 376: // RPL_ENDOFMOTD
        ServMsg(from, param, rest);
        break;
    
    case 401: // ERR_NOSUCNICK
        Errhandler(from, param, rest);
        break;

    case 403: // ERR_NOSUCHCHANNEL
        Errhandler(from, param, rest);
        break;

    case 404: // ERR_CANNOTSENDTOCHAN
        Errhandler(from, param, rest);
        break;

    case 405: // ERR_TOOMANYCHANNELS
        Errhandler(from, param, rest);
        break;

    case 412: // ERR_NOTEXTTOSEND (or something)
        ServMsg(from, param, rest);
        break;

    case 422: // ERR_NOTONCHANNEL
        Errhandler(from, param, rest);
        break;

    case 433: // ERR_NICKNAMEINUSE
        // Apply a _ to the nickname - XXX: also send msg to frontend?
        _conn->sendNick(_conn->Session.nick += "_");
        break;
        
    case 438: // Nick change to fast
        Errhandler(from, param, rest);
        break;
        
    case 442: // ERR_NOTONCHANNEL
        Errhandler(from, param, rest);
        break;

    case 443: // ERR_USERONCHANNEL
        Errhandler(from, param, rest);
        break;

    case 451: // ERR_NOTREGISTERED
        Errhandler(from, param, rest);
        break;

    case 461: // ERR_NEEDMOREPARAMS
        Errhandler(from, param, rest);
        break;

    case 462: // ERR_ALLREADYREGISTERED
        Errhandler(from, param, rest);
        break;

    case 464: // ERR_PASSWDMISMATCH
        Errhandler(from, param, rest);
        break;

    case 465: // ERR_YOUREBANNEDCREEP
        Errhandler(from, param, rest);
        break;

    case 467: // ERR_KEYSET
        Errhandler(from, param, rest);
        break;

    case 471: // ERR_CHANNELISFULL
        Errhandler(from, param, rest);
        break;

    case 472: // ERR_UNKNOWMODE
        Errhandler(from, param, rest);
        break;

    case 473: // ERR_INVITEONLYCHAN
        Errhandler(from, param, rest);
        break;

    case 474: // ERR_BANNEDFROMCHAN
        Errhandler(from, param, rest);
        break;

    case 475: // ERR_BADCHANNELKEY
        Errhandler(from, param, rest);
        break;

    case 481: // ERR_NOPRIVILEGES
        Errhandler(from, param, rest);
        break;

    case 482: // ERR_CHANOPRIVSNEEDED
        Errhandler(from, param, rest);
        break;

    case 491: // ERR_NOOPERHOST
        Errhandler(from, param, rest);
        break;

    case 501: // ERR_UMODEUNKNOWNFLAG
        Errhandler(from, param, rest);
        break;

    case 502: // ERR_USERSDONTMATCH
        Errhandler(from, param, rest);
        break;

    case 353: // RPL_NAMREPLY
        Names(param, rest);
        break;

    case 366: // RPL_ENDOFNAMES
        break; // Ignored.

    case 311: // RPL_WHOISUSER

        Whois(from, param, rest);
        break;

    case 312: // RPL_WHOISSERVER
        Whois(from, param, rest);
        break;

    case 313: // RPL_WHOISOPERATOR
        Whois(from, param, rest);
        break;

    case 317: // RPL_WHOISIDLE
        Whois(from, param, rest);
        break;

    case 318: // RPL_ENDOFWHOIS
        Whois(from, param, rest);
        break;

    case 319: // RPL_WHOISCHANNELS
        Whois(from, param, rest);
        break;

    default:
        _evts->emit(_evts->get(SERVMSG) << from + " " + param + " " + rest, "", _conn);
    }

}

void Parser::Names(const string& chan, const string& names)
{
    // Find channel from a string like 'nick = #chan'
    string::size_type pos = chan.find_last_of("#");
    string channel = chan.substr(pos);

    stringstream ss(names);
    string buf;

    vector<vector<string> > vecvec;

    while(ss >> buf)
    {
        vector<string> vec;
        if (buf[0] == '@' || buf[0] == '+') {
            vec.push_back(buf.substr(0, 1));
            vec.push_back(buf.substr(1));
            // Add to internal Channel's, XXX this should be fixed so it doesn't
            // have to search for the channel every time
            _conn->findChannel(channel)->addUser(buf.substr(1));
        } else {
            vec.push_back(" ");
            vec.push_back(buf);
            _conn->findChannel(channel)->addUser(buf);
        }
        vecvec.push_back(vec);
    }

    _app->evtNames(channel, vecvec, _conn);
}

string Parser::findNick(const string& str)
{
    return str.substr(0, str.find_first_of("!"));
}

string Parser::findHost(const string& str)
{
    return str.substr(str.find_first_of("!") + 1);
}
