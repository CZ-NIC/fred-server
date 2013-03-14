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
 *  @file info_domain.h
 *  domain info
 */

#ifndef INFO_DOMAIN_H_
#define INFO_DOMAIN_H_

#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

namespace Fred
{

    ///enum domain validation extension
    struct ENUMValidationExtension
    {
        boost::gregorian::date validation_expiration;//expiration_time date of validation
        bool publish;//publish in ENUM dictionary
        ENUMValidationExtension()
        : validation_expiration()//not a date time
        , publish(false)
        {}
        ENUMValidationExtension(const boost::gregorian::date& _validation_expiration
                , bool _publish)
        : validation_expiration(_validation_expiration)
        , publish(_publish)
        {}
    };

    struct InfoDomainData
    {
        std::string roid;//domain identifier
        std::string fqdn;//domain name
        std::string registrant_handle;//domain owner
        Nullable<std::string> nsset_handle;//nssset might not be set
        Nullable<std::string> keyset_handle;//keyset might not be set
        std::string sponsoring_registrar_handle;//registrar which have right for change
        std::string create_registrar_handle;//registrar which created domain
        Nullable<std::string> update_registrar_handle;//registrar which last time changed domain
        boost::posix_time::ptime creation_time;//time of domain creation
        boost::posix_time::ptime update_time; //last update time
        boost::posix_time::ptime transfer_time; //last transfer time
        boost::posix_time::ptime expiration_time; //domain expiration time
        std::string authinfopw;//password for domain transfer
        std::vector<std::string> admin_contacts;//list of administrative contacts
        Nullable<ENUMValidationExtension > enum_domain_validation;//enum domain validation info
        boost::posix_time::ptime outzone_time; //domain outzone time
        boost::posix_time::ptime cancel_time; //domain cancel time

        bool operator==(const InfoDomainData& rhs) const
        {
            bool result_simple =
            (roid.compare(rhs.roid) == 0)
            && (boost::algorithm::to_lower_copy(fqdn).compare(boost::algorithm::to_lower_copy(rhs.fqdn)) == 0)
            && (boost::algorithm::to_upper_copy(registrant_handle).compare(boost::algorithm::to_upper_copy(rhs.registrant_handle)) == 0)
            && (boost::algorithm::to_upper_copy(sponsoring_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.sponsoring_registrar_handle)) == 0)
            && (boost::algorithm::to_upper_copy(create_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.create_registrar_handle)) == 0)
            && (creation_time == rhs.creation_time)
            && (update_time == rhs.update_time)
            && (transfer_time == rhs.transfer_time)
            && (expiration_time == rhs.expiration_time)
            && (authinfopw.compare(rhs.authinfopw) == 0)
            && (outzone_time == rhs.outzone_time)
            && (cancel_time == rhs.cancel_time);

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

            bool result_enum_domain_validation = (enum_domain_validation.isnull() == rhs.enum_domain_validation.isnull());
            if(!enum_domain_validation.isnull() && !rhs.enum_domain_validation.isnull())
            {
                result_enum_domain_validation = (ENUMValidationExtension(enum_domain_validation).publish
                        == ENUMValidationExtension(rhs.enum_domain_validation).publish)
                && (ENUMValidationExtension(enum_domain_validation).validation_expiration
                        == ENUMValidationExtension(rhs.enum_domain_validation).validation_expiration);
            }

            bool result_admin_contacts = (admin_contacts.size() == rhs.admin_contacts.size());
            if (result_admin_contacts)
            {
                for(std::vector<std::string>::size_type i = 0
                    ; i != admin_contacts.size(); ++i)
                {
                    result_admin_contacts = (result_admin_contacts && (boost::algorithm::to_upper_copy(admin_contacts[i])
                        .compare(boost::algorithm::to_upper_copy(rhs.admin_contacts[i])) == 0));
                    if (!result_admin_contacts) break;
                }
            }

            return result_simple && result_update_registrar_handle && result_nsset_handle && result_keyset_handle
                    && result_enum_domain_validation && result_admin_contacts;
        }

        bool operator!=(const InfoDomainData& rhs) const
        {
            return !this->operator ==(rhs);
        }

    };

    class InfoDomain
    {
        const std::string fqdn_;//domain identifier
        const std::string registrar_;//registrar identifier
        bool lock_;//lock object_registry row for domain

    public:
        InfoDomain(const std::string& fqdn
                , const std::string& registrar);
        InfoDomain& set_lock(bool lock = true);//set lock object_registry row for domain
        InfoDomainData exec(OperationContext& ctx);//return data
    };//class InfoDomain

//exception impl
    class InfoDomainException
    : public OperationExceptionImpl<InfoDomainException, 8192>
    {
    public:
        InfoDomainException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<InfoDomainException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={
                    "not found:fqdn"
                    , "not found:registrar"
            };
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }
    };//class InfoDomainException

    typedef InfoDomainException::OperationErrorType InfoDomainError;
#define IDEX(DATA) InfoDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define IDERR(DATA) InfoDomainError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}//namespace Fred

#endif//INFO_DOMAIN_H_
