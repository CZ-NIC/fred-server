/*
 * Copyright (C) 2009-2020  CZ.NIC, z. s. p. o.
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
#include "corba/Admin.hh"

#include "src/bin/corba/admin/bankinginvoicing_impl.hh"

#include "src/bin/corba/admin/common.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "src/deprecated/libfred/banking/bank_manager.hh"

#include "src/bin/corba/file_manager_client.hh"
#include "src/bin/corba/connection_releaser.hh"

bool
ccReg_BankingInvoicing_i::pairPaymentRegistrarId(
        CORBA::ULongLong,
        CORBA::ULongLong)
{
    return false;
}

bool ccReg_BankingInvoicing_i::pairPaymentRegistrarHandle(
        CORBA::ULongLong,
        const char*)
{
    return false;
}

bool ccReg_BankingInvoicing_i::setPaymentType(
        CORBA::ULongLong,
        CORBA::Short)
{
    return false;
}

