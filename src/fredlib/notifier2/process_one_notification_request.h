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

#ifndef FREDLIB_NOTIFIER2_SEND_NOTIFICATION_1453430151
#define FREDLIB_NOTIFIER2_SEND_NOTIFICATION_1453430151

#include "src/fredlib/mailer.h"
#include "src/fredlib/opcontext.h"

#include <boost/shared_ptr.hpp>

namespace Notification {

/**
 * Process one notification enqueued in table notification_queue.
 * Respects do-not-send-empty-update-notification rule from ticket #6547 - skips notification of update where no change in data is found.
 * @returns true if any notification was processed (because of do-not-send-empty-update-notification rule it does not guarantee any e-mail was actually sent)
 */
bool process_one_notification_request(Fred::OperationContext& _ctx, boost::shared_ptr<Fred::Mailer::Manager> _mailer);

}
#endif
