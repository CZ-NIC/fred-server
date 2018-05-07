/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef UPDATE_ZONE_HH_766AB0C28E7C44F6AEC45570BC6E00D7
#define UPDATE_ZONE_HH_766AB0C28E7C44F6AEC45570BC6E00D7

#include "src/libfred/opcontext.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>

namespace LibFred {
namespace Zone {

class UpdateZone
{
public:
    UpdateZone(
            const std::string& _fqdn)
    : fqdn_(_fqdn)
    {}

    UpdateZone& set_ex_period_min(boost::gregorian::months _ex_period_min);
    UpdateZone& set_ex_period_max(boost::gregorian::months _ex_period_max);
    UpdateZone& set_enum_validation_period(boost::gregorian::months _val_period);
    UpdateZone& set_sending_warning_letter(bool _warning_letter);

    unsigned long long exec(OperationContext& _ctx);

private:
    std::string fqdn_;
    boost::optional<boost::gregorian::months> ex_period_min_;
    boost::optional<boost::gregorian::months> ex_period_max_;
    boost::optional<boost::gregorian::months> val_period_;
    boost::optional<bool> warning_letter_;
};

} // namespace Zone
} // namespace LibFred

#endif
