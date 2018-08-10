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

#ifndef INVOICE_REFERENCE_HH_378B5644BFF0427BB8F2E198446B6802
#define INVOICE_REFERENCE_HH_378B5644BFF0427BB8F2E198446B6802

#include "src/backend/credit.hh"
#include "src/util/types/money.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace Accounting {

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
            const InvoiceType _type,
            const Credit& _credit_change)
        : id(_id),
          number(_number),
          type(_type),
          credit_change(_credit_change)
    {
    }

    unsigned long long id;
    std::string number;
    InvoiceType type;
    Credit credit_change;
};

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
