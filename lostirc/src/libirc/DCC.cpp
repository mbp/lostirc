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

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#define stat _stat
#endif

#include <iostream>
#include <sstream>
#include <glibmm/fileutils.h>
#include "ServerConnection.h"
#include "LostIRCApp.h"
#include "DCC.h"

void DCC::setStatus(Status s)
{
    _status = s;
    App->getDcc().statusChange(this);
}

DCC_Send_In::DCC_Send_In(const Glib::ustring& filename, const Glib::ustring& nick, unsigned long address, unsigned short port, unsigned long size)
: _outfile(), _filename(filename), _nick(nick), _address(address),
    _port(port), _size(size), _pos(0)
{
    setStatus(WAITING);
    _socket.on_connected.connect(SigC::slot(*this, &DCC_Send_In::on_connected));
    _socket.on_error.connect(SigC::slot(*this, &DCC_Send_In::on_connection_failed));
    _socket.on_data_pending.connect(SigC::slot(*this, &DCC_Send_In::onReadData));
    _downloaddir = Glib::ustring(App->home) + "/.lostirc/downloads/";
    #ifndef WIN32
    mkdir(_downloaddir.c_str(), 0700);
    #else
    mkdir(_downloaddir.c_str());
    #endif

    _filename = _downloaddir + _filename;

    if (Glib::file_test(_filename, Glib::FILE_TEST_EXISTS))
          getUseableFilename(1);
}

void DCC_Send_In::on_connected(Glib::IOCondition cond)
{
    FE::emit(FE::get(CLIENTMSG) << _("DCC connected. Receiving file..."), FE::CURRENT);
    setStatus(ONGOING);
}

void DCC_Send_In::on_connection_failed(const char *str)
{
    FE::emit(FE::get(CLIENTMSG) << _("Couldn't connect: ") << Util::convert_to_utf8(str), FE::CURRENT);
    setStatus(FAIL);
}

void DCC_Send_In::start()
{
    _outfile.open(_filename.c_str());

    _socket.connect(_address, _port);

    FE::emit(FE::get(CLIENTMSG) << _("Receiving from:") << _socket.getRemoteIP(), FE::CURRENT);
}

void DCC_Send_In::cancel()
{
    setStatus(STOPPED);
    _socket.disconnect();
    _outfile.close();
}

void DCC_Send_In::onReadData()
{
    #ifdef DEBUG
    App->log << "DCC_Send_In::onReadData(): reading.." << std::endl;
    #endif

    try {
        char buf[4096];
        int received = 0;
        if (_socket.receive(buf, 4095, received)) {
            _pos += received;

            if (_outfile.good())
                  _outfile.write(buf, received);

            int tmp = 0;
            unsigned long pos = htonl(_pos);
            _socket.send(reinterpret_cast<char *>(&pos), 4, tmp);


            #ifdef DEBUG
            App->log << "DCC_Send_In::onReadData(): _pos: " << _pos << std::endl;
            App->log << "DCC_Send_In::onReadData(): _size: " << _size << std::endl;
            #endif

            if (_pos >= _size) {
                #ifdef DEBUG
                App->log << "DCC_Send_In::onReadData(): done receiving!" << std::endl;
                #endif
                _outfile.close();
                FE::emit(FE::get(CLIENTMSG) << _("File received successfully:") << _filename, FE::CURRENT);
                setStatus(DONE);
                _socket.disconnect();
            }
        }

    } catch (SocketException &e) {
            FE::emit(FE::get(CLIENTMSG) << _("Couldn't receive: ") << Util::convert_to_utf8(e.what()), FE::CURRENT);
            setStatus(FAIL);

    } catch (SocketDisconnected &e) {
        FE::emit(FE::get(CLIENTMSG) << _("DCC connection closed."), FE::CURRENT);
        _socket.disconnect();
    }
}

void DCC_Send_In::getUseableFilename(int i)
{
    std::stringstream ss;
    Glib::ustring myint;
    ss << i;
    ss >> myint;
    Glib::ustring newfilename = _filename + "." + myint;
    if (Glib::file_test(newfilename, Glib::FILE_TEST_EXISTS))
          getUseableFilename(++i);
    else
          _filename = newfilename;
}

DCC_Send_Out::DCC_Send_Out(const Glib::ustring& filename, const Glib::ustring& nick, ServerConnection *conn)
    : _infile(), _filename(filename), _nick(nick), _pos(0)
{
    setStatus(WAITING);
    _socket.on_error.connect(SigC::slot(*this, &DCC_Send_Out::on_bind_failed));
    _socket.on_accepted_connection.connect(SigC::slot(*this, &DCC_Send_Out::onAccept));
    _socket.on_can_send_data.connect(SigC::slot(*this, &DCC_Send_Out::onSendData));
    Glib::ustring localip;

    struct stat st;
    stat(filename.c_str(), &st);

    _size = st.st_size;
    if (App->options.dccip->empty())
          localip = conn->getLocalIP();
    else
          localip = App->options.dccip;

    #ifdef DEBUG
    App->log << "DCC_Send_Out::DCC_Send_Out(): size: " << st.st_size << std::endl;
    App->log << "DCC_Send_Out::DCC_Send_Out(): ip: " << localip << std::endl;
    #endif

    if (_socket.bind(App->options.dccport)) {
        std::ostringstream ss;
        ss << "DCC SEND " << stripPath(_filename) << " " << ntohl(inet_addr(localip.c_str())) << " " << ntohs(_socket.getSockAddr().sin_port) << " " << _size;
        conn->sendCtcp(nick, ss.str());

        _infile.open(_filename.c_str());

        FE::emit(FE::get(CLIENTMSG) << _("DCC SEND request sent. Sending from:") << localip, FE::CURRENT);
    }

}

void DCC_Send_Out::cancel()
{
    setStatus(STOPPED);
    _socket.disconnect();
    _infile.close();
}

void DCC_Send_Out::on_bind_failed(const char *str)
{
    FE::emit(FE::get(CLIENTMSG) << _("Couldn't bind: ") << Util::convert_to_utf8(str), FE::CURRENT);
    setStatus(FAIL);
    // FIXME: add dcc-done?
}

void DCC_Send_Out::onAccept()
{
    FE::emit(FE::get(CLIENTMSG) << _("Connection accepted."), FE::CURRENT);
    setStatus(ONGOING);
}

void DCC_Send_Out::onSendData()
{
    #ifdef DEBUG
    App->log << "DCC_Send_Out::onSendData(): sending..." << std::endl;
    #endif
    char buf[1024];
    _infile.read(buf, sizeof(buf));

    int read_chars = _infile.gcount();

    try {
        int sent = 0;
        if (_socket.send(buf, read_chars, sent)) {
            _pos += read_chars;

            #ifdef DEBUG
            App->log << "DCC_Send_Out::onSendData(): sent: " << sent << std::endl;
            App->log << "DCC_Send_Out::onSendData(): _pos: " << _pos << std::endl;
            App->log << "DCC_Send_Out::onSendData(): _size: " << _size << std::endl;
            #endif

            if (_pos >= _size) {
                #ifdef DEBUG
                App->log << "DCC_Send_Out::onSendData(): done sending!" << std::endl;
                #endif
                _infile.close();
                FE::emit(FE::get(CLIENTMSG) << _("File sent successfully:") << _filename, FE::CURRENT);
                setStatus(DONE);
                _socket.disconnect();
            }

        }

    } catch (SocketException &e) {
            FE::emit(FE::get(CLIENTMSG) << _("Couldn't send: ") << Util::convert_to_utf8(e.what()), FE::CURRENT);
            setStatus(FAIL);
    }
}

bool DCC_queue::start_dcc(int n)
{
    std::map<int, DCC*>::const_iterator i = _dccs.find(n);
    if (i != _dccs.end()) {
        i->second->start();
        return true;
    }
    return false;
}

int DCC_queue::addDccSendIn(const Glib::ustring& filename, const Glib::ustring& nick, unsigned long address, unsigned short port, unsigned long size)
{
    if (size == 0) {
        FE::emit(FE::get(CLIENTMSG) << _("Incoming file has zero size. Sender:") << nick, FE::CURRENT);
        return 0;
    } else {
        DCC_Send_In *d = new DCC_Send_In(filename, nick, address, port, size);
        d->_number_in_queue = ++_count;
        _dccs[_count] = d;
        App->fe->newDCC(d);
        return _count;
    }
}

int DCC_queue::addDccSendOut(const Glib::ustring& file, const Glib::ustring& nick, ServerConnection *conn)
{
    Glib::ustring filename = expandHome(file);

    if (!Glib::file_test(filename.c_str(), Glib::FILE_TEST_EXISTS)) {
        FE::emit(FE::get(CLIENTMSG) << _("File not found: ") << filename, FE::CURRENT);
        return 0;
    } else {
        DCC_Send_Out *d = new DCC_Send_Out(filename, nick, conn);
        d->_number_in_queue = ++_count;
        _dccs[_count] = d;
        App->fe->newDCC(d);
        return _count;
    }
}

void DCC_queue::statusChange(DCC *dcc)
{
    App->fe->dccStatusChanged(dcc);
}

Glib::ustring expandHome(const Glib::ustring& str)
{
    if (!str.empty() && str.at(0) == '~') {
        Glib::ustring new_str = App->home + str.substr(1);
        return new_str;
    }
    return str;
}

Glib::ustring stripPath(const Glib::ustring& str)
{
    return str.substr(str.find_last_of('/') + 1);
}
