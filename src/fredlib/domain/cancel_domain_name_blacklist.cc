/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @file cancel_domain_name_blacklist.cc
 *  cancel domain name blacklist
 */

#include "fredlib/domain/cancel_domain_name_blacklist.h"
#include "fredlib/domain/get_blocking_status_desc_list.h"
#include "fredlib/domain/get_object_state_id_map.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/object.h"

#include <boost/algorithm/string.hpp>

#ifndef __ASSERT_FUNCTION
#define __ASSERT_FUNCTION __PRETTY_FUNCTION__
#endif

#define MY_EXCEPTION_CLASS(DATA) CancelDomainNameBlacklist::Exception(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MY_ERROR_CLASS(DATA) CancelDomainNameBlacklist::Error(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

namespace Fred
{

    CancelDomainNameBlacklist::CancelDomainNameBlacklist(const std::string &_domain)
    :   domain_(_domain)
    {}

    void CancelDomainNameBlacklist::exec(OperationContext &_ctx)
    {
        Database::Result cancel_result = _ctx.get_conn().exec_params(
            "UPDATE domain_blacklist SET valid_to=CURRENT_TIMESTAMP "
            "WHERE regexp=$1::text AND "
                  "valid_from<=CURRENT_TIMESTAMP AND "
                  "(CURRENT_TIMESTAMP<valid_to OR valid_to IS NULL) "
            "RETURNING id",
            Database::query_param_list(domain_));
        if (0 < cancel_result.size()) {
            return;
        }
        std::string errmsg("|| domain:not found: ");
        errmsg += boost::replace_all_copy(domain_,"|", "[pipe]");//quote pipes;
        errmsg += " |";
        throw MY_EXCEPTION_CLASS(errmsg.c_str());
    }//CancelDomainNameBlacklist::exec

}//namespace Fred
