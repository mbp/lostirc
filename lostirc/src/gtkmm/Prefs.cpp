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
#include <gtkmm/frame.h>
#include <Utils.h>
#include "MainWindow.h"
#include "Prefs.h"

using std::vector;
using std::string;

Prefs::Prefs()
    : Gtk::VBox(),
    _columns(),
    _liststore(Gtk::ListStore::create(_columns)),
    _treeview(_liststore)
{

    set_spacing(2);
    pack_start(notebook);

    Gtk::VBox *generalbox = manage(new Gtk::VBox());
    Gtk::VBox *performbox = manage(new Gtk::VBox());
    Gtk::VBox *prefsbox = manage(new Gtk::VBox());

    // General options-tab

    // IRC nick
    ircnickentry.set_text(AppWin->getApp().getCfg().getOpt("nick"));
    Gtk::Frame *frame20 = manage(new Gtk::Frame("Nickname"));
    frame20->add(ircnickentry);
    generalbox->pack_start(*frame20, Gtk::SHRINK);

    // IRC nick
    realnameentry.set_text(AppWin->getApp().getCfg().getOpt("realname"));
    Gtk::Frame *frame21 = manage(new Gtk::Frame("Real name"));
    frame21->add(realnameentry);
    generalbox->pack_start(*frame21, Gtk::SHRINK);

    // IRC nick
    ircuserentry.set_text(AppWin->getApp().getCfg().getOpt("ircuser"));
    Gtk::Frame *frame22 = manage(new Gtk::Frame("IRC username (ident)"));
    frame22->add(ircuserentry);
    generalbox->pack_start(*frame22, Gtk::SHRINK);

    notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem(*generalbox, "General"));

    // Preferences-tab

    // nickcompletion character
    nickcompletionentry.set_max_length(1);
    nickcompletionentry.set_text(AppWin->getApp().getCfg().getOpt("nickcompletion_character"));
    Gtk::Frame *frame10 = manage(new Gtk::Frame("Nick-completion character"));
    frame10->add(nickcompletionentry);
    prefsbox->pack_start(*frame10, Gtk::SHRINK);

    // DCC ip
    dccipentry.set_text(AppWin->getApp().getCfg().getOpt("dccip"));
    Gtk::Frame *frame11 = manage(new Gtk::Frame("DCC IP-Address"));
    frame11->add(dccipentry);
    prefsbox->pack_start(*frame11, Gtk::SHRINK);

    // Highligted words
    highlightentry.set_text(AppWin->getApp().getCfg().getOpt("highlight_words"));
    Gtk::Frame *frame12 = manage(new Gtk::Frame("Words to highlight on (space seperated)"));
    frame12->add(highlightentry);
    prefsbox->pack_start(*frame12, Gtk::SHRINK);

    notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem(*prefsbox, "Preferences"));

    // Autojoin/perform-tab

    Gtk::HBox *serverhbox = manage(new Gtk::HBox());
    Gtk::Frame *frame = manage(new Gtk::Frame("Available servers"));

    _treeview.append_column("", _columns.servername);
    _treeview.set_headers_visible(false);
    _treeview.signal_select_cursor_row().connect(slot(*this, &Prefs::onSelectRow));
    _treeview.signal_toggle_cursor_row().connect(slot(*this, &Prefs::onUnSelectRow));

    vector<struct autoJoin*> servers = AppWin->getApp().getCfg().getServers();
    vector<struct autoJoin*>::iterator i;

    for (i = servers.begin(); i != servers.end(); ++i) {
        Gtk::TreeModel::Row row = *_liststore->append();
        row[_columns.servername] = (*i)->hostname;
        row[_columns.autojoin] = *i;
    }
    frame->add(_treeview);
    serverhbox->pack_start(*frame);
    Gtk::VBox *serverinfobox = manage(new Gtk::VBox());
    serverhbox->pack_start(*serverinfobox);

    // hbox for buttons
    savehbox.set_spacing(5);
    serverinfobox->pack_start(savehbox, Gtk::SHRINK);

    // hostname
    Gtk::Frame *frame1 = manage(new Gtk::Frame("Hostname"));
    frame1->add(hostentry);
    serverinfobox->pack_start(*frame1, Gtk::SHRINK);

    // port
    Gtk::Frame *frame2 = manage(new Gtk::Frame("Port"));
    frame2->add(portentry);
    serverinfobox->pack_start(*frame2, Gtk::SHRINK);

    // password
    Gtk::Frame *frame3 = manage(new Gtk::Frame("Password"));
    frame3->add(passentry);
    serverinfobox->pack_start(*frame3, Gtk::SHRINK);

    // nick
    Gtk::Frame *frame4 = manage(new Gtk::Frame("Nick"));
    frame4->add(nickentry);
    serverinfobox->pack_start(*frame4, Gtk::SHRINK);

    // nick
    cmdtext.set_editable(true);
    Gtk::Frame *frame5 = manage(new Gtk::Frame("Commands to perform on connect"));
    frame5->add(cmdtext);
    serverinfobox->pack_start(*frame5, Gtk::SHRINK);

    // save button
    Gtk::Button *savebutton = manage(new Gtk::Button("Save entry"));
    savebutton->signal_clicked().connect(slot(*this, &Prefs::saveEntry));
    savehbox.pack_start(*savebutton, Gtk::SHRINK);

    removebutton = new Gtk::Button("Remove entry");
    removebutton->signal_clicked().connect(slot(*this, &Prefs::removeEntry));

    addnewbutton = new Gtk::Button("Add new entry");
    addnewbutton->signal_clicked().connect(slot(*this, &Prefs::addEntry));

    performbox->pack_start(*serverhbox);
    notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem(*performbox, "Autojoin servers"));

    // Ok, Apply and Cancel buttons
    Gtk::Button *closebutt = manage(new Gtk::Button("Close"));
    Gtk::Button *savebutt = manage(new Gtk::Button("Save settings"));

    savebutt->signal_clicked().connect(slot(*this, &Prefs::saveSettings));
    closebutt->signal_clicked().connect(slot(*this, &Prefs::endPrefs));

    Gtk::HBox *bottommenubox = manage(new Gtk::HBox());
    bottommenubox->pack_end(*savebutt, Gtk::SHRINK);
    bottommenubox->pack_end(*closebutt, Gtk::SHRINK);

    pack_start(*bottommenubox, Gtk::FILL);

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
    AppWin->getApp().getCfg().setOpt("nickcompletion_character", nickcompletionentry.get_text());
    AppWin->getApp().getCfg().setOpt("dccip", dccipentry.get_text());
    AppWin->getApp().getCfg().setOpt("highlight_words", highlightentry.get_text());
    AppWin->getApp().getCfg().setOpt("realname", realnameentry.get_text());
    AppWin->getApp().getCfg().setOpt("ircuser", ircuserentry.get_text());
    AppWin->getApp().getCfg().setOpt("nick", ircnickentry.get_text());
}

void Prefs::saveEntry()
{
    struct autoJoin *autojoin;
    Gtk::TreeModel::iterator iter;

    // See whether no rows were selected.
    if (!_treeview.get_selection()->get_selected()) {
        // we need to add a new one
        autojoin = new autoJoin();

        AppWin->getApp().getCfg().addServer(autojoin);

        iter = _liststore->append();

        ( *iter )[_columns.servername] = hostentry.get_text();
        ( *iter )[_columns.autojoin] = autojoin;

    } else {
        // we need to save the current selected one

        Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
        iter = selection->get_selected();

        autojoin = ( *iter )[_columns.autojoin];
    }

    autojoin->hostname = hostentry.get_text();
    autojoin->password = passentry.get_text();
    autojoin->nick = nickentry.get_text();

    int port;
    if (portentry.get_text_length() == 0)
          port = 6667;
    else
          port = Util::stoi(portentry.get_text());

    autojoin->port = port;

    Glib::RefPtr<Gtk::TextBuffer> textbuffer = cmdtext.get_buffer();

    // push back commands, for each and every line 
    std::istringstream ss(textbuffer->get_text(textbuffer->begin(), textbuffer->end(), true));
    autojoin->cmds.clear();

    string tmp;
    while (getline(ss, tmp))
          autojoin->cmds.push_back(tmp);

    AppWin->getApp().getCfg().writeServers();

    _treeview.get_selection()->unselect_all();
    _treeview.get_selection()->select(iter);
}

void Prefs::onSelectRow(bool start_editing)
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
    Gtk::TreeModel::Row row = *selection->get_selected();

    struct autoJoin* a = row[_columns.autojoin];
    hostentry.set_text(a->hostname);
    passentry.set_text(a->password);
    nickentry.set_text(a->nick);
    std::ostringstream ss;
    ss << a->port;
    portentry.set_text(ss.str());

    Glib::RefPtr<Gtk::TextBuffer> textbuffer = cmdtext.get_buffer();
    textbuffer->set_text("");

    vector<string>::const_iterator i;
    for (i = a->cmds.begin(); i != a->cmds.end(); ++i) {
        Gtk::TextIter iter = textbuffer->get_iter_at_offset(0);
        textbuffer->insert(iter, *i + '\n');
    }
    savehbox.pack_start(*removebutton, Gtk::SHRINK);
    savehbox.pack_start(*addnewbutton, Gtk::SHRINK);
    show_all();
}

void Prefs::onUnSelectRow()
{
    savehbox.remove(*removebutton);
    savehbox.remove(*addnewbutton);
    clearEntries();
    show_all();
}

void Prefs::removeEntry()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
    Gtk::TreeModel::Row row = *selection->get_selected();

    struct autoJoin *autojoin = row[_columns.autojoin];
    _liststore->erase(selection->get_selected());
    AppWin->getApp().getCfg().removeServer(autojoin);
    AppWin->getApp().getCfg().writeServers();
}

void Prefs::addEntry()
{
    clearEntries();
    _treeview.get_selection()->unselect_all();
    hostentry.grab_focus();
}

void Prefs::clearEntries()
{
    passentry.set_text("");
    portentry.set_text("");
    hostentry.set_text("");
    nickentry.set_text("");
    cmdtext.get_buffer()->set_text("");
}

Tab* Prefs::currentTab = 0;

