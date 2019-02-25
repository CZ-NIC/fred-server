/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#include <string>
#include <exception>
#include <boost/date_time.hpp>
#include "src/util/types/money.hh"

#include "src/deprecated/libfred/registrar.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"

static std::string zone_registrar_credit_query (
        "SELECT credit FROM registrar_credit"
        " WHERE zone_id = $1::bigint AND registrar_id =$2::bigint");



Database::ID get_zone_cz_id();
LibFred::Registrar::Registrar::AutoPtr createTestRegistrarClass();
//LibFred::Registrar::Registrar::AutoPtr createTestRegistrarClassNoCz(const std::string &varsymb);
Database::ID createTestRegistrar();

bool check_std_exception_invoice_prefix(std::exception const & ex);
bool check_std_exception_out_of_range(std::exception const & ex);
bool check_std_exception_archiveInvoices(std::exception const & ex);
bool check_std_exception_createAccountInvoice(std::exception const & ex);
bool check_std_exception_billing_fail(std::exception const & ex);
bool check_dummy(std::exception const & ex);

void try_insert_invoice_prefix();
Money getOperationPrice(unsigned op, Database::ID zone_id, unsigned requested_quantity);

const Decimal get_credit(Database::ID reg_id, Database::ID zone_id);

void get_vat(int &vat_percent, std::string &vat_koef, date taxdate = day_clock::local_day());

void insert_poll_request_fee(Database::ID reg_id,
        Decimal price,
        date poll_from = date(),
        date poll_to   = date()
        );
