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
    auto_connect_button(_("_Connect automatically"), true),
    _columns(),
    _liststore(Gtk::ListStore::create(_columns)),
    _treeview(_liststore),
    _server_options_table(2, 4)
{
    _server_options_table.set_row_spacings(6);
    _server_options_table.set_col_spacings(12);
    set_border_width(5);
    get_vbox()->set_spacing(18);

    // Autojoin/perform-tab

    _treeview.append_column(_("Hostname"), _columns.servername);
    _treeview.append_column(_("Port"), _columns.port);
    _treeview.append_column(_("Auto-connect"), _columns.auto_connect);
    _treeview.get_selection()->signal_changed().connect(slot(*this, &ServerWindow::onChangeRow));

    vector<Server*> servers = App->cfgservers.getServers();
    vector<Server*>::iterator i;

    for (i = servers.begin(); i != servers.end(); ++i) {
        Gtk::TreeModel::Row row = *_liststore->append();
        row[_columns.servername] = (*i)->hostname;
        row[_columns.port] = (*i)->port;
        row[_columns.auto_connect] = (*i)->auto_connect;
        row[_columns.autojoin] = *i;
    }

    // Button box.
    Gtk::HButtonBox *buttbox = manage(new Gtk::HButtonBox());
    buttbox->set_spacing(6);
    Gtk::Button *addbutton = manage(new Gtk::Button(Gtk::Stock::ADD));
    addbutton->signal_clicked().connect(slot(*this, &ServerWindow::addEntry));
    Gtk::Button *modifybutton = manage(create_imagebutton(_("_Modify"), Gtk::Stock::PREFERENCES));
    Gtk::Button *deletebutton = manage(new Gtk::Button(Gtk::Stock::DELETE));
    Gtk::Button *closebutton = manage(new Gtk::Button(Gtk::Stock::CLOSE));

    buttbox->pack_end(*addbutton, Gtk::PACK_SHRINK);
    buttbox->pack_end(*modifybutton, Gtk::PACK_SHRINK);
    buttbox->pack_end(*deletebutton, Gtk::PACK_SHRINK);
    buttbox->pack_end(*closebutton, Gtk::PACK_SHRINK);

    get_vbox()->pack_end(*buttbox, Gtk::PACK_SHRINK);
    get_vbox()->pack_end(*manage(new Gtk::HSeparator()), Gtk::PACK_EXPAND_PADDING);
    get_vbox()->pack_end(_treeview, Gtk::PACK_SHRINK);

    serverinfobox = manage(new Gtk::VBox());

    // auto connect
    serverinfobox->pack_start(auto_connect_button, Gtk::PACK_SHRINK);

    serverinfobox->pack_start(_server_options_table, Gtk::PACK_SHRINK);

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
    Gtk::Label *label5 = manage(new Gtk::Label(_("Commmands to perform on connect:")));
    serverinfobox->pack_start(*label5, Gtk::PACK_SHRINK);
    serverinfobox->pack_start(cmdtext);

    // buttons
    Gtk::Button *savebutton = manage(create_imagebutton(_("Save this entry"), Gtk::Stock::SAVE));
    savebutton->signal_clicked().connect(slot(*this, &ServerWindow::saveEntry));
    hboxserver.pack_end(*savebutton, Gtk::PACK_SHRINK);
    serverinfobox->pack_end(hboxserver, Gtk::PACK_SHRINK);

    get_vbox()->pack_start(*serverinfobox);

    show_all();

    serverinfobox->hide();
}

void ServerWindow::saveEntry()
{
    Server *autojoin;
    Gtk::TreeModel::iterator iter;

    // See whether no rows were selected.
    if (!_treeview.get_selection()->get_selected()) {
        // we need to add a new one
        autojoin = new Server();

        App->cfgservers.addServer(autojoin);

        iter = _liststore->append();

        ( *iter )[_columns.servername] = hostentry.get_text();
        ( *iter )[_columns.autojoin] = autojoin;
        ( *iter )[_columns.auto_connect] = true;

    } else {
        // we need to save the current selected one

        Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
        iter = selection->get_selected();

        autojoin = ( *iter )[_columns.autojoin];
    }

    autojoin->hostname = hostentry.get_text();
    autojoin->password = passentry.get_text();
    autojoin->nick = nickentry.get_text();
    autojoin->auto_connect = auto_connect_button.get_active();
    ( *iter )[_columns.auto_connect] = auto_connect_button.get_active();

    int port;
    if (portentry.get_text_length() == 0)
          port = 6667;
    else
          port = Util::stoi(portentry.get_text());

    autojoin->port = port;

    Glib::RefPtr<Gtk::TextBuffer> textbuffer = cmdtext.get_buffer();

    // push back commands, for each and every line 
    std::istringstream ss(textbuffer->get_text(textbuffer->begin(), textbuffer->end(), true).raw());
    autojoin->cmds.clear();

    std::string tmp;
    while (getline(ss, tmp))
          autojoin->cmds.push_back(tmp);

    App->cfgservers.writeServersFile();

    _treeview.get_selection()->unselect_all();
    _treeview.get_selection()->select(iter);
}

void ServerWindow::onChangeRow()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
    Gtk::TreeModel::iterator iterrow = selection->get_selected();

    if (iterrow) {
        serverinfobox->show();
        // Row selected
        Gtk::TreeModel::Row row = *iterrow;

        Server* a = row[_columns.autojoin];
        hostentry.set_text(a->hostname);
        passentry.set_text(a->password);
        nickentry.set_text(a->nick);
        auto_connect_button.set_active(a->auto_connect);
        std::ostringstream ss;
        ss << a->port;
        portentry.set_text(ss.str());

        Glib::RefPtr<Gtk::TextBuffer> textbuffer = cmdtext.get_buffer();
        textbuffer->set_text("");

        vector<Glib::ustring>::const_iterator i;
        for (i = a->cmds.begin(); i != a->cmds.end(); ++i) {
            Gtk::TextIter iter = textbuffer->end();
            textbuffer->insert(iter, *i + '\n');
        }
        show_all();
    } else {
        serverinfobox->hide();
        // No row selected
        clearEntries();
        show_all();
    }
}

void ServerWindow::removeEntry()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
    Gtk::TreeModel::Row row = *selection->get_selected();

    Server *autojoin = row[_columns.autojoin];
    _liststore->erase(selection->get_selected());
    App->cfgservers.removeServer(autojoin);
    App->cfgservers.writeServersFile();
}

void ServerWindow::addEntry()
{
    clearEntries();
    _treeview.get_selection()->unselect_all();
    hostentry.grab_focus();
}

void ServerWindow::clearEntries()
{
    passentry.set_text("");
    portentry.set_text("");
    hostentry.set_text("");
    nickentry.set_text("");
    cmdtext.get_buffer()->set_text("");
}
