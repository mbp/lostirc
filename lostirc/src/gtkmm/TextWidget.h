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

#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

#include <map>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>
#include <gtkmm/style.h>
#include "TextWidget.h"

class TextWidget : public Gtk::ScrolledWindow
{
public:
    TextWidget(Pango::FontDescription font);

    TextWidget& operator<<(const char * str);
    TextWidget& operator<<(const std::string& str);
    TextWidget& operator<<(const Glib::ustring& str);

    void clearText();
    void setFont(const Pango::FontDescription& font);

private:
    void insertWithColor(int color, const Glib::ustring& str);
    void realInsert(int color, const Glib::ustring& str);
    void setStyle();

    void initializeColorMap();
    void helperInitializer(int i, const Glib::ustring& colorname);

    Gtk::TextView _textview;

    std::map<int, Glib::RefPtr<Gtk::TextTag> > colorMap;
    Glib::RefPtr<Gtk::TextTag> underlinetag;

    std::string _fallback_encoding;

};

#endif
