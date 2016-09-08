/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 *  @file domain_info_corba_conversions.h
 *  <++>
 */

#ifndef SRC_CORBA_EPP_DOMAIN_DOMAIN_INFO_CORBA_CONVERSIONS_H
#define SRC_CORBA_EPP_DOMAIN_DOMAIN_INFO_CORBA_CONVERSIONS_H

#include "util/corba_conversion.h"

#include "src/corba/EPP.hh"

#include "src/epp/domain/domain_info.h"

namespace CorbaConversion {

    void wrap_LocalizedDomainInfoOutputData(
        const Epp::Domain::LocalizedDomainInfoOutputData& src,
        ccReg::Domain& dst
    );

}

#endif
