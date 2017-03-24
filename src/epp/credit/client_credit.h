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

#ifndef CLIENT_CREDIT_H_E89D3E7C5F1FFE3E22340654403DA23C//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CLIENT_CREDIT_H_E89D3E7C5F1FFE3E22340654403DA23C

#include "src/fredlib/opcontext.h"
#include "util/decimal/decimal.h"

#include <set>
#include <string>

#include <boost/optional.hpp>

namespace Epp {
namespace Credit {

class ZoneCredit
{
public:
    explicit ZoneCredit(const std::string& _zone_fqdn);
    ZoneCredit(const std::string& _zone_fqdn, const Decimal& _credit);
    const std::string& get_zone_fqdn()const;
    bool has_credit()const;
    const Decimal& get_credit()const;
private:
    const std::string zone_fqdn_;
    const boost::optional<Decimal> credit_;
};

struct OrderZoneCreditByZoneFqdn
{
    bool operator()(const ZoneCredit& a, const ZoneCredit& b)const;
};

typedef std::set<ZoneCredit, OrderZoneCreditByZoneFqdn> ClientCreditOutputData;

/**
 * @throws ExceptionAuthErrorServerClosingConnection
 */
ClientCreditOutputData client_credit(
        Fred::OperationContext& _ctx,
        unsigned long long _registrar_id);

} // namespace Epp::Credit
} // namespace Epp

#endif//CLIENT_CREDIT_H_E89D3E7C5F1FFE3E22340654403DA23C
