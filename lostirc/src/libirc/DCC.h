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

#ifndef DCC_H
#define DCC_H

#include <map>
#include <fstream>
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

    enum Status { DONE, ONGOING, WAITING, STOPPED, ERROR };

    virtual void go_ahead() = 0;

    virtual Glib::ustring getFilename() const = 0;
    virtual unsigned long getSize() const = 0;
    virtual unsigned long getPosition() const = 0;
    virtual Glib::ustring getNick() const = 0;
    virtual Status getStatus() const = 0;
};

class DCC_Send_In : public DCC {
public:
    DCC_Send_In(const Glib::ustring& filename, const Glib::ustring& nick, unsigned long address, unsigned short port, unsigned long size = 0);
    virtual ~DCC_Send_In() { }

    void go_ahead();
    bool onReadData(Glib::IOCondition cond);
    void getUseableFilename(int i);

    virtual Glib::ustring getFilename() const { return _filename; }
    virtual unsigned long getSize() const { return _size; }
    virtual unsigned long getPosition() const { return _pos; }
    virtual Glib::ustring getNick() const { return _nick; }
    virtual Status getStatus() const { return _status; }

    int _number_in_queue;

private:
    std::ofstream _outfile;
    Glib::ustring _filename;
    Glib::ustring _downloaddir;
    Glib::ustring _nick;
    unsigned long _address;
    unsigned short _port;
    unsigned long _size;
    unsigned long _pos;

    Status _status;

    int fd;
    struct sockaddr_in sockaddr;
};

class DCC_Send_Out : public DCC {
public:
    DCC_Send_Out(const Glib::ustring& filename, const Glib::ustring& nick, ServerConnection* conn);
    virtual ~DCC_Send_Out() { }

    void go_ahead() { }
    bool onAccept(Glib::IOCondition cond);
    bool onSendData(Glib::IOCondition cond);

    int _number_in_queue;

    virtual Glib::ustring getFilename() const { return _filename; }
    virtual unsigned long getSize() const { return _size; }
    virtual unsigned long getPosition() const { return _pos; }
    virtual Glib::ustring getNick() const { return _nick; }
    virtual Status getStatus() const { return _status; }

private:
    std::ifstream _infile;
    Glib::ustring _filename;
    Glib::ustring _nick;

    int fd;
    int accept_fd;
    struct sockaddr_in sockaddr;
    struct sockaddr_in remoteaddr;
    unsigned long _pos;
    unsigned long _size;

    Status _status;
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

    int addDccSendIn(const Glib::ustring& filename, const Glib::ustring& nick, unsigned long address, unsigned short port, unsigned long size);
    int addDccSendOut(const Glib::ustring& filename, const Glib::ustring& nick, ServerConnection *conn);

    void statusChange(int n);

};

Glib::ustring expandHome(const Glib::ustring& str);
Glib::ustring stripPath(const Glib::ustring& str);

#endif
