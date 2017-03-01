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

#ifndef INFO_DOMAIN_LOCALIZED_OUTPUT_DATA_H_A77108D55D9D4F31AC54E93686386A62
#define INFO_DOMAIN_LOCALIZED_OUTPUT_DATA_H_A77108D55D9D4F31AC54E93686386A62

#include "src/epp/domain/impl/domain_enum_validation.h"
#include "src/epp/impl/object_states_localized.h"
#include "src/fredlib/domain/enum_validation_extension.h"
#include "util/db/nullable.h"

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <set>
#include <string>

namespace Epp {
namespace Domain {

struct InfoDomainLocalizedOutputData
{

    typedef std::set<Fred::Object_State::Enum> States;

    std::string roid; ///< Domain repository ID
    std::string fqdn; ///< Domain FQDN
    std::string registrant;
    Nullable<std::string> nsset;
    Nullable<std::string> keyset;
    ObjectStatesLocalized localized_external_states; ///< Domain states list
    std::string sponsoring_registrar_handle; ///< Registrar which has to right for change
    std::string creating_registrar_handle; ///< Registrar which created contact
    Nullable<std::string> last_update_registrar_handle; ///< Registrar which realized changes
    boost::posix_time::ptime crdate; ///< Creation date and time
    Nullable<boost::posix_time::ptime> last_update; ///< Date and time of last change
    Nullable<boost::posix_time::ptime> last_transfer; ///< Date and time of last transfer
    boost::gregorian::date exdate;
    boost::optional<std::string> authinfopw; ///< Password for keyset transfer
    std::set<std::string> admin; ///< List of contacts identifier
    Nullable<EnumValidationExtension> ext_enum_domain_validation; ///< ENUM domain validation extension info
    std::set<std::string> tmpcontact; ///< List of contacts identifier OBSOLETE


    InfoDomainLocalizedOutputData(
            const std::string _roid,
            const std::string _fqdn,
            const std::string _registrant,
            const Nullable<std::string> _nsset,
            const Nullable<std::string> _keyset,
            const ObjectStatesLocalized _localized_external_states,
            const std::string _sponsoring_registrar_handle,
            const std::string _creating_registrar_handle,
            const Nullable<std::string> _last_update_registrar_handle,
            const boost::posix_time::ptime _crdate,
            const Nullable<boost::posix_time::ptime> _last_update,
            const Nullable<boost::posix_time::ptime> _last_transfer,
            const boost::gregorian::date _exdate,
            const boost::optional<std::string> _authinfopw,
            const std::set<std::string> _admin,
            const Nullable<EnumValidationExtension> _ext_enum_domain_validation,
            const std::set<std::string> _tmpcontact)
        : roid(_roid),
          fqdn(_fqdn),
          registrant(_registrant),
          nsset(_nsset),
          keyset(_keyset),
          localized_external_states(_localized_external_states),
          sponsoring_registrar_handle(_sponsoring_registrar_handle),
          creating_registrar_handle(_creating_registrar_handle),
          last_update_registrar_handle(_last_update_registrar_handle),
          crdate(_crdate),
          last_update(_last_update),
          last_transfer(_last_transfer),
          exdate(_exdate),
          authinfopw(_authinfopw),
          admin(_admin),
          ext_enum_domain_validation(_ext_enum_domain_validation),
          tmpcontact(_tmpcontact)
    {
    }


};

} // namespace Epp::Domain
} // namespace Epp

#endif
