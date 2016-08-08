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
 *  @file
 *  header of contact disclose items
 */

#ifndef DISCLOSE_H_6CCA57EF67A8AD5CE22CBA54280EAEE7//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define DISCLOSE_H_6CCA57EF67A8AD5CE22CBA54280EAEE7

namespace Epp {

struct ContactDisclose
{
    enum Enum
    {
        name,
        organization,
        address,
        telephone,
        fax,
        email,
        vat,
        ident,
        notify_email,
    };
};

}//namespace Epp

#endif//DISCLOSE_H_6CCA57EF67A8AD5CE22CBA54280EAEE7
