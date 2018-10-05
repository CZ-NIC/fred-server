/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_HH_AFD2D0F474253871EF07D5B1B8BBC6E8//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CONFIG_HH_AFD2D0F474253871EF07D5B1B8BBC6E8

#include <string>

namespace Epp {
namespace Contact {
namespace Impl {
namespace Dummy {

struct Config
{
    static const std::string& get_check_name();
    static bool is_name_of_this_check(const std::string& check_name);
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

}//namespace Epp::Contact::Impl::Dummy
}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//CONFIG_HH_AFD2D0F474253871EF07D5B1B8BBC6E8
