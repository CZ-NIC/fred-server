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
 *  @info_domain_compare.h
 *  comparsion of domain info
 */

#ifndef INFO_DOMAIN_COMPARE_H_
#define INFO_DOMAIN_COMPARE_H_

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/info_domain_history.h"

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    bool operator==(const InfoDomainData& lhs, const InfoDomainHistoryData& rhs)
    {
        bool result_simple =
        (lhs.roid.compare(rhs.roid) == 0)
        && (boost::algorithm::to_lower_copy(lhs.fqdn).compare(boost::algorithm::to_lower_copy(rhs.fqdn)) == 0)
        && (boost::algorithm::to_upper_copy(lhs.registrant_handle).compare(boost::algorithm::to_upper_copy(rhs.registrant_handle)) == 0)
        && (boost::algorithm::to_upper_copy(lhs.sponsoring_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.sponsoring_registrar_handle)) == 0)
        && (boost::algorithm::to_upper_copy(lhs.create_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.create_registrar_handle)) == 0)
        && (lhs.creation_time == rhs.creation_time)
        && (lhs.expiration_date == rhs.expiration_date)
        && (lhs.authinfopw.compare(rhs.authinfopw) == 0)
        && (lhs.outzone_time == rhs.outzone_time)
        && (lhs.cancel_time == rhs.cancel_time)
        && (lhs.historyid == rhs.historyid);

        bool result_update_registrar_handle = (lhs.update_registrar_handle.isnull() == rhs.update_registrar_handle.isnull());
        if(!lhs.update_registrar_handle.isnull() && !rhs.update_registrar_handle.isnull())
        {
            result_update_registrar_handle = (boost::algorithm::to_upper_copy(std::string(lhs.update_registrar_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.update_registrar_handle))) == 0);
        }

        bool result_nsset_handle = (lhs.nsset_handle.isnull() == rhs.nsset_handle.isnull());
        if(!lhs.nsset_handle.isnull() && !rhs.nsset_handle.isnull())
        {
            result_nsset_handle = (boost::algorithm::to_upper_copy(std::string(lhs.nsset_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.nsset_handle))) == 0);
        }

        bool result_keyset_handle = (lhs.keyset_handle.isnull() == rhs.keyset_handle.isnull());
        if(!lhs.keyset_handle.isnull() && !rhs.keyset_handle.isnull())
        {
            result_keyset_handle = (boost::algorithm::to_upper_copy(std::string(lhs.keyset_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.keyset_handle))) == 0);
        }

        bool result_update_time = (lhs.update_time.isnull() == rhs.update_time.isnull());
        if(!lhs.update_time.isnull() && !rhs.update_time.isnull())
        {
            result_update_time = (boost::posix_time::ptime(lhs.update_time) == boost::posix_time::ptime(rhs.update_time));
        }

        bool result_transfer_time = (lhs.transfer_time.isnull() == rhs.transfer_time.isnull());
        if(!lhs.delete_time.isnull() && !rhs.delete_time.isnull())
        {
            result_transfer_time = (boost::posix_time::ptime(lhs.transfer_time) == boost::posix_time::ptime(rhs.transfer_time));
        }

        bool result_enum_domain_validation = (lhs.enum_domain_validation.isnull() == rhs.enum_domain_validation.isnull());
        if(!lhs.enum_domain_validation.isnull() && !rhs.enum_domain_validation.isnull())
        {
            result_enum_domain_validation = (ENUMValidationExtension(lhs.enum_domain_validation)
                == ENUMValidationExtension(rhs.enum_domain_validation));
        }

        std::set<std::string> lhs_admin_contacts;
        for(std::vector<std::string>::size_type i = 0
            ; i != lhs.admin_contacts.size(); ++i)
        {
            lhs_admin_contacts.insert(boost::algorithm::to_upper_copy(lhs.admin_contacts[i]));
        }

        std::set<std::string> rhs_admin_contacts;
        for(std::vector<std::string>::size_type i = 0
            ; i != rhs.admin_contacts.size(); ++i)
        {
            rhs_admin_contacts.insert(boost::algorithm::to_upper_copy(rhs.admin_contacts[i]));
        }

        bool result_admin_contacts = (lhs_admin_contacts == rhs_admin_contacts);

        bool result_delete_time = (lhs.delete_time.isnull() == rhs.delete_time.isnull());
        if(!lhs.delete_time.isnull() && !rhs.delete_time.isnull())
        {
            result_delete_time = (boost::posix_time::ptime(lhs.delete_time) == boost::posix_time::ptime(rhs.delete_time));
        }

        return result_simple && result_update_registrar_handle && result_nsset_handle && result_keyset_handle
                && result_update_time && result_transfer_time && result_enum_domain_validation && result_admin_contacts
                && result_delete_time;
    }

    bool operator==(const InfoDomainHistoryData& lhs, const InfoDomainData& rhs)
    {
        return operator==(rhs,lhs);
    }

    bool operator!=(const InfoDomainData& lhs, const InfoDomainHistoryData& rhs)
    {
        return !operator==(lhs,rhs);
    }

    bool operator!=(const InfoDomainHistoryData& lhs, const InfoDomainData& rhs)
    {
        return !operator==(rhs,lhs);
    }

}//namespace Fred

#endif//INFO_DOMAIN_COMPARE_H_
