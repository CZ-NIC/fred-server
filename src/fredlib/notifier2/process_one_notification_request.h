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

#include "src/fredlib/notifier2/gather_email_data/notification_request.h"

#include <map>
#include <set>
#include <string>
#include <boost/shared_ptr.hpp>

namespace Notification {

struct FailedToSendMail {
    const notification_request failed_request_data;
    const std::string failed_recipient;
    const std::set<std::string> skipped_recipients;
    const std::string template_name;
    const std::map<std::string, std::string> template_parameters;

    FailedToSendMail(
        const notification_request& _failed_request_data,
        const std::string& _failed_recipient,
        const std::set<std::string>& _skipped_recipients,
        const std::string& _template_name,
        const std::map<std::string, std::string>& _template_parameters
    ) :
        failed_request_data(_failed_request_data),
        failed_recipient(_failed_recipient),
        skipped_recipients(_skipped_recipients),
        template_name(_template_name),
        template_parameters(_template_parameters)
    { }
};

struct FailedToLockRequest { };

/**
 * Process one notification enqueued in table notification_queue.
 * Respects do-not-send-empty-update-notification rule from ticket #6547 - skips notification of update where no change in data is found.
 * @returns true if any notification was processed (because of do-not-send-empty-update-notification rule it does not guarantee any e-mail was actually sent)
 * @throws FailedToSendMail
 * @throws FailedToLockRequest in case other transaction is holding lock
 */
bool process_one_notification_request(Fred::OperationContext& _ctx, boost::shared_ptr<Fred::Mailer::Manager> _mailer);

}
#endif
