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

#ifndef SOCKET_H
#define SOCKET_H

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <exception>
#include <sigc++/sigc++.h>
#include <glibmm/main.h>

class Socket : public SigC::Object
{
    int port;
    int fd;
    struct sockaddr_in sockaddr;
    pid_t resolve_pid;
    Glib::ustring hostname;

    void resolvehost(const Glib::ustring& host);

    void host_resolved();
    bool on_host_resolve(Glib::IOCondition cond, int readpipe);
    bool connected(Glib::IOCondition cond);
    bool data_pending(Glib::IOCondition);
    bool accepted_connection(Glib::IOCondition);
    bool can_send_data(Glib::IOCondition);

    SigC::Connection signal_write;
    SigC::Connection signal_read;

public:
    Socket();
    ~Socket();

    void connect(const Glib::ustring& hostname, int port);
    void connect(unsigned long address, int port);
    bool bind(int port);
    void disconnect();
    bool send(const Glib::ustring& data);
    bool send(const char *buf, int len, int& received);
    bool receive(char *buf, int len, int& received);
    void setNonBlocking();
    void setBlocking();
    int getfd() const { return fd; }
    int close();
    const char * getLocalIP();
    const char * getRemoteIP();
    sockaddr_in& getSockAddr() { return sockaddr; };

    SigC::Signal0<void> on_host_resolved;
    SigC::Signal1<void, Glib::IOCondition> on_connected;
    SigC::Signal0<void> on_data_pending;
    SigC::Signal0<void> on_can_send_data;
    SigC::Signal0<void> on_accepted_connection;

    SigC::Signal1<void, const char *> on_error;
};

class SocketException : public std::exception
{
    const char *error;
public:
    SocketException(const char *e) : error(e) { }
    const char * what() const throw() {
        return error;
    }
};

class SocketDisconnected : public std::exception { };

#endif
