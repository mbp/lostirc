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

#include <gtkmm/optionmenu.h>
#include <gtkmm/separator.h>
#include <Utils.h>
#include "MainWindow.h"
#include "Prefs.h"

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

Prefs::Prefs()
    : Gtk::Window(Gtk::WINDOW_TOPLEVEL),
    bufferadj(1.0, 1.0, 20000.0, 50.0, 100.0, 0.0),
    bufferspin(bufferadj),
    stripcolorsbutton(_("Strip _color codes from incoming messages"), true),
    stripothersbutton(_("Strip _bold and underline codes from incoming messages"), true),
    loggingbutton(_("_Log conversations to disk"), true),
    _general_table(2, 2),
    _prefs_table(2, 3),
    _dcc_table(2, 2)
{
    set_title(_("LostIRC Preferences"));
    notebook.set_tab_pos(Gtk::POS_TOP);
    _general_table.set_row_spacings(6);
    _general_table.set_col_spacings(12);
    _prefs_table.set_row_spacings(6);
    _prefs_table.set_col_spacings(12);
    _dcc_table.set_row_spacings(6);
    _dcc_table.set_col_spacings(12);

    set_border_width(5);
    set_resizable(false);
    mainvbox.pack_start(notebook);
    mainvbox.set_spacing(6);

    add(mainvbox);

    Gtk::VBox *generalbox = addPage(_("General"));
    Gtk::VBox *prefsbox = addPage(_("Preferences"));
    Gtk::VBox *dccbox = addPage(_("DCC"));
    //Gtk::VBox *colourbox = addPage(_("Colours"));

    // General options-tab
    int row = 1;

    // IRC username
    ircuserentry.set_text(App->options.ircuser);
    Gtk::Label *glabel2 = manage(new Gtk::Label(_("IRC username (ident):"), Gtk::ALIGN_LEFT));
    _general_table.attach(*glabel2, 0, 1, row, row + 1);
    _general_table.attach(ircuserentry, 1, 2, row, row + 1);

    row++;

    generalbox->pack_start(_general_table, Gtk::PACK_SHRINK);

    // Encoding
    encodingcombo.set_popdown_strings(encodings);
    encodingcombo.get_entry()->set_text(App->options.encoding);
    Gtk::Label *glabel3 = manage(new Gtk::Label(_("Encoding to use on IRC:"), Gtk::ALIGN_LEFT));
    _general_table.attach(*glabel3, 0, 1, row, row + 1);
    _general_table.attach(encodingcombo, 1, 2, row, row + 1);

    row++;

    // Font
    if (!App->options.font->empty())
          fontentry.set_text(App->options.font);
    fontentry.set_sensitive(false);
    Gtk::HBox *fontbox = manage(new Gtk::HBox());
    Gtk::Button *fontbutton = manage(new Gtk::Button(_("Browse...")));
    fontbutton->signal_clicked().connect(slot(*this, &Prefs::openFontWindow));
    fontbox->pack_start(fontentry, Gtk::PACK_EXPAND_WIDGET);
    fontbox->pack_start(*fontbutton, Gtk::PACK_SHRINK);
    Gtk::Label *glabel4 = manage(new Gtk::Label(_("Main window font:"), Gtk::ALIGN_LEFT));
    _general_table.attach(*glabel4, 0, 1, row, row + 1);
    _general_table.attach(*fontbox, 1, 2, row, row + 1);

    // Preferences-tab
    row = 1;

    // nickcompletion character
    nickcompletionentry.set_max_length(1);
    nickcompletionentry.set_text(App->options.nickcompletion_char().getString());
    Gtk::Label *plabel0 = manage(new Gtk::Label(_("Nick-completion character:"), Gtk::ALIGN_LEFT));
    _prefs_table.attach(*plabel0, 0, 1, row, row + 1);
    _prefs_table.attach(nickcompletionentry, 1, 2, row, row + 1);

    row++;

    // Highlighted words
    highlightentry.set_text(App->options.highlight_words);
    Gtk::Label *plabel3 = manage(new Gtk::Label(_("Words to highlight on (space seperated):"), Gtk::ALIGN_LEFT));
    _prefs_table.attach(*plabel3, 0, 1, row, row + 1);
    _prefs_table.attach(highlightentry, 1, 2, row, row + 1);

    row++;

    // Buffer size for text
    bufferspin.set_value(App->options.buffer_size());
    Gtk::Label *plabel4 = manage(new Gtk::Label(_("Maximium number of lines to cache:"), Gtk::ALIGN_LEFT));
    _prefs_table.attach(*plabel4, 0, 1, row, row + 1);
    _prefs_table.attach(bufferspin, 1, 2, row, row + 1);

    prefsbox->pack_start(_prefs_table, Gtk::PACK_SHRINK);

    // Strip colors
    stripcolorsbutton.set_active(App->options.strip_colors);
    prefsbox->pack_start(stripcolorsbutton, Gtk::PACK_SHRINK);

    // Strip bold and underline
    stripothersbutton.set_active(App->options.strip_boldandunderline);
    prefsbox->pack_start(stripothersbutton, Gtk::PACK_SHRINK);

    // Logging
    loggingbutton.set_active(App->options.logging);
    prefsbox->pack_start(loggingbutton, Gtk::PACK_SHRINK);

    row = 1;

    // DCC ip
    dccipentry.set_text(App->options.dccip().getString());
    Gtk::Label *plabel1 = manage(new Gtk::Label(_("DCC IP address:"), Gtk::ALIGN_LEFT));
    _dcc_table.attach(*plabel1, 0, 1, row, row + 1);
    _dcc_table.attach(dccipentry, 1, 2, row, row + 1);

    row++;

    // DCC port
    dccportentry.set_text(App->options.dccport().getString());
    Gtk::Label *plabel2 = manage(new Gtk::Label(_("DCC Port (0 = random):"), Gtk::ALIGN_LEFT));
    _dcc_table.attach(*plabel2, 0, 1, row, row + 1);
    _dcc_table.attach(dccportentry, 1, 2, row, row + 1);

    dccbox->pack_start(_dcc_table, Gtk::PACK_SHRINK);

    // Colour-tab
    Gtk::OptionMenu *optionmenu = new Gtk::OptionMenu();

    Gtk::Menu *colorschemes = new Gtk::Menu();

    optionmenu->set_menu(*colorschemes);

    {
        Gtk::Menu::MenuList& menulist = colorschemes->items();

        menulist.push_back( Gtk::Menu_Helpers::MenuElem("White on black",
                    SigC::slot(*this, &Prefs::saveSettings) ) );

        menulist.push_back( Gtk::Menu_Helpers::MenuElem("Black on white",
                    SigC::slot(*this, &Prefs::saveSettings) ) );

    }

    colorschemes->set_active(1);

    //colourbox->pack_start(*optionmenu, Gtk::PACK_SHRINK);



    // Final setup
    Gtk::HBox *closehbox = manage(new Gtk::HBox());
    Gtk::Button *closebutton = manage(new Gtk::Button(Gtk::Stock::CLOSE));
    closebutton->signal_clicked().connect(slot(*this, &Prefs::onClose));
    closehbox->pack_end(*closebutton, Gtk::PACK_SHRINK);
    mainvbox.pack_start(*closehbox);


    show_all();
}

void Prefs::saveSettings()
{
    // General.
    App->options.ircuser = ircuserentry.get_text();
    Glib::ustring encoding = encodingcombo.get_entry()->get_text();
    App->options.encoding = encoding.substr(0, encoding.find_first_of(' '));

    // Preferences.
    App->options.nickcompletion_char = nickcompletionentry.get_text();
    App->options.dccip = dccipentry.get_text();
    App->options.dccport = dccportentry.get_text();
    App->options.highlight_words = highlightentry.get_text();
    App->options.buffer_size = static_cast<int>(bufferspin.get_value());

    App->options.strip_colors = stripcolorsbutton.get_active();
    App->options.strip_boldandunderline = stripothersbutton.get_active();
    App->options.logging = loggingbutton.get_active();
}

void Prefs::openFontWindow()
{
    Gtk::FontSelectionDialog dialog;

    if (!App->options.font->empty())
          dialog.set_font_name(App->options.font);

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK)
    {
        App->options.font = dialog.get_font_name();
        AppWin->getNotebook().setFont(dialog.get_font_name());
        fontentry.set_text(dialog.get_font_name());
    }
}

Gtk::VBox* Prefs::addPage(const Glib::ustring& str)
{
    Gtk::VBox *vbox = manage(new Gtk::VBox());
    vbox->set_border_width(12);
    notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem(*vbox, str));
    return vbox;
}
