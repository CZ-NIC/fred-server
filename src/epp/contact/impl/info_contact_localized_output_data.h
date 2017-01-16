/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef INFO_CONTACT_LOCALIZED_OUTPUT_DATA_H_671EB821E0A641C5BE786A96B551FE13
#define INFO_CONTACT_LOCALIZED_OUTPUT_DATA_H_671EB821E0A641C5BE786A96B551FE13

#include "src/epp/contact/contact_disclose.h"
#include "util/db/nullable.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>

namespace Epp {
namespace Contact {

struct InfoContactLocalizedOutputData
{
    std::string handle;
    std::string roid;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    Nullable<std::string> last_update_registrar_handle;
    std::map<std::string, std::string> localized_external_states;
    boost::posix_time::ptime crdate;
    Nullable<boost::posix_time::ptime> last_update;
    Nullable<boost::posix_time::ptime> last_transfer;
    Nullable<std::string> name;
    Nullable<std::string> organization;
    Nullable<std::string> street1;
    Nullable<std::string> street2;
    Nullable<std::string> street3;
    Nullable<std::string> city;
    Nullable<std::string> state_or_province;
    Nullable<std::string> postal_code;
    Nullable<std::string> country_code;
    Nullable<std::string> telephone;
    Nullable<std::string> fax;
    Nullable<std::string> email;
    Nullable<std::string> notify_email;
    Nullable<std::string> VAT;
    Nullable<std::string> ident;
    struct IdentType
    {
        enum Enum
        {
            op,
            pass,
            ico,
            mpsv,
            birthday
        };
    };
    Nullable<IdentType::Enum> identtype;
    boost::optional<std::string> authinfopw;
    boost::optional<ContactDisclose> disclose;

    explicit InfoContactLocalizedOutputData(const boost::optional<ContactDisclose>& _disclose)
    :   disclose(_disclose)
    { }
};

} // namespace Epp::Contact
} // namespace Epp

#endif
