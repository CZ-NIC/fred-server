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

/**
 *  @file
 *  registry record statement corba implementation
 */

//pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/bin/corba ~/workspace_18680/enum/idl/idl/Accounting.idl

#include "src/bin/corba/accounting/accounting_i.hh"

#include "src/backend/accounting/accounting.hh"
#include "src/backend/accounting/payment_data.hh"
#include "src/backend/accounting/registrar.hh"
#include "src/backend/credit.hh"
#include "src/bin/corba/Accounting.hh"
#include "src/bin/corba/accounting/impl/corba_conversions.hh"
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

void AccountingImpl::increase_zone_credit_of_registrar(
        const char* _transaction_ident,
        const char* _registrar_handle,
        const char* _zone,
        const Registry::Accounting::Credit& _credit_amount_to_add)
{
    try
    {
        Fred::Backend::Accounting::increase_zone_credit_of_registrar(
                LibFred::Corba::unwrap_string_from_const_char_ptr(_transaction_ident),
                LibFred::Corba::unwrap_string_from_const_char_ptr(_registrar_handle),
                LibFred::Corba::unwrap_string_from_const_char_ptr(_zone),
                Impl::unwrap_Registry_Accounting_Credit(_credit_amount_to_add));
    }
    catch (const Fred::Backend::Accounting::CreditAlreadyProcessed&)
    {
        throw Registry::Accounting::CREDIT_ALREADY_PROCESSED();
    }
    catch (const Fred::Backend::Accounting::RegistrarNotFound&)
    {
        throw Registry::Accounting::REGISTRAR_NOT_FOUND();
    }
    catch (const Fred::Backend::Accounting::InvalidZone&)
    {
        throw Registry::Accounting::INVALID_ZONE();
    }
    catch (const Fred::Backend::Accounting::InvalidCreditValue&)
    {
        throw Registry::Accounting::INVALID_CREDIT_VALUE();
    }
    catch (...)
    {
        throw Registry::Accounting::INTERNAL_SERVER_ERROR();
    }
}

void AccountingImpl::decrease_zone_credit_of_registrar(
        const char* _transaction_ident,
        const char* _registrar_handle,
        const char* _zone,
        const Registry::Accounting::Credit& _credit_amount_to_substract)
{
    try
    {
        Fred::Backend::Accounting::decrease_zone_credit_of_registrar(
                LibFred::Corba::unwrap_string_from_const_char_ptr(_transaction_ident),
                LibFred::Corba::unwrap_string_from_const_char_ptr(_registrar_handle),
                LibFred::Corba::unwrap_string_from_const_char_ptr(_zone),
                Impl::unwrap_Registry_Accounting_Credit(_credit_amount_to_substract));
    }
    catch (const Fred::Backend::Accounting::CreditAlreadyProcessed&)
    {
        throw Registry::Accounting::CREDIT_ALREADY_PROCESSED();
    }
    catch (const Fred::Backend::Accounting::RegistrarNotFound&)
    {
        throw Registry::Accounting::REGISTRAR_NOT_FOUND();
    }
    catch (const Fred::Backend::Accounting::InvalidZone&)
    {
        throw Registry::Accounting::INVALID_ZONE();
    }
    catch (const Fred::Backend::Accounting::InvalidCreditValue&)
    {
        throw Registry::Accounting::INVALID_CREDIT_VALUE();
    }
    catch (...)
    {
        throw Registry::Accounting::INTERNAL_SERVER_ERROR();
    }
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

void AccountingImpl::import_payment(
    const Registry::Accounting::PaymentData& _payment_data,
    Registry::Accounting::Credit_out _remaining_credit)
{
    try
    {
        Fred::Backend::Credit credit("0");
        Fred::Backend::Accounting::import_payment(
                Impl::unwrap_Registry_Accounting_PaymentData(_payment_data),
                credit);

        Registry::Accounting::Credit_var remaining_credit = new Registry::Accounting::Credit;
        Impl::wrap_Fred_Backend_Credit_to_Registry_Accounting_Credit(credit, remaining_credit.inout());
        _remaining_credit = remaining_credit._retn();
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

void AccountingImpl::import_payment_by_registrar_handle(
    const Registry::Accounting::PaymentData& _payment_data,
    const char* _registrar_handle,
    Registry::Accounting::Credit_out _remaining_credit)
{
    try
    {
        Fred::Backend::Credit credit("0");
        Fred::Backend::Accounting::import_payment_by_registrar_handle(
                Impl::unwrap_Registry_Accounting_PaymentData(_payment_data),
                LibFred::Corba::unwrap_string_from_const_char_ptr(_registrar_handle),
                credit);

        Registry::Accounting::Credit_var remaining_credit = new Registry::Accounting::Credit;
        Impl::wrap_Fred_Backend_Credit_to_Registry_Accounting_Credit(credit, remaining_credit.inout());
        _remaining_credit = remaining_credit._retn();
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

} // namespace CorbaConversion::Accounting
} // namespace CorbaConversion
