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

#ifndef ACCOUNTING_HH_B0BB895B67184C40A0F08C5E9A452B84
#define ACCOUNTING_HH_B0BB895B67184C40A0F08C5E9A452B84

#include "src/backend/credit.hh"
#include "src/backend/accounting/exceptions.hh"
#include "src/backend/accounting/invoice_reference.hh"
#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/libfred/opcontext.hh"

#include <set>
#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Accounting {

void increase_zone_credit_of_registrar(
        const std::string& _transaction_ident,
        const std::string& _registrar_handle,
        const std::string& _zone,
        const Credit& _credit_amount_to_add);

void decrease_zone_credit_of_registrar(
        const std::string& _transaction_ident,
        const std::string& _registrar_handle,
        const std::string& _zone,
        const Credit& _credit_amount_to_subtract);

Fred::Backend::Accounting::Registrar get_registrar_by_payment(
        const PaymentData& _payment_data,
        std::string& _zone);

Fred::Backend::Accounting::Registrar get_registrar_by_handle_and_payment(
        const std::string& _registrar_handle,
        const PaymentData& _payment_data,
        std::string& _zone);

std::vector<InvoiceReference> import_payment(
        const PaymentData& _payment_data);

std::vector<InvoiceReference> import_payment_by_registrar_handle(
        const PaymentData& _payment_data,
        const std::string& _registrar_handle);

std::set<std::string> get_registrar_handles();

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
