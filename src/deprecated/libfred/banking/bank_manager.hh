/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#ifndef BANK_MANAGER_HH_5C8C2E1D0B944AA19ED43A94CE1AE7FC
#define BANK_MANAGER_HH_5C8C2E1D0B944AA19ED43A94CE1AE7FC

#include "src/deprecated/libfred/banking/invoice_reference.hh"
#include "src/deprecated/libfred/banking/payment_invoices.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/file.hh"
#include "src/util/types/money.hh"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/optional.hpp>

namespace LibFred {
namespace Banking {


class Manager {
public:
    static Manager *create();

    virtual void addBankAccount(
            const std::string& _account_number,
            const std::string& _bank_code,
            const std::string& _zone,
            const std::string& _account_name) = 0;

    virtual PaymentInvoices importPayment(
            const std::string& _uuid,
            const std::string& _account_number,
            const boost::optional<std::string>& _account_bank_code,
            const std::string& _account_payment_ident,
            const std::string& _counter_account_number,
            const boost::optional<std::string>& _counter_account_bank_code,
            const std::string& _counter_account_name,
            const std::string& _constant_symbol,
            const std::string& _variable_symbol,
            const std::string& _specific_symbol,
            const Money& _price,
            const boost::gregorian::date _date,
            const std::string& _memo,
            const boost::posix_time::ptime _creation_time,
            const boost::optional<std::string>& _registrar_handle,
            const boost::optional<boost::gregorian::date>& _tax_date) = 0;

}; // class Manager


// smart pointer
typedef std::unique_ptr<Manager> ManagerPtr;


} // namespace Banking
} // namespace LibFred

#endif
