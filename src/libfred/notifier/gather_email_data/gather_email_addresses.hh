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

#ifndef GATHER_EMAIL_ADDRESSES_HH_A7F1C3A8599647FEB69BC0931A7C3FC6
#define GATHER_EMAIL_ADDRESSES_HH_A7F1C3A8599647FEB69BC0931A7C3FC6

#include "src/libfred/opcontext.hh"
#include "src/libfred/notifier/event_on_object_enum.hh"

#include <set>
#include <string>

namespace Notification {

std::set<std::string> gather_email_addresses(
    LibFred::OperationContext& _ctx,
    const EventOnObject& _event_on_object,
    unsigned long long _history_id_post_change
);

}

#endif
