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

#include <iostream>
#include <sstream>
#include "FrontEnd.h"
#include "ServerConnection.h"
#include "DCC.h"

DCC_Send_In::DCC_Send_In(const std::string& filename, unsigned long address, unsigned short port, unsigned long size)
: _outfile(), _filename(filename), _address(address), _port(port),
  _size(size), _pos(0)
{
    _downloaddir = std::string(App->home) + "/.lostirc/downloads/";
    mkdir(_downloaddir.c_str(), 0700);

    _filename = _downloaddir + _filename;

    struct stat st;
    if (stat(_filename.c_str(), &st) == 0) {
        getUseableFilename(1);
    }
}

void DCC_Send_In::go_ahead()
{
    _outfile.open(_filename.c_str());
    fd = socket(AF_INET, SOCK_STREAM, 0);

    // nonblocking
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1)
          fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(_port);
    sockaddr.sin_addr.s_addr = htonl(_address);
    memset(&(sockaddr.sin_zero), '\0', 8);

    FE::emit(FE::get(SERVMSG2) << "Receiving from:" << inet_ntoa(sockaddr.sin_addr), FE::CURRENT);

    if (::connect(fd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(struct sockaddr)) < 0 && errno != EINPROGRESS) {
        FE::emit(FE::get(SERVMSG2) << "Couldn't connect:" << strerror(errno), FE::CURRENT);
        App->getDcc().dccDone(_number_in_queue);
    }

    _watchid = g_io_add_watch(g_io_channel_unix_new(fd),
            GIOCondition (G_IO_IN | G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
            &DCC_Send_In::onReadData, this);
}

gboolean DCC_Send_In::onReadData(GIOChannel* io_channel, GIOCondition cond, gpointer data)
{
    DCC_Send_In& dcc = *(static_cast<DCC_Send_In*>(data));

    char buf[4096];
    int retval = recv(dcc.fd, buf, sizeof(buf), 0);

    if (retval == 0) FE::emit(FE::get(SERVMSG) << "DCC connection closed.", FE::CURRENT);
    else if (retval == -1) {
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
            FE::emit(FE::get(SERVMSG2) << "Couldn't receive:" << strerror(errno), FE::CURRENT);
            App->getDcc().dccDone(dcc._number_in_queue);
            return FALSE;
        }
    } else {
        dcc._pos += retval;

        if (dcc._outfile.good())
              dcc._outfile.write(buf, retval);

        unsigned long pos = htonl(dcc._pos);
        send(dcc.fd, (char *)&pos, 4, 0);

        #ifdef DEBUG
        App->log << "DCC_Send_In::onReadData(): _pos: " << dcc._pos << std::endl;
        App->log << "DCC_Send_In::onReadData(): _size: " << dcc._size << std::endl;
        #endif

        if (dcc._pos >= dcc._size) {
            #ifdef DEBUG
            App->log << "DCC_Send_In::onReadData(): done receiving!" << std::endl;
            #endif
            dcc._outfile.close();
            FE::emit(FE::get(SERVMSG2) << "File received successfully:" << dcc._filename, FE::CURRENT);
            App->getDcc().dccDone(dcc._number_in_queue);
            return FALSE;
        }
    }

    return TRUE;
}

void DCC_Send_In::getUseableFilename(int i)
{
    struct stat st;
    std::stringstream ss;
    std::string myint;
    ss << i;
    ss >> myint;
    std::string newfilename = _filename + "." + myint;
    if (stat(newfilename.c_str(), &st) == 0)
          getUseableFilename(++i);
    else
          _filename = newfilename;
}

DCC_Send_Out::DCC_Send_Out(const std::string& filename, const std::string& nick, ServerConnection *conn)
    : _infile(), _filename(filename), _pos(0)
{
    struct stat st;

    if (stat(filename.c_str(), &st) == -1) {
        FE::emit(FE::get(SERVMSG2) << "File not found:" << filename, FE::CURRENT);
        // FIXME: add dcc-done?
    } else {
        _size = st.st_size;
        if (App->options.dccip->empty()) {
            _localip = conn->getLocalIP();
        } else {
            _localip = App->options.dccip;
        }


        #ifdef DEBUG
        App->log << "DCC_Send_Out::DCC_Send_Out(): size: " << st.st_size << std::endl;
        App->log << "DCC_Send_Out::DCC_Send_Out(): ip: " << _localip << std::endl;
        #endif

        fd = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(0);
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        memset(&(sockaddr.sin_zero), '\0', 8);

        if (bind(fd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(struct sockaddr)) == -1) {
            FE::emit(FE::get(SERVMSG2) << "Couldn't bind:" << strerror(errno), FE::CURRENT);
        }
        socklen_t add_len = sizeof(struct sockaddr_in);
        getsockname(fd, (struct sockaddr *) &sockaddr, &add_len);

        #ifdef DEBUG
        App->log << "DCC_Send_Out::DCC_Send_Out(): new port: " << ntohs(sockaddr.sin_port) << std::endl;
        #endif

        std::ostringstream ss;
        ss << "DCC SEND " << filename << " " << ntohl(inet_addr(_localip.c_str())) << " " << ntohs(sockaddr.sin_port) << " " << _size;
        conn->sendCtcp(nick, ss.str());

        _infile.open(filename.c_str());

        listen(fd, 1);

        FE::emit(FE::get(SERVMSG2) << "DCC SEND request sent. Sending from:" << _localip, FE::CURRENT);

        g_io_add_watch(g_io_channel_unix_new(fd),
                GIOCondition (G_IO_IN | G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
                &DCC_Send_Out::onAccept, this);

    }

}

gboolean DCC_Send_Out::onAccept(GIOChannel* io_channel, GIOCondition cond, gpointer data)
{
    DCC_Send_Out& dcc = *(static_cast<DCC_Send_Out*>(data));

    socklen_t size = sizeof(struct sockaddr_in);
    dcc.accept_fd = accept(dcc.fd, reinterpret_cast<struct sockaddr *>(&dcc.remoteaddr), &size);

    FE::emit(FE::get(SERVMSG) << "Connection accepted.", FE::CURRENT);

    g_io_add_watch(g_io_channel_unix_new(dcc.accept_fd),
            GIOCondition (G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
            &DCC_Send_Out::onSendData, &dcc);

    close(dcc.fd);
    return FALSE;
}

gboolean DCC_Send_Out::onSendData(GIOChannel* io_channel, GIOCondition cond, gpointer data)
{
    DCC_Send_Out& dcc = *(static_cast<DCC_Send_Out*>(data));

    char buf[1024];
    dcc._infile.read(buf, sizeof(buf));

    int read_chars = dcc._infile.gcount();

    int retval = send(dcc.accept_fd, buf, read_chars, 0);
    if (retval == -1) {
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
            FE::emit(FE::get(SERVMSG2) << "Couldn't send:" << strerror(errno), FE::CURRENT);
            App->getDcc().dccDone(dcc._number_in_queue);
            return FALSE;
        }
    } else {
        dcc._pos += read_chars;

        #ifdef DEBUG
        App->log << "DCC_Send_Out::onSendData(): retval: " << retval << std::endl;
        App->log << "DCC_Send_Out::onSendData(): _pos: " << dcc._pos << std::endl;
        App->log << "DCC_Send_Out::onSendData(): _size: " << dcc._size << std::endl;
        #endif

        if (dcc._pos >= dcc._size) {
            #ifdef DEBUG
            App->log << "DCC_Send_Out::onSendData(): done sending!" << std::endl;
            #endif
            dcc._infile.close();
            FE::emit(FE::get(SERVMSG2) << "File sent successfully:" << dcc._filename, FE::CURRENT);
            App->getDcc().dccDone(dcc._number_in_queue);
            return FALSE;
        }

    }

    return TRUE;
}

bool DCC_queue::do_dcc(int n)
{
    std::map<int, DCC*>::const_iterator i = _dccs.find(n);
    if (i != _dccs.end()) {
        i->second->go_ahead();
        return true;
    }
    return false;
}

int DCC_queue::addDccSendIn(const std::string& filename, unsigned long address, unsigned short port, unsigned long size)
{
    DCC_Send_In *d = new DCC_Send_In(filename, address, port, size);
    d->_number_in_queue = ++_count;
    _dccs[_count] = d;
    return _count;
}

int DCC_queue::addDccSendOut(const std::string& filename, const std::string& nick, ServerConnection *conn)
{
    DCC_Send_Out *d = new DCC_Send_Out(filename, nick, conn);
    d->_number_in_queue = ++_count;
    _dccs[_count] = d;
    return _count;
}

void DCC_queue::dccDone(int n)
{
    std::map<int, DCC*>::iterator i = _dccs.find(n);
    if (i != _dccs.end()) {
        delete i->second;
        _dccs.erase(i);
    }
}
