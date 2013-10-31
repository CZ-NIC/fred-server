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
 * contact info data diff
 */

#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/contact/info_contact_diff.h"

namespace Fred
{

    InfoContactDiff::InfoContactDiff()
    {}

    std::string InfoContactDiff::to_string() const
    {
        return Util::format_data_structure("InfoContactDiff",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("crhistoryid", crhistoryid.print_quoted()))
        (std::make_pair("historyid", historyid.print_quoted()))
        (std::make_pair("id", id.print_quoted()))
        (std::make_pair("delete_time", delete_time.print_quoted()))
        (std::make_pair("handle", handle.print_quoted()))
        (std::make_pair("roid", roid.print_quoted()))
        (std::make_pair("sponsoring_registrar_handle", sponsoring_registrar_handle.print_quoted()))
        (std::make_pair("create_registrar_handle", create_registrar_handle.print_quoted()))
        (std::make_pair("update_registrar_handle", update_registrar_handle.print_quoted()))
        (std::make_pair("creation_time", creation_time.print_quoted()))
        (std::make_pair("update_time", update_time.print_quoted()))
        (std::make_pair("transfer_time", transfer_time.print_quoted()))
        (std::make_pair("authinfopw", authinfopw.print_quoted()))
        (std::make_pair("name", name.print_quoted()))
        (std::make_pair("organization", organization.print_quoted()))
        (std::make_pair("street1", street1.print_quoted()))
        (std::make_pair("street2", street2.print_quoted()))
        (std::make_pair("street3", street3.print_quoted()))
        (std::make_pair("city", city.print_quoted()))
        (std::make_pair("stateorprovince", stateorprovince.print_quoted()))
        (std::make_pair("postalcode", postalcode.print_quoted()))
        (std::make_pair("country", country.print_quoted()))
        (std::make_pair("telephone", telephone.print_quoted()))
        (std::make_pair("fax", fax.print_quoted()))
        (std::make_pair("email", email.print_quoted()))
        (std::make_pair("notifyemail", notifyemail.print_quoted()))
        (std::make_pair("vat", vat.print_quoted()))
        (std::make_pair("ssntype", ssntype.print_quoted()))
        (std::make_pair("ssn", ssn.print_quoted()))
        (std::make_pair("disclosename", disclosename.print_quoted()))
        (std::make_pair("discloseorganization", discloseorganization.print_quoted()))
        (std::make_pair("discloseaddress", discloseaddress.print_quoted()))
        (std::make_pair("disclosetelephone", disclosetelephone.print_quoted()))
        (std::make_pair("disclosefax", disclosefax.print_quoted()))
        (std::make_pair("discloseemail", discloseemail.print_quoted()))
        (std::make_pair("disclosevat", disclosevat.print_quoted()))
        (std::make_pair("discloseident", discloseident.print_quoted()))
        (std::make_pair("disclosenotifyemail", disclosenotifyemail.print_quoted()))
        );//format_data_structure InfoContactDiff
    }


}//namespace Fred


