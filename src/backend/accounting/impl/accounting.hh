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

#ifndef ACCOUNTING_HH_3CD8689C985C4099A4B543F90F67A195
#define ACCOUNTING_HH_3CD8689C985C4099A4B543F90F67A195

#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/backend/credit.hh"
#include "src/libfred/opcontext.hh"


namespace Fred {
namespace Backend {
namespace Accounting {
namespace Impl {

void increase_zone_credit_of_registrar(
        LibFred::OperationContext& _ctx,
        const std::string& _transaction_ident,
        const std::string& _registrar_handle,
        const std::string& _zone,
        const Credit& _credit_amount_to_add);

void decrease_zone_credit_of_registrar(
        LibFred::OperationContext& _ctx,
        const std::string& _transaction_ident,
        const std::string& _registrar_handle,
        const std::string& _zone,
        const Credit& _credit_amount_to_substract);

Fred::Backend::Accounting::Registrar get_registrar_by_payment(
        LibFred::OperationContext& _ctx,
        const PaymentData& _payment_data);

Fred::Backend::Accounting::Registrar get_registrar_by_handle(
        LibFred::OperationContext& _ctx,
        const std::string& _handle);

std::string get_zone_by_payment(
        LibFred::OperationContext& _ctx,
        const PaymentData& _payment_data);

void import_payment(
        const PaymentData& _payment_data,
        Credit& _remaining_credit);

void import_payment_by_registrar_handle(
        const PaymentData& _payment_data,
        const std::string& _registrar_handle,
        Credit& _remaining_credit);

} // namespace Fred::Backend::Accounting::Impl
} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
