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
/**
 *  @file
 *  registry record statement corba implementation
 */

//pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/bin/corba ~/workspace_18680/enum/idl/idl/Accounting.idl

#include "src/bin/corba/accounting/accounting_i.hh"

#include "src/backend/accounting/accounting.hh"
#include "src/backend/accounting/exceptions.hh"
#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/backend/credit.hh"
#include "src/bin/corba/Accounting.hh"
#include "src/bin/corba/accounting/impl/corba_conversions.hh"
#include "src/bin/corba/accounting/impl/exceptions.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"

#include <boost/date_time/gregorian/greg_date.hpp>

#include <stdexcept>
#include <string>

namespace CorbaConversion {
namespace Accounting {

AccountingImpl::AccountingImpl(
        const std::string& _server_name)
    : server_name_(_server_name)
{
}

Registry::Accounting::Registrar* AccountingImpl::get_registrar_by_payment(
        const Registry::Accounting::PaymentData& _payment_data,
        CORBA::String_out _zone)
{
    try
    {
        std::string zone;
        Fred::Backend::Accounting::Registrar registrar =
                Fred::Backend::Accounting::get_registrar_by_payment(
                        Impl::unwrap_Registry_Accounting_PaymentData(_payment_data),
                        zone);

        Registry::Accounting::Registrar_var return_value =
                new Registry::Accounting::Registrar(
                        Impl::wrap_Backend_Accounting_Registrar_to_Registry_Accounting_Registrar(registrar));

        // no exception shall be thrown from here onwards

        _zone = LibFred::Corba::wrap_string_to_corba_string(zone)._retn();
        return return_value._retn();
    }
    catch (const Impl::InvalidPaymentData&)
    {
        throw Registry::Accounting::INVALID_PAYMENT_DATA();
    }
    catch (const Fred::Backend::Accounting::RegistrarNotFound&)
    {
        throw Registry::Accounting::REGISTRAR_NOT_FOUND();
    }
    catch (const Fred::Backend::Accounting::InvalidPaymentData&)
    {
        throw Registry::Accounting::INVALID_PAYMENT_DATA();
    }
    catch (...)
    {
        throw Registry::Accounting::INTERNAL_SERVER_ERROR();
    }
}

Registry::Accounting::Registrar* AccountingImpl::get_registrar_by_handle_and_payment(
        const char* _registrar_handle,
        const Registry::Accounting::PaymentData& _payment_data,
        CORBA::String_out _zone)
{
    try
    {
        std::string zone;
        Fred::Backend::Accounting::Registrar registrar =
                Fred::Backend::Accounting::get_registrar_by_handle_and_payment(
                        LibFred::Corba::unwrap_string_from_const_char_ptr(_registrar_handle),
                        Impl::unwrap_Registry_Accounting_PaymentData(_payment_data),
                        zone);

        Registry::Accounting::Registrar_var return_value =
                new Registry::Accounting::Registrar(
                        Impl::wrap_Backend_Accounting_Registrar_to_Registry_Accounting_Registrar(registrar));

        // no exception shall be thrown from here onwards

        _zone = LibFred::Corba::wrap_string_to_corba_string(zone)._retn();
        return return_value._retn();
    }
    catch (const Impl::InvalidPaymentData&)
    {
        throw Registry::Accounting::INVALID_PAYMENT_DATA();
    }
    catch (const Fred::Backend::Accounting::RegistrarNotFound&)
    {
        throw Registry::Accounting::REGISTRAR_NOT_FOUND();
    }
    catch (const Fred::Backend::Accounting::InvalidPaymentData&)
    {
        throw Registry::Accounting::INVALID_PAYMENT_DATA();
    }
    catch (...)
    {
        throw Registry::Accounting::INTERNAL_SERVER_ERROR();
    }
}

Registry::Accounting::InvoiceReferenceSeq* AccountingImpl::import_payment(
    const Registry::Accounting::PaymentData& _payment_data,
    Registry::Accounting::Credit_out _remaining_credit)
{
    try
    {
        const Fred::Backend::Accounting::PaymentInvoices payment_invoices =
                Fred::Backend::Accounting::import_payment(
                        Impl::unwrap_Registry_Accounting_PaymentData(_payment_data));

        const auto credit =
                payment_invoices.advance_invoice != boost::none
                ? (*payment_invoices.advance_invoice).credit_change
                : Fred::Backend::Credit("0");
        Registry::Accounting::Credit_var remaining_credit = new Registry::Accounting::Credit();
        Impl::wrap_Fred_Backend_Credit_to_Registry_Accounting_Credit(credit, remaining_credit.inout());

        Registry::Accounting::InvoiceReferenceSeq_var result =
                Impl::wrap_Fred_Backend_Accounting_PaymentInvoices_to_Registry_Accounting_InvoiceReferenceSeq(
                        payment_invoices);

        // no exception shall be thrown from here onwards

        _remaining_credit = remaining_credit._retn();
        return result._retn();
    }
    catch (const Impl::InvalidPaymentData&)
    {
        throw Registry::Accounting::INVALID_PAYMENT_DATA();
    }
    catch (const Fred::Backend::Accounting::RegistrarNotFound&)
    {
        throw Registry::Accounting::INVALID_PAYMENT_DATA();
    }
    catch (const Fred::Backend::Accounting::InvalidPaymentData&)
    {
        throw Registry::Accounting::INVALID_PAYMENT_DATA();
    }
    catch (const Fred::Backend::Accounting::PaymentTooOld&)
    {
        throw Registry::Accounting::PAYMENT_TOO_OLD();
    }
    catch (const Fred::Backend::Accounting::PaymentAlreadyProcessed&)
    {
        throw Registry::Accounting::PAYMENT_ALREADY_PROCESSED();
    }
    catch (...)
    {
        throw Registry::Accounting::INTERNAL_SERVER_ERROR();
    }
}

Registry::Accounting::InvoiceReferenceSeq* AccountingImpl::import_payment_by_registrar_handle(
    const Registry::Accounting::PaymentData& _payment_data,
    const char* _registrar_handle,
    Registry::NullableIsoDate* _tax_date,
    Registry::Accounting::Credit_out _remaining_credit)
{
    try
    {
        const Fred::Backend::Accounting::PaymentInvoices payment_invoices =
                Fred::Backend::Accounting::import_payment_by_registrar_handle(
                        Impl::unwrap_Registry_Accounting_PaymentData(_payment_data),
                        Impl::unwrap_TaxDate(_tax_date),
                        LibFred::Corba::unwrap_string_from_const_char_ptr(_registrar_handle));

        const auto credit =
                payment_invoices.advance_invoice != boost::none
                ? (*payment_invoices.advance_invoice).credit_change
                : Fred::Backend::Credit("0");
        Registry::Accounting::Credit_var remaining_credit = new Registry::Accounting::Credit();
        Impl::wrap_Fred_Backend_Credit_to_Registry_Accounting_Credit(credit, remaining_credit.inout());

        Registry::Accounting::InvoiceReferenceSeq_var result =
                Impl::wrap_Fred_Backend_Accounting_PaymentInvoices_to_Registry_Accounting_InvoiceReferenceSeq(
                        payment_invoices);

        // no exception shall be thrown from here onwards

        _remaining_credit = remaining_credit._retn();
        return result._retn();
    }
    catch (const Impl::InvalidPaymentData&)
    {
        throw Registry::Accounting::INVALID_PAYMENT_DATA();
    }
    catch (const Impl::InvalidTaxDateFormat&)
    {
        throw Registry::Accounting::INVALID_TAX_DATE_FORMAT();
    }
    catch (const Fred::Backend::Accounting::InvalidTaxDateFormat&)
    {
        throw Registry::Accounting::INVALID_TAX_DATE_FORMAT();
    }
    catch (const Fred::Backend::Accounting::TaxDateTooOld&)
    {
        throw Registry::Accounting::INVALID_TAX_DATE_VALUE();
    }
    catch (const Fred::Backend::Accounting::RegistrarNotFound&)
    {
        throw Registry::Accounting::REGISTRAR_NOT_FOUND();
    }
    catch (const Fred::Backend::Accounting::InvalidPaymentData&)
    {
        throw Registry::Accounting::INVALID_PAYMENT_DATA();
    }
    catch (const Fred::Backend::Accounting::PaymentAlreadyProcessed&)
    {
        throw Registry::Accounting::PAYMENT_ALREADY_PROCESSED();
    }
    catch (...)
    {
        throw Registry::Accounting::INTERNAL_SERVER_ERROR();
    }
}

Registry::Accounting::RegistrarReferenceSeq* AccountingImpl::get_registrar_references()
{
    try
    {
        const std::vector<Fred::Backend::Accounting::RegistrarReference> registrar_handles =
                Fred::Backend::Accounting::get_registrar_references();

        return Impl::wrap_vector_of_Fred_Backend_Accounting_RegistrarReference_to_Registry_Accounting_RegistrarReferenceSeq(
                registrar_handles);
    }
    catch (...)
    {
        throw Registry::Accounting::INTERNAL_SERVER_ERROR();
    }
}

} // namespace CorbaConversion::Accounting
} // namespace CorbaConversion
