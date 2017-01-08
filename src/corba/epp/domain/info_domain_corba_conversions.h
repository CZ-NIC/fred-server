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

#ifndef INFO_DOMAIN_CORBA_CONVERSIONS_H_368D6EEFB187456AA743F124CF1612EB
#define INFO_DOMAIN_CORBA_CONVERSIONS_H_368D6EEFB187456AA743F124CF1612EB

#include "util/corba_conversion.h"

#include "src/corba/EPP.hh"

#include "src/epp/domain/info_domain_localized.h"

namespace Corba {

    void wrap_Epp_Domain_InfoDomainLocalizedOutputData(
        const Epp::Domain::InfoDomainLocalizedOutputData& _src,
        ccReg::Domain& _dst
    );

}

#endif
