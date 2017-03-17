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

#ifndef CREATE_DOMAIN_INPUT_DATA_UNWRAPPED_H_5A60652656BE4A72A527C0479B96DA58
#define CREATE_DOMAIN_INPUT_DATA_UNWRAPPED_H_5A60652656BE4A72A527C0479B96DA58

#include "src/corba/EPP.hh"

#include "src/corba/epp/corba_conversions.h"
#include "src/corba/epp/domain/domain_corba_conversions.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/domain/domain_enum_validation.h"
#include "src/epp/domain/domain_registration_time.h"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Fred {
namespace Corba {
namespace Epp {
namespace Domain {

struct CreateDomainInputDataUnwrapped
{
    std::string fqdn;
    std::string registrant;
    std::string nsset;
    std::string keyset;
    boost::optional<std::string> authinfopw;
    ::Epp::Domain::DomainRegistrationTime period;
    std::vector<std::string> admin_contacts;
    std::vector< ::Epp::Domain::EnumValidationExtension> enum_validation_extension_list;


    CreateDomainInputDataUnwrapped(
            const char* _fqdn,
            const char* _registrant,
            const char* _nsset,
            const char* _keyset,
            const char* _authinfopw,
            const ccReg::Period_str& _period,
            const ccReg::AdminContact& _admin_contact,
            const ccReg::ExtensionList& _enum_validation_extension_list)
        : fqdn(Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn)),
          registrant(Fred::Corba::unwrap_string_from_const_char_ptr(_registrant)),
          nsset(Fred::Corba::unwrap_string_from_const_char_ptr(_nsset)),
          keyset(Fred::Corba::unwrap_string_from_const_char_ptr(_keyset)),
          period(unwrap_domain_registration_period(_period)),
          admin_contacts(unwrap_ccreg_admincontacts_to_vector_string(_admin_contact)),
          enum_validation_extension_list(unwrap_enum_validation_extension_list(_enum_validation_extension_list))

    {
        const std::string authinfopw_value = Fred::Corba::unwrap_string_from_const_char_ptr(_authinfopw);
        authinfopw = authinfopw_value.empty()
                             ? boost::optional<std::string>()
                             : boost::optional<std::string>(authinfopw_value);
    }


};


} // namespace Fred::Corba::Epp::Domain
} // namespace Fred::Corba::Epp
} // namespace Fred::Corba
} // namespace Fred

#endif
