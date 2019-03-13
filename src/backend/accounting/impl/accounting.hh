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
#ifndef ACCOUNTING_HH_3CD8689C985C4099A4B543F90F67A195
#define ACCOUNTING_HH_3CD8689C985C4099A4B543F90F67A195

#include "src/backend/accounting/invoice_reference.hh"
#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/payment_invoices.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/backend/accounting/registrar_reference.hh"
#include "src/backend/credit.hh"
#include "libfred/opcontext.hh"

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/optional/optional.hpp>

#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Accounting {
namespace Impl {

Fred::Backend::Accounting::Registrar get_registrar_by_payment(
        LibFred::OperationContext& _ctx,
        const PaymentData& _payment_data);

Fred::Backend::Accounting::Registrar get_registrar_by_handle(
        LibFred::OperationContext& _ctx,
        const std::string& _handle);

std::string get_zone_by_payment(
        LibFred::OperationContext& _ctx,
        const PaymentData& _payment_data);

PaymentInvoices import_payment(
        const PaymentData& _payment_data);

PaymentInvoices import_payment_by_registrar_handle(
        const PaymentData& _payment_data,
        const boost::optional<boost::gregorian::date>& _tax_date,
        const std::string& _registrar_handle);

std::vector<RegistrarReference> get_registrar_references(
        LibFred::OperationContext& _ctx);

} // namespace Fred::Backend::Accounting::Impl
} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
