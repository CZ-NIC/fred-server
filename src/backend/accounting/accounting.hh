/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef ACCOUNTING_HH_B0BB895B67184C40A0F08C5E9A452B84
#define ACCOUNTING_HH_B0BB895B67184C40A0F08C5E9A452B84

#include "src/backend/credit.hh"
#include "src/backend/accounting/exceptions.hh"
#include "src/backend/accounting/invoice_reference.hh"
#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/payment_invoices.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/backend/accounting/registrar_reference.hh"
#include "libfred/opcontext.hh"

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/optional/optional.hpp>

#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Accounting {

Fred::Backend::Accounting::Registrar get_registrar_by_payment(
        const PaymentData& _payment_data,
        std::string& _zone);

Fred::Backend::Accounting::Registrar get_registrar_by_handle_and_payment(
        const std::string& _registrar_handle,
        const PaymentData& _payment_data,
        std::string& _zone);

PaymentInvoices import_payment(
        const PaymentData& _payment_data);

PaymentInvoices import_payment_by_registrar_handle(
        const PaymentData& _payment_data,
        const boost::optional<boost::gregorian::date>& _tax_date,
        const std::string& _registrar_handle);

std::vector<RegistrarReference> get_registrar_references();

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
