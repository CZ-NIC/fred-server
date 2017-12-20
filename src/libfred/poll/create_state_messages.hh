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

#ifndef CREATE_STATE_MESSAGES_H_1B7FEABF62D24E6EB8C4FF50F8CF4F5B
#define CREATE_STATE_MESSAGES_H_1B7FEABF62D24E6EB8C4FF50F8CF4F5B

#include "src/libfred/opcontext.hh"
#include "src/libfred/poll/message_type_set.hh"

#include <boost/optional.hpp>

#include <set>
#include <string>

namespace LibFred {
namespace Poll {

class CreateStateMessages
{
public:
    CreateStateMessages(const std::set<LibFred::Poll::MessageType::Enum>& _except_list, const boost::optional<int>& _limit);
    unsigned long long exec(OperationContext& _ctx) const;
private:
    std::set<LibFred::Poll::MessageType::Enum> except_list_;
    boost::optional<int> limit_;
};

} // namespace LibFred::Poll
} // namespace LibFred

#endif
