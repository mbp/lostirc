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

#include <algorithm>
#include <sstream>
#include <glibmm/convert.h>
#include <cctype>
#include <cstdlib>
#include "Utils.h"

using Glib::ustring;
using std::vector;

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

int stoi(const ustring& str)
{
    /* TODO: this function is implemented using atoi, only because
     * stringstreams is broken on gcc 2.96 (redhat, mandrake) */
    return std::atoi(str.c_str());
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

        if (!Glib::get_charset()) {// locale charset is not UTF-8?
            try // ignore errors -- go on with the fallback if the conversion fails
            {
                str_utf8 = Glib::locale_to_utf8(str);
                did_conversion = true;
            }
            catch(const Glib::ConvertError&)
            {}
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

}

