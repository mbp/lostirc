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

#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

#include <map>
#include <vector>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textmark.h>
#include <gtkmm/textview.h>
#include <gtkmm/menu.h>


class TextWidget : public Gtk::ScrolledWindow
{
    struct TextProperties;
public:
    TextWidget(Pango::FontDescription font);

    TextWidget& operator<<(const char * str);
    TextWidget& operator<<(const Glib::ustring& str);

    void clearText();
    void setFont(const Pango::FontDescription& font);

    void setHighlightMark();
    void scrollToHighlightMark();

    void scrollUpPage();
    void scrollDownPage();
    void scrollToBottom();
    void scrollToTop();

private:
    void insertText(const TextProperties& tp, const Glib::ustring& str);
    void populateMenu(Gtk::Menu*);
    void setStyle();

    void onResize(GtkAllocation *alloc);
    void onScroll();

    void removeTopBuffer();

    void initializeColorMap();
    Glib::RefPtr<Gtk::TextTag> initializeFG(const Glib::ustring& colorname);
    Glib::RefPtr<Gtk::TextTag> initializeBG(const Glib::ustring& colorname);

    Gtk::TextView _textview;

    std::map<int, Glib::RefPtr<Gtk::TextTag> > fgColorMap;
    std::map<int, Glib::RefPtr<Gtk::TextTag> > bgColorMap;
    Glib::RefPtr<Gtk::TextTag> underlinetag;
    Glib::RefPtr<Gtk::TextTag> boldtag;
    Glib::RefPtr<Gtk::TextBuffer::Mark> pos;

    std::vector<Glib::RefPtr<Gtk::TextBuffer::Mark> > highlight_marks;
    std::vector<Glib::RefPtr<Gtk::TextBuffer::Mark> >::reverse_iterator highlight_mark_pos;

    struct TextProperties
    {
        bool fgcolor;
        bool bgcolor;
        bool bold;
        bool underline;
        int numbercount;
        Glib::ustring fgnumber;
        Glib::ustring bgnumber;
        void clear()
        {
            bgcolor = false;
            fgcolor = false;
            bold = false;
            underline = false;
            numbercount = 0;
            fgnumber.clear();
            bgnumber.clear();
        }
    };

};


#endif
