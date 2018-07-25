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

#include "src/bin/corba/accounting/impl/corba_conversions.hh"

#include "src/bin/corba/util/corba_conversions_buffer.hh"
#include "src/bin/corba/util/corba_conversions_isodate.hh"
#include "src/bin/corba/util/corba_conversions_isodatetime.hh"
#include "src/bin/corba/util/corba_conversions_money.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"

#include <string>

namespace CorbaConversion {
namespace Accounting {
namespace Impl {

Money
unwrap_Registry_Accounting_Money(
       Registry::Accounting::Money _money)
{
    return Money(LibFred::Corba::unwrap_string_from_const_char_ptr(_money.value.in()));
}

Fred::Backend::Accounting::PaymentData
unwrap_Registry_Accounting_PaymentData(
        const Registry::Accounting::PaymentData& _payment_data)
{
    Fred::Backend::Accounting::PaymentData payment_data;
    payment_data.bank_payment = LibFred::Corba::unwrap_string_from_const_char_ptr(_payment_data.bank_payment.in());
    payment_data.uuid = LibFred::Corba::unwrap_string_from_const_char_ptr(_payment_data.uuid.in());
    payment_data.account_number = LibFred::Corba::unwrap_string_from_const_char_ptr(_payment_data.account_number.in());
    payment_data.counter_account_number = LibFred::Corba::unwrap_string_from_const_char_ptr(_payment_data.counter_account_number.in());
    payment_data.counter_account_name = LibFred::Corba::unwrap_string_from_const_char_ptr(_payment_data.counter_account_name.in());
    payment_data.constant_symbol = LibFred::Corba::unwrap_string_from_const_char_ptr(_payment_data.constant_symbol.in());
    payment_data.variable_symbol = LibFred::Corba::unwrap_string_from_const_char_ptr(_payment_data.variable_symbol.in());
    payment_data.specific_symbol = LibFred::Corba::unwrap_string_from_const_char_ptr(_payment_data.specific_symbol.in());
    payment_data.price = unwrap_Registry_Accounting_Money(_payment_data.price);
    payment_data.date = CorbaConversion::Util::unwrap_IsoDate_to_boost_gregorian_date(_payment_data.date);
    payment_data.memo = LibFred::Corba::unwrap_string_from_const_char_ptr(_payment_data.memo.in());
    payment_data.creation_time = CorbaConversion::Util::unwrap_IsoDateTime_to_boost_posix_time_ptime(_payment_data.creation_time);
    return payment_data;
}

Fred::Backend::Credit
unwrap_Registry_Accounting_Credit(
       Registry::Accounting::Credit _credit)
{
    return Fred::Backend::Credit(LibFred::Corba::unwrap_string_from_const_char_ptr(_credit.value.in()));
}

Registry::Accounting::PlaceAddress
wrap_Backend_Accounting_PlaceAddress_to_Registry_Accounting_PlaceAddress(
        const Fred::Backend::Accounting::PlaceAddress& _place_address)
{
    Registry::Accounting::PlaceAddress place_address;
    place_address.street1 = LibFred::Corba::wrap_string_to_corba_string(_place_address.street1);
    place_address.street2 = LibFred::Corba::wrap_string_to_corba_string(_place_address.street2);
    place_address.street3 = LibFred::Corba::wrap_string_to_corba_string(_place_address.street3);
    place_address.city = LibFred::Corba::wrap_string_to_corba_string(_place_address.city);
    place_address.stateorprovince = LibFred::Corba::wrap_string_to_corba_string(_place_address.stateorprovince);
    place_address.postalcode = LibFred::Corba::wrap_string_to_corba_string(_place_address.postalcode);
    place_address.country_code = LibFred::Corba::wrap_string_to_corba_string(_place_address.country_code);
    return place_address;
}

Registry::Accounting::Registrar
wrap_Backend_Accounting_Registrar_to_Registry_Accounting_Registrar(
        const Fred::Backend::Accounting::Registrar& _registrar)
{
    Registry::Accounting::Registrar registrar;
    registrar.handle = LibFred::Corba::wrap_string_to_corba_string(_registrar.handle);
    registrar.name = LibFred::Corba::wrap_string_to_corba_string(_registrar.name);
    registrar.organization = LibFred::Corba::wrap_string_to_corba_string(_registrar.organization);
    registrar.cin = LibFred::Corba::wrap_string_to_corba_string(_registrar.cin);
    registrar.tin = LibFred::Corba::wrap_string_to_corba_string(_registrar.tin);
    registrar.url = LibFred::Corba::wrap_string_to_corba_string(_registrar.url);
    registrar.phone = LibFred::Corba::wrap_string_to_corba_string(_registrar.phone);
    registrar.fax = LibFred::Corba::wrap_string_to_corba_string(_registrar.fax);
    registrar.address = wrap_Backend_Accounting_PlaceAddress_to_Registry_Accounting_PlaceAddress(_registrar.address);
    return registrar;
}

void
wrap_Fred_Backend_Credit_to_Registry_Accounting_Credit(
        const Fred::Backend::Credit& _src,
        Registry::Accounting::Credit& _dst)
{
    _dst.value = LibFred::Corba::wrap_string_to_corba_string(_src.value.get_string());
}

} // namespace CorbaConversion::Accounting::Impl
} // namespace CorbaConversion::Accounting
} // namespace CorbaConversion
