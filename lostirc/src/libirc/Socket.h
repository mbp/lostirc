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

#ifndef WIN32
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif

#include <exception>
#include <sigc++/sigc++.h>
#include <glibmm/main.h>

class Socket : public SigC::Object
{
    // Data
    #ifndef WIN32
    int fd;
    pid_t resolve_pid;
    #else
    SOCKET fd;
    #endif
    int port;
    struct sockaddr_in sockaddr;

    // Host resolving
    void resolvehost(const Glib::ustring& host);
    void host_resolved();
    #ifndef WIN32
    bool on_host_resolve(Glib::IOCondition cond, int readpipe);
    #endif

    // Signal listeners
    bool connected(Glib::IOCondition cond);
    bool data_pending(Glib::IOCondition);
    bool accepted_connection(Glib::IOCondition);
    bool can_send_data(Glib::IOCondition);

    // Signal connections
    SigC::Connection signal_write;
    SigC::Connection signal_read;

public:
    Socket();
    ~Socket();

    // Connection
    void connect(const Glib::ustring& hostname, int port);
    void connect(unsigned long address, int port);
    void disconnect();
    int  close();

    bool bind(int port);

    // send/recv
    bool send(const Glib::ustring& data);
    bool send(const char *buf, int len, int& received);
    bool receive(char *buf, int len, int& received);

    // blocking config
    void setNonBlocking();
    void setBlocking();

    // data abstraction
    int          getfd() const { return fd; }
    sockaddr_in& getSockAddr() { return sockaddr; };
    const char*  getLocalIP();
    const char*  getRemoteIP();  

    // Signals
    SigC::Signal0<void> on_host_resolved;
    SigC::Signal0<void> on_data_pending;
    SigC::Signal0<void> on_can_send_data;
    SigC::Signal0<void> on_accepted_connection;
    SigC::Signal1<void, Glib::IOCondition> on_connected;
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
