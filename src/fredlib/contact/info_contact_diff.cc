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

#include <algorithm>
#include <string>

#include <boost/algorithm/string.hpp>

#include "util/util.h"
#include "util/is_equal_optional_nullable.h"
#include "info_contact_diff.h"

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
        (std::make_pair("place", place.print_quoted()))
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
        (std::make_pair("id", id.print_quoted()))
        );//format_data_structure InfoContactDiff
    }

    bool InfoContactDiff::is_empty() const
    {
        return
            !( crhistoryid.isset()
            || historyid.isset()
            || delete_time.isset()
            || handle.isset()
            || roid.isset()
            || sponsoring_registrar_handle.isset()
            || create_registrar_handle.isset()
            || update_registrar_handle.isset()
            || creation_time.isset()
            || update_time.isset()
            || transfer_time.isset()
            || authinfopw.isset()
            || name.isset()
            || organization.isset()
            || place.isset()
            || telephone.isset()
            || fax.isset()
            || email.isset()
            || notifyemail.isset()
            || vat.isset()
            || ssntype.isset()
            || ssn.isset()
            || disclosename.isset()
            || discloseorganization.isset()
            || discloseaddress.isset()
            || disclosetelephone.isset()
            || disclosefax.isset()
            || discloseemail.isset()
            || disclosevat.isset()
            || discloseident.isset()
            || disclosenotifyemail.isset()
            || id.isset()
            );
    }

    InfoContactDiff diff_contact_data(const InfoContactData& first, const InfoContactData& second)
    {
        Fred::InfoContactDiff diff;

        //differing data
        if(first.crhistoryid != second.crhistoryid)
        {
            diff.crhistoryid = std::make_pair(first.crhistoryid,second.crhistoryid);
        }

        if(first.historyid != second.historyid)
        {
            diff.historyid = std::make_pair(first.historyid,second.historyid);
        }

        if(!Util::is_equal(first.delete_time, second.delete_time))
        {
            diff.delete_time = std::make_pair(first.delete_time,second.delete_time);
        }

        if(boost::algorithm::to_upper_copy(first.handle)
            .compare(boost::algorithm::to_upper_copy(second.handle)) != 0)
        {
            diff.handle = std::make_pair(first.handle, second.handle);
        }

        if(first.roid.compare(second.roid) != 0)
        {
            diff.roid = std::make_pair(first.roid,second.roid);
        }

        if(boost::algorithm::to_upper_copy(first.sponsoring_registrar_handle)
            .compare(boost::algorithm::to_upper_copy(second.sponsoring_registrar_handle)) != 0)
        {
            diff.sponsoring_registrar_handle = std::make_pair(first.sponsoring_registrar_handle
                    ,second.sponsoring_registrar_handle);
        }

        if(boost::algorithm::to_upper_copy(first.create_registrar_handle)
        .compare(boost::algorithm::to_upper_copy(second.create_registrar_handle)) != 0)
        {
            diff.create_registrar_handle = std::make_pair(first.create_registrar_handle
                    ,second.create_registrar_handle);
        }

        if(!Util::is_equal_upper(first.update_registrar_handle, second.update_registrar_handle))
        {
            diff.update_registrar_handle = std::make_pair(first.update_registrar_handle
                    ,second.update_registrar_handle);
        }

        if(first.creation_time != second.creation_time)
        {
            diff.creation_time = std::make_pair(first.creation_time,second.creation_time);
        }

        if(!Util::is_equal(first.update_time, second.update_time))
        {
            diff.update_time = std::make_pair(first.update_time
                    ,second.update_time);
        }

        if(!Util::is_equal(first.transfer_time, second.transfer_time))
        {
            diff.transfer_time = std::make_pair(first.transfer_time
                    ,second.transfer_time);
        }

        if(first.authinfopw.compare(second.authinfopw) != 0)
        {
            diff.authinfopw = std::make_pair(first.authinfopw,second.authinfopw);
        }

        if(!Util::is_equal(first.name, second.name))
        {
            diff.name = std::make_pair(first.name,second.name);
        }

        if(!Util::is_equal(first.organization, second.organization))
        {
            diff.organization = std::make_pair(first.organization,second.organization);
        }

        if(!Util::is_equal(first.place, second.place))
        {
            diff.place = std::make_pair(first.place, second.place);
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

        if(!Util::is_equal(first.notifyemail, second.notifyemail))
        {
            diff.notifyemail = std::make_pair(first.notifyemail,second.notifyemail);
        }

        if(!Util::is_equal(first.vat, second.vat))
        {
            diff.vat = std::make_pair(first.vat,second.vat);
        }

        if(!Util::is_equal(first.ssntype, second.ssntype))
        {
            diff.ssntype = std::make_pair(first.ssntype,second.ssntype);
        }

        if(!Util::is_equal(first.ssn, second.ssn))
        {
            diff.ssn = std::make_pair(first.ssn,second.ssn);
        }

        if(first.disclosename != second.disclosename)
        {
            diff.disclosename = std::make_pair(first.disclosename,second.disclosename);
        }

        if(first.discloseorganization != second.discloseorganization)
        {
            diff.discloseorganization = std::make_pair(first.discloseorganization,second.discloseorganization);
        }

        if(first.discloseaddress != second.discloseaddress)
        {
            diff.discloseaddress = std::make_pair(first.discloseaddress,second.discloseaddress);
        }

        if(first.disclosetelephone != second.disclosetelephone)
        {
            diff.disclosetelephone = std::make_pair(first.disclosetelephone,second.disclosetelephone);
        }

        if(first.disclosefax != second.disclosefax)
        {
            diff.disclosefax = std::make_pair(first.disclosefax,second.disclosefax);
        }

        if(first.discloseemail != second.discloseemail)
        {
            diff.discloseemail = std::make_pair(first.discloseemail,second.discloseemail);
        }

        if(first.disclosevat != second.disclosevat)
        {
            diff.disclosevat = std::make_pair(first.disclosevat,second.disclosevat);
        }

        if(first.discloseident != second.discloseident)
        {
            diff.discloseident = std::make_pair(first.discloseident,second.discloseident);
        }

        if(first.disclosenotifyemail != second.disclosenotifyemail)
        {
            diff.disclosenotifyemail = std::make_pair(first.disclosenotifyemail,second.disclosenotifyemail);
        }

        if(first.id != second.id)
        {
            diff.id = std::make_pair(first.id,second.id);
        }

        return diff;
    }


}//namespace Fred

