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

#include "ServerConnection.h"
#include "Utils.h"
#include "Parser.h"

using std::vector;
using std::string;
using std::stringstream;
using std::cout;

Parser::Parser(InOut *inout, ServerConnection *conn)
    : _conn(conn), _io(inout), _evts(new Events(inout))
{

}

void Parser::parseLine(string& data)
{
    #ifdef DEBUG
    cout << "<< " + data;
    #endif
    // Erase \r and \n, we dont need them when parsing the messages.
    data.erase(data.rfind("\r\n"), strlen("\r\n"));

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
        cout << " [rest '" << rest << "']" << endl;
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
        else
              _evts->emitEvent("wnknown", data, "", _conn);

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
        cout << " [rest '" << rest << "']" << endl;
        #endif

        // Redirect to the right parsing function... 
        if (command == "PING")
              Ping(rest);
        else if (command == "NOTICE")
              Notice(param + " :" + rest);
        else if (command == "ERROR")
              _evts->emitEvent("error", param + " " + rest, "", _conn);
        else
              _evts->emitEvent("unknown", data, "", _conn);
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
        CTCP(from, param, rest);
    } else {
        // Normal privmsg 
        vector<string> args;
        string nick = param;
        if (param == _conn->Session.nick)
              nick = from;

        args.push_back(findNick(from));
        args.push_back(rest);

        string::size_type pos = rest.find(_conn->Session.nick);

        if (pos != string::npos) {
            _evts->emitEvent("privmsg_highlight", args, findNick(nick), _conn);
        } else {
            _evts->emitEvent("privmsg", args, findNick(nick), _conn);
        }
    }
}

void Parser::CTCP(const string& from, const string& param, const string& rest)
{
    string::size_type pos = rest.find_first_of(" \001", 1);
    string command = rest.substr(1, pos - 1);

    if (command == "VERSION") {
        _conn->sendVersion(findNick(from));
        vector<string> args;
        args.push_back(command);
        args.push_back(findNick(from));
        _evts->emitEvent("ctcp", args, "", _conn);
    } else if (command == "ACTION") {

        string rest_ = rest.substr(pos + 1, (rest.length() - pos) - 2);
        vector<string> args;
        args.push_back(findNick(from));
        args.push_back(rest_);

        string nick = param;
        if (param == _conn->Session.nick)
              nick = from;

        _evts->emitEvent("action", args, findNick(nick), _conn);
    } else {
        vector<string> args;
        args.push_back(command);
        args.push_back(findNick(from));
        _evts->emitEvent("ctcp", args, "", _conn);
    }

}

void Parser::Notice(const string& from, const string& to, const string& rest)
{
    if (rest[0] == '\001') {
        // CTCP notice
        
        string tmp = rest;
        string::iterator i = remove(tmp.begin(), tmp.end(), '\001');
        string output(tmp.begin(), i);

        _evts->emitEvent("servmsg", output, "", _conn);
    } else {
        // Normal notice
        
        vector<string> args;
        args.push_back(findNick(from));
        args.push_back(to);
        args.push_back(rest);

        _evts->emitEvent("noticepubl", args, "", _conn);
    }
}

void Parser::Notice(const string& msg)
{
    string::size_type pos = msg.find_first_of(" ");
    string from = msg.substr(0, pos);
    string rest = msg.substr(pos + 1);

    vector<string> args;
    args.push_back(from);
    args.push_back(rest);

    _evts->emitEvent("noticepriv", args, "", _conn);
}

void Parser::Kick(const string& from, const string& param, const string& msg)
{
    string chan, nick;
    stringstream ss(param);
    ss >> chan;
    ss >> nick;

    vector<string> args;
    args.push_back(nick);
    args.push_back(chan);
    args.push_back(findNick(from));
    args.push_back(msg);

    _evts->emitEvent("kicked", args, chan, _conn);
    _io->evtKick(findNick(from), chan, nick, msg, _conn);
}

void Parser::Join(const string& nick, const string& chan)
{
    vector<string> args;
    args.push_back(findNick(nick));
    args.push_back(chan);
    args.push_back(findHost(nick));
    _io->evtJoin(findNick(nick), chan, _conn);
    _evts->emitEvent("join", args, chan, _conn);
}

void Parser::Whois(const string& from, const string& param, const string& rest)
{
    vector<string> args;
    args.push_back(param);
    args.push_back(rest);

    _evts->emitEvent("servmsg2", args, "", _conn);
}

void Parser::Part(const string& nick, const string& chan)
{
    vector<string> args;
    args.push_back(findNick(nick));
    args.push_back(chan);
    args.push_back(findHost(nick));
    _io->evtPart(findNick(nick), chan, _conn);
    _evts->emitEvent("part", args, chan, _conn);
}

void Parser::Quit(const string& nick, const string& msg)
{
    _io->evtQuit(findNick(nick), msg, _conn);
}

void Parser::Nick(const string& from, const string& to)
{
    vector<string> args;
    args.push_back(from);
    args.push_back(to);

    _io->evtNick(findNick(from), to, _conn);
}

void Parser::Topic(const string& from, const string& to, const string& rest)
{
    vector<string> args;
    args.push_back(findNick(from));
    args.push_back(rest);

    _evts->emitEvent("topicchange", args, to, _conn);
}

void Parser::Mode(const string& from, const string& param, const string& rest)
{
    if (rest.empty()) {
        // We encountered a channel mode message
        CMode(from, param);
    } else {
        // User mode message
        // We got line in the form: 'user +x'
        _io->evtMode(findNick(from), param, rest, _conn);
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

    vector<vector<string> > vecvec;

    // Get arguments
    vector<string> arguments;
    stringstream ss(args);
    string buf;
    while (ss >> buf)
          arguments.push_back(buf);

    vector<string>::iterator arg_i = arguments.begin();

    if (arguments.empty()) {
          // Received a channel mode, like '#chan +n'
          _io->evtCMode(findNick(from), chan, modes.at(0), modes, _conn);
          return;
    }

    char sign;

    string::iterator i;
    vector<string> tmp;
    for (i = modes.begin(); i != modes.end(); ++i) {
        if (!tmp.empty()) {
            vecvec.push_back(tmp);
            tmp.clear();
        }

        switch (*i) {
            case '+':
                sign = '+';
                break;
            case '-':
                sign = '-';
                break;
            case 'o':
                if (arg_i != arguments.begin())
                      arg_i++;

                if (sign == '+') {
                    tmp.push_back("@");
                } else {
                    tmp.push_back(" ");
                }
                tmp.push_back(*arg_i);
                break;
            case 'v':
                if (arg_i != arguments.begin())
                      arg_i++;

                if (sign == '+') {
                    tmp.push_back("+");
                } else {
                    tmp.push_back(" ");
                }
                tmp.push_back(*arg_i);
                break;
        }

    }
    if (!tmp.empty()) {
        vecvec.push_back(tmp);
        tmp.clear();
    }

    // Channel user mode
    _io->evtCUMode(findNick(from), chan, vecvec, _conn);
        
}

void Parser::Topic(const string& param, const string& rest)
{
    string::size_type pos1 = param.find_first_of("#");
    string::size_type pos2 = param.find_first_of(" ", pos1);

    string chan = param.substr(pos1, pos2 - pos1);

    vector<string> args;
    args.push_back(chan);
    args.push_back(rest);
    _evts->emitEvent("topicis", args, chan, _conn);
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

    vector<string> args;
    args.push_back(nick);
    args.push_back(time.substr(0, time.size() - 1));

    _evts->emitEvent("topictime", args, chan, _conn);
}


void Parser::ServMsg(const string& from, const string& param, const string& msg)
{
    _evts->emitEvent("servmsg", msg, "", _conn);
}

void Parser::Away(const string& from, const string& param, const string& rest)
{
    string param1, param2;
    stringstream ss(param);
    ss >> param1;
    ss >> param2;

    vector<string> args;
    args.push_back(param2);
    args.push_back(rest);

    _evts->emitEvent("away", args, param2, _conn);
}

void Parser::Selfaway(const string& rest)
{
    vector<string> args;
    args.push_back(rest);
    _evts->emitEvent("servmsg", args, "", _conn);
}

void Parser::Wallops(const string& from, const string& rest)
{
    vector<string> args;
    args.push_back(from);
    args.push_back(rest);
    _evts->emitEvent("wallops", args, "", _conn);
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

    vector<string> args;
    args.push_back(banmask + " " + time);
    args.push_back(owner);

    _evts->emitEvent("banlist", args, chan, _conn);
}
     
void Parser::Errhandler(const string& from, const string& param, const string& rest)
{
    _evts->emitEvent("error", param + " " + rest, "", _conn);
}

void Parser::numeric(int n, const string& from, const string& param, const string& rest)
{
    vector<string> args;
    args.push_back(rest);
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
        Selfaway(rest);
        break;

    case 306: // RPL_NOUNAWAY
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
        _evts->emitEvent("servmsg", args, "", _conn);
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
        _evts->emitEvent("unknown", from + " " + param + " " + rest, "", _conn);
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
        } else {
            vec.push_back(" ");
            vec.push_back(buf);
        }
        vecvec.push_back(vec);
    }

    _io->evtNames(channel, vecvec, _conn);
}

string Parser::findNick(const string& str)
{
    return str.substr(0, str.find_first_of("!"));
}

string Parser::findHost(const string& str)
{
    return str.substr(str.find_first_of("!") + 1);
}
