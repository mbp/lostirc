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

#include <iostream>
#include <string>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <glibmm/main.h>
#include <glibmm/convert.h>
#include "LostIRCApp.h"
#include "Socket.h"

using Glib::ustring;
using SigC::slot;

Socket::Socket()
    : fd(-1), resolve_pid(0)
{
    on_host_resolved.connect(SigC::slot(*this, &Socket::host_resolved));
}

Socket::~Socket()
{
    close();
}

void Socket::resolvehost(const ustring& host)
{
    int thepipe[2];
    hostname = host;
    if (pipe(thepipe) == -1) {
        on_error(strerror(errno));
        return;
    }

    resolve_pid = fork();

    if (resolve_pid == 0) {
        // child
        ::close(thepipe[0]); // close read-pipe
        struct hostent *he;
        if ((he = gethostbyname(host.c_str())) == NULL) {
            int size = 0;
            if (write(thepipe[1], &size, sizeof(int)) == -1)
                  std::cerr << _("Error writing to pipe: ") << strerror(errno) << std::endl;
        } else {
            struct in_addr ia = *reinterpret_cast<struct in_addr *>(he->h_addr_list[0]);

            int size = sizeof(struct in_addr);
            if (write(thepipe[1], &size, sizeof(int)) == -1 ||
                    write(thepipe[1], &ia, sizeof(struct in_addr)) == -1)
                  std::cerr << _("Error writing to pipe: ") << strerror(errno) << std::endl;

        }
        ::close(thepipe[1]);
        _exit(0);
    } else if (resolve_pid > 0) {
        // parent
        ::close(thepipe[1]); // close write-pipe

        Glib::signal_io().connect(
                SigC::bind(slot(*this, &Socket::on_host_resolve), thepipe[0]),
                thepipe[0],
                Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);

    } else {
        on_error(strerror(errno));
    }

}

bool Socket::on_host_resolve(Glib::IOCondition cond, int readpipe)
{
    static const int size_to_be_read = sizeof(int) + sizeof(struct in_addr);
    char buf[size_to_be_read];

    int bytes_read = read(readpipe, buf, size_to_be_read);

    if (bytes_read == -1) {
        on_error(strerror(errno));
    } else if (size_to_be_read != bytes_read) {
        sleep(1);
        int new_retval = read(readpipe, &buf[bytes_read], size_to_be_read - bytes_read);

        if (new_retval != -1) {
            bytes_read += new_retval;
            if (bytes_read != size_to_be_read) {
                on_error(_("An error occured while reading from pipe (Internal error 2)"));
            } else {
                // copy the struct we received into the sockaddr member
                memcpy(static_cast<void*>(&sockaddr.sin_addr),
                        static_cast<void*>(&buf[sizeof(int)]),
                        sizeof(struct in_addr));

                on_host_resolved();
            }
        } else {
            on_error(_("An error occured while reading from pipe (Internal error 3)"));
        }

    } else {
        // copy the struct we received into the sockaddr member
        memcpy(static_cast<void*>(&sockaddr.sin_addr),
                static_cast<void*>(&buf[sizeof(int)]),
                sizeof(struct in_addr));

        on_host_resolved();
    }

    ::close(readpipe);

    // wait for our child to exit (it probably already exit'ed, but we need
    // this to avoid defunct childs).
    if (resolve_pid != -1) {
        waitpid(resolve_pid, NULL, 0);
        resolve_pid = -1;
    }

    return false;
}

void Socket::connect(const ustring& host, int p)
{
    port = p;

    // Before connecting, we need to resolve the hostname. When the hostname
    // is resolved, host_resolved() will be called which then does the real
    // connect.
    resolvehost(host);
}

void Socket::connect(unsigned long address, int p)
{
    port = p;

    sockaddr.sin_addr.s_addr = htonl(address);
    host_resolved();
}

bool Socket::bind(int p)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    port = p;
    
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(sockaddr.sin_zero), '\0', 8);

    if (::bind(fd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(struct sockaddr)) == -1) {
        on_error(strerror(errno));
        return false;
    } else {
        socklen_t add_len = sizeof(struct sockaddr_in);
        getsockname(fd, reinterpret_cast<struct sockaddr *>(&sockaddr), &add_len);
        #ifdef DEBUG
        App->log << "Socket::bind(): new port: " << ntohs(sockaddr.sin_port) << std::endl;
        #endif

        listen(fd, 1);

        Glib::signal_io().connect(
                SigC::slot(*this, &Socket::accepted_connection),
                fd,
                Glib::IO_IN | Glib::IO_ERR | Glib::IO_OUT | Glib::IO_HUP | Glib::IO_NVAL);

        return true;
    }

}

bool Socket::accepted_connection(Glib::IOCondition cond)
{
    struct sockaddr_in remoteaddr;
    socklen_t size = sizeof(struct sockaddr_in);
    int accept_fd = accept(fd, reinterpret_cast<struct sockaddr *>(&remoteaddr), &size);

    ::close(fd);
    fd = accept_fd;

    signal_write = Glib::signal_io().connect(
                SigC::slot(*this, &Socket::can_send_data),
                fd,
                Glib::IO_OUT | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);

    on_accepted_connection();
    return false;
}

bool Socket::can_send_data(Glib::IOCondition cond)
{
    on_can_send_data();
    return true;
}

void Socket::host_resolved()
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        on_error(strerror(errno));

    setNonBlocking();

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    memset(&(sockaddr.sin_zero), '\0', 8); // zero the rest of the struct

    if (::connect(fd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(struct sockaddr)) == 0 || errno == EINPROGRESS) {
        // This is a temporary watch to see when we can write, when we can
        // write - we are (hopefully) connected
        Glib::signal_io().connect(
                SigC::slot(*this, &Socket::connected),
                fd,
                Glib::IO_OUT | Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_PRI | Glib::IO_NVAL);
    } else {
        on_error(strerror(errno));
    }
}

bool Socket::connected(Glib::IOCondition cond)
{
    // We can write to the socket, so we must be connected!
    on_connected(cond);

    // Watch for incoming data from now on
    signal_read = Glib::signal_io().connect(
            SigC::slot(*this, &Socket::data_pending),
            fd,
            Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);

    return false;
}

void Socket::disconnect()
{
    if (signal_write.connected())
          signal_write.disconnect();
    if (signal_read.connected())
          signal_read.disconnect();
    close();
}

bool Socket::data_pending(Glib::IOCondition)
{
    on_data_pending();
    return true;
}

bool Socket::send(const ustring& data)
{
    const std::string msg = Util::convert_from_utf8(data);

    if (msg.empty()) {
        FE::emit(FE::get(ERRORMSG) << _("Message not sent because of locale problems"), FE::CURRENT);
        return false;
    }

    #ifdef DEBUG
    App->log << ">> " << data << std::flush;
    #endif

    int tmp = 0;
    return send(msg.c_str(), msg.length(), tmp);
}

bool Socket::send(const char *buf, int len, int& sent)
{
    sent = ::send(fd, buf, len, 0);

    if (sent == -1) {
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
            throw(SocketException(strerror(errno)));
        }
    }
    return true;
}

const char * Socket::getLocalIP()
{
    struct sockaddr_in localaddr;
    socklen_t add_len = sizeof(struct sockaddr_in);
    getsockname(fd, reinterpret_cast<struct sockaddr *>(&localaddr), &add_len);
    return inet_ntoa(localaddr.sin_addr);
}

const char * Socket::getRemoteIP()
{
    return inet_ntoa(sockaddr.sin_addr);
}

bool Socket::receive(char *buf, int len, int& received)
{
    received = recv(fd, buf, len, 0);

    if (received == 0) throw SocketDisconnected();
    else if (received == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return false;
        } else {
            throw SocketException(strerror(errno));
        }
    }
    buf[received] = '\0';

    return true;
}

// FIXME: the return value of this function is always 0.
int Socket::close()
{
    if (fd != -1) {
        ::close(fd);
        fd = -1;
    }
    return 0;
}

void Socket::setNonBlocking()
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
          on_error(strerror(errno));
    else
          if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
                on_error(strerror(errno));
}

void Socket::setBlocking()
{
    fcntl(fd, F_SETFL, 0);
}
