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
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/frame.h>
#include <Utils.h>
#include "MainWindow.h"
#include "Prefs.h"

using std::vector;

static const char *encodings[]=
{
    "System default",
    "UTF-8",
    "ISO-8859-1 (Western Europe)",
    "ISO-8859-2 (Central Europe)",
    "ISO-8859-7 (Greek)",
    "ISO-8859-8 (Hebrew)",
    "ISO-8859-9 (Turkish)",
    "ISO-8859-15 (Western Europe, but with Euro)",
    "CP1251 (Cyrillic)",
    "CP1256 (Arabic)",
    "GB18030 (Chinese)",
    "SJIS (Japanese)",
    NULL
};


Gtk::Button* create_imagebutton(const Glib::ustring& str, const Gtk::StockID& stock_id)
{
    Gtk::Button *button = new Gtk::Button();

    Gtk::HBox *hbox = manage(new Gtk::HBox());

    hbox->pack_start(*manage(new Gtk::Image(stock_id, Gtk::ICON_SIZE_MENU)));
    hbox->pack_start(*manage(new Gtk::Label(str)));
    button->add(*hbox);
    return button;
}

Prefs::Prefs(Gtk::Window& parent)
    : Gtk::Dialog(_("LostIRC Preferences"), parent),
    highlightingbutton(_("Limited _highlighting (don't mark tabs red on joins/parts etc.)"), true),
    stripcolorsbutton(_("Strip _color codes from incoming messages"), true),
    stripothersbutton(_("Strip _bold and underline codes from incoming messages"), true),
    loggingbutton(_("_Log conversations to disk"), true),
    auto_connect_button(_("_Connect automatically"), true),
    _columns(),
    _liststore(Gtk::ListStore::create(_columns)),
    _treeview(_liststore),
    _general_table(2, 4),
    _prefs_table(2, 5),
    _server_options_table(2, 4)
{
    add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
    notebook.set_tab_pos(Gtk::POS_TOP);
    _general_table.set_row_spacings(6);
    _general_table.set_col_spacings(12);
    _prefs_table.set_row_spacings(6);
    _prefs_table.set_col_spacings(12);
    _server_options_table.set_row_spacings(6);
    _server_options_table.set_col_spacings(12);

    get_vbox()->pack_start(notebook);

    Gtk::VBox *generalbox = addPage(_("General"));
    Gtk::VBox *prefsbox = addPage(_("Preferences"));
    Gtk::VBox *fontbox = addPage(_("Font selection"));
    Gtk::VBox *performbox = addPage(_("Servers"));

    // General options-tab

    // Apply and Cancel buttons
    Gtk::HBox *hboxgeneral = manage(new Gtk::HBox());
    Gtk::Button *buttgeneral1 = manage(create_imagebutton(_("Apply settings"), Gtk::Stock::APPLY));
    Gtk::Button *buttgeneral2 = manage(create_imagebutton(_("Cancel"), Gtk::Stock::CANCEL));
    buttgeneral1->signal_clicked().connect(slot(*this, &Prefs::applyGeneral));
    buttgeneral2->signal_clicked().connect(slot(*this, &Prefs::cancelGeneral));
    hboxgeneral->pack_end(*buttgeneral2, Gtk::PACK_SHRINK);
    hboxgeneral->pack_end(*buttgeneral1, Gtk::PACK_SHRINK);
    generalbox->pack_end(*hboxgeneral, Gtk::PACK_SHRINK);

    int row = 1;

    // IRC nick
    ircnickentry.set_text(App->options.nick);
    Gtk::Label *glabel0 = manage(new Gtk::Label(_("Nickname:"), Gtk::ALIGN_LEFT));
    _general_table.attach(*glabel0, 0, 1, row, row + 1);
    _general_table.attach(ircnickentry, 1, 2, row, row + 1);

    row++;

    // Real name
    realnameentry.set_text(App->options.realname);
    Gtk::Label *glabel1 = manage(new Gtk::Label(_("Real name:"), Gtk::ALIGN_LEFT));
    _general_table.attach(*glabel1, 0, 1, row, row + 1);
    _general_table.attach(realnameentry, 1, 2, row, row + 1);

    row++;

    // IRC username
    ircuserentry.set_text(App->options.ircuser);
    Gtk::Label *glabel2 = manage(new Gtk::Label(_("IRC username (ident):"), Gtk::ALIGN_LEFT));
    _general_table.attach(*glabel2, 0, 1, row, row + 1);
    _general_table.attach(ircuserentry, 1, 2, row, row + 1);

    row++;

    generalbox->pack_start(_general_table, Gtk::PACK_SHRINK);

    // Encoding
    encodingcombo.set_popdown_strings(encodings);
    Gtk::Label *glabel3 = manage(new Gtk::Label(_("Encoding to use on IRC:"), Gtk::ALIGN_LEFT));
    _general_table.attach(*glabel3, 0, 1, row, row + 1);
    _general_table.attach(encodingcombo, 1, 2, row, row + 1);

    // Preferences-tab

    // Apply and Cancel buttons
    Gtk::HBox *hboxprefs = manage(new Gtk::HBox());
    Gtk::Button *buttprefs1 = manage(create_imagebutton(_("Apply settings"), Gtk::Stock::APPLY));
    Gtk::Button *buttprefs2 = manage(create_imagebutton(_("Cancel"), Gtk::Stock::CANCEL));
    buttprefs1->signal_clicked().connect(slot(*this, &Prefs::applyPreferences));
    buttprefs2->signal_clicked().connect(slot(*this, &Prefs::cancelPreferences));
    hboxprefs->pack_end(*buttprefs2, Gtk::PACK_SHRINK);
    hboxprefs->pack_end(*buttprefs1, Gtk::PACK_SHRINK);
    prefsbox->pack_end(*hboxprefs, Gtk::PACK_SHRINK);

    row = 1;

    // nickcompletion character
    nickcompletionentry.set_max_length(1);
    nickcompletionentry.set_text(App->options.nickcompletion_char().getString());
    Gtk::Label *plabel0 = manage(new Gtk::Label(_("Nick-completion character:"), Gtk::ALIGN_LEFT));
    _prefs_table.attach(*plabel0, 0, 1, row, row + 1);
    _prefs_table.attach(nickcompletionentry, 1, 2, row, row + 1);

    row++;

    // DCC ip
    dccipentry.set_text(App->options.dccip().getString());
    Gtk::Label *plabel1 = manage(new Gtk::Label(_("DCC IP address:"), Gtk::ALIGN_LEFT));
    _prefs_table.attach(*plabel1, 0, 1, row, row + 1);
    _prefs_table.attach(dccipentry, 1, 2, row, row + 1);

    row++;

    // DCC port
    dccportentry.set_text(App->options.dccport().getString());
    Gtk::Label *plabel2 = manage(new Gtk::Label(_("DCC Port (0 = random):"), Gtk::ALIGN_LEFT));
    _prefs_table.attach(*plabel2, 0, 1, row, row + 1);
    _prefs_table.attach(dccportentry, 1, 2, row, row + 1);

    row++;

    // Highligted words
    highlightentry.set_text(App->options.highlight_words);
    Gtk::Label *plabel3 = manage(new Gtk::Label(_("Words to highlight on (space seperated):"), Gtk::ALIGN_LEFT));
    _prefs_table.attach(*plabel3, 0, 1, row, row + 1);
    _prefs_table.attach(highlightentry, 1, 2, row, row + 1);

    row++;

    // Buffer size for text
    bufferentry.set_text(App->options.buffer_size().getString());
    Gtk::Label *plabel4 = manage(new Gtk::Label(_("Maxmium number of lines to cache:"), Gtk::ALIGN_LEFT));
    _prefs_table.attach(*plabel4, 0, 1, row, row + 1);
    _prefs_table.attach(bufferentry, 1, 2, row, row + 1);

    prefsbox->pack_start(_prefs_table, Gtk::PACK_SHRINK);

    // Limited tab highlighting
    highlightingbutton.set_active(App->options.limited_highlighting);
    prefsbox->pack_start(highlightingbutton, Gtk::PACK_SHRINK);

    // Strip colors
    stripcolorsbutton.set_active(App->options.strip_colors);
    prefsbox->pack_start(stripcolorsbutton, Gtk::PACK_SHRINK);

    // Strip bold and underline
    stripothersbutton.set_active(App->options.strip_boldandunderline);
    prefsbox->pack_start(stripothersbutton, Gtk::PACK_SHRINK);

    // Logging
    loggingbutton.set_active(App->options.logging);
    prefsbox->pack_start(loggingbutton, Gtk::PACK_SHRINK);

    // Font selection

    // Apply and Cancel buttons
    Gtk::HBox *hboxfont = manage(new Gtk::HBox());
    Gtk::Button *buttfont1 = manage(create_imagebutton(_("Apply font"), Gtk::Stock::APPLY));
    Gtk::Button *buttfont2 = manage(create_imagebutton(_("Cancel"), Gtk::Stock::CANCEL));
    buttfont1->signal_clicked().connect(slot(*this, &Prefs::applyFont));
    buttfont2->signal_clicked().connect(slot(*this, &Prefs::cancelFont));
    hboxfont->pack_end(*buttfont2, Gtk::PACK_SHRINK);
    hboxfont->pack_end(*buttfont1, Gtk::PACK_SHRINK);
    fontbox->pack_end(*hboxfont, Gtk::PACK_SHRINK);

    fontsel.set_preview_text("<" + ircnickentry.get_text() + _("> Hello World!"));
    if (!App->options.font->empty())
          fontsel.set_font_name(App->options.font);
    fontbox->pack_start(fontsel);

    // Autojoin/perform-tab

    Gtk::HPaned *server_pane = manage(new Gtk::HPaned());

    _treeview.append_column(_("Auto"), _columns.auto_connect);
    _treeview.append_column(_("Servers"), _columns.servername);
    _treeview.get_selection()->signal_changed().connect(slot(*this, &Prefs::onChangeRow));

    vector<Server*> servers = App->cfgservers.getServers();
    vector<Server*>::iterator i;

    for (i = servers.begin(); i != servers.end(); ++i) {
        Gtk::TreeModel::Row row = *_liststore->append();
        row[_columns.auto_connect] = (*i)->auto_connect;
        row[_columns.servername] = (*i)->hostname;
        row[_columns.autojoin] = *i;
    }
    server_pane->pack1(_treeview);
    Gtk::VBox *serverinfobox = manage(new Gtk::VBox());
    server_pane->pack2(*serverinfobox);

    // auto connect
    serverinfobox->pack_start(auto_connect_button, Gtk::PACK_SHRINK);

    serverinfobox->pack_start(_server_options_table, Gtk::PACK_SHRINK);

    row = 1;

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
    Gtk::Label *label4 = manage(new Gtk::Label(_("Nick:"), Gtk::ALIGN_LEFT));
    _server_options_table.attach(*label4, 0, 1, row, row + 1);
    _server_options_table.attach(nickentry, 1, 2, row, row + 1);

    // commmands
    cmdtext.set_editable(true);
    Gtk::Label *label5 = manage(new Gtk::Label(_("Commmands to perform on connect:")));
    serverinfobox->pack_start(*label5, Gtk::PACK_SHRINK);
    serverinfobox->pack_start(cmdtext);

    // buttons
    Gtk::Button *savebutton = manage(create_imagebutton(_("Save this entry"), Gtk::Stock::SAVE));
    savebutton->signal_clicked().connect(slot(*this, &Prefs::saveEntry));
    hboxserver.pack_end(*savebutton, Gtk::PACK_SHRINK);
    serverinfobox->pack_end(hboxserver, Gtk::PACK_SHRINK);

    addnewbutton = manage(create_imagebutton(_("Add entry"), Gtk::Stock::ADD));
    addnewbutton->signal_clicked().connect(slot(*this, &Prefs::addEntry));
    hboxserver.pack_end(*addnewbutton, Gtk::PACK_SHRINK);

    removebutton = manage(create_imagebutton(_("Remove entry"), Gtk::Stock::REMOVE));
    removebutton->signal_clicked().connect(slot(*this, &Prefs::removeEntry));
    hboxserver.pack_end(*removebutton, Gtk::PACK_SHRINK);

    performbox->pack_start(*server_pane);

    show_all();
    removebutton->hide();
    addnewbutton->hide();
}

void Prefs::applyPreferences()
{
    App->options.nickcompletion_char = nickcompletionentry.get_text();
    App->options.dccip = dccipentry.get_text();
    App->options.dccport = dccportentry.get_text();
    App->options.highlight_words = highlightentry.get_text();
    App->options.buffer_size =bufferentry.get_text();

    App->options.limited_highlighting = highlightingbutton.get_active();
    App->options.strip_colors = stripcolorsbutton.get_active();
    App->options.strip_boldandunderline = stripothersbutton.get_active();
    App->options.logging = loggingbutton.get_active();
}

void Prefs::applyGeneral()
{
    App->options.realname = realnameentry.get_text();
    App->options.ircuser = ircuserentry.get_text();
    App->options.nick = ircnickentry.get_text();
    Glib::ustring encoding = encodingcombo.get_entry()->get_text();
    App->options.encoding = encoding.substr(0, encoding.find_first_of(' '));
}

void Prefs::applyFont()
{
    App->options.font = fontsel.get_font_name();
    AppWin->getNotebook().setFont(fontsel.get_font_name());
}

void Prefs::cancelPreferences()
{
    nickcompletionentry.set_text(App->options.nickcompletion_char().getString());
    dccipentry.set_text(App->options.dccip);
    dccportentry.set_text(App->options.dccport().getString());
    highlightentry.set_text(App->options.highlight_words);
    bufferentry.set_text(App->options.buffer_size().getString());

    highlightingbutton.set_active(App->options.limited_highlighting);
    stripcolorsbutton.set_active(App->options.strip_colors);
    stripothersbutton.set_active(App->options.strip_boldandunderline);
    loggingbutton.set_active(App->options.logging);
}

void Prefs::cancelGeneral()
{
    realnameentry.set_text(App->options.realname);
    ircuserentry.set_text(App->options.ircuser);
    ircnickentry.set_text(App->options.nick);
}

void Prefs::cancelFont()
{
    if (!App->options.font->empty())
          fontsel.set_font_name(App->options.font);
}

void Prefs::saveEntry()
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

void Prefs::onChangeRow()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _treeview.get_selection();
    Gtk::TreeModel::iterator iterrow = selection->get_selected();

    if (iterrow) {
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

    Server *autojoin = row[_columns.autojoin];
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
    vbox->set_border_width(12);
    notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem(*vbox, str));
    return vbox;
}
