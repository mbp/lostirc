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

#include <gtkmm/separator.h>
#include <Utils.h>
#include "MainWindow.h"
#include "Prefs.h"

const unsigned int encodings_size = 12;
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
    Gtk::VBox *colourbox = addPage(_("Colours"));

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
    unsigned int active = 0;
    for (unsigned int i = 0; i < encodings_size; i++)
    {
        Glib::ustring enc = encodings[i];
        if (enc.substr(0, enc.find_first_of(' ')) == App->options.encoding)
              active = i;
        encodingcombo.append_text(enc);
    }

    encodingcombo.set_active(active);
    Gtk::Label *glabel3 = manage(new Gtk::Label(_("Encoding to use on IRC:"), Gtk::ALIGN_LEFT));
    _general_table.attach(*glabel3, 0, 1, row, row + 1);
    _general_table.attach(encodingcombo, 1, 2, row, row + 1);

    row++;

    // Font
    fontbutton.signal_font_set().connect(sigc::mem_fun(*this, &Prefs::saveFont));
    if (!App->options.font->empty())
          fontbutton.set_font_name(App->options.font);
    Gtk::Label *glabel4 = manage(new Gtk::Label(_("Main window font:"), Gtk::ALIGN_LEFT));
    _general_table.attach(*glabel4, 0, 1, row, row + 1);
    _general_table.attach(fontbutton, 1, 2, row, row + 1);

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
    bufferspin.set_value(double(*App->options.buffer_size()));
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
    Gtk::Label *colorlabel1 = new Gtk::Label(_("Pick a colorscheme"));
    Gtk::Label *colorlabel2 = new Gtk::Label(_("Changes to colorschemes only apply to newly created tabs"));
    colorschemes.append_text(_("White on black"));
    colorschemes.append_text(_("Black on white"));
    colorschemes.set_active(0);
    colorschemes.signal_changed().connect(sigc::mem_fun(*this, &Prefs::changeColors));

    colourbox->pack_start(*colorlabel1, Gtk::PACK_SHRINK);
    colourbox->pack_start(colorschemes, Gtk::PACK_SHRINK);
    colourbox->pack_start(*colorlabel2, Gtk::PACK_SHRINK);

    // Final setup
    Gtk::HBox *closehbox = manage(new Gtk::HBox());
    Gtk::Button *closebutton = manage(new Gtk::Button(Gtk::Stock::CLOSE));
    closebutton->signal_clicked().connect(sigc::mem_fun(*this, &Prefs::onClose));
    closehbox->pack_end(*closebutton, Gtk::PACK_SHRINK);
    mainvbox.pack_start(*closehbox);


    show_all();
}

void Prefs::changeColors()
{
    if (colorschemes.get_active_row_number() == 0) {
        AppWin->_current_tag_table = AppWin->_tag_table1;
        AppWin->background_color = App->colors1.bgcolor;
    } else {
        AppWin->_current_tag_table = AppWin->_tag_table2;
        AppWin->background_color = App->colors2.bgcolor;
    }

}

void Prefs::saveSettings()
{
    // General.
    App->options.ircuser = ircuserentry.get_text();
    Glib::ustring encoding = encodingcombo.get_active_text();
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

void Prefs::saveFont()
{
    App->options.font = fontbutton.get_font_name();
    AppWin->getNotebook().setFont(fontbutton.get_font_name());
}

Gtk::VBox* Prefs::addPage(const Glib::ustring& str)
{
    Gtk::VBox *vbox = manage(new Gtk::VBox());
    vbox->set_border_width(12);
    notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem(*vbox, str));
    return vbox;
}
