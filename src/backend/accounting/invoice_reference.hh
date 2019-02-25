/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef INVOICE_REFERENCE_HH_378B5644BFF0427BB8F2E198446B6802
#define INVOICE_REFERENCE_HH_378B5644BFF0427BB8F2E198446B6802

#include "src/backend/credit.hh"
#include "src/util/types/money.hh"

#include <boost/optional.hpp>
#include <boost/version.hpp>

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


#if BOOST_VERSION && BOOST_VERSION < 105600

// The (in)equality comparison with boost::none does not require that T be EqualityComparable (fixed in boost-1.56.0)

inline bool operator==(const boost::optional<InvoiceReference>& _optional, boost::none_t)
{
    return static_cast<bool>(_optional);
}
#endif

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
