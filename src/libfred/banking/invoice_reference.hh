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

#ifndef INVOICE_REFERENCE_HH_C8E3F378923646C3B8794166E548E810
#define INVOICE_REFERENCE_HH_C8E3F378923646C3B8794166E548E810

#include "src/util/types/money.hh"

#include <string>

namespace LibFred {
namespace Banking {

enum struct InvoiceType
{
    advance,
    account
};

struct InvoiceReference
{
    InvoiceReference(
            const unsigned long long _id,
            const std::string& _number,
            const InvoiceType& _type,
            const Money& _credit_change)
        : id(_id),
          number(_number),
          type(_type),
          credit_change(_credit_change)
    {
    }

    bool operator==(const InvoiceReference& _other) const
    {
        return (this->id == _other.id &&
                this->number == _other.number &&
                this->type == _other.type &&
                this->credit_change == _other.credit_change);
    }

    unsigned long long id;
    std::string number;
    InvoiceType type;
    Money credit_change;
};

} // namespace LibFred::Banking
} // namespace LibFred

#endif
