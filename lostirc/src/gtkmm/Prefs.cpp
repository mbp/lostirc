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
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/frame.h>
#include <Utils.h>
#include "MainWindow.h"
#include "Prefs.h"

using std::vector;

Gtk::Button* create_imagebutton(const Glib::ustring& str, const Gtk::StockID& stock_id)
{
    Gtk::Button *button = new Gtk::Button();

    Gtk::HBox *hbox = manage(new Gtk::HBox());

    hbox->pack_start(*manage(new Gtk::Image(stock_id, Gtk::ICON_SIZE_MENU)));
    hbox->pack_start(*manage(new Gtk::Label(str)));
    button->add(*hbox);
    return button;
}

Prefs::Prefs()
    : Gtk::VBox(),
    highlightingbutton("Limited highlighting (don't mark tabs red on joins/parts etc.)"),
    stripcolorsbutton("Strip color codes from incoming messages"),
    stripothersbutton("Strip bold and underline codes from incoming messages"),
    _columns(),
    _liststore(Gtk::ListStore::create(_columns)),
    _treeview(_liststore)
{
    notebook.set_tab_pos(Gtk::POS_LEFT);

    pack_start(notebook);

    Gtk::VBox *generalbox = addPage("General Settings");
    Gtk::VBox *prefsbox = addPage("Preferences");
    Gtk::VBox *fontbox = addPage("Font selection");
    Gtk::VBox *performbox = addPage("Autojoin servers");

    // General options-tab

    // Apply and Cancel buttons
    Gtk::HBox *hboxgeneral = manage(new Gtk::HBox());
    Gtk::Button *buttgeneral1 = manage(create_imagebutton("Apply settings", Gtk::Stock::APPLY));
    Gtk::Button *buttgeneral2 = manage(create_imagebutton("Cancel", Gtk::Stock::CANCEL));
    buttgeneral1->signal_clicked().connect(slot(*this, &Prefs::applyGeneral));
    buttgeneral2->signal_clicked().connect(slot(*this, &Prefs::cancelGeneral));
    hboxgeneral->pack_end(*buttgeneral2, Gtk::PACK_SHRINK);
    hboxgeneral->pack_end(*buttgeneral1, Gtk::PACK_SHRINK);
    generalbox->pack_end(*hboxgeneral, Gtk::PACK_SHRINK);

    // IRC nick
    ircnickentry.set_text(convert_to_utf8(App->options.nick));
    Gtk::Frame *frame20 = manage(new Gtk::Frame("Nickname"));
    frame20->add(ircnickentry);
    generalbox->pack_start(*frame20, Gtk::PACK_SHRINK);

    // IRC nick
    realnameentry.set_text(convert_to_utf8(App->options.realname));
    Gtk::Frame *frame21 = manage(new Gtk::Frame("Real name"));
    frame21->add(realnameentry);
    generalbox->pack_start(*frame21, Gtk::PACK_SHRINK);

    // IRC nick
    ircuserentry.set_text(convert_to_utf8(App->options.ircuser));
    Gtk::Frame *frame22 = manage(new Gtk::Frame("IRC username (ident)"));
    frame22->add(ircuserentry);
    generalbox->pack_start(*frame22, Gtk::PACK_SHRINK);

    notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem(*generalbox, "General"));

    // Preferences-tab

    // Apply and Cancel buttons
    Gtk::HBox *hboxprefs = manage(new Gtk::HBox());
    Gtk::Button *buttprefs1 = manage(create_imagebutton("Apply settings", Gtk::Stock::APPLY));
    Gtk::Button *buttprefs2 = manage(create_imagebutton("Cancel", Gtk::Stock::CANCEL));
    buttprefs1->signal_clicked().connect(slot(*this, &Prefs::applyPreferences));
    buttprefs2->signal_clicked().connect(slot(*this, &Prefs::cancelPreferences));
    hboxprefs->pack_end(*buttprefs2, Gtk::PACK_SHRINK);
    hboxprefs->pack_end(*buttprefs1, Gtk::PACK_SHRINK);
    prefsbox->pack_end(*hboxprefs, Gtk::PACK_SHRINK);

    // nickcompletion character
    nickcompletionentry.set_max_length(1);
    nickcompletionentry.set_text(convert_to_utf8(App->options.nickcompletion_char().getString()));
    Gtk::Frame *frame10 = manage(new Gtk::Frame("Nick-completion character"));
    frame10->add(nickcompletionentry);
    prefsbox->pack_start(*frame10, Gtk::PACK_SHRINK);

    // DCC ip
    dccipentry.set_text(App->options.dccip().getString());
    Gtk::Frame *frame11 = manage(new Gtk::Frame("DCC IP-Address"));
    frame11->add(dccipentry);
    prefsbox->pack_start(*frame11, Gtk::PACK_SHRINK);

    // DCC port
    dccportentry.set_text(App->options.dccport().getString());
    Gtk::Frame *frame15 = manage(new Gtk::Frame("DCC Port (0 = random)"));
    frame15->add(dccportentry);
    prefsbox->pack_start(*frame15, Gtk::PACK_SHRINK);

    // Highligted words
    highlightentry.set_text(convert_to_utf8(App->options.highlight_words));
    Gtk::Frame *frame12 = manage(new Gtk::Frame("Words to highlight on (space seperated)"));
    frame12->add(highlightentry);
    prefsbox->pack_start(*frame12, Gtk::PACK_SHRINK);

    // Buffer size for text
    bufferentry.set_text(App->options.buffer_size().getString());
    Gtk::Frame *frame13 = manage(new Gtk::Frame("Maximum number of lines to save"));
    frame13->add(bufferentry);
    prefsbox->pack_start(*frame13, Gtk::PACK_SHRINK);

    notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem(*prefsbox, "Preferences"));

    // Limited tab highlighting
    highlightingbutton.set_active(App->options.limited_highlighting);

    Gtk::Frame *frame14 = manage(new Gtk::Frame());
    frame14->add(highlightingbutton);
    prefsbox->pack_start(*frame14, Gtk::PACK_SHRINK);

    // Strip colors
    stripcolorsbutton.set_active(App->options.strip_colors);

    Gtk::Frame *frame16 = manage(new Gtk::Frame());
    frame16->add(stripcolorsbutton);
    prefsbox->pack_start(*frame16, Gtk::PACK_SHRINK);

    // Strip bold and underline
    stripothersbutton.set_active(App->options.strip_boldandunderline);

    Gtk::Frame *frame17 = manage(new Gtk::Frame());
    frame17->add(stripothersbutton);
    prefsbox->pack_start(*frame17, Gtk::PACK_SHRINK);

    // Font selection

    // Apply and Cancel buttons
    Gtk::HBox *hboxfont = manage(new Gtk::HBox());
    Gtk::Button *buttfont1 = manage(create_imagebutton("Apply font", Gtk::Stock::APPLY));
    Gtk::Button *buttfont2 = manage(create_imagebutton("Cancel", Gtk::Stock::CANCEL));
    buttfont1->signal_clicked().connect(slot(*this, &Prefs::applyFont));
    buttfont2->signal_clicked().connect(slot(*this, &Prefs::cancelFont));
    hboxfont->pack_end(*buttfont2, Gtk::PACK_SHRINK);
    hboxfont->pack_end(*buttfont1, Gtk::PACK_SHRINK);
    fontbox->pack_end(*hboxfont, Gtk::PACK_SHRINK);

    fontsel.set_preview_text("<" + ircnickentry.get_text() + "> Hello World!");
    if (!App->options.font->empty())
          fontsel.set_font_name(convert_to_utf8(App->options.font));
    fontbox->pack_start(fontsel);
    notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem(*fontbox, "Font selection"));

    // Autojoin/perform-tab

    Gtk::HBox *serverhbox = manage(new Gtk::HBox());
    Gtk::Frame *frame = manage(new Gtk::Frame("Available servers"));

    _treeview.append_column("", _columns.servername);
    _treeview.set_headers_visible(false);
    _treeview.get_selection()->signal_changed().connect(slot(*this, &Prefs::onChangeRow));

    vector<struct autoJoin*> servers = App->cfgservers.getServers();
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

    // hostname
    Gtk::Frame *frame1 = manage(new Gtk::Frame("Hostname"));
    frame1->add(hostentry);
    serverinfobox->pack_start(*frame1, Gtk::PACK_SHRINK);

    // port
    Gtk::Frame *frame2 = manage(new Gtk::Frame("Port"));
    frame2->add(portentry);
    serverinfobox->pack_start(*frame2, Gtk::PACK_SHRINK);

    // password
    Gtk::Frame *frame3 = manage(new Gtk::Frame("Password"));
    frame3->add(passentry);
    serverinfobox->pack_start(*frame3, Gtk::PACK_SHRINK);

    // nick
    Gtk::Frame *frame4 = manage(new Gtk::Frame("Nick"));
    frame4->add(nickentry);
    serverinfobox->pack_start(*frame4, Gtk::PACK_SHRINK);

    // nick
    cmdtext.set_editable(true);
    Gtk::Frame *frame5 = manage(new Gtk::Frame("Commands to perform on connect"));
    frame5->add(cmdtext);
    serverinfobox->pack_start(*frame5);

    // buttons
    Gtk::Button *savebutton = manage(create_imagebutton("Save this entry", Gtk::Stock::SAVE));
    savebutton->signal_clicked().connect(slot(*this, &Prefs::saveEntry));
    hboxserver.pack_end(*savebutton, Gtk::PACK_SHRINK);
    serverinfobox->pack_end(hboxserver, Gtk::PACK_SHRINK);

    addnewbutton = manage(create_imagebutton("Add entry", Gtk::Stock::ADD));
    addnewbutton->signal_clicked().connect(slot(*this, &Prefs::addEntry));
    hboxserver.pack_end(*addnewbutton, Gtk::PACK_SHRINK);

    removebutton = manage(create_imagebutton("Remove entry", Gtk::Stock::REMOVE));
    removebutton->signal_clicked().connect(slot(*this, &Prefs::removeEntry));
    hboxserver.pack_end(*removebutton, Gtk::PACK_SHRINK);


    performbox->pack_start(*serverhbox);
    notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem(*performbox, "Autojoin servers"));

    // Final Close button
    Gtk::Button *close_button = manage(create_imagebutton("Close", Gtk::Stock::CLOSE));
    close_button->signal_clicked().connect(slot(*this, &Prefs::endPrefs));

    Gtk::HBox *bottommenubox = manage(new Gtk::HBox());
    bottommenubox->pack_end(*close_button, Gtk::PACK_SHRINK);

    pack_start(*bottommenubox, Gtk::PACK_SHRINK);

    show_all();
    removebutton->hide();
    addnewbutton->hide();
}

void Prefs::endPrefs()
{
    currentTab->endPrefs();
}

void Prefs::applyPreferences()
{
    App->options.nickcompletion_char = Glib::locale_from_utf8(nickcompletionentry.get_text());
    App->options.dccip = dccipentry.get_text();
    App->options.dccport = dccportentry.get_text();
    App->options.highlight_words = Glib::locale_from_utf8(highlightentry.get_text());
    App->options.buffer_size = Glib::locale_from_utf8(bufferentry.get_text());

    App->options.limited_highlighting = highlightingbutton.get_active();
    App->options.strip_colors = stripcolorsbutton.get_active();
    App->options.strip_boldandunderline = stripothersbutton.get_active();
}

void Prefs::applyGeneral()
{
    App->options.realname = Glib::locale_from_utf8(realnameentry.get_text());
    App->options.ircuser = Glib::locale_from_utf8(ircuserentry.get_text());
    App->options.nick = Glib::locale_from_utf8(ircnickentry.get_text());
}

void Prefs::applyFont()
{
    App->options.font = fontsel.get_font_name();
    AppWin->getNotebook().setFont(fontsel.get_font_name());
}

void Prefs::cancelPreferences()
{
    nickcompletionentry.set_text(convert_to_utf8(App->options.nickcompletion_char().getString()));
    dccipentry.set_text(convert_to_utf8(App->options.dccip));
    dccportentry.set_text(convert_to_utf8(App->options.dccport().getString()));
    highlightentry.set_text(convert_to_utf8(App->options.highlight_words));
    bufferentry.set_text(convert_to_utf8(App->options.buffer_size().getString()));

    highlightingbutton.set_active(App->options.limited_highlighting);
    stripcolorsbutton.set_active(App->options.strip_colors);
    stripothersbutton.set_active(App->options.strip_boldandunderline);
}

void Prefs::cancelGeneral()
{
    realnameentry.set_text(convert_to_utf8(App->options.realname));
    ircuserentry.set_text(convert_to_utf8(App->options.ircuser));
    ircnickentry.set_text(convert_to_utf8(App->options.nick));
}

void Prefs::cancelFont()
{
    if (!App->options.font->empty())
          fontsel.set_font_name(convert_to_utf8(App->options.font));
}

void Prefs::saveEntry()
{
    struct autoJoin *autojoin;
    Gtk::TreeModel::iterator iter;

    // See whether no rows were selected.
    if (!_treeview.get_selection()->get_selected()) {
        // we need to add a new one
        autojoin = new autoJoin();

        App->cfgservers.addServer(autojoin);

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
    std::istringstream ss(Glib::locale_from_utf8(textbuffer->get_text(textbuffer->begin(), textbuffer->end(), true)));
    autojoin->cmds.clear();

    std::string tmp;
    while (getline(ss, tmp))
          autojoin->cmds.push_back(tmp);

    App->cfgservers.writeServersFile();

    _treeview.get_selection()->unselect_all();
    _treeview.get_selection()->select(iter);
}

void Prefs::onChangeRow()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
    Gtk::TreeModel::iterator iterrow = selection->get_selected();

    if (iterrow) {
        // Row selected
        Gtk::TreeModel::Row row = *iterrow;

        struct autoJoin* a = row[_columns.autojoin];
        hostentry.set_text(a->hostname);
        passentry.set_text(a->password);
        nickentry.set_text(a->nick);
        std::ostringstream ss;
        ss << a->port;
        portentry.set_text(ss.str());

        Glib::RefPtr<Gtk::TextBuffer> textbuffer = cmdtext.get_buffer();
        textbuffer->set_text("");

        vector<std::string>::const_iterator i;
        for (i = a->cmds.begin(); i != a->cmds.end(); ++i) {
            Gtk::TextIter iter = textbuffer->end();
            textbuffer->insert(iter, convert_to_utf8(*i + '\n'));
        }
        show_all();
    } else {
        // No row selected
        clearEntries();
        show_all();
        removebutton->hide();
        addnewbutton->hide();
    }
}

void Prefs::removeEntry()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
    Gtk::TreeModel::Row row = *selection->get_selected();

    struct autoJoin *autojoin = row[_columns.autojoin];
    _liststore->erase(selection->get_selected());
    App->cfgservers.removeServer(autojoin);
    App->cfgservers.writeServersFile();
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

Gtk::VBox* Prefs::addPage(const Glib::ustring& str)
{
    Gtk::VBox *vbox = manage(new Gtk::VBox());

    // label with frame
    Gtk::Label *label = manage(new Gtk::Label(str));
    Gtk::Frame *labelframe = manage(new Gtk::Frame());
    labelframe->add(*label);
    labelframe->set_shadow_type(Gtk::SHADOW_ETCHED_IN);

    vbox->pack_start(*labelframe, Gtk::PACK_SHRINK);
    return vbox;
}

Tab* Prefs::currentTab = 0;

