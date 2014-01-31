/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 * registrar info data diff
 */

#include <algorithm>
#include <string>

#include <boost/algorithm/string.hpp>

#include "util/util.h"
#include "util/is_equal_optional_nullable.h"

#include "info_registrar_diff.h"

namespace Fred
{
    InfoRegistrarDiff::InfoRegistrarDiff()
    {}

    std::string InfoRegistrarDiff::to_string() const
    {
        return Util::format_data_structure("InfoRegistrarDiff",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle.print_quoted()))
        (std::make_pair("name",name.print_quoted()))
        (std::make_pair("organization",organization.print_quoted()))
        (std::make_pair("street1",street1.print_quoted()))
        (std::make_pair("street2",street2.print_quoted()))
        (std::make_pair("street3",street3.print_quoted()))
        (std::make_pair("city",city.print_quoted()))
        (std::make_pair("stateorprovince",stateorprovince.print_quoted()))
        (std::make_pair("postalcode",postalcode.print_quoted()))
        (std::make_pair("country",country.print_quoted()))
        (std::make_pair("telephone",telephone.print_quoted()))
        (std::make_pair("fax",fax.print_quoted()))
        (std::make_pair("email",email.print_quoted()))
        (std::make_pair("url",url.print_quoted()))
        (std::make_pair("system",system.print_quoted()))
        (std::make_pair("ico",ico.print_quoted()))
        (std::make_pair("dic",dic.print_quoted()))
        (std::make_pair("variable_symbol",variable_symbol.print_quoted()))
        (std::make_pair("payment_memo_regex",payment_memo_regex.print_quoted()))
        (std::make_pair("vat_payer",vat_payer.print_quoted()))
        (std::make_pair("id",id.print_quoted()))
        );//format_data_structure InfoRegistrarDiff
    }

    bool InfoRegistrarDiff::is_empty() const
    {
        return
            !( handle.isset()
            || name.isset()
            || organization.isset()
            || street1.isset()
            || street2.isset()
            || street3.isset()
            || city.isset()
            || stateorprovince.isset()
            || postalcode.isset()
            || country.isset()
            || telephone.isset()
            || fax.isset()
            || email.isset()
            || url.isset()
            || system.isset()
            || ico.isset()
            || dic.isset()
            || variable_symbol.isset()
            || payment_memo_regex.isset()
            || vat_payer.isset()
            || id.isset()
            );
    }

    InfoRegistrarDiff diff_registrar_data(const InfoRegistrarData& first, const InfoRegistrarData& second)
    {
        Fred::InfoRegistrarDiff diff;

        //differing data
        if(boost::algorithm::to_upper_copy(first.handle)
            .compare(boost::algorithm::to_upper_copy(second.handle)) != 0)
        {
            diff.handle = std::make_pair(first.handle, second.handle);
        }

        if(!Util::is_equal(first.name, second.name))
        {
            diff.name = std::make_pair(first.name,second.name);
        }

        if(!Util::is_equal(first.organization, second.organization))
        {
            diff.organization = std::make_pair(first.organization,second.organization);
        }

        if(!Util::is_equal(first.street1, second.street1))
        {
            diff.street1 = std::make_pair(first.street1,second.street1);
        }

        if(!Util::is_equal(first.street2, second.street2))
        {
            diff.street2 = std::make_pair(first.street2,second.street2);
        }

        if(!Util::is_equal(first.street3, second.street3))
        {
            diff.street3 = std::make_pair(first.street3,second.street3);
        }

        if(!Util::is_equal(first.city, second.city))
        {
            diff.city = std::make_pair(first.city,second.city);
        }

        if(!Util::is_equal(first.stateorprovince, second.stateorprovince))
        {
            diff.stateorprovince = std::make_pair(first.stateorprovince,second.stateorprovince);
        }

        if(!Util::is_equal(first.postalcode, second.postalcode))
        {
            diff.postalcode = std::make_pair(first.postalcode,second.postalcode);
        }

        if(!Util::is_equal(first.country, second.country))
        {
            diff.country = std::make_pair(first.country,second.country);
        }

        if(!Util::is_equal(first.telephone, second.telephone))
        {
            diff.telephone = std::make_pair(first.telephone,second.telephone);
        }

        if(!Util::is_equal(first.fax, second.fax))
        {
            diff.fax = std::make_pair(first.fax,second.fax);
        }

        if(!Util::is_equal(first.email, second.email))
        {
            diff.email = std::make_pair(first.email,second.email);
        }

        if(!Util::is_equal(first.url, second.url))
        {
            diff.url = std::make_pair(first.url,second.url);
        }
        if(!Util::is_equal(first.system, second.system))
        {
            diff.system = std::make_pair(first.system,second.system);
        }
        if(!Util::is_equal(first.ico, second.ico))
        {
            diff.ico = std::make_pair(first.ico,second.ico);
        }
        if(!Util::is_equal(first.dic, second.dic))
        {
            diff.dic = std::make_pair(first.dic,second.dic);
        }
        if(!Util::is_equal(first.variable_symbol, second.variable_symbol))
        {
            diff.variable_symbol = std::make_pair(first.variable_symbol,second.variable_symbol);
        }
        if(!Util::is_equal(first.payment_memo_regex, second.payment_memo_regex))
        {
            diff.payment_memo_regex = std::make_pair(first.payment_memo_regex,second.payment_memo_regex);
        }
        if(!Util::is_equal(first.vat_payer, second.vat_payer))
        {
            diff.vat_payer = std::make_pair(first.vat_payer,second.vat_payer);
        }

        if(first.id != second.id)
        {
            diff.id = std::make_pair(first.id,second.id);
        }

        return diff;
    }

}//namespace Fred

