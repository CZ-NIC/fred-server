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

#ifndef EPP_POST_CONTACT_UPDATE_HOOKS_H_30869764400878103641
#define EPP_POST_CONTACT_UPDATE_HOOKS_H_30869764400878103641

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {

    void post_contact_update_hooks(
        Fred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const Optional<unsigned long long>& _logd_requst_id,
        bool _epp_update_contact_enqueue_check
    );
}

#endif