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
#ifndef CORBA_CONVERSIONS_HH_65490EF25C6A46EBA2460E1AF2C4C7D7
#define CORBA_CONVERSIONS_HH_65490EF25C6A46EBA2460E1AF2C4C7D7

#include "src/backend/accounting/invoice_reference.hh"
#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/payment_invoices.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/backend/accounting/registrar_reference.hh"
#include "src/backend/credit.hh"
#include "src/bin/corba/Accounting.hh"
#include "src/bin/corba/util/corba_conversions_nullableisodate.hh"
#include "src/util/types/money.hh"

#include <set>
#include <string>
#include <vector>

namespace CorbaConversion {
namespace Accounting {
namespace Impl {

Money
unwrap_Registry_Accounting_Money(
       Registry::Accounting::Money _money);

Fred::Backend::Accounting::PaymentData
unwrap_Registry_Accounting_PaymentData(
        const Registry::Accounting::PaymentData& _payment_data);

Fred::Backend::Credit
unwrap_Registry_Accounting_Credit(
       Registry::Accounting::Credit _credit);

boost::optional<boost::gregorian::date>
unwrap_TaxDate(
        const Registry::NullableIsoDate* src_ptr);

Registry::Accounting::PlaceAddress
wrap_Backend_Accounting_PlaceAddress_to_Registry_Accounting_PlaceAddress(
        const Fred::Backend::Accounting::PlaceAddress& _place_address);

Registry::Accounting::Registrar
wrap_Backend_Accounting_Registrar_to_Registry_Accounting_Registrar(
        const Fred::Backend::Accounting::Registrar& _registrar);

void
wrap_Fred_Backend_Credit_to_Registry_Accounting_Credit(
        const Fred::Backend::Credit& _src,
        Registry::Accounting::Credit& _dst);

Registry::Accounting::RegistrarReferenceSeq*
wrap_vector_of_Fred_Backend_Accounting_RegistrarReference_to_Registry_Accounting_RegistrarReferenceSeq(
        const std::vector<Fred::Backend::Accounting::RegistrarReference>& _registrar_references);

Registry::Accounting::InvoiceReferenceSeq*
wrap_Fred_Backend_Accounting_PaymentInvoices_to_Registry_Accounting_InvoiceReferenceSeq(
        const Fred::Backend::Accounting::PaymentInvoices& _invoice_references);

} // namespace CorbaConversions::Accounting::Impl
} // namespace CorbaConversions::Accounting
} // namespace CorbaConversions

#endif
