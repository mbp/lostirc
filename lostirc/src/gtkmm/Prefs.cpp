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

#include <sstream>
#include <gtk--/frame.h>
#include <Utils.h>
#include "MainWindow.h"
#include "Prefs.h"

using std::vector;
using std::string;

Prefs::Prefs()
    : Gtk::VBox()
{

    notebook = manage(new Gtk::Notebook());
    set_spacing(2);
    pack_start(*notebook, 1, 1);

    Gtk::VBox *generalbox = manage(new Gtk::VBox());
    Gtk::VBox *performbox = manage(new Gtk::VBox());
    Gtk::VBox *prefsbox = manage(new Gtk::VBox());

    // General options-tab

    // IRC nick
    ircnickentry = manage(new Gtk::Entry());
    ircnickentry->set_text(AppWin->getApp()->getCfg().getOpt("nick"));
    Gtk::Frame *frame20 = manage(new Gtk::Frame("Nickname"));
    frame20->add(*ircnickentry);
    generalbox->pack_start(*frame20, 0, 0);

    // IRC nick
    realnameentry = manage(new Gtk::Entry());
    realnameentry->set_text(AppWin->getApp()->getCfg().getOpt("realname"));
    Gtk::Frame *frame21 = manage(new Gtk::Frame("Real name"));
    frame21->add(*realnameentry);
    generalbox->pack_start(*frame21, 0, 0);

    // IRC nick
    ircuserentry = manage(new Gtk::Entry());
    ircuserentry->set_text(AppWin->getApp()->getCfg().getOpt("ircuser"));
    Gtk::Frame *frame22 = manage(new Gtk::Frame("IRC username (ident)"));
    frame22->add(*ircuserentry);
    generalbox->pack_start(*frame22, 0, 0);

    notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*generalbox, "General"));

    // Preferences-tab

    // nickcompletion character
    nickcompletionentry = manage(new Gtk::Entry(1));
    nickcompletionentry->set_text(AppWin->getApp()->getCfg().getOpt("nickcompletion_character"));
    Gtk::Frame *frame10 = manage(new Gtk::Frame("Nick-completion character"));
    frame10->add(*nickcompletionentry);
    prefsbox->pack_start(*frame10, 0, 0);

    // DCC ip
    dccipentry = manage(new Gtk::Entry());
    dccipentry->set_text(AppWin->getApp()->getCfg().getOpt("dccip"));
    Gtk::Frame *frame11 = manage(new Gtk::Frame("DCC IP-Address"));
    frame11->add(*dccipentry);
    prefsbox->pack_start(*frame11, 0, 0);

    // Highligted words
    highlightentry = manage(new Gtk::Entry());
    highlightentry->set_text(AppWin->getApp()->getCfg().getOpt("highlight_words"));
    Gtk::Frame *frame12 = manage(new Gtk::Frame("Words to highlight on (space seperated)"));
    frame12->add(*highlightentry);
    prefsbox->pack_start(*frame12, 0, 0);

    notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*prefsbox, "Preferences"));

    // Autojoin/perform-tab

    Gtk::HBox *serverhbox = manage(new Gtk::HBox());
    Gtk::Frame *frame = manage(new Gtk::Frame("Available servers"));
    clist = manage(new Gtk::CList(1));
    clist->select_row.connect(slot(this, &Prefs::onSelectRow));
    clist->unselect_row.connect(slot(this, &Prefs::onUnSelectRow));

    vector<struct autoJoin*> servers = AppWin->getApp()->getCfg().getServers();
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

    // hbox for buttons
    savehbox = manage(new Gtk::HBox());
    savehbox->set_spacing(5);
    serverinfobox->pack_start(*savehbox, 0, 0);

    // hostname
    hostentry = manage(new Gtk::Entry());
    Gtk::Frame *frame1 = manage(new Gtk::Frame("Hostname"));
    frame1->add(*hostentry);
    serverinfobox->pack_start(*frame1, 0, 0);

    // port
    portentry = manage(new Gtk::Entry());
    Gtk::Frame *frame2 = manage(new Gtk::Frame("Port"));
    frame2->add(*portentry);
    serverinfobox->pack_start(*frame2, 0, 0);

    // password
    passentry = manage(new Gtk::Entry());
    Gtk::Frame *frame3 = manage(new Gtk::Frame("Password"));
    frame3->add(*passentry);
    serverinfobox->pack_start(*frame3, 0, 0);

    // nick
    nickentry = manage(new Gtk::Entry());
    Gtk::Frame *frame4 = manage(new Gtk::Frame("Nick"));
    frame4->add(*nickentry);
    serverinfobox->pack_start(*frame4, 0, 0);

    // nick
    cmdtext = manage(new Gtk::Text());
    cmdtext->set_editable(true);
    Gtk::Frame *frame5 = manage(new Gtk::Frame("Commands to perform on connect"));
    frame5->add(*cmdtext);
    serverinfobox->pack_start(*frame5, 0, 0);

    // save button
    Gtk::Button *savebutton = manage(new Gtk::Button("Save entry"));
    savebutton->clicked.connect(slot(this, &Prefs::saveEntry));
    savehbox->pack_start(*savebutton, 0, 0);

    removebutton = new Gtk::Button("Remove entry");
    removebutton->clicked.connect(slot(this, &Prefs::removeEntry));

    addnewbutton = new Gtk::Button("Add new entry");
    addnewbutton->clicked.connect(slot(this, &Prefs::addEntry));

    performbox->pack_start(*serverhbox, 1, 1);
    notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*performbox, "Autojoin servers"));

    // Ok, Apply and Cancel buttons
    Gtk::Button *closebutt = manage(new Gtk::Button("Close"));
    Gtk::Button *savebutt = manage(new Gtk::Button("Save settings"));

    savebutt->clicked.connect(slot(this, &Prefs::saveSettings));
    closebutt->clicked.connect(slot(this, &Prefs::endPrefs));

    Gtk::HBox *bottommenubox = manage(new Gtk::HBox());
    bottommenubox->pack_end(*savebutt, 0, 0);
    bottommenubox->pack_end(*closebutt, 0, 0);

    pack_start(*bottommenubox, 0, 1);

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

void Prefs::saveSettings()
{
    AppWin->getApp()->getCfg().setOpt("nickcompletion_character", nickcompletionentry->get_text());
    AppWin->getApp()->getCfg().setOpt("dccip", dccipentry->get_text());
    AppWin->getApp()->getCfg().setOpt("highlight_words", highlightentry->get_text());
    AppWin->getApp()->getCfg().setOpt("realname", realnameentry->get_text());
    AppWin->getApp()->getCfg().setOpt("ircuser", ircuserentry->get_text());
    AppWin->getApp()->getCfg().setOpt("nick", ircnickentry->get_text());
}

void Prefs::saveEntry()
{
    struct autoJoin *a;
    if (clist->selection().size() == 0) {
        // we need to add a new one
        a = new autoJoin();

        AppWin->getApp()->getCfg().addServer(a);

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

    AppWin->getApp()->getCfg().writeServers();

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
    AppWin->getApp()->getCfg().removeServer(a);
    AppWin->getApp()->getCfg().writeServers();
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

