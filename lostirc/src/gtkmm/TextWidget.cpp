/* 
 * Copyright (C) 2002-2005 Morten Brix Pedersen <morten@wtf.dk>
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

#include <gtkmm/scrollbar.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/style.h>
#include "TextWidget.h"
#include "MainWindow.h"

using std::vector;
using Glib::ustring;

TextWidget::TextWidget(Pango::FontDescription font)
    : highlight_mark_pos(highlight_marks.rend())
{
    _textview.set_wrap_mode(Gtk::WRAP_WORD_CHAR);
    _textview.unset_flags(Gtk::CAN_FOCUS);
    _textview.set_editable(false);

    set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    set_size_request(0, -1);
    add(_textview);
    setStyle();
    _textview.modify_font(font);
    Glib::RefPtr<Gtk::TextBuffer> buffer = Gtk::TextBuffer::create(AppWin->_current_tag_table);
    _textview.set_buffer(buffer);
    pos = buffer->create_mark(buffer->end());
    scrollToBottom();
    get_vscrollbar()->signal_size_allocate().connect(sigc::mem_fun(*this, &TextWidget::onResize));
    get_vscrollbar()->signal_value_changed().connect(sigc::mem_fun(*this, &TextWidget::onScroll));

    _textview.signal_populate_popup().connect(sigc::mem_fun(*this, &TextWidget::populateMenu));
}

void TextWidget::onScroll()
{
    if (get_vscrollbar()->get_value() >= (get_vscrollbar()->get_adjustment()->get_upper() - get_vscrollbar()->get_adjustment()->get_page_size() - 1e-12)) {
        scrollToBottom();
    }
}

void TextWidget::onResize(Gtk::Allocation& alloc)
{
    if (get_vscrollbar()->get_value() >= (get_vscrollbar()->get_adjustment()->get_upper() - get_vscrollbar()->get_adjustment()->get_page_size() - 1e-12)) {
        scrollToBottom();
    } else {
        _textview.scroll_to(pos, 0.0, 0.5, 1.0);
    }
}

void TextWidget::setHighlightMark()
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
    highlight_marks.push_back(buffer->create_mark(buffer->end()));
    highlight_mark_pos = highlight_marks.rbegin();
}

void TextWidget::scrollToHighlightMark()
{
    if (!highlight_marks.empty()) {
        highlight_mark_pos++;
        if (highlight_mark_pos == highlight_marks.rend())
              highlight_mark_pos = highlight_marks.rbegin();

        _textview.scroll_to(*highlight_mark_pos, 0.1);
    }
}

void TextWidget::scrollUpPage()
{
    Gtk::Adjustment *vadj = get_vadjustment();

    double value = vadj->get_value() - (vadj->get_page_size() - 1);

    get_vscrollbar()->set_value(value);
}

void TextWidget::scrollDownPage()
{
    Gtk::Adjustment *vadj = get_vadjustment();

    double value = vadj->get_value() + (vadj->get_page_size() - 1);

    get_vscrollbar()->set_value(value);
}

void TextWidget::scrollToBottom()
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
    buffer->move_mark(pos, buffer->end());
    _textview.scroll_to(pos, 0.0);
}

void TextWidget::scrollToTop()
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
    buffer->move_mark(pos, buffer->begin());
    _textview.scroll_to(pos, 0.0);
}

void TextWidget::setStyle() {
    // TODO: Should this go into a ressource file?
    Gdk::Color col1(AppWin->background_color);

    _textview.modify_base(Gtk::STATE_NORMAL, col1);
}

void TextWidget::clearText()
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
    buffer->erase(buffer->begin(), buffer->end());
}

void TextWidget::setFont(const Pango::FontDescription& font)
{
    _textview.modify_font(font);
}

TextWidget& TextWidget::operator<<(const char * str)
{
    return operator<<(std::string(str));
}

TextWidget& TextWidget::operator<<(const ustring& line)
{
    // see if the scrollbar is located in the bottom, then we need to scroll
    // after insert
    bool need_to_scroll = false;
    if (get_vadjustment()->get_value() >= (get_vadjustment()->get_upper() - get_vadjustment()->get_page_size() - 1e-12))
          need_to_scroll = true;

    TextProperties tp;
    tp.clear();
    // Parse string and insert character by character
    for (ustring::size_type i = 0; i < line.length(); ++i)
    {
        if (line[i] == '\017') { // RESET
            tp.clear();
        } else if (line[i] == '\002') { // BOLD
            tp.bold = !tp.bold;
        } else if (line[i] == '\037') { // UNDERLINE
            tp.underline = !tp.underline;
        } else if (line[i] == '\003') { // COLOR
            tp.fgcolor = true;
            tp.numbercount = 0;
            tp.fgnumber.clear();
            tp.bgnumber.clear();
        } else if (tp.fgcolor && isdigit(line[i]) && tp.numbercount < 2) {
            tp.numbercount++;
            tp.fgnumber += line[i];
        } else if (tp.fgcolor && line[i] == ',' && tp.numbercount < 3) {
            tp.numbercount = 0;
            tp.bgcolor = true;
            tp.fgcolor = false;
        } else if (tp.bgcolor && isdigit(line[i]) && tp.numbercount < 2) {
            tp.numbercount++; 
            tp.bgnumber += line[i];
        } else {
            tp.numbercount = 0;
            tp.fgcolor = false;
            tp.bgcolor = false;

            if (tp.bgnumber.empty())
                  tp.bgnumber = "1";
            if (tp.fgnumber.empty())
                  tp.fgnumber = "0";

            Glib::ustring text;
            text = line[i];
            insertText(tp, text);
        }   
    } 

    removeTopBuffer();

    if (need_to_scroll) {
        scrollToBottom();
    }

    return *this;
}

Glib::ustring crop(Glib::ustring str)
{
    if (str.size() > 1 && str[0] == '0')
          return str.substr(1);
    else
          return str;
}

void TextWidget::insertText(const TextProperties& tp, const ustring& line)
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();

    std::vector< Glib::RefPtr<Gtk::TextTag> > tags;

    Glib::RefPtr<Gtk::TextTag> fg = buffer->get_tag_table()->lookup(Glib::ustring("f")+crop(tp.fgnumber));
    if (fg == 0)
          fg = buffer->get_tag_table()->lookup("f0");

    Glib::RefPtr<Gtk::TextTag> bg = buffer->get_tag_table()->lookup(Glib::ustring("b")+crop(tp.bgnumber));
    if (bg == 0)
          bg = buffer->get_tag_table()->lookup("b0");

    tags.push_back(fg);
    tags.push_back(bg);

    if (tp.bold)
          tags.push_back(buffer->get_tag_table()->lookup("B"));
    if (tp.underline)
          tags.push_back(buffer->get_tag_table()->lookup("U"));

    buffer->insert_with_tags(buffer->end(), line, tags);

#if 0
    /* below URL handling is broken at the moment, so disabled. It's broken
     * because we only receive one character at the time in this function.
    */
    ustring::size_type pos1;

    pos1 = line.find("http:");

    if (pos1 == ustring::npos)
          pos1 = line.find("www.");

    if (pos1 == ustring::npos)
          pos1 = line.find("ftp.");

    if (pos1 == ustring::npos)
          pos1 = line.find("ftp:");

    if (pos1 != ustring::npos) {
        // Found an URL - insert the front and end of the line, and insert
        // the URL with special markup.
        ustring::size_type pos2 = line.find(" ", pos1 + 1);

        // What's before the URL
        buffer->insert_with_tags(buffer->end(), line.substr(0, pos1), tags),

        // The URL
        buffer->insert_with_tag(buffer->end(), line.substr(pos1, pos2 - pos1), underlinetag);

        // After the URL
        if (pos2 != ustring::npos)
              buffer->insert_with_tags(buffer->end(), line.substr(pos2), tags);
    } else {
        // Just insert the line, no URLs were found
        buffer->insert_with_tags(buffer->end(), line, tags);
    }
#endif
}

void TextWidget::removeTopBuffer()
{
    // Remove X number of lines from top of the buffer which we don't want.
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();

    // FIXME: possible performance critical
    int buffer_size = App->options.buffer_size;
    if (buffer_size && buffer->get_line_count() > buffer_size)
          buffer->erase(buffer->begin(), buffer->get_iter_at_line(buffer->get_line_count() - buffer_size));
}

void TextWidget::populateMenu(Gtk::Menu *contextmenu)
{
    Gtk::Menu::MenuList& menulist = contextmenu->items();

    menulist.push_back(Gtk::Menu_Helpers::MenuElem(("_Menubar"), Gtk::AccelKey("<control>m"), sigc::mem_fun(*AppWin, &MainWindow::hideMenu)));
}
