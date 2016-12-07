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
 */

#ifndef INFO_KEYSET_H_6F5C2120747B47CB9758B4E30FA6FB73
#define INFO_KEYSET_H_6F5C2120747B47CB9758B4E30FA6FB73

#include "src/epp/keyset/impl/keyset_info_data.h"
#include "src/fredlib/opcontext.h"

namespace Epp {

/**
 * @throws NonexistentHandle
 */
KeysetInfoData keyset_info(Fred::OperationContext &_ctx,
                           const std::string &_keyset_handle,
                           unsigned long long _registrar_id);

}//namespace Epp

#endif
