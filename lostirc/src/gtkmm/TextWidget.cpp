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

#include "TextWidget.h"
#include "MainWindow.h"

using std::vector;
using Glib::ustring;

TextWidget::TextWidget(Pango::FontDescription font)
    : _fallback_encoding("ISO-8859-15")
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
}

void TextWidget::setStyle() {
    // TODO: Should this go into a ressource file?
    Gdk::Color col1(convert_to_utf8(App->colors.bgcolor));

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

TextWidget& TextWidget::operator<<(const std::string& str)
{
    return operator<<(convert_to_utf8(str));
}

TextWidget& TextWidget::operator<<(const ustring& line)
{
    // Add timestamp
    time_t timeval = time(0);
    char tim[11];
    strftime(tim, 10, "%H:%M:%S ", localtime(&timeval));

    insertWithColor(0, ustring(tim));

    // FIXME: can be done prettier and better with TextBuffer marks

    ustring::size_type lastPos = line.find_first_not_of("\003", 0);
    ustring::size_type pos = line.find_first_of("\003", lastPos);

    while (ustring::npos != pos || ustring::npos != lastPos)
    {
        // Check for digits
        if (Util::isDigit(line.substr(lastPos, 2))) {
            int color = Util::stoi(line.substr(lastPos, 2));
            insertWithColor(color, line.substr(lastPos + 2, (pos - lastPos) - 2));
        } else if (Util::isDigit(line.substr(lastPos, 1))) {
            int color = Util::stoi(line.substr(lastPos, 1));
            insertWithColor(color, line.substr(lastPos + 1, (pos - lastPos) - 1));
        } else {
            insertWithColor(0, line.substr(lastPos, pos - lastPos));
        }

        lastPos = line.find_first_not_of("\003", pos);
        pos = line.find_first_of("\003", lastPos);
    }
    return *this;
}

void TextWidget::insertWithColor(int color, const ustring& str)
{
    // see if the scrollbar is located in the bottom, then we need to scroll
    // after insert
    bool scroll = false;
    if (get_vadjustment()->get_value() >= (get_vadjustment()->get_upper() - get_vadjustment()->get_page_size() - 1e-12))
          scroll = true;

    // FIXME: temp hack.
    if (color > colorMap.size())
        color = colorMap.size() - 1;

    // Insert the text
    realInsert(color, str);
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();

    if (scroll)
          _textview.scroll_to_mark(buffer->create_mark("e", buffer->end()), 0.0);

    // FIXME: possible performance critical
    int buffer_size = App->options.buffer_size;
    if (buffer_size && buffer->get_line_count() > buffer_size)
          buffer->erase(buffer->begin(), buffer->get_iter_at_line(buffer->get_line_count() - buffer_size));
}

void TextWidget::realInsert(int color, const ustring& line)
{
    // This function has the purpose to insert the line - but first check to
    // see whether we have an URL.
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();

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
        buffer->insert_with_tag(buffer->end(), line.substr(0, pos1), colorMap[color]);

        // The URL
        buffer->insert_with_tag(buffer->end(), line.substr(pos1, pos2 - pos1), underlinetag);

        // After the URL
        if (pos2 != ustring::npos)
              buffer->insert_with_tag(buffer->end(), line.substr(pos2), colorMap[color]);
    } else {
        // Just insert the line, no URLs were found
        buffer->insert_with_tag(buffer->end(), line, colorMap[color]);
    }
}

void TextWidget::helperInitializer(int i, const Glib::ustring& colorname)
{
    Glib::RefPtr<Gtk::TextTag> texttag = Gtk::TextTag::create();
    Glib::PropertyProxy_WriteOnly<Glib::ustring> fg = texttag->property_foreground();
    fg.set_value(colorname);
    _textview.get_buffer()->get_tag_table()->add(texttag);
    colorMap[i] = texttag;
}

void TextWidget::initializeColorMap()
{
    helperInitializer(0, convert_to_utf8(App->colors.color0));
    helperInitializer(1, convert_to_utf8(App->colors.color1));
    helperInitializer(2, convert_to_utf8(App->colors.color2));
    helperInitializer(3, convert_to_utf8(App->colors.color3));
    helperInitializer(4, convert_to_utf8(App->colors.color4));
    helperInitializer(5, convert_to_utf8(App->colors.color5));
    helperInitializer(6, convert_to_utf8(App->colors.color6));
    helperInitializer(7, convert_to_utf8(App->colors.color7));
    helperInitializer(8, convert_to_utf8(App->colors.color8));
    helperInitializer(9, convert_to_utf8(App->colors.color9));
    helperInitializer(10, convert_to_utf8(App->colors.color10));
    helperInitializer(11, convert_to_utf8(App->colors.color11));
    helperInitializer(12, convert_to_utf8(App->colors.color12));
    helperInitializer(13, convert_to_utf8(App->colors.color13));
    helperInitializer(14, convert_to_utf8(App->colors.color14));
    helperInitializer(15, convert_to_utf8(App->colors.color15));
    helperInitializer(16, convert_to_utf8(App->colors.color16));
    helperInitializer(17, convert_to_utf8(App->colors.color17));
    helperInitializer(18, convert_to_utf8(App->colors.color18));
    helperInitializer(19, convert_to_utf8(App->colors.color19));

    // Create a underlined-tag.
    underlinetag = Gtk::TextTag::create();
    underlinetag->property_underline() = Pango::UNDERLINE_SINGLE;
    underlinetag->property_foreground().set_value(convert_to_utf8(App->colors.color0));
    _textview.get_buffer()->get_tag_table()->add(underlinetag);
}
