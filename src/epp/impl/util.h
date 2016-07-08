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
 */

#ifndef EPP_UTIL_H_873559873922
#define EPP_UTIL_H_873559873922

#include "src/fredlib/opcontext.h"
#include "src/fredlib/contact/info_contact_data.h"
#include "util/optional_value.h"
#include "src/epp/contact/contact_info.h"
#include "src/admin/contact/verification/contact_states/enum.h"

#include <string>

// TODO fredlib_modification mozna by vsechno tohle casem melo byt soucasti fredlibu
namespace Epp {

    inline bool is_country_code_valid(Fred::OperationContext& ctx, const std::string& cc) {
        return
            ctx.get_conn().exec_params(
                "SELECT 1 FROM enum_country WHERE id = $1::text FOR SHARE ",
                Database::query_param_list(cc)
            ).size() > 0;
    }
}

#endif
