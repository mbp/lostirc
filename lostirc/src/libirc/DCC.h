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
#include <glib.h>

class ServerConnection;

class DCC {
public:
    virtual void go_ahead() = 0;
};

class DCC_Send_In : public DCC {
public:
    DCC_Send_In(const std::string& filename, unsigned long address, unsigned short port, unsigned long size = 0);
    virtual ~DCC_Send_In() { }

    void go_ahead();
    static gboolean onReadData(GIOChannel* io_channel, GIOCondition cond, gpointer data);
    void getUseableFilename(int i);

    int _number_in_queue;

private:
    std::ofstream _outfile;
    std::string _filename;
    std::string _downloaddir;
    unsigned long _address;
    unsigned short _port;
    unsigned long _size;
    unsigned long _pos;

    int fd;
    struct sockaddr_in sockaddr;
    guint _watchid;
};

class DCC_Send_Out : public DCC {
public:
    DCC_Send_Out(const std::string& filename, const std::string& nick, ServerConnection* conn);
    virtual ~DCC_Send_Out() { }

    void go_ahead() { }
    static gboolean onAccept(GIOChannel* io_channel, GIOCondition cond, gpointer data);
    static gboolean onSendData(GIOChannel* io_channel, GIOCondition cond, gpointer data);

    int _number_in_queue;

private:
    std::ifstream _infile;
    std::string _filename;
    std::string _localip;

    int fd;
    int accept_fd;
    struct sockaddr_in sockaddr;
    struct sockaddr_in remoteaddr;
    unsigned long _pos;
    unsigned long _size;
    guint _watchid;
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

    int addDccSendIn(const std::string& filename, unsigned long address, unsigned short port, unsigned long size);
    int addDccSendOut(const std::string& filename, const std::string& nick, ServerConnection *conn);

    void dccDone(int n);

};

#endif
