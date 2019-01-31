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

#ifndef PAYMENT_INVOICES_HH_9A939EB606DD40639302C6C9B0043DA3
#define PAYMENT_INVOICES_HH_9A939EB606DD40639302C6C9B0043DA3

#include "src/deprecated/libfred/banking/invoice_reference.hh"

#include <boost/optional/optional.hpp>

#include <vector>

namespace LibFred {
namespace Banking {

struct PaymentInvoices
{
    boost::optional<InvoiceReference> advance_invoice;
    std::vector<InvoiceReference> account_invoices;
};

} // namespace Banking
} // namespace LibFred


#endif
