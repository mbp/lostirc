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

#ifndef CONFIGVALUE_H
#define CONFIGVALUE_H

#include <string>
#include <sstream>
#include "Utils.h"

class baseConfigValue
{
public:
    virtual std::string getString() = 0;
    virtual baseConfigValue& operator=(const std::string&) = 0;
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

    std::string getString()
    {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }

    baseConfigValue& operator=(const std::string& str)
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

// Specialization for std::string
template<>
class ConfigValue<std::string> : public baseConfigValue
{
    std::string value;

public:
    ConfigValue() { }

    ConfigValue(std::string defaultvalue)
            : value(defaultvalue)
            { }

    virtual ~ConfigValue() { }

    std::string getString()
    {
        return value;
    }

    ConfigValue<std::string>& operator=(const std::string& val) { value = val; return *this; }
    operator std::string() { return value; }
    std::string operator *() { return value; }
    std::string* get() { return &value; }
};

class baseConfig
{
public:
    baseConfig(const char *filename);

    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);


protected:
    bool readConfigFile();
    bool writeConfigFile();

    void add(const char *name, baseConfigValue* t) { configvalues[name] = t; }

    std::map<std::string, baseConfigValue*> configvalues;
    std::string filename;

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
    Value<T>& operator=(const std::string& val) { *valueptr = val; holder->writeConfigFile(); return *this; }
    operator T() { return *(*valueptr); }
    T* operator->() { return valueptr->get(); }
    ConfigValue<T>& operator() () { return *valueptr; }
    ConfigValue<T>* operator*() { return valueptr; }
};

// Specialization for std::string
template<>
class Value<std::string>
{
    ConfigValue<std::string> *valueptr;
    baseConfig *holder;

public:
    Value(baseConfig *holder, const char *name)
        : valueptr(new ConfigValue<std::string>), holder(holder)
        { holder->add(name, valueptr); }

    Value(baseConfig *holder, const char *name, std::string defaultvalue)
        : valueptr(new ConfigValue<std::string>(defaultvalue)), holder(holder)
        { holder->add(name, valueptr); }

    ~Value() { delete valueptr; }

    Value<std::string>& operator=(const std::string& val) { *valueptr = val; holder->writeConfigFile(); return *this; }
    operator std::string() { return *(*valueptr); }
    std::string* operator->() { return valueptr->get(); }
    ConfigValue<std::string>& operator() () { return *valueptr; }
    ConfigValue<std::string>* operator*() { return valueptr; }
};


#endif
