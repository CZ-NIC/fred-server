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

#ifndef TYPEDEF_H_fd8fae73c3e44523ae3e21c3805a288d
#define TYPEDEF_H_fd8fae73c3e44523ae3e21c3805a288d


#include <string>
#include <set>

namespace Fred
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
