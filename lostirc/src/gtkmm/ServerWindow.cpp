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

#include <sstream>
#include <gtkmm/separator.h>
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <Utils.h>
#include "MainWindow.h"
#include "ServerWindow.h"

using std::vector;

Gtk::Button* create_imagebutton(const Glib::ustring& str, const Gtk::StockID& stock_id)
{
    Gtk::Button *button = new Gtk::Button();
    Gtk::HBox *hbox = manage(new Gtk::HBox());
    hbox->pack_start(*manage(new Gtk::Image(stock_id, Gtk::ICON_SIZE_MENU)));
    hbox->pack_start(*manage(new Gtk::Label(str, true)));
    button->add(*hbox);
    return button;
}

ServerWindow::ServerWindow(Gtk::Window& parent)
    : Gtk::Dialog(_("LostIRC Server Window"), parent),
    _columns(),
    _liststore(Gtk::ListStore::create(_columns)),
    _treeview(_liststore)
{
    set_border_width(5);
    get_vbox()->set_spacing(18);

    // Autojoin/perform-tab

    _treeview.append_column(_("Hostname"), _columns.servername);
    _treeview.append_column(_("Port"), _columns.port);
    _treeview.append_column(_("Auto-connect"), _columns.auto_connect);

    updateList();

    // Button box.
    Gtk::HButtonBox *buttbox = manage(new Gtk::HButtonBox());
    buttbox->set_spacing(6);
    Gtk::Button *addbutton = manage(new Gtk::Button(Gtk::Stock::ADD));
    addbutton->signal_clicked().connect(slot(*this, &ServerWindow::addEntry));
    Gtk::Button *modifybutton = manage(create_imagebutton(_("_Modify"), Gtk::Stock::PREFERENCES));
    modifybutton->signal_clicked().connect(slot(*this, &ServerWindow::modifyEntry));
    Gtk::Button *deletebutton = manage(new Gtk::Button(Gtk::Stock::DELETE));
    deletebutton->signal_clicked().connect(slot(*this, &ServerWindow::deleteEntry));
    Gtk::Button *closebutton = manage(new Gtk::Button(Gtk::Stock::CLOSE));
    closebutton->signal_clicked().connect(slot(*this, &Gtk::Dialog::hide));

    buttbox->pack_end(*addbutton, Gtk::PACK_SHRINK);
    buttbox->pack_end(*modifybutton, Gtk::PACK_SHRINK);
    buttbox->pack_end(*deletebutton, Gtk::PACK_SHRINK);
    buttbox->pack_end(*closebutton, Gtk::PACK_SHRINK);

    get_vbox()->pack_end(*buttbox, Gtk::PACK_SHRINK);
    get_vbox()->pack_end(*manage(new Gtk::HSeparator()), Gtk::PACK_EXPAND_PADDING);
    get_vbox()->pack_end(_treeview, Gtk::PACK_SHRINK);
    show_all();
}

void ServerWindow::updateList()
{
    _liststore->clear();
    vector<Server*> servers = App->cfgservers.getServers();
    vector<Server*>::const_iterator i;

    for (i = servers.begin(); i != servers.end(); ++i) {
        Gtk::TreeModel::Row row = *_liststore->append();
        row[_columns.servername] = (*i)->hostname;
        row[_columns.port] = (*i)->port;
        row[_columns.auto_connect] = (*i)->auto_connect;
        row[_columns.serverptr] = *i;
    }
}

void ServerWindow::addEntry()
{
    Server *server = new Server;

    ServerEditDialog dialog(*this, server);
    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) {
        App->cfgservers.addServer(server);
        updateList();
    } else {
        delete server;
    }
}

void ServerWindow::modifyEntry()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
    Gtk::TreeModel::iterator iterrow = selection->get_selected();

    if (iterrow) {
        Gtk::TreeModel::Row row = *iterrow;
        ServerEditDialog dialog(*this, row[_columns.serverptr]);

        int result = dialog.run();

        if (result == Gtk::RESPONSE_OK)
            updateList();
    }
}

void ServerWindow::deleteEntry()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
    Gtk::TreeModel::iterator iterrow = selection->get_selected();

    if (iterrow) {
        Gtk::TreeModel::Row row = *selection->get_selected();

        Server *server = row[_columns.serverptr];
        _liststore->erase(selection->get_selected());
        App->cfgservers.removeServer(server);
        App->cfgservers.writeServersFile();
    }
}

ServerEditDialog::ServerEditDialog(Gtk::Window& parent, Server* server)
    : Gtk::Dialog(_("LostIRC Server Edit"), parent),
    auto_connect_button(_("_Connect automatically"), true),
    _server_options_table(2, 4),
    _server(server)
{
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    set_border_width(5);
    get_vbox()->set_spacing(6);
    _server_options_table.set_row_spacings(6);
    _server_options_table.set_col_spacings(12);

    // auto connect
    get_vbox()->pack_start(auto_connect_button, Gtk::PACK_SHRINK);

    get_vbox()->pack_start(_server_options_table, Gtk::PACK_SHRINK);

    int row = 1;

    // hostname
    Gtk::Label *label1 = manage(new Gtk::Label(_("Hostname:"), Gtk::ALIGN_LEFT));
    _server_options_table.attach(*label1, 0, 1, row, row + 1);
    _server_options_table.attach(hostentry, 1, 2, row, row + 1);

    row++;

    // port
    Gtk::Label *label2 = manage(new Gtk::Label(_("Port:"), Gtk::ALIGN_LEFT));
    _server_options_table.attach(*label2, 0, 1, row, row + 1);
    _server_options_table.attach(portentry, 1, 2, row, row + 1);

    row++;

    // password
    Gtk::Label *label3 = manage(new Gtk::Label(_("Password:"), Gtk::ALIGN_LEFT));
    _server_options_table.attach(*label3, 0, 1, row, row + 1);
    _server_options_table.attach(passentry, 1, 2, row, row + 1);

    row++;

    // nick
    Gtk::Label *label4 = manage(new Gtk::Label(_("Nickname:"), Gtk::ALIGN_LEFT));
    _server_options_table.attach(*label4, 0, 1, row, row + 1);
    _server_options_table.attach(nickentry, 1, 2, row, row + 1);

    // commmands
    cmdtext.set_editable(true);
    Gtk::Label *label5 = manage(new Gtk::Label(_("Commmands to perform when connected:")));
    get_vbox()->pack_start(*label5, Gtk::PACK_SHRINK);
    get_vbox()->pack_start(cmdtext);

    // initialize.

    hostentry.set_text(_server->hostname);
    passentry.set_text(_server->password);
    nickentry.set_text(_server->nick);
    auto_connect_button.set_active(_server->auto_connect);
    std::ostringstream ss;
    ss << _server->port;
    portentry.set_text(ss.str());

    Glib::RefPtr<Gtk::TextBuffer> textbuffer = cmdtext.get_buffer();
    textbuffer->set_text("");

    vector<Glib::ustring>::const_iterator i;
    for (i = _server->cmds.begin(); i != _server->cmds.end(); ++i) {
        Gtk::TextIter iter = textbuffer->end();
        textbuffer->insert(iter, *i + '\n');
    }
    show_all();
}

void ServerEditDialog::on_response(int response)
{
    if (response == Gtk::RESPONSE_OK) {
        _server->hostname = hostentry.get_text();
        _server->password = passentry.get_text();
        _server->nick = nickentry.get_text();
        _server->auto_connect = auto_connect_button.get_active();

        int port;
        if (portentry.get_text_length() == 0)
              port = 6667;
        else
              port = Util::stoi(portentry.get_text());

        _server->port = port;

        Glib::RefPtr<Gtk::TextBuffer> textbuffer = cmdtext.get_buffer();

        // push back commands, for each and every line 
        std::istringstream ss(textbuffer->get_text(textbuffer->begin(), textbuffer->end(), true).raw());
        _server->cmds.clear();

        std::string tmp;
        while (getline(ss, tmp))
              _server->cmds.push_back(tmp);

        App->cfgservers.writeServersFile();
    }
}
