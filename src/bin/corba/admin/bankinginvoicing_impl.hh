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
#ifndef BANKINGINVOICING_IMPL_HH_03F685E625E04510AC2B1C6B1B6B5E20
#define BANKINGINVOICING_IMPL_HH_03F685E625E04510AC2B1C6B1B6B5E20

#include "corba/Admin.hh"

#include "src/bin/corba/nameservice.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"

#include <string>

class ccReg_BankingInvoicing_i:
    public POA_ccReg::BankingInvoicing,
    public PortableServer::RefCountServantBase {
private:
    NameService *ns_;

    std::string m_connection_string;

public:
    ccReg_BankingInvoicing_i(NameService *_ns) : ns_(_ns)
    { }
    ~ccReg_BankingInvoicing_i()
    { }

    bool pairPaymentRegistrarId(
            CORBA::ULongLong paymentId,
            CORBA::ULongLong registrarId);

    bool pairPaymentRegistrarHandle(
            CORBA::ULongLong paymentId,
            const char *registrarHandle);

    bool setPaymentType(
            CORBA::ULongLong payment_id,
            CORBA::Short payment_type);

};

#endif
