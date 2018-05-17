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

#include "src/backend/buffer.hh"
#include "src/backend/credit.hh"
#include "src/backend/accounting/exceptions.hh"
#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/libfred/documents.hh"
#include "src/libfred/mailer.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/registrable_object/contact/place_address.hh"
#include "src/libfred/registrable_object/domain/enum_validation_extension.hh"
#include "src/libfred/registrable_object/nsset/nsset_dns_host.hh"
#include "src/util/db/nullable.hh"
#include "src/util/optional_value.hh"
#include "src/util/types/money.hh"
#include "src/util/tz/get_psql_handle_of.hh"
#include "src/util/tz/local_timestamp.hh"
#include "src/libfred/banking/bank_manager.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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
        const Credit& _credit_amount_to_substract);

Fred::Backend::Accounting::Registrar get_registrar_by_payment(
        const PaymentData& _payment_data,
        std::string& _zone);

Fred::Backend::Accounting::Registrar get_registrar_by_handle_and_payment(
        const std::string& _registrar_handle,
        const PaymentData& _payment_data,
        std::string& _zone);

void import_payment(
        const PaymentData& _payment_data,
        Credit& _credit);

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred

#endif
