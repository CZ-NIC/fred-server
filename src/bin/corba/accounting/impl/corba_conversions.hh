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

#ifndef CORBA_CONVERSIONS_HH_65490EF25C6A46EBA2460E1AF2C4C7D7
#define CORBA_CONVERSIONS_HH_65490EF25C6A46EBA2460E1AF2C4C7D7

#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/backend/credit.hh"
#include "src/bin/corba/Accounting.hh"
#include "src/util/types/money.hh"

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

} // namespace CorbaConversions::Accounting::Impl
} // namespace CorbaConversions::Accounting
} // namespace CorbaConversions

#endif
