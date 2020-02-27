/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef POLL_CORBA_CONVERSIONS_HH_720E05E6CEB844E586A28DBD31072D7F
#define POLL_CORBA_CONVERSIONS_HH_720E05E6CEB844E586A28DBD31072D7F

#include "src/backend/epp/poll/poll_request.hh"
#include "corba/EPP.hh"

namespace LibFred
{
namespace Corba
{

struct PollMessage
{
    CORBA::Any_var content;
    ccReg::PollType type;
};

PollMessage wrap_into_poll_message(const ::Epp::Poll::PollRequestOutputData::Message& _src);

} // namespace LibFred::Corba
} // namespace LibFred

#endif
