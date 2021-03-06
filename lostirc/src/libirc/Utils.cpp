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

#include <algorithm>
#include <glibmm/convert.h>
#include <cctype>
#include "Utils.h"
#include "LostIRCApp.h"

using Glib::ustring;

namespace Util {

std::string upper(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}

std::string lower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

bool isDigit(const ustring& str)
{
    ustring::const_iterator i;
    for (i = str.begin(); i != str.end(); ++i)
          if (isdigit(*i) == 0)
                return false;


    return true;
}

Glib::ustring convert_to_utf8(const std::string& str)
{
    Glib::ustring str_utf8 (str);

    if (!str_utf8.validate()) { // invalid UTF-8?
        bool did_conversion = false;

        if (!Glib::get_charset()) { // locale charset is not UTF-8?
            try { // ignore errors -- go on with the fallback if the conversion fails
                str_utf8 = Glib::locale_to_utf8(str);
                did_conversion = true;
            } catch(const Glib::ConvertError&) {}
        }

        if (!did_conversion) {
            // Fallback conversion -- used either if the conversion from the
            // current locale's encoding failed, or if the user is running a
            // UTF-8 locale.
            str_utf8 = Glib::convert(str, "UTF-8", "ISO-8859-15");

            // ISO-8859-15 is the default fallback encoding. Might want to
            // make it configurable some day.
        }
    }

    return str_utf8;
}

std::string convert_from_utf8(const Glib::ustring& str_utf8)
{
    static bool displayed_message = false;
    bool tried_custom_encoding = false;

    std::string str;

    try {

        if (App->options.encoding == "System") {
            str = Glib::locale_from_utf8(str_utf8);
        } else {
            tried_custom_encoding = true;
            str = Glib::convert(str_utf8, App->options.encoding().getString(), "UTF-8");
        }

    } catch (const Glib::ConvertError&) {

        if (!displayed_message) {
            displayed_message = true;
            App->fe->localeError(tried_custom_encoding);
        }

    }
    return str;
}

void tokenizeWords(const Glib::ustring& str, std::vector<Glib::ustring>& container)
{

    ustring::size_type lastPos = str.find_first_not_of(' ', 0);
    ustring::size_type pos = str.find_first_of(' ', lastPos);

    while (ustring::npos != pos || ustring::npos != lastPos)
    {
        container.push_back(str.substr(lastPos, pos - lastPos));

        lastPos = str.find_first_not_of(' ', pos);
        pos = str.find_first_of(' ', lastPos);
    }
}

}

