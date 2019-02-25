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
#ifndef CONFIG_HH_AFD2D0F474253871EF07D5B1B8BBC6E8//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CONFIG_HH_AFD2D0F474253871EF07D5B1B8BBC6E8

#include <string>

namespace Epp {
namespace Contact {
namespace Impl {
namespace SetUnused {

struct Config
{
    static const std::string& get_contact_data_filter_name();
    static bool is_name_of_this_contact_data_filter(const std::string& filter_name);
    struct CreateContact
    {
        struct Disclose
        {
            struct Name;
            struct Organization;
            struct Address;
            struct Telephone;
            struct Fax;
            struct Email;
            struct Vat;
            struct Ident;
            struct NotifyEmail;
        };
    };
    struct UpdateContact
    {
        struct Disclose
        {
            struct Name;
            struct Organization;
            struct Address;
            struct Telephone;
            struct Fax;
            struct Email;
            struct Vat;
            struct Ident;
            struct NotifyEmail;
        };
    };
};

}//namespace Epp::Contact::Impl::SetUnused
}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//CONFIG_HH_AFD2D0F474253871EF07D5B1B8BBC6E8
