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

#include <Utils.h>
#include "Tab.h"
#include "Prefs.h"
#include <sstream>
#include "GuiCommands.h"
#include <gtk--/frame.h>

using std::vector;
using std::string;

Prefs::Prefs()
    : Gtk::Notebook()
{

    Gtk::VBox *performbox = manage(new Gtk::VBox());
    Gtk::VBox *prefsbox = manage(new Gtk::VBox());
    Gtk::HBox *serverhbox = manage(new Gtk::HBox());
    Gtk::Frame *frame = manage(new Gtk::Frame("Available servers"));
    clist = manage(new Gtk::CList(1));
    clist->select_row.connect(slot(this, &Prefs::onSelectRow));
    clist->unselect_row.connect(slot(this, &Prefs::onUnSelectRow));


    vector<struct autoJoin*> servers = GuiCommands::mw->getApp()->getCfg().getServers();
    vector<struct autoJoin*>::const_iterator i;

    for (i = servers.begin(); i != servers.end(); ++i) {
        vector<string> v; // FIXME: ugly as hell.
        v.push_back((*i)->hostname);
        clist->rows().push_back(v);
        clist->rows().back().set_data(*i);
    }
    frame->add(*clist);
    serverhbox->pack_start(*frame);
    Gtk::VBox *serverinfobox = manage(new Gtk::VBox());
    serverhbox->pack_start(*serverinfobox);

    /* hbox for buttons */
    savehbox = manage(new Gtk::HBox());
    savehbox->set_spacing(5);
    serverinfobox->pack_start(*savehbox, 0, 0);

    /* hostname */
    hostentry = manage(new Gtk::Entry());
    Gtk::Frame *frame1 = manage(new Gtk::Frame("Hostname"));
    frame1->add(*hostentry);
    serverinfobox->pack_start(*frame1, 0, 0);

    /* port */
    portentry = manage(new Gtk::Entry());
    Gtk::Frame *frame2 = manage(new Gtk::Frame("Port"));
    frame2->add(*portentry);
    serverinfobox->pack_start(*frame2, 0, 0);

    /* password */
    passentry = manage(new Gtk::Entry());
    Gtk::Frame *frame3 = manage(new Gtk::Frame("Password"));
    frame3->add(*passentry);
    serverinfobox->pack_start(*frame3, 0, 0);

    /* nick */
    nickentry = manage(new Gtk::Entry());
    Gtk::Frame *frame4 = manage(new Gtk::Frame("Nick"));
    frame4->add(*nickentry);
    serverinfobox->pack_start(*frame4, 0, 0);

    /* nick */
    cmdtext = manage(new Gtk::Text());
    cmdtext->set_editable(true);
    Gtk::Frame *frame5 = manage(new Gtk::Frame("Commands to perform on connect"));
    frame5->add(*cmdtext);
    serverinfobox->pack_start(*frame5, 0, 0);

    /* lower hbox */
    Gtk::HBox *bottomhbox = manage(new Gtk::HBox());
    serverinfobox->pack_end(*bottomhbox, 0, 0);

    /* close prefs button */
    closebutton = manage(new Gtk::Button("Close prefs"));
    closebutton->clicked.connect(slot(this, &Prefs::endPrefs));
    bottomhbox->pack_end(*closebutton, 0, 0);

    /* save button */
    Gtk::Button *savebutton = manage(new Gtk::Button("Save entry"));
    savebutton->clicked.connect(slot(this, &Prefs::saveEntry));
    savehbox->pack_start(*savebutton, 0, 0);

    removebutton = new Gtk::Button("Remove entry");
    removebutton->clicked.connect(slot(this, &Prefs::removeEntry));

    addnewbutton = new Gtk::Button("Add new entry");
    addnewbutton->clicked.connect(slot(this, &Prefs::addEntry));

    performbox->pack_start(*serverhbox, 1, 1);
    pages().push_back(Gtk::Notebook_Helpers::TabElem(*performbox, "Autojoin servers"));
    pages().push_back(Gtk::Notebook_Helpers::TabElem(*prefsbox, "Preferences"));
    show_all();
}

Prefs::~Prefs()
{
    delete removebutton;
    delete addnewbutton;
}

void Prefs::endPrefs()
{
    currentTab->endPrefs();
}

void Prefs::saveEntry()
{
    struct autoJoin *a;
    if (clist->selection().size() == 0) {
        // we need to add a new one
        a = new autoJoin();

        GuiCommands::mw->getApp()->getCfg().addServer(a);

        vector<string> v; // FIXME: ugly as hell.
        v.push_back(hostentry->get_text());
        clist->rows().push_back(v);
        clist->rows().back().set_data(a);

    } else {
        // we need to save the current selected one

        a = static_cast<struct autoJoin*>(clist->selection().front().get_data());
    }

    a->hostname = hostentry->get_text();
    a->password = passentry->get_text();
    a->nick = nickentry->get_text();

    int port;
    if (portentry->get_text_length() == 0)
          port = 6667;
    else
          port = Util::stoi(portentry->get_text());

    a->port = port;

    /* push back commands, for each and every line */
    std::istringstream ss(cmdtext->get_chars(0, -1));
    a->cmds.clear();

    string tmp;
    while (getline(ss, tmp))
          a->cmds.push_back(tmp);

    GuiCommands::mw->getApp()->getCfg().writeServers();

    clist->unselect_all();
    clist->select_row(clist->find_row_from_data(a));
}

void Prefs::onSelectRow(int r, int col, GdkEvent *e)
{
    struct autoJoin *a = static_cast<struct autoJoin*>(clist->row(r).get_data());
    hostentry->set_text(a->hostname);
    passentry->set_text(a->password);
    nickentry->set_text(a->nick);
    std::ostringstream ss;
    ss << a->port;
    portentry->set_text(ss.str());

    cmdtext->delete_text(0, -1);
    vector<string>::const_iterator i;
    for (i = a->cmds.begin(); i != a->cmds.end(); ++i) {
        cmdtext->insert(*i + '\n');
    }
    savehbox->pack_start(*removebutton, 0, 0);
    savehbox->pack_start(*addnewbutton, 0, 0);
    show_all();
}

void Prefs::onUnSelectRow(int r, int col, GdkEvent *e)
{
    savehbox->remove(*removebutton);
    savehbox->remove(*addnewbutton);
    clearEntries();
    show_all();
}

void Prefs::removeEntry()
{
    struct autoJoin *a = static_cast<struct autoJoin*>(clist->selection().front().get_data());
    clist->remove_row(clist->find_row_from_data(a));
    GuiCommands::mw->getApp()->getCfg().removeServer(a);
    GuiCommands::mw->getApp()->getCfg().writeServers();
}

void Prefs::addEntry()
{
    clearEntries();
    clist->unselect_all();
    hostentry->grab_focus();
}

void Prefs::clearEntries()
{
    passentry->set_text("");
    portentry->set_text("");
    hostentry->set_text("");
    nickentry->set_text("");
    cmdtext->delete_text(0, -1);
}

Tab* Prefs::currentTab = 0;

