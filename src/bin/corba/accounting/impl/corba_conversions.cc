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

#include "src/bin/corba/accounting/impl/exceptions.hh"
#include "src/bin/corba/util/corba_conversions_buffer.hh"
#include "src/bin/corba/util/corba_conversions_int.hh"
#include "src/bin/corba/util/corba_conversions_isodate.hh"
#include "src/bin/corba/util/corba_conversions_isodatetime.hh"
#include "src/bin/corba/util/corba_conversions_money.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/util/log/logger.hh"

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
    try {
        std::string (*unwrap_string)(const char*) = LibFred::Corba::unwrap_string_from_const_char_ptr;

        Fred::Backend::Accounting::PaymentData payment_data;
        payment_data.uuid = unwrap_string(_payment_data.uuid.in());
        payment_data.account_number = unwrap_string(_payment_data.account_number.in());
        payment_data.account_payment_ident = unwrap_string(_payment_data.account_payment_ident.in());
        payment_data.counter_account_number = unwrap_string(_payment_data.counter_account_number.in());
        payment_data.counter_account_name = unwrap_string(_payment_data.counter_account_name.in());
        payment_data.constant_symbol = unwrap_string(_payment_data.constant_symbol.in());
        payment_data.variable_symbol = unwrap_string(_payment_data.variable_symbol.in());
        payment_data.specific_symbol = unwrap_string(_payment_data.specific_symbol.in());
        payment_data.price = unwrap_Registry_Accounting_Money(_payment_data.price);
        payment_data.date = CorbaConversion::Util::unwrap_IsoDate_to_boost_gregorian_date(_payment_data.date);
        payment_data.memo = unwrap_string(_payment_data.memo.in());
        payment_data.creation_time =
                CorbaConversion::Util::unwrap_IsoDateTime_to_boost_posix_time_ptime(
                        _payment_data.creation_time);
        return payment_data;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw InvalidPaymentData();
    }
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
    try {
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
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw InvalidPaymentData();
    }
}

Registry::Accounting::Registrar
wrap_Backend_Accounting_Registrar_to_Registry_Accounting_Registrar(
        const Fred::Backend::Accounting::Registrar& _registrar)
{
    try {
        Registry::Accounting::Registrar registrar;
        LibFred::Corba::unwrap_int(_registrar.id, registrar.id);
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
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw InvalidPaymentData();
    }
}

void
wrap_Fred_Backend_Credit_to_Registry_Accounting_Credit(
        const Fred::Backend::Credit& _src,
        Registry::Accounting::Credit& _dst)
{
    _dst.value = LibFred::Corba::wrap_string_to_corba_string(_src.value.get_string());
}

namespace {

Registry::Accounting::RegistrarReference
wrap_Fred_Backend_Accounting_RegistrarReference_to_Registry_Accounting_RegistrarReference(
        const Fred::Backend::Accounting::RegistrarReference& _registrar_reference)
{
    Registry::Accounting::RegistrarReference registrar_reference;
    registrar_reference.handle = LibFred::Corba::wrap_string_to_corba_string(_registrar_reference.handle);
    registrar_reference.name = LibFred::Corba::wrap_string_to_corba_string(_registrar_reference.name);
    return registrar_reference;
}

} // namespace CorbaConversion::Accounting::Impl::{anonymous}

Registry::Accounting::RegistrarReferenceSeq*
wrap_vector_of_Fred_Backend_Accounting_RegistrarReference_to_Registry_Accounting_RegistrarReferenceSeq(
        const std::vector<Fred::Backend::Accounting::RegistrarReference>& _registrar_references)
{
    const auto registrar_reference_seq = new Registry::Accounting::RegistrarReferenceSeq();
    registrar_reference_seq->length(_registrar_references.size());

    CORBA::ULong i = 0;
    for (const auto& registrar_reference : _registrar_references)
    {
        (*registrar_reference_seq)[i] =
                wrap_Fred_Backend_Accounting_RegistrarReference_to_Registry_Accounting_RegistrarReference(
                        registrar_reference);
        ++i;
    }
    return registrar_reference_seq;
}

namespace {

Registry::Accounting::InvoiceType::Type
 wrap_Fred_Backend_Accounting_InvoiceType_to_Registry_Accounting_InvoiceType_Type(
        const Fred::Backend::Accounting::InvoiceType _invoice_type)
{
    switch (_invoice_type)
    {
        case Fred::Backend::Accounting::InvoiceType::advance:
            return Registry::Accounting::InvoiceType::advance;

        case Fred::Backend::Accounting::InvoiceType::account:
            return Registry::Accounting::InvoiceType::account;
    }
    throw std::logic_error("unexpected Fred::Backend::Accounting::InvoiceType");
}

Registry::Accounting::InvoiceReference
wrap_Fred_Backend_Accounting_InvoiceReference_to_Registry_Accounting_InvoiceReference(
        const Fred::Backend::Accounting::InvoiceReference& _invoice_reference)
{
    Registry::Accounting::InvoiceReference invoice_reference;
    LibFred::Corba::int_to_int(_invoice_reference.id, invoice_reference.id);
    invoice_reference.number = LibFred::Corba::wrap_string_to_corba_string(_invoice_reference.number);
    invoice_reference.type = wrap_Fred_Backend_Accounting_InvoiceType_to_Registry_Accounting_InvoiceType_Type(_invoice_reference.type);
    return invoice_reference;
}

} // namespace CorbaConversion::Accounting::Impl::{anonymous}

Registry::Accounting::InvoiceReferenceSeq*
wrap_Fred_Backend_Accounting_PaymentInvoices_to_Registry_Accounting_InvoiceReferenceSeq(
        const Fred::Backend::Accounting::PaymentInvoices& _payment_invoices)
{
    const auto invoice_reference_seq = new Registry::Accounting::InvoiceReferenceSeq();
    invoice_reference_seq->length(
            (_payment_invoices.advance_invoice != boost::none ? 1 : 0) +
            _payment_invoices.account_invoices.size());

    CORBA::ULong i = 0;
    if (_payment_invoices.advance_invoice != boost::none)
    {
        (*invoice_reference_seq)[i] =
                wrap_Fred_Backend_Accounting_InvoiceReference_to_Registry_Accounting_InvoiceReference(
                        *_payment_invoices.advance_invoice);
        ++i;
    }
    for (const auto& account_invoice : _payment_invoices.account_invoices)
    {
        (*invoice_reference_seq)[i] =
                wrap_Fred_Backend_Accounting_InvoiceReference_to_Registry_Accounting_InvoiceReference(
                        account_invoice);
        ++i;
    }
    return invoice_reference_seq;
}

} // namespace CorbaConversion::Accounting::Impl
} // namespace CorbaConversion::Accounting
} // namespace CorbaConversion
