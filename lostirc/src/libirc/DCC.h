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

#ifndef DCC_H
#define DCC_H

#include <map>
#include <fstream>
#include <cerrno>
#include <cstdio>
#include <glibmm/main.h>
#include <sigc++/sigc++.h>
#include "Socket.h"

class ServerConnection;

class DCC : public sigc::trackable {

public:
    enum Status { DONE, ONGOING, WAITING, STOPPED, FAIL };
    virtual ~DCC() { }

    virtual void start() = 0;
    virtual void cancel() = 0;
    void setStatus(Status s);

    virtual Glib::ustring getFilename() const = 0;
    virtual unsigned long getSize() const = 0;
    virtual unsigned long getPosition() const = 0;
    virtual Glib::ustring getNick() const = 0;
    virtual Status getStatus() const { return _status; }

    int _number_in_queue;
protected:
    Status _status;
};

class DCC_Send_In : public DCC {
public:
    DCC_Send_In(const Glib::ustring& filename, const Glib::ustring& nick, unsigned long address, unsigned short port, unsigned long size = 0);
    virtual ~DCC_Send_In() { }

    void start();
    void cancel();
    void onReadData();
    void getUseableFilename(int i);

    void on_connected(Glib::IOCondition cond);
    void on_connection_failed(const char *str);

    virtual Glib::ustring getFilename() const { return _filename; }
    virtual unsigned long getSize() const { return _size; }
    virtual unsigned long getPosition() const { return _pos; }
    virtual Glib::ustring getNick() const { return _nick; }

private:
    std::ofstream _outfile;
    Glib::ustring _filename;
    Glib::ustring _downloaddir;
    Glib::ustring _nick;
    unsigned long _address;
    unsigned short _port;
    unsigned long _size;
    unsigned long _pos;

    Socket _socket;
};

class DCC_Send_Out : public DCC {
public:
    DCC_Send_Out(const Glib::ustring& filename, const Glib::ustring& nick, ServerConnection* conn);
    virtual ~DCC_Send_Out() { }

    void start() { }
    void cancel();
    void onAccept();
    void onSendData();

    void on_bind_failed(const char *str);

    virtual Glib::ustring getFilename() const { return _filename; }
    virtual unsigned long getSize() const { return _size; }
    virtual unsigned long getPosition() const { return _pos; }
    virtual Glib::ustring getNick() const { return _nick; }

private:
    std::ifstream _infile;
    Glib::ustring _filename;
    Glib::ustring _nick;

    unsigned long _pos;
    unsigned long _size;

    Socket _socket;
};

class DCC_queue {
    std::map<int, DCC*> _dccs;
    int _count;
public:
    DCC_queue() : _count(0) { }

    bool start_dcc(int n);

    int addDccSendIn(const Glib::ustring& filename, const Glib::ustring& nick, unsigned long address, unsigned short port, unsigned long size);
    int addDccSendOut(const Glib::ustring& filename, const Glib::ustring& nick, ServerConnection *conn);

    void statusChange(DCC *dcc);

};

Glib::ustring expandHome(const Glib::ustring& str);
Glib::ustring stripPath(const Glib::ustring& str);

#endif
