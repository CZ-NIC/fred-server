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

#ifndef ACCOUNTING_I_HH_F0DCD580DEA74C35B7172B9034F8FB79
#define ACCOUNTING_I_HH_F0DCD580DEA74C35B7172B9034F8FB79

#include "src/bin/corba/Accounting.hh"
#include "src/bin/corba/IsoDateTime.hh"
#include "src/bin/corba/Registry.hh"
#include "src/libfred/banking/bank_manager.hh"
#include "src/libfred/documents.hh"

#include <boost/utility.hpp>

#include <string>

namespace CorbaConversion {
namespace Accounting {

class AccountingImpl
        : public POA_Registry::Accounting::AccountingIntf,
          private boost::noncopyable
{
public:
    AccountingImpl(const std::string& _server_name);

    // methods corresponding to defined IDL attributes and operations

    /*
    void increase_zone_credit_of_registrar(
            const char* _transaction_ident,
            const char* _registrar_handle,
            const char* _zone,
            const Registry::Accounting::Credit& _credit_amount_to_add) final override;

    void decrease_zone_credit_of_registrar(
            const char* _transaction_ident,
            const char* _registrar_handle,
            const char* _zone,
            const Registry::Accounting::Credit& _credit_amount_to_subtract) final override;
    */

    Registry::Accounting::Registrar* get_registrar_by_payment(
            const Registry::Accounting::PaymentData& _payment_data,
            CORBA::String_out _zone) final override;

    Registry::Accounting::Registrar* get_registrar_by_handle_and_payment(
            const char* _registrar_handle,
            const Registry::Accounting::PaymentData& _payment_data,
            CORBA::String_out _zone) final override;

    Registry::Accounting::InvoiceReferenceSeq* import_payment(
            const Registry::Accounting::PaymentData& _payment_data,
            Registry::Accounting::Credit_out _remaining_credit) final override;

    Registry::Accounting::InvoiceReferenceSeq* import_payment_by_registrar_handle(
            const Registry::Accounting::PaymentData& _payment_data,
            const char* _registrar_handle,
            Registry::Accounting::Credit_out _remaining_credit) final override;

    Registry::Accounting::RegistrarHandleSeq* get_registrar_handles() final override;

private:
    // Make sure all instances are built on the heap by making the destructor non-public
    ~AccountingImpl() final override = default;

    std::string server_name_;
};

} // namespace CorbaConversion::Accounting
} // namespace CorbaConversion

#endif
