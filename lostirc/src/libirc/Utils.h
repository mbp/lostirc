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

#ifndef UTILS_H
#define UTILS_H

#include <glibmm/ustring.h>
#include <string> 

namespace Util {

    std::string upper(std::string str);
    std::string lower(std::string str);
    int stoi(const Glib::ustring& str);
    bool isDigit(const Glib::ustring& str);

    Glib::ustring convert_to_utf8(const std::string& str);
}
#endif
