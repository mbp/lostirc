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

#include <cerrno>
#include <cstdio>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "Socket.h"

using std::string;

Socket::Socket()
{
}

Socket::~Socket()
{
    close();
}

void Socket::connect(const string& host, int port)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    setNonBlocking();

    struct hostent *he;
    if ((he = gethostbyname(host.c_str())) == NULL) {
        throw SocketException(strerror(errno));
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(addr.sin_zero), '\0', 8); // zero the rest of the struct

    if (::connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == 0 || errno == EINPROGRESS) {
        return;
    } else {
        throw SocketException(strerror(errno));
    }
}

void Socket::disconnect()
{
    close();
}

bool Socket::send(const string& data)
{
    if (::send(fd, data.c_str(), data.length(), 0) > 0) {
        #ifdef DEBUG
        std::cout << ">> " << data << std::endl;
        #endif
        return true;
    } else {
        return false;
    }
}

bool Socket::receive(char *buf, int len)
{
    int retval = recv(fd, buf, len, 0);

    if (retval == 0) throw SocketException("Disconnected.");
    else if (retval == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return false;
        } else {
            throw SocketException(strerror(errno));
        }
    }
    buf[retval] = '\0';

    return true;
}

int Socket::close()
{
    return ::close(fd);
}

void Socket::setNonBlocking()
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1)
          fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Socket::setBlocking()
{
    fcntl(fd, F_SETFL, 0);
}
