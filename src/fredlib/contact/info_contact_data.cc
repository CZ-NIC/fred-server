/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 *  common contact info data
 */

#include <algorithm>
#include <string>

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "info_contact_data.h"
#include "info_contact_diff.h"

namespace Fred
{

    std::string ContactAddressType::to_string(const struct ContactAddressType &_src)
    {
        switch (_src.value) {
            case MAILING:
                return "MAILING";
            case BILLING:
                return "BILLING";
            case SHIPPING:
                return "SHIPPING";
        }
        std::ostringstream msg;
        msg << "invalid value " << static_cast< int >(_src.value) << " of ContactAddressType";
        throw std::runtime_error(msg.str());
    }

    ContactAddressType::Value ContactAddressType::from_string(const std::string &_src)
    {
        if (_src == "MAILING") {
            return MAILING;
        }
        if (_src == "BILLING") {
            return BILLING;
        }
        if (_src == "SHIPPING") {
            return SHIPPING;
        }
        std::ostringstream msg;
        msg << "\"" << _src << "\" unable convert to ContactAddressType";
        throw std::runtime_error(msg.str());
    }

    InfoContactData::InfoContactData()
    : crhistoryid(0)
    , historyid(0)
    , id(0)
    {}

    bool InfoContactData::operator==(const InfoContactData& rhs) const
    {
        return diff_contact_data(*this, rhs).is_empty();
    }

    bool InfoContactData::operator!=(const InfoContactData& rhs) const
    {
        return !this->operator ==(rhs);
    }

    std::string InfoContactData::to_string() const
    {
        return Util::format_data_structure("InfoContactData",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("crhistoryid",boost::lexical_cast<std::string>(crhistoryid)))
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid)))
        (std::make_pair("id",boost::lexical_cast<std::string>(id)))
        (std::make_pair("delete_time",delete_time.print_quoted()))
        (std::make_pair("handle",handle))
        (std::make_pair("roid",roid))
        (std::make_pair("sponsoring_registrar_handle",sponsoring_registrar_handle))
        (std::make_pair("create_registrar_handle",create_registrar_handle))
        (std::make_pair("update_registrar_handle",update_registrar_handle.print_quoted()))
        (std::make_pair("creation_time",boost::lexical_cast<std::string>(creation_time)))
        (std::make_pair("update_time",update_time.print_quoted()))
        (std::make_pair("transfer_time",transfer_time.print_quoted()))
        (std::make_pair("authinfopw",authinfopw))
        (std::make_pair("name",name.print_quoted()))
        (std::make_pair("organization",organization.print_quoted()))
        (std::make_pair("place",place.print_quoted()))
        (std::make_pair("telephone",telephone.print_quoted()))
        (std::make_pair("fax",fax.print_quoted()))
        (std::make_pair("email",email.print_quoted()))
        (std::make_pair("notifyemail_",notifyemail.print_quoted()))
        (std::make_pair("vat",vat.print_quoted()))
        (std::make_pair("ssntype",ssntype.print_quoted()))
        (std::make_pair("ssn",ssn.print_quoted()))
        (std::make_pair("disclosename",disclosename ? "true" : "false"))
        (std::make_pair("discloseorganization",discloseorganization ? "true" : "false"))
        (std::make_pair("discloseaddress",discloseaddress ? "true" : "false"))
        (std::make_pair("disclosetelephone",disclosetelephone ? "true" : "false"))
        (std::make_pair("disclosefax",disclosefax ? "true" : "false"))
        (std::make_pair("discloseemail",discloseemail ? "true" : "false"))
        (std::make_pair("disclosevat",disclosevat ? "true" : "false"))
        (std::make_pair("discloseident",discloseident ? "true" : "false"))
        (std::make_pair("disclosenotifyemail",disclosenotifyemail ? "true" : "false"))
        );
    }

}//namespace Fred
