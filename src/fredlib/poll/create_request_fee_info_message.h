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

#ifndef CREATE_REQUEST_FEE_INFO_MESSAGE_H_77A356ACEC01445E95360A5D704119D6
#define CREATE_REQUEST_FEE_INFO_MESSAGE_H_77A356ACEC01445E95360A5D704119D6

#include "src/fredlib/opcontext.h"
#include "util/decimal/decimal.h"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace Fred {
namespace Poll {

class CreateRequestFeeInfoMessage
{
public:
    CreateRequestFeeInfoMessage(
        unsigned long long _registrar_id,
        const boost::posix_time::ptime& _period_from,
        const boost::posix_time::ptime& _period_to,
        unsigned long long _total_free_count,
        unsigned long long _request_count,
        const Decimal& _price,
        const std::string& _time_zone = "Europe/Prague")
    :
        registrar_id_(_registrar_id),
        period_from_(_period_from),
        period_to_(_period_to),
        total_free_count_(_total_free_count),
        request_count_(_request_count),
        price_(_price),
        time_zone_(_time_zone)
    {}

    unsigned long long exec(OperationContext& _ctx) const;
private:
    unsigned long long registrar_id_;
    boost::posix_time::ptime period_from_;
    boost::posix_time::ptime period_to_;
    unsigned long long total_free_count_;
    unsigned long long request_count_;
    Decimal price_;
    std::string time_zone_;
};

} // namespace Fred::Poll
} // namespace Fred

#endif
