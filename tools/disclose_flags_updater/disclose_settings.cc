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
#include "tools/disclose_flags_updater/disclose_settings.hh"

namespace Tools {
namespace DiscloseFlagsUpdater {


std::ostream& operator<<(std::ostream& _os, DiscloseSettings _value)
{
    return _os << "{"
        << " name='" << _value.name << "'"
        << " org='" << _value.org << "'"
        << " addr='" << _value.addr << "'"
        << " voice='" << _value.voice << "'"
        << " fax='" << _value.fax << "'"
        << " email='" << _value.email << "'"
        << " vat='" << _value.vat << "'"
        << " ident='" << _value.ident << "'"
        << " notify_email='" << _value.notify_email << "'"
        << " }";
}


}
}
