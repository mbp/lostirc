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

#ifndef DCC_H
#define DCC_H

#include <map>
#include <fstream>
#include <string>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glibmm/main.h>
#include <sigc++/sigc++.h>

class ServerConnection;

class DCC : public SigC::Object {
public:
    virtual void go_ahead() = 0;

    virtual std::string getFilename() = 0;
    virtual unsigned long getSize() = 0;
    virtual unsigned long getPosition() = 0;
    virtual std::string getNick() = 0;
};

class DCC_Send_In : public DCC {
public:
    DCC_Send_In(const std::string& filename, const std::string& nick, unsigned long address, unsigned short port, unsigned long size = 0);
    virtual ~DCC_Send_In() { }

    void go_ahead();
    bool onReadData(Glib::IOCondition cond);
    void getUseableFilename(int i);

    virtual std::string getFilename() { return _filename; }
    virtual unsigned long getSize() { return _size; }
    virtual unsigned long getPosition() { return _pos; }
    virtual std::string getNick() { return _nick; }

    int _number_in_queue;

private:
    std::ofstream _outfile;
    std::string _filename;
    std::string _downloaddir;
    std::string _nick;
    unsigned long _address;
    unsigned short _port;
    unsigned long _size;
    unsigned long _pos;

    int fd;
    struct sockaddr_in sockaddr;
};

class DCC_Send_Out : public DCC {
public:
    DCC_Send_Out(const std::string& filename, const std::string& nick, ServerConnection* conn);
    virtual ~DCC_Send_Out() { }

    void go_ahead() { }
    bool onAccept(Glib::IOCondition cond);
    bool onSendData(Glib::IOCondition cond);

    int _number_in_queue;

    virtual std::string getFilename() { return _filename; }
    virtual unsigned long getSize() { return _size; }
    virtual unsigned long getPosition() { return _pos; }
    virtual std::string getNick() { return _nick; }

private:
    std::ifstream _infile;
    std::string _filename;
    std::string _localip;
    std::string _nick;

    int fd;
    int accept_fd;
    struct sockaddr_in sockaddr;
    struct sockaddr_in remoteaddr;
    unsigned long _pos;
    unsigned long _size;
};

class DCC_queue {
    std::map<int, DCC*> _dccs;
    int _count;
public:
    DCC_queue() : _count(0) { }

    bool do_dcc(int n);

    /*int addDccChat()
    {
        _dccs.push_back(d);
    }*/

    int addDccSendIn(const std::string& filename, const std::string& nick, unsigned long address, unsigned short port, unsigned long size);
    int addDccSendOut(const std::string& filename, const std::string& nick, ServerConnection *conn);

    void dccDone(int n);

};

#endif
