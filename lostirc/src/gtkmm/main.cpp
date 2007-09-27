/* 
 * Copyright (C) 2002-2004 Morten Brix Pedersen <morten@wtf.dk>
 * Copyright (C) 2007 Martin Braure de Calignon <braurede@free.fr>
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

#include <config.h>
#include <glibmm/ustring.h>
#include "MainWindow.h"
#include <LostIRC.h>

const char *const icon_filename = LOSTIRC_DATADIR G_DIR_SEPARATOR_S "pixmaps" G_DIR_SEPARATOR_S "lostirc.png";

int main(int argc, char** argv)
{
    bool autoconnect = true;

    for (int i = 0; i < argc; ++i)
          if (Glib::ustring(argv[i]).find("--noauto") != Glib::ustring::npos)
                autoconnect = false;

    Gtk::Main app(argc, argv);

    try {
        Gtk::Window::set_default_icon_from_file(icon_filename);
    } catch (Glib::Error e) {
        // Probably an error. Ignore it.
    }

    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "UTF-8");
    textdomain(PACKAGE);

    MainWindow window(autoconnect);

    app.run();
    window.show();
}
