/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "tools/disclose_flags_updater/disclose_value.hh"

namespace Tools {
namespace DiscloseFlagsUpdater {


std::istream& operator>>(std::istream& _in, DiscloseValue& _value)
{
    std::string token;
    _in >> token;
    if (token == "hide") {
        _value = DiscloseValue::hide;
    }
    else if (token == "show") {
        _value = DiscloseValue::show;
    }
    else {
        _in.setstate(std::ios_base::failbit);
    }
    return _in;
}


std::ostream& operator<<(std::ostream& _os, DiscloseValue _value)
{
    return _os << to_string(_value);
}


std::string to_string(DiscloseValue _value)
{
    switch (_value)
    {
        case DiscloseValue::not_set:
            return "not-set";
        case DiscloseValue::hide:
            return "hide";
        case DiscloseValue::show:
            return "show";
        default:
            throw std::invalid_argument("not handled DiscloseValue");
    }
}


bool to_db_value(DiscloseValue _value)
{
    if (_value == DiscloseValue::show)
    {
        return true;
    }
    else if (_value == DiscloseValue::hide)
    {
        return false;
    }
    throw std::invalid_argument("cannot convert DiscloseValue::" + to_string(_value) + "to database representation");
}


std::istream& operator>>(std::istream& _in, DiscloseAddressValue& _value)
{
    std::string token;
    _in >> token;
    if (token == "hide") {
        _value = DiscloseAddressValue::hide;
    }
    else if (token == "show") {
        _value = DiscloseAddressValue::show;
    }
    else if (token == "hide-verified") {
        _value = DiscloseAddressValue::hide_verified;
    }
    else {
        _in.setstate(std::ios_base::failbit);
    }
    return _in;
}


std::ostream& operator<<(std::ostream& _os, DiscloseAddressValue _value)
{
    return _os << to_string(_value);
}


std::string to_string(DiscloseAddressValue _value)
{
    switch (_value)
    {
        case DiscloseAddressValue::not_set:
            return "not-set";
        case DiscloseAddressValue::hide:
            return "hide";
        case DiscloseAddressValue::show:
            return "show";
        case DiscloseAddressValue::hide_verified:
            return "hide-verified";
        default:
            throw std::invalid_argument("not handled DiscloseAddressValue");
    }
}


bool to_db_value(DiscloseAddressValue _value)
{
    if (_value == DiscloseAddressValue::show)
    {
        return true;
    }
    else if (_value == DiscloseAddressValue::hide)
    {
        return false;
    }
    else if (_value == DiscloseAddressValue::hide_verified)
    {
        return false;
    }
    throw std::invalid_argument("cannot convert DiscloseAddressValue::" + to_string(_value) + "to database representation");
}


}
}
