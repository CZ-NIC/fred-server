/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef DOMAIN_ENUM_VALIDATION_HH_DCB9A66C2D764A84863FBFAA5ADB7651
#define DOMAIN_ENUM_VALIDATION_HH_DCB9A66C2D764A84863FBFAA5ADB7651

#include "libfred/opcontext.hh"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <stdexcept>

namespace Epp {
namespace Domain {

/**
 *
 */
class EnumValidationExtension
{
    boost::gregorian::date valexdate;
    bool publish;

public:
    EnumValidationExtension()
        : publish(false)
    {
    }


    EnumValidationExtension(
            const boost::gregorian::date& _valexdate,
            bool _publish)
        : valexdate(_valexdate),
          publish(_publish)
    {
    }


    boost::gregorian::date get_valexdate() const
    {
        return valexdate;
    }


    bool get_publish() const
    {
        return publish;
    }


};

bool is_new_enum_domain_validation_expiration_date_invalid(
        const boost::gregorian::date& new_valexdate, // local date
        const boost::gregorian::date& current_local_date,
        unsigned enum_validation_period, // in months
        const boost::optional<boost::gregorian::date>& current_valexdate, // if not set, ENUM domain is not currently validated
        LibFred::OperationContext& _ctx);


} // namespece Epp::Domain
} // namespace Epp

#endif
