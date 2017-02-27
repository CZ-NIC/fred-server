/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef POLL_CORBA_CONVERSIONS_H_F60DDE04DB084511B791D6DF93B5643B
#define POLL_CORBA_CONVERSIONS_H_F60DDE04DB084511B791D6DF93B5643B

#include "src/epp/poll/poll_request_localized.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/corba/EPP.hh"

namespace Corba
{

struct PollMessage
{
    CORBA::Any_var content;
    ccReg::PollType type;
};

PollMessage wrap_transfer_event_into_any(const Epp::Poll::TransferEvent& _src);
template<typename Flag>
PollMessage wrap_transfer_event_data_into_any(const Epp::Poll::TransferEvent::Data<Flag>& _src);

PollMessage wrap_message_event_into_any(const Epp::Poll::MessageEvent& _src);
template<typename Flag>
PollMessage wrap_message_event_data_into_any(const Epp::Poll::MessageEvent::Data<Flag>& _src);

PollMessage wrap_low_credit_event_into_any(const Epp::Poll::LowCreditEvent& _src);

PollMessage wrap_low_request_fee_info_event_into_any(const Epp::Poll::RequestFeeInfoEvent& _src);

PollMessage wrap_tech_check_event_into_any(const Epp::Poll::TechCheckEvent& _src);

PollMessage wrap_update_info_event_into_any(const Epp::Poll::UpdateInfoEvent& _src);
template<typename Flag>
PollMessage wrap_update_info_event_data_into_any(const Epp::Poll::UpdateInfoEvent::Data<Flag>& _src);

} // namespace Corba

#endif
