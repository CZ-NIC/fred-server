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

#ifndef DOMAIN_INFO_CORBA_CONVERSIONS_H_548B1620C07440B0ADA4C99C8BF99CE5
#define DOMAIN_INFO_CORBA_CONVERSIONS_H_548B1620C07440B0ADA4C99C8BF99CE5

#include "util/corba_conversion.h"

#include "src/corba/EPP.hh"

#include "src/epp/domain/info_domain_localized.h"

namespace Corba {

    void wrap_Epp_Domain_DomainInfoLocalizedOutputData(
        const Epp::Domain::DomainInfoLocalizedOutputData& _src,
        ccReg::Domain& _dst
    );

}

#endif
