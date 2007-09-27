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

#ifndef CONFIGVALUE_H
#define CONFIGVALUE_H

#include <sstream>
#include <glibmm/ustring.h>
#include "Utils.h"

class baseConfigValue
{
public:
    virtual Glib::ustring getString() = 0;
    virtual baseConfigValue& operator=(const Glib::ustring&) = 0;
    virtual ~baseConfigValue() { }
};

template<typename T>
class ConfigValue : public baseConfigValue
{
  T value;

public:
    ConfigValue() { }

    ConfigValue(T defaultvalue)
            : value(defaultvalue)
            { }

    virtual ~ConfigValue() { }

    Glib::ustring getString()
    {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }

    baseConfigValue& operator=(const Glib::ustring& str)
    {
        std::stringstream ss;
        ss << str;
        ss >> value;
        return *this;
    }

    ConfigValue<T>& operator=(const T& val) { value = val; return *this; }
    operator T() { return value; }
    T operator *() { return value; }
    T* get() { return &value; }
};

// Specialization for Glib::ustring
template<>
class ConfigValue<Glib::ustring> : public baseConfigValue
{
    Glib::ustring value;

public:
    ConfigValue() { }

    ConfigValue(Glib::ustring defaultvalue)
            : value(defaultvalue)
            { }

    virtual ~ConfigValue() { }

    Glib::ustring getString()
    {
        return value;
    }

    ConfigValue<Glib::ustring>& operator=(const Glib::ustring& val) { value = val; return *this; }
    operator Glib::ustring() { return value; }
    Glib::ustring operator *() { return value; }
    Glib::ustring* get() { return &value; }
};

class baseConfig
{
public:
    baseConfig(const char *filename);

    void set(const Glib::ustring& key, const Glib::ustring& value);
    Glib::ustring get(const Glib::ustring& key);


protected:
    bool readConfigFile();
    bool writeConfigFile();

    void add(const char *name, baseConfigValue* t) { configvalues[name] = t; }

    std::map<Glib::ustring, baseConfigValue*> configvalues;
    Glib::ustring filename;

    template<typename T> friend class Value;
};


/* Value is a wrapper class for ConfigValue, it hides the pointer type so
 * access is more easily accessible. It's also a smartpointer, which means
 * the the valueptr is deleted when the instance of the Value class goes
 * out of scope */
template<typename T>
class Value
{
    ConfigValue<T> *valueptr;
    baseConfig *holder;

public:
    Value(baseConfig *holder, const char *name)
        : valueptr(new ConfigValue<T>), holder(holder)
        { holder->add(name, valueptr); }

    Value(baseConfig *holder, const char *name, T defaultvalue)
        : valueptr(new ConfigValue<T>(defaultvalue)), holder(holder)
        { holder->add(name, valueptr); }

    ~Value() { delete valueptr; }

    Value<T>& operator=(const T& val) { *valueptr = val; holder->writeConfigFile(); return *this; }
    Value<T>& operator=(const Glib::ustring& val) { *valueptr = val; holder->writeConfigFile(); return *this; }
    operator T() { return *(*valueptr); }
    T* operator->() { return valueptr->get(); }
    ConfigValue<T>& operator() () { return *valueptr; }
    ConfigValue<T>* operator*() { return valueptr; }
};

// Specialization for Glib::ustring
template<>
class Value<Glib::ustring>
{
    ConfigValue<Glib::ustring> *valueptr;
    baseConfig *holder;

public:
    Value(baseConfig *holder, const char *name)
        : valueptr(new ConfigValue<Glib::ustring>), holder(holder)
        { holder->add(name, valueptr); }

    Value(baseConfig *holder, const char *name, Glib::ustring defaultvalue)
        : valueptr(new ConfigValue<Glib::ustring>(defaultvalue)), holder(holder)
        { holder->add(name, valueptr); }

    ~Value() { delete valueptr; }

    Value<Glib::ustring>& operator=(const Glib::ustring& val) { *valueptr = val; holder->writeConfigFile(); return *this; }
    operator Glib::ustring() { return *(*valueptr); }
    Glib::ustring* operator->() { return valueptr->get(); }
    ConfigValue<Glib::ustring>& operator() () { return *valueptr; }
    ConfigValue<Glib::ustring>* operator*() { return valueptr; }
};


#endif
