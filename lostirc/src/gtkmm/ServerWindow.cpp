/* 
 * Copyright (C) 2002-2004 Morten Brix Pedersen <morten@wtf.dk>
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
#include <gtkmm/messagedialog.h>
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
    _treeview(_liststore),
    _pref_table(2, 2)
{
    set_default_size(300, 325);
    set_border_width(5);

    int row = 1;

    // IRC nick
    ircnickentry.set_text(App->options.nick);
    Gtk::Label *glabel0 = manage(new Gtk::Label(_("Nickname:"), Gtk::ALIGN_LEFT));
    _pref_table.attach(*glabel0, 0, 1, row, row + 1);
    _pref_table.attach(ircnickentry, 1, 2, row, row + 1);

    row++;

    // Real name
    realnameentry.set_text(App->options.realname);
    Gtk::Label *glabel1 = manage(new Gtk::Label(_("Real name:"), Gtk::ALIGN_LEFT));
    _pref_table.attach(*glabel1, 0, 1, row, row + 1);
    _pref_table.attach(realnameentry, 1, 2, row, row + 1);

    realnameentry.signal_focus_out_event().connect(sigc::mem_fun(*this, &ServerWindow::focusChangeEvent));
    ircnickentry.signal_focus_out_event().connect(sigc::mem_fun(*this, &ServerWindow::focusChangeEvent));


    Gtk::ScrolledWindow *swin = manage(new Gtk::ScrolledWindow());
    swin->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    swin->add(_treeview);

    // Autojoin/perform-tab

    _treeview.append_column_editable(_("Auto-connect"), _columns.auto_connect);
    _treeview.append_column(_("Hostname"), _columns.servername);
    _treeview.append_column(_("Port"), _columns.port);

    updateList();

    // Button box.
    Gtk::HButtonBox *buttbox = manage(new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6));
    Gtk::Button *connectbutton = manage(create_imagebutton(_("_Connect"), Gtk::Stock::JUMP_TO));
    connectbutton->signal_clicked().connect(sigc::mem_fun(*this, &ServerWindow::connectEntry));
    Gtk::Button *addbutton = manage(new Gtk::Button(Gtk::Stock::ADD));
    addbutton->signal_clicked().connect(sigc::mem_fun(*this, &ServerWindow::addEntry));
    Gtk::Button *modifybutton = manage(create_imagebutton(_("_Modify"), Gtk::Stock::PREFERENCES));
    modifybutton->signal_clicked().connect(sigc::mem_fun(*this, &ServerWindow::modifyEntry));
    Gtk::Button *deletebutton = manage(new Gtk::Button(Gtk::Stock::DELETE));
    deletebutton->signal_clicked().connect(sigc::mem_fun(*this, &ServerWindow::deleteEntry));
    Gtk::Button *closebutton = manage(new Gtk::Button(Gtk::Stock::CLOSE));
    closebutton->signal_clicked().connect(sigc::mem_fun(*this, &Gtk::Dialog::hide));

    buttbox->pack_end(*manage(new Gtk::VBox()), Gtk::PACK_EXPAND_WIDGET);
    buttbox->pack_end(*connectbutton, Gtk::PACK_SHRINK);
    buttbox->pack_end(*addbutton, Gtk::PACK_SHRINK);
    buttbox->pack_end(*modifybutton, Gtk::PACK_SHRINK);
    buttbox->pack_end(*deletebutton, Gtk::PACK_SHRINK);
    buttbox->pack_end(*closebutton, Gtk::PACK_SHRINK);

    Gtk::Label *servlabel = manage(new Gtk::Label());
    servlabel->set_markup("<b>Servers</b>");
    get_vbox()->pack_start(_pref_table, Gtk::PACK_SHRINK, 5);
    get_vbox()->pack_start(*manage(new Gtk::HSeparator()), Gtk::PACK_SHRINK, 5);
    get_vbox()->pack_start(*servlabel, Gtk::PACK_SHRINK, 5);
    get_vbox()->pack_start(*swin, Gtk::PACK_EXPAND_WIDGET, 5);
    get_vbox()->pack_start(*manage(new Gtk::HSeparator()), Gtk::PACK_SHRINK, 5);
    get_vbox()->pack_start(*buttbox, Gtk::PACK_SHRINK);

    show_all();
}

bool ServerWindow::focusChangeEvent(GdkEventFocus* event)
{
    App->options.nick = ircnickentry.get_text();
    App->options.realname = realnameentry.get_text();
    return true;
}

void ServerWindow::updateList()
{
    _column_signal.disconnect();

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

    _column_signal = _liststore->signal_row_changed().connect(sigc::mem_fun(*this, &ServerWindow::onColumnChanged));
}

void ServerWindow::connectEntry()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
    Gtk::TreeModel::iterator iterrow = selection->get_selected();

    if (iterrow) {
        Gtk::TreeModel::Row row = *iterrow;
        ServerConnection *conn = App->newServer(row[_columns.serverptr]);
        conn->connect();
    }
}

void ServerWindow::addEntry()
{
    Server *server = new Server;

    ServerEditDialog dialog(*this, server);
    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) {
        App->cfgservers.addServer(server);
        App->cfgservers.writeServersFile();
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
        // Ask the user whether he/she is sure.
        Gtk::MessageDialog dialog("Are you sure you want to delete this server?", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);

        int result = dialog.run();

        if (result == Gtk::RESPONSE_YES) {
            // Delete the entry.
            Gtk::TreeModel::Row row = *selection->get_selected();

            Server *server = row[_columns.serverptr];
            _liststore->erase(selection->get_selected());
            App->cfgservers.removeServer(server);
            App->cfgservers.writeServersFile();
        }
    }
}

void ServerWindow::onColumnChanged(const Gtk::TreeModel::Path&, const Gtk::TreeModel::iterator& iterrow)
{
    if (iterrow) {
        Gtk::TreeModel::Row row = *iterrow;
        Server *server = row[_columns.serverptr];

        server->auto_connect = row[_columns.auto_connect];
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
    get_vbox()->pack_start(_server_options_table, Gtk::PACK_SHRINK);
    get_vbox()->pack_start(auto_connect_button, Gtk::PACK_SHRINK);

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
    passentry.set_visibility(false);
    Gtk::Label *label3 = manage(new Gtk::Label(_("Password:"), Gtk::ALIGN_LEFT));
    _server_options_table.attach(*label3, 0, 1, row, row + 1);
    _server_options_table.attach(passentry, 1, 2, row, row + 1);

    row++;

    // nick
    Gtk::Label *label4 = manage(new Gtk::Label(_("Nickname:"), Gtk::ALIGN_LEFT));
    _server_options_table.attach(*label4, 0, 1, row, row + 1);
    _server_options_table.attach(nickentry, 1, 2, row, row + 1);

    // commmands
    cmdtext.set_size_request(400, 120);
    cmdtext.set_editable(true);
    Gtk::Label *label5 = manage(new Gtk::Label(_("Commands to perform when connected:")));
    get_vbox()->pack_start(*label5, Gtk::PACK_SHRINK);

    Gtk::ScrolledWindow *swin = manage(new Gtk::ScrolledWindow());
    swin->set_shadow_type(Gtk::SHADOW_IN);
    swin->add(cmdtext);
    swin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    get_vbox()->pack_start(*swin);

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
              port = Util::convert<int>(portentry.get_text());

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
