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

    insertText(0, 1, ustring(tim));

    bool fgcolor = false;
    bool bgcolor = false;
    int numbercount = 0;
    Glib::ustring fgnumber;
    Glib::ustring bgnumber;
    for (int i = 0; i < line.length(); ++i)
    {
        if (line[i] == '\003') {
            fgcolor = true;
            numbercount = 0;
            fgnumber.clear();
            bgnumber.clear();
        } else if (fgcolor && isdigit(line[i]) && numbercount < 2) {
            numbercount++;
            fgnumber += line[i];
        } else if (fgcolor && line[i] == ',' && numbercount < 3) {
            numbercount = 0;
            bgcolor = true;
            fgcolor = false;
        } else if (bgcolor && isdigit(line[i]) && numbercount < 2) {
            numbercount++; 
            bgnumber += line[i];
        } else {
            numbercount = 0;
            fgcolor = false;
            bgcolor = false;

            if (bgnumber.empty())
                  bgnumber = "1";
            if (fgnumber.empty())
                  fgnumber = "0";

            Glib::ustring text;
            text = line[i];
            insertText(Util::stoi(fgnumber), Util::stoi(bgnumber), text);
        }   
    } 
    return *this;
}

void TextWidget::insertText(int fgcolor, int bgcolor, const ustring& str)
{
    // see if the scrollbar is located in the bottom, then we need to scroll
    // after insert
    bool scroll = false;
    if (get_vadjustment()->get_value() >= (get_vadjustment()->get_upper() - get_vadjustment()->get_page_size() - 1e-12))
          scroll = true;

    // FIXME: temp hack.
    if (fgcolor > fgColorMap.size())
        fgcolor = fgColorMap.size() - 1;

    // Insert the text
    realInsert(fgcolor, bgcolor, str);
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();

    if (scroll)
          _textview.scroll_to_mark(buffer->create_mark(buffer->end()), 0.0);

    // FIXME: possible performance critical
    int buffer_size = App->options.buffer_size;
    if (buffer_size && buffer->get_line_count() > buffer_size)
          buffer->erase(buffer->begin(), buffer->get_iter_at_line(buffer->get_line_count() - buffer_size));
}

void TextWidget::realInsert(int fgcolor, int bgcolor, const ustring& line)
{
    // This function has the purpose to insert the line - but first check to
    // see whether we have an URL.
    Glib::RefPtr<Gtk::TextBuffer> buffer = _textview.get_buffer();

    std::vector< Glib::RefPtr<Gtk::TextTag> > tags;
    tags.push_back(fgColorMap[fgcolor]);
    tags.push_back(bgColorMap[bgcolor]);

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
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color0));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color1));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color2));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color3));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color4));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color5));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color6));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color7));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color8));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color9));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color10));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color11));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color12));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color13));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color14));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color15));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color16));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color17));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color18));
    fgColorMap[i++] = initializeFG(convert_to_utf8(App->colors.color19));

    i = 0;
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color0));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color1));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color2));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color3));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color4));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color5));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color6));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color7));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color8));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color9));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color10));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color11));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color12));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color13));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color14));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color15));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color16));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color17));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color18));
    bgColorMap[i++] = initializeBG(convert_to_utf8(App->colors.color19));

    // Create a underlined-tag.
    underlinetag = Gtk::TextTag::create();
    underlinetag->property_underline() = Pango::UNDERLINE_SINGLE;
    underlinetag->property_foreground() = convert_to_utf8(App->colors.color0);
    _textview.get_buffer()->get_tag_table()->add(underlinetag);
}
