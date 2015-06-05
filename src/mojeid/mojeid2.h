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
 *  header of mojeid2 implementation
 */

#ifndef MOJEID2_H_06D795C17DD0FF3D98B375032F99493A//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID2_H_06D795C17DD0FF3D98B375032F99493A

#include "src/fredlib/opcontext.h"

#include <string>
#include <vector>

namespace Registry {
namespace MojeID {

typedef unsigned long long ContactId;

typedef std::vector< std::string > HandleList;

class MojeID2Impl
{
public:
    MojeID2Impl(const std::string &_server_name);
    ~MojeID2Impl();

    const std::string& get_server_name()const;

    static const ContactId contact_handles_start = 0;
    static const ContactId contact_handles_end_reached = 0;
    HandleList& get_unregistrable_contact_handles(
        Fred::OperationContext &_ctx,
        ::size_t _chunk_size,
        ContactId &_start_from,
        HandleList &_result)const;
private:
    const std::string server_name_;
};//class MojeID2Impl

}//namespace Registry::MojeID
}//namespace Registry

#endif // MOJEID2_H_06D795C17DD0FF3D98B375032F99493A
