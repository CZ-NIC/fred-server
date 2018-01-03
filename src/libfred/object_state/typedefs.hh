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
 *  type aliases for object_state implementation
 */

#ifndef TYPEDEFS_HH_F871C7189C80499AB88D772CB0034FBE
#define TYPEDEFS_HH_F871C7189C80499AB88D772CB0034FBE


#include <string>
#include <set>

namespace LibFred
{
    typedef unsigned long long ObjectId;
    typedef unsigned long long ObjectStateId;
    typedef std::set< ObjectStateId > MultipleObjectStateId;
    typedef short int ObjectType;
    typedef std::set< std::string > StatusList;
    typedef ObjectId PublicRequestId;
    typedef ObjectId RegistrarId;
}

#endif
