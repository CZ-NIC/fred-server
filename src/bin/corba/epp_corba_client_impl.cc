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
#include "src/bin/corba/epp_corba_client_impl.hh"

EppCorbaClientImpl::EppCorbaClientImpl()
{
    boost::mutex::scoped_lock lock(ref_mutex);

    epp_ref = ccReg::EPP::_narrow(CorbaContainer::get_instance()->nsresolve("EPP"));
}

void EppCorbaClientImpl::callDestroyAllRegistrarSessions(Database::ID reg_id) const
{
    boost::mutex::scoped_lock lock(ref_mutex);

    epp_ref->destroyAllRegistrarSessions(reg_id);
}
