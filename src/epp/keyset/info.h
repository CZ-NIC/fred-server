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

#ifndef INFO_H_80E554AABD337834F50F713FEACE5923//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define INFO_H_80E554AABD337834F50F713FEACE5923

#include "src/epp/keyset/info_data.h"
#include "src/fredlib/opcontext.h"

namespace Epp {

/**
 * @throws NonexistentHandle
 */
KeysetInfoData keyset_info(Fred::OperationContext &_ctx,
                           const std::string &_keyset_handle,
                           unsigned long long _registrar_id);

}//namespace Epp

#endif//INFO_H_80E554AABD337834F50F713FEACE5923
