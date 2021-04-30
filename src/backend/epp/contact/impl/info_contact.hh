/*
 * Copyright (C) 2021  CZ.NIC, z. s. p. o.
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

#ifndef INFO_CONTACT_HH_AA52097DC4C91BF7ED98380AEC346CDD//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define INFO_CONTACT_HH_AA52097DC4C91BF7ED98380AEC346CDD

#include <boost/program_options.hpp>

namespace Epp {
namespace Contact {
namespace Impl {

struct InfoContact
{
    enum class DataSharePolicy
    {
        cznic_specific,
        show_all
    };
    struct ShowPrivateDataTo;
};

void add_info_contact_options_description(boost::program_options::options_description&);

template <typename>
bool has_value(const std::string&);

template <InfoContact::DataSharePolicy>
bool has_value(const std::string&);

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//INFO_CONTACT_HH_AA52097DC4C91BF7ED98380AEC346CDD
