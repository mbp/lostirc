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
#ifndef WIN32
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#endif
#include <glibmm/main.h>
#include <glibmm/convert.h>
#include "LostIRCApp.h"
#include "Socket.h"

using Glib::ustring;
using SigC::bind;
using SigC::slot;


Socket::Socket()
    : fd(-1), resolve_pid(-1)
{
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
            struct in_addr ia = *(struct in_addr *)he->h_addr_list[0];

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
                bind(slot(*this, &Socket::on_host_resolve), thepipe[0]),
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

void Socket::connect(int port)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        throw SocketException(strerror(errno));

    setNonBlocking();

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    memset(&(sockaddr.sin_zero), '\0', 8); // zero the rest of the struct

    if (::connect(fd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(struct sockaddr)) == 0 || errno == EINPROGRESS) {
        return;
    } else {
        throw SocketException(strerror(errno));
    }
}

void Socket::disconnect()
{
    close();
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
    if (::send(fd, msg.c_str(), msg.length(), 0) > 0) {
        return true;
    } else {
        #ifdef DEBUG
        App->log << " -- send failed! (" << strerror(errno) << ")" << std::endl;
        #endif
        return false;
    }
}

const char * Socket::getLocalIP()
{
    socklen_t add_len = sizeof(struct sockaddr_in);
    getsockname(fd, (struct sockaddr *) &localaddr, &add_len);
    return inet_ntoa(localaddr.sin_addr);
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
