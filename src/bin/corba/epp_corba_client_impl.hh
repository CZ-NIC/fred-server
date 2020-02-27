/*
 * Copyright (C) 2011-2020  CZ.NIC, z. s. p. o.
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
#ifndef EPP_CORBA_CLIENT_IMPL_HH_E0FFD7FBE9284C4593DCD9E28A896CCF
#define EPP_CORBA_CLIENT_IMPL_HH_E0FFD7FBE9284C4593DCD9E28A896CCF

#include "corba/EPP.hh"

#include "src/util/corba_wrapper_decl.hh"
#include "src/deprecated/libfred/epp_corba_client.hh"

#include <boost/thread/thread.hpp>

class EppCorbaClientImpl : public EppCorbaClient
{
public:
    EppCorbaClientImpl();

    void callDestroyAllRegistrarSessions(Database::ID reg_id) const;
private:
    ccReg::EPP_var epp_ref;
    mutable boost::mutex ref_mutex;
};

#endif
