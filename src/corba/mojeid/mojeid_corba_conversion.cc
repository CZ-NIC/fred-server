/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  implementation for MojeID CORBA conversion
 */

#include "mojeid_corba_conversion.h"

namespace CorbaConversion
{



    void Unwrapper_Registry_MojeID_Date_var_into_boost_date::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in.operator->() == NULL)
        {
            throw PointerIsNULL();
        }

        nct_out = boost::gregorian::from_simple_string(ct_in->value.in());

        if(nct_out.is_special())
        {
            throw ArgumentIsSpecial();
        }
    }

    void Wrapper_boost_date_into_Registry_MojeID_Date_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.is_special())
        {
            throw ArgumentIsSpecial();
        }

        Registry::MojeID::Date_var res = new Registry::MojeID::Date;
        res->value = wrap_into<CORBA::String_var>(boost::gregorian::to_iso_extended_string(nct_in));
        ct_out = res._retn();

    }


}
