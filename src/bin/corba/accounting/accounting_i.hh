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
#ifndef ACCOUNTING_I_HH_F0DCD580DEA74C35B7172B9034F8FB79
#define ACCOUNTING_I_HH_F0DCD580DEA74C35B7172B9034F8FB79

#include "src/bin/corba/Accounting.hh"
#include "src/bin/corba/IsoDate.hh"
#include "src/bin/corba/Registry.hh"
#include "src/deprecated/libfred/banking/bank_manager.hh"
#include "src/deprecated/libfred/documents.hh"

#include <string>

namespace CorbaConversion {
namespace Accounting {

class AccountingImpl final
        : public POA_Registry::Accounting::AccountingIntf
{
public:
    explicit AccountingImpl(const std::string& _server_name);

    AccountingImpl(const AccountingImpl&) = delete;

    AccountingImpl& operator=(const AccountingImpl&) = delete;

    // methods corresponding to defined IDL attributes and operations

    Registry::Accounting::Registrar* get_registrar_by_payment(
            const Registry::Accounting::PaymentData& _payment_data,
            CORBA::String_out _zone) override;

    Registry::Accounting::Registrar* get_registrar_by_handle_and_payment(
            const char* _registrar_handle,
            const Registry::Accounting::PaymentData& _payment_data,
            CORBA::String_out _zone) override;

    Registry::Accounting::InvoiceReferenceSeq* import_payment(
            const Registry::Accounting::PaymentData& _payment_data,
            Registry::Accounting::Credit_out _remaining_credit) override;

    Registry::Accounting::InvoiceReferenceSeq* import_payment_by_registrar_handle(
            const Registry::Accounting::PaymentData& _payment_data,
            const char* _registrar_handle,
            const Registry::IsoDate& _tax_date,
            Registry::Accounting::Credit_out _remaining_credit) override;

    Registry::Accounting::RegistrarReferenceSeq* get_registrar_references() override;

private:
    // Make sure all instances are built on the heap by making the destructor non-public
    ~AccountingImpl() override = default;

    std::string server_name_;
};

} // namespace CorbaConversion::Accounting
} // namespace CorbaConversion

#endif
