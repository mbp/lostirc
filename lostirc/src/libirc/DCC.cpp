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
    if (stat(_filename.c_str(), &st) == 0)
          getUseableFilename(1);
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

    FE::emit(FE::get(SERVERMSG1) << "Receiving from:" << inet_ntoa(sockaddr.sin_addr), FE::CURRENT);

    if (::connect(fd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(struct sockaddr)) < 0 && errno != EINPROGRESS) {
        FE::emit(FE::get(SERVERMSG1) << "Couldn't connect:" << strerror(errno), FE::CURRENT);
        App->getDcc().dccDone(_number_in_queue);
    }

    Glib::signal_io().connect(
            SigC::slot(*this, &DCC_Send_In::onReadData),
            fd,
            Glib::IO_IN | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);
}

bool DCC_Send_In::onReadData(Glib::IOCondition cond)
{
    char buf[4096];
    int retval = recv(fd, buf, sizeof(buf), 0);

    if (retval == 0) FE::emit(FE::get(SERVERMSG1) << "DCC connection closed.", FE::CURRENT);
    else if (retval == -1) {
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
            FE::emit(FE::get(SERVERMSG1) << "Couldn't receive:" << strerror(errno), FE::CURRENT);
            App->getDcc().dccDone(_number_in_queue);
            return false;
        }
    } else {
        _pos += retval;

        if (_outfile.good())
              _outfile.write(buf, retval);

        unsigned long pos = htonl(_pos);
        send(fd, (char *)&pos, 4, 0);

        #ifdef DEBUG
        App->log << "DCC_Send_In::onReadData(): _pos: " << _pos << std::endl;
        App->log << "DCC_Send_In::onReadData(): _size: " << _size << std::endl;
        #endif

        if (_pos >= _size) {
            #ifdef DEBUG
            App->log << "DCC_Send_In::onReadData(): done receiving!" << std::endl;
            #endif
            _outfile.close();
            FE::emit(FE::get(SERVERMSG1) << "File received successfully:" << _filename, FE::CURRENT);
            App->getDcc().dccDone(_number_in_queue);
            return false;
        }
    }

    return true;
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
        FE::emit(FE::get(SERVERMSG1) << "File not found:" << filename, FE::CURRENT);
        // FIXME: add dcc-done?
    } else {
        _size = st.st_size;
        if (App->options.dccip->empty())
              _localip = conn->getLocalIP();
        else
              _localip = App->options.dccip;

        #ifdef DEBUG
        App->log << "DCC_Send_Out::DCC_Send_Out(): size: " << st.st_size << std::endl;
        App->log << "DCC_Send_Out::DCC_Send_Out(): ip: " << _localip << std::endl;
        #endif

        fd = socket(AF_INET, SOCK_STREAM, 0);

        int yes = 1;
        setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));

        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(App->options.dccport);
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        memset(&(sockaddr.sin_zero), '\0', 8);

        if (bind(fd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(struct sockaddr)) == -1) {
            FE::emit(FE::get(SERVERMSG1) << "Couldn't bind:" << strerror(errno), FE::CURRENT);
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

        FE::emit(FE::get(SERVERMSG1) << "DCC SEND request sent. Sending from:" << _localip, FE::CURRENT);

        Glib::signal_io().connect(
                SigC::slot(*this, &DCC_Send_Out::onAccept),
                fd,
                Glib::IO_IN | Glib::IO_ERR | Glib::IO_OUT | Glib::IO_HUP | Glib::IO_NVAL);

    }

}

bool DCC_Send_Out::onAccept(Glib::IOCondition cond)
{
    socklen_t size = sizeof(struct sockaddr_in);
    accept_fd = accept(fd, reinterpret_cast<struct sockaddr *>(&remoteaddr), &size);

    FE::emit(FE::get(SERVERMSG1) << "Connection accepted.", FE::CURRENT);

    Glib::signal_io().connect(
            SigC::slot(*this, &DCC_Send_Out::onSendData),
            accept_fd,
            Glib::IO_OUT | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);

    close(fd);
    return false;
}

bool DCC_Send_Out::onSendData(Glib::IOCondition cond)
{
    char buf[1024];
    _infile.read(buf, sizeof(buf));

    int read_chars = _infile.gcount();

    int retval = send(accept_fd, buf, read_chars, 0);
    if (retval == -1) {
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
            FE::emit(FE::get(SERVERMSG1) << "Couldn't send:" << strerror(errno), FE::CURRENT);
            App->getDcc().dccDone(_number_in_queue);
            return false;
        }
    } else {
        _pos += read_chars;

        #ifdef DEBUG
        App->log << "DCC_Send_Out::onSendData(): retval: " << retval << std::endl;
        App->log << "DCC_Send_Out::onSendData(): _pos: " << _pos << std::endl;
        App->log << "DCC_Send_Out::onSendData(): _size: " << _size << std::endl;
        #endif

        if (_pos >= _size) {
            #ifdef DEBUG
            App->log << "DCC_Send_Out::onSendData(): done sending!" << std::endl;
            #endif
            _infile.close();
            FE::emit(FE::get(SERVERMSG1) << "File sent successfully:" << _filename, FE::CURRENT);
            App->getDcc().dccDone(_number_in_queue);
            return false;
        }

    }

    return true;
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
