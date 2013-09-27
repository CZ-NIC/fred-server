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

#include <iterator>
#include <algorithm>
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
    , print_diff_(false)
    {}

    bool InfoDomainData::operator==(const InfoDomainData& rhs) const
    {
        if(print_diff_) std::cout << "\nInfoDomainData::operator== print_diff\n" << std::endl;

        bool result_roid = (roid.compare(rhs.roid) == 0);
        if(print_diff_ && !result_roid) std::cout << "roid: " << roid << " != "<< rhs.roid << std::endl;

        bool result_fqdn = (boost::algorithm::to_lower_copy(fqdn).compare(boost::algorithm::to_lower_copy(rhs.fqdn)) == 0);
        if(print_diff_ && !result_fqdn) std::cout << "fqdn: " << fqdn << " != "<< rhs.fqdn << std::endl;

        bool result_registrant_handle = (boost::algorithm::to_upper_copy(registrant_handle).compare(boost::algorithm::to_upper_copy(rhs.registrant_handle)) == 0);
        if(print_diff_ && !result_registrant_handle) std::cout << "registrant_handle: " << registrant_handle << " != "<< rhs.registrant_handle << std::endl;

        bool result_sponsoring_registrar_handle = (boost::algorithm::to_upper_copy(sponsoring_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.sponsoring_registrar_handle)) == 0);
        if(print_diff_ && !result_sponsoring_registrar_handle) std::cout << "sponsoring_registrar_handle: " << sponsoring_registrar_handle << " != "<< rhs.sponsoring_registrar_handle << std::endl;

        bool result_create_registrar_handle = (boost::algorithm::to_upper_copy(create_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.create_registrar_handle)) == 0);
        if(print_diff_ && !result_create_registrar_handle) std::cout << "create_registrar_handle: " << create_registrar_handle << " != "<< rhs.create_registrar_handle << std::endl;

        bool result_creation_time = (creation_time == rhs.creation_time);
        if(print_diff_ && !result_creation_time) std::cout << "creation_time: " << creation_time << " != "<< rhs.creation_time << std::endl;

        bool result_expiration_date = (expiration_date == rhs.expiration_date);
        if(print_diff_ && !result_expiration_date) std::cout << "expiration_date: " << expiration_date << " != "<< rhs.expiration_date << std::endl;

        bool result_authinfopw = (authinfopw.compare(rhs.authinfopw) == 0);
        if(print_diff_ && !result_authinfopw) std::cout << "authinfopw: " << authinfopw << " != "<< rhs.authinfopw << std::endl;

        bool result_outzone_time = (outzone_time == rhs.outzone_time);
        if(print_diff_ && !result_outzone_time) std::cout << "outzone_time: " << outzone_time << " != "<< rhs.outzone_time << std::endl;

        bool result_cancel_time = (cancel_time == rhs.cancel_time);
        if(print_diff_ && !result_cancel_time) std::cout << "cancel_time: " << cancel_time << " != "<< rhs.cancel_time << std::endl;

        bool result_historyid = (historyid == rhs.historyid);
        if(print_diff_ && !result_historyid) std::cout << "historyid: " << historyid << " != "<< rhs.historyid << std::endl;

        bool result_crhistoryid = (crhistoryid == rhs.crhistoryid);
        if(print_diff_ && !result_crhistoryid) std::cout << "crhistoryid: " << crhistoryid << " != "<< rhs.crhistoryid << std::endl;

        bool result_update_registrar_handle = (update_registrar_handle.isnull() == rhs.update_registrar_handle.isnull());
        if(!update_registrar_handle.isnull() && !rhs.update_registrar_handle.isnull())
        {
            result_update_registrar_handle = (boost::algorithm::to_upper_copy(std::string(update_registrar_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.update_registrar_handle))) == 0);
        }
        if(print_diff_ && !result_update_registrar_handle) std::cout << "update_registrar_handle: " << update_registrar_handle.print_quoted() << " != "<< rhs.update_registrar_handle.print_quoted() << std::endl;

        bool result_nsset_handle = (nsset_handle.isnull() == rhs.nsset_handle.isnull());
        if(!nsset_handle.isnull() && !rhs.nsset_handle.isnull())
        {
            result_nsset_handle = (boost::algorithm::to_upper_copy(std::string(nsset_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.nsset_handle))) == 0);
        }
        if(print_diff_ && !result_nsset_handle) std::cout << "nsset_handle: " << nsset_handle.print_quoted() << " != "<< rhs.nsset_handle.print_quoted() << std::endl;

        bool result_keyset_handle = (keyset_handle.isnull() == rhs.keyset_handle.isnull());
        if(!keyset_handle.isnull() && !rhs.keyset_handle.isnull())
        {
            result_keyset_handle = (boost::algorithm::to_upper_copy(std::string(keyset_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.keyset_handle))) == 0);
        }
        if(print_diff_ && !result_keyset_handle) std::cout << "keyset_handle: " << keyset_handle.print_quoted() << " != "<< rhs.keyset_handle.print_quoted() << std::endl;

        bool result_update_time = (update_time.isnull() == rhs.update_time.isnull());
        if(!update_time.isnull() && !rhs.update_time.isnull())
        {
            result_update_time = (boost::posix_time::ptime(update_time) == boost::posix_time::ptime(rhs.update_time));
        }
        if(print_diff_ && !result_update_time) std::cout << "update_time: " << update_time.print_quoted() << " != "<< rhs.update_time.print_quoted() << std::endl;

        bool result_transfer_time = (transfer_time.isnull() == rhs.transfer_time.isnull());
        if(!transfer_time.isnull() && !rhs.transfer_time.isnull())
        {
            result_transfer_time = (boost::posix_time::ptime(transfer_time) == boost::posix_time::ptime(rhs.transfer_time));
        }
        if(print_diff_ && !result_transfer_time) std::cout << "transfer_time: " << transfer_time.print_quoted() << " != "<< rhs.transfer_time.print_quoted() << std::endl;

        bool result_enum_domain_validation = (enum_domain_validation.isnull() == rhs.enum_domain_validation.isnull());
        if(!enum_domain_validation.isnull() && !rhs.enum_domain_validation.isnull())
        {
            result_enum_domain_validation = (ENUMValidationExtension(enum_domain_validation)
                == ENUMValidationExtension(rhs.enum_domain_validation));
        }
        if(print_diff_ && !result_enum_domain_validation)
        {
            std::cout << "enum_domain_validation: " << enum_domain_validation.print_quoted()
                << " != " << rhs.enum_domain_validation.print_quoted() << std::endl;
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
        if(print_diff_ && !result_admin_contacts)
        {
            std::set<std::string> lhs_rhs;
            std::set<std::string> rhs_lhs;

            std::set_difference(lhs_admin_contacts.begin(), lhs_admin_contacts.end()
                , rhs_admin_contacts.begin(), rhs_admin_contacts.end()
                , std::inserter(lhs_rhs, lhs_rhs.end()));

            std::set_difference(rhs_admin_contacts.begin(), rhs_admin_contacts.end()
                , lhs_admin_contacts.begin(), lhs_admin_contacts.end()
                , std::inserter(rhs_lhs, rhs_lhs.end()));

            std::cout << "admin_contacts differ\nlhs - rhs: ";
            std::copy(lhs_rhs.begin(),lhs_rhs.end(),std::ostream_iterator<std::string>(std::cout, " "));
            std::cout << "\nrhs - lhs: ";
            std::copy(rhs_lhs.begin(),rhs_lhs.end(),std::ostream_iterator<std::string>(std::cout, " "));
            std::cout << std::endl;
        }

        bool result_delete_time = (delete_time.isnull() == rhs.delete_time.isnull());
        if(!delete_time.isnull() && !rhs.delete_time.isnull())
        {
            result_delete_time = (boost::posix_time::ptime(delete_time) == boost::posix_time::ptime(rhs.delete_time));
        }
        if(print_diff_ && !result_delete_time) std::cout << "delete_time: " << delete_time.print_quoted() << " != "<< rhs.delete_time.print_quoted() << std::endl;

        return  result_roid
                && result_fqdn
                && result_registrant_handle
                && result_sponsoring_registrar_handle
                && result_create_registrar_handle
                && result_creation_time
                && result_expiration_date
                && result_authinfopw
                && result_outzone_time
                && result_cancel_time
                && result_historyid
                && result_crhistoryid
                && result_update_registrar_handle
                && result_nsset_handle
                && result_keyset_handle
                && result_update_time
                && result_transfer_time
                && result_enum_domain_validation
                && result_admin_contacts
                && result_delete_time
                ;
    }

    bool InfoDomainData::operator!=(const InfoDomainData& rhs) const
    {
        return !this->operator ==(rhs);
    }

    void InfoDomainData::set_diff_print(bool print_diff)
    {
        print_diff_ = print_diff;
    }

    std::string InfoDomainData::to_string() const
    {
        return Util::format_data_structure("InfoDomainData",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid))
        (std::make_pair("fqdn",fqdn))
        (std::make_pair("registrant_handle",registrant_handle))
        (std::make_pair("nsset_handle",nsset_handle.print_quoted()))
        (std::make_pair("keyset_handle",keyset_handle.print_quoted()))
        (std::make_pair("sponsoring_registrar_handle",sponsoring_registrar_handle))
        (std::make_pair("create_registrar_handle",create_registrar_handle))
        (std::make_pair("update_registrar_handle",update_registrar_handle.print_quoted()))
        (std::make_pair("creation_time",boost::lexical_cast<std::string>(creation_time)))
        (std::make_pair("update_time",update_time.print_quoted()))
        (std::make_pair("transfer_time",transfer_time.print_quoted()))
        (std::make_pair("expiration_date",boost::lexical_cast<std::string>(expiration_date)))
        (std::make_pair("authinfopw",authinfopw))
        (std::make_pair("admin_contacts",Util::format_vector(admin_contacts)))
        (std::make_pair("enum_domain_validation",enum_domain_validation.print_quoted()))
        (std::make_pair("outzone_time",boost::lexical_cast<std::string>(outzone_time)))
        (std::make_pair("cancel_time",boost::lexical_cast<std::string>(cancel_time)))
        (std::make_pair("delete_time",delete_time.print_quoted()))
        (std::make_pair("crhistoryid",boost::lexical_cast<std::string>(crhistoryid)))
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid)))
        );


    }


}//namespace Fred

