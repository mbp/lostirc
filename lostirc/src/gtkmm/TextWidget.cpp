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
    _textview.set_wrap_mode(Gtk::WRAP_CHAR);
    _textview.unset_flags(Gtk::CAN_FOCUS);
    _textview.set_editable(false);

    set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    set_size_request(0, -1);
    add(_textview);
    initializeColorMap();
    setStyle();
    _textview.modify_font(font);
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
    pos = buffer->create_mark(buffer->end());
    get_vscrollbar()->signal_size_allocate().connect(SigC::slot(*this, &TextWidget::onResize));
    get_vscrollbar()->signal_value_changed().connect(SigC::slot(*this, &TextWidget::onScroll));
}

void TextWidget::onResize(GtkAllocation *alloc)
{
    _textview.scroll_to_mark(pos, 0.0);
}

void TextWidget::onScroll()
{
    _textview.move_mark_onscreen(pos);
    if (get_vscrollbar()->get_value() >= (get_vscrollbar()->get_adjustment()->get_upper() - get_vscrollbar()->get_adjustment()->get_page_size() - 1e-12)) {
        Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
        pos = buffer->create_mark(buffer->end());
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

        _textview.scroll_to_mark(*highlight_mark_pos, 0.1, 0.0, 1.0);
    }
}

void TextWidget::scrollUpPage()
{
    Gtk::Adjustment *vadj = get_vadjustment();

    double value = vadj->get_value() - (vadj->get_page_size() - 1);

    if (value < 0)
          value = 0;

    vadj->set_value(value);
}

void TextWidget::scrollDownPage()
{
    Gtk::Adjustment *vadj = get_vadjustment();

    double end = vadj->get_upper() - vadj->get_lower() - vadj->get_page_size();
    double value = vadj->get_value() + (vadj->get_page_size() - 1);

    if (value > end)
          value = end;

    vadj->set_value(value);
}

void TextWidget::scrollToBottom()
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
    Gtk::TextIter iter = buffer->end();
    _textview.scroll_to_iter(iter, 0.0);
}

void TextWidget::scrollToTop()
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
    Gtk::TextIter iter = buffer->begin();
    _textview.scroll_to_iter(iter, 0.0);
}

void TextWidget::setStyle() {
    // TODO: Should this go into a ressource file?
    Gdk::Color col1(App->colors.bgcolor);

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

    // scroll if we need to
    if (need_to_scroll) {
        Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();
        pos = buffer->create_mark(buffer->end());
        _textview.scroll_to_mark(pos, 0.0);
    }

    removeTopBuffer();

    return *this;
}

void TextWidget::insertText(const TextProperties& tp, const ustring& line)
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();

    std::vector< Glib::RefPtr<Gtk::TextTag> > tags;
    if (Util::stoi(tp.fgnumber) > fgColorMap.size())
          tags.push_back(fgColorMap[0]);
    else
          tags.push_back(fgColorMap[Util::stoi(tp.fgnumber)]);

    if (Util::stoi(tp.bgnumber) > bgColorMap.size())
          tags.push_back(bgColorMap[1]);
    else
          tags.push_back(bgColorMap[Util::stoi(tp.bgnumber)]);

    if (tp.bold)
          tags.push_back(boldtag);
    if (tp.underline)
          tags.push_back(underlinetag);

    buffer->insert_with_tags(buffer->end(), line, tags);

    /* below URL handling is broken at the moment, so disabled. It's broken
     * because we only receive one character at the time in this function.
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
    */
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

Glib::RefPtr<Gtk::TextTag> TextWidget::initializeFG(const Glib::ustring& colorname)
{
    Glib::RefPtr<Gtk::TextTag> texttag = Gtk::TextTag::create();
    texttag->property_foreground() = colorname;
    _textview.get_buffer()->get_tag_table()->add(texttag);
    return texttag;
}

Glib::RefPtr<Gtk::TextTag> TextWidget::initializeBG(const Glib::ustring& colorname)
{
    Glib::RefPtr<Gtk::TextTag> texttag = Gtk::TextTag::create();
    texttag->property_background() = colorname;
    _textview.get_buffer()->get_tag_table()->add(texttag);
    return texttag;
}

void TextWidget::initializeColorMap()
{
    int i = 0;
    fgColorMap[i++] = initializeFG(App->colors.color0);
    fgColorMap[i++] = initializeFG(App->colors.color1);
    fgColorMap[i++] = initializeFG(App->colors.color2);
    fgColorMap[i++] = initializeFG(App->colors.color3);
    fgColorMap[i++] = initializeFG(App->colors.color4);
    fgColorMap[i++] = initializeFG(App->colors.color5);
    fgColorMap[i++] = initializeFG(App->colors.color6);
    fgColorMap[i++] = initializeFG(App->colors.color7);
    fgColorMap[i++] = initializeFG(App->colors.color8);
    fgColorMap[i++] = initializeFG(App->colors.color9);
    fgColorMap[i++] = initializeFG(App->colors.color10);
    fgColorMap[i++] = initializeFG(App->colors.color11);
    fgColorMap[i++] = initializeFG(App->colors.color12);
    fgColorMap[i++] = initializeFG(App->colors.color13);
    fgColorMap[i++] = initializeFG(App->colors.color14);
    fgColorMap[i++] = initializeFG(App->colors.color15);
    fgColorMap[i++] = initializeFG(App->colors.color16);
    fgColorMap[i++] = initializeFG(App->colors.color17);
    fgColorMap[i++] = initializeFG(App->colors.color18);
    fgColorMap[i++] = initializeFG(App->colors.color19);

    i = 0;
    bgColorMap[i++] = initializeBG(App->colors.color0);
    bgColorMap[i++] = initializeBG(App->colors.color1);
    bgColorMap[i++] = initializeBG(App->colors.color2);
    bgColorMap[i++] = initializeBG(App->colors.color3);
    bgColorMap[i++] = initializeBG(App->colors.color4);
    bgColorMap[i++] = initializeBG(App->colors.color5);
    bgColorMap[i++] = initializeBG(App->colors.color6);
    bgColorMap[i++] = initializeBG(App->colors.color7);
    bgColorMap[i++] = initializeBG(App->colors.color8);
    bgColorMap[i++] = initializeBG(App->colors.color9);
    bgColorMap[i++] = initializeBG(App->colors.color10);
    bgColorMap[i++] = initializeBG(App->colors.color11);
    bgColorMap[i++] = initializeBG(App->colors.color12);
    bgColorMap[i++] = initializeBG(App->colors.color13);
    bgColorMap[i++] = initializeBG(App->colors.color14);
    bgColorMap[i++] = initializeBG(App->colors.color15);
    bgColorMap[i++] = initializeBG(App->colors.color16);
    bgColorMap[i++] = initializeBG(App->colors.color17);
    bgColorMap[i++] = initializeBG(App->colors.color18);
    bgColorMap[i++] = initializeBG(App->colors.color19);

    // Create a underlined-tag.
    underlinetag = Gtk::TextTag::create();
    underlinetag->property_underline() = Pango::UNDERLINE_SINGLE;
    underlinetag->property_foreground() = App->colors.color0;
    _textview.get_buffer()->get_tag_table()->add(underlinetag);

    // Create a bold-tag
    boldtag = Gtk::TextTag::create();
    boldtag->property_weight() = Pango::WEIGHT_BOLD;
    _textview.get_buffer()->get_tag_table()->add(boldtag);
}
