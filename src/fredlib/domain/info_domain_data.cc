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
 *  @file info_domain_data.cc
 *  common domain info data
 */

#include <string>
#include <vector>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/domain/enum_validation_extension.h"
#include "fredlib/domain/info_domain_data.h"

namespace Fred
{

    InfoDomainData::InfoDomainData()
    : historyid(0)
    , crhistoryid(0)
    {}

    bool InfoDomainData::operator==(const InfoDomainData& rhs) const
    {
        bool result_simple =
        (roid.compare(rhs.roid) == 0)
        && (boost::algorithm::to_lower_copy(fqdn).compare(boost::algorithm::to_lower_copy(rhs.fqdn)) == 0)
        && (boost::algorithm::to_upper_copy(registrant_handle).compare(boost::algorithm::to_upper_copy(rhs.registrant_handle)) == 0)
        && (boost::algorithm::to_upper_copy(sponsoring_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.sponsoring_registrar_handle)) == 0)
        && (boost::algorithm::to_upper_copy(create_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.create_registrar_handle)) == 0)
        && (creation_time == rhs.creation_time)
        && (expiration_date == rhs.expiration_date)
        && (authinfopw.compare(rhs.authinfopw) == 0)
        && (outzone_time == rhs.outzone_time)
        && (cancel_time == rhs.cancel_time)
        && (historyid == rhs.historyid)
        && (crhistoryid == rhs.crhistoryid)
        ;

        bool result_update_registrar_handle = (update_registrar_handle.isnull() == rhs.update_registrar_handle.isnull());
        if(!update_registrar_handle.isnull() && !rhs.update_registrar_handle.isnull())
        {
            result_update_registrar_handle = (boost::algorithm::to_upper_copy(std::string(update_registrar_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.update_registrar_handle))) == 0);
        }

        bool result_nsset_handle = (nsset_handle.isnull() == rhs.nsset_handle.isnull());
        if(!nsset_handle.isnull() && !rhs.nsset_handle.isnull())
        {
            result_nsset_handle = (boost::algorithm::to_upper_copy(std::string(nsset_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.nsset_handle))) == 0);
        }

        bool result_keyset_handle = (keyset_handle.isnull() == rhs.keyset_handle.isnull());
        if(!keyset_handle.isnull() && !rhs.keyset_handle.isnull())
        {
            result_keyset_handle = (boost::algorithm::to_upper_copy(std::string(keyset_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.keyset_handle))) == 0);
        }

        bool result_update_time = (update_time.isnull() == rhs.update_time.isnull());
        if(!update_time.isnull() && !rhs.update_time.isnull())
        {
            result_update_time = (boost::posix_time::ptime(update_time) == boost::posix_time::ptime(rhs.update_time));
        }

        bool result_transfer_time = (transfer_time.isnull() == rhs.transfer_time.isnull());
        if(!delete_time.isnull() && !rhs.delete_time.isnull())
        {
            result_transfer_time = (boost::posix_time::ptime(transfer_time) == boost::posix_time::ptime(rhs.transfer_time));
        }

        bool result_enum_domain_validation = (enum_domain_validation.isnull() == rhs.enum_domain_validation.isnull());
        if(!enum_domain_validation.isnull() && !rhs.enum_domain_validation.isnull())
        {
            result_enum_domain_validation = (ENUMValidationExtension(enum_domain_validation)
                == ENUMValidationExtension(rhs.enum_domain_validation));
        }

        std::set<std::string> lhs_admin_contacts;
        for(std::vector<std::string>::size_type i = 0
            ; i != admin_contacts.size(); ++i)
        {
            lhs_admin_contacts.insert(boost::algorithm::to_upper_copy(admin_contacts[i]));
        }

        std::set<std::string> rhs_admin_contacts;
        for(std::vector<std::string>::size_type i = 0
            ; i != rhs.admin_contacts.size(); ++i)
        {
            rhs_admin_contacts.insert(boost::algorithm::to_upper_copy(rhs.admin_contacts[i]));
        }

        bool result_admin_contacts = (lhs_admin_contacts == rhs_admin_contacts);

        bool result_delete_time = (delete_time.isnull() == rhs.delete_time.isnull());
        if(!delete_time.isnull() && !rhs.delete_time.isnull())
        {
            result_delete_time = (boost::posix_time::ptime(delete_time) == boost::posix_time::ptime(rhs.delete_time));
        }


        return result_simple && result_update_registrar_handle && result_nsset_handle && result_keyset_handle
                && result_update_time && result_transfer_time && result_enum_domain_validation && result_admin_contacts
                && result_delete_time;
    }

    bool InfoDomainData::operator!=(const InfoDomainData& rhs) const
    {
        return !this->operator ==(rhs);
    }



}//namespace Fred

