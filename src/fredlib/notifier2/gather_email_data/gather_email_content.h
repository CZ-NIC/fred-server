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

#ifndef FREDLIB_NOTIFIER2_GATHER_EMAIL_CONTENT_4533447354
#define FREDLIB_NOTIFIER2_GATHER_EMAIL_CONTENT_4533447354

#include "src/fredlib/notifier2/gather_email_data/notification_request.h"
#include "src/fredlib/opcontext.h"

#include <map>
#include <string>

namespace Notification {

std::map<std::string, std::string> gather_email_content(
    Fred::OperationContext& _ctx,
    const notification_request& _request
);

}

#endif
