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
    // Get us a file descriptor
    fd = socket(AF_INET, SOCK_STREAM, 0);
}

Socket::~Socket()
{
    close();
}

bool Socket::connect(const string& host, int port)
{
    struct hostent *he;
    if ((he = gethostbyname(host.c_str())) == NULL) {
        error = strerror(errno);
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(addr.sin_zero), '\0', 8); // zero the rest of the struct

    if (::connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
        error = strerror(errno);
        return false;
    }

    return true;
}

bool Socket::send(const string& data)
{
    int size = data.length();
    const char *msg;

    msg = data.c_str();

    if (::send(fd, msg, size, 0) > 0) {
        #ifdef DEBUG
        std::cout << ">> " << msg << std::endl;
        #endif
        return true;
    } else {
        return false;
    }

}

string Socket::receive()
{
    if (isBlocking)
          isBlocking = false;
    else
          buf = ""; // FIXME: why isn't there a clear() on gcc 2.95.4?

    while (1)
    {
        char r;

        int i = recv(fd, &r, 1, 0);

        switch(i) {
            case 0:
                std::cerr << "0.. returning." << std::endl;
                error = "Disconnected.";
                return "";
            case -1:
                if (errno == EAGAIN) {
                    // It's just blocking. Return false and set isBlocking
                    // to true
                    isBlocking = true;
                    return buf;
                } else {
                    error = "Disconnected.";
                    std::cerr << error << std::endl;
                    return "";
                }
        }

        buf += r;

        if (r == '\n')
              return buf;
    }
}

int Socket::getfd()
{
    return fd;
}

int Socket::close()
{
    return ::close(fd);
}

void Socket::setNonBlocking()
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
}

void Socket::setBlocking()
{
    fcntl(fd, F_SETFL, 0);
}
