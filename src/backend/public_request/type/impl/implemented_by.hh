/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef IMPLEMENTED_BY_HH_CEE815BC12D247A297FBBE6E96897259
#define IMPLEMENTED_BY_HH_CEE815BC12D247A297FBBE6E96897259

#include "libfred/public_request/public_request_type_iface.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {
namespace Impl {

template <class T>
struct ImplementedBy
{
    template <const char* name>
    class Named final : public LibFred::PublicRequestTypeIface
    {
    public:
        typedef T Type;
        Named()
            : implementation_()
        {
        }
        std::string get_public_request_type() const override
        {
            return name;
        }

    private:
        PublicRequestTypes get_public_request_types_to_cancel_on_create() const override
        {
            return implementation_.template get_public_request_types_to_cancel_on_create<Named>();
        }
        PublicRequestTypes get_public_request_types_to_cancel_on_update(
                LibFred::PublicRequest::Status::Enum _old_status,
                LibFred::PublicRequest::Status::Enum _new_status) const override
        {
            return implementation_.template get_public_request_types_to_cancel_on_update<Named>(
                    _old_status,
                    _new_status);
        }
        LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(
                LibFred::PublicRequest::Status::Enum _status) const override
        {
            return implementation_.template get_on_status_action<Named>(_status);
        }
        const Type implementation_;
    };
};

} // namespace Fred::Backend::PublicRequest::Type::Impl
} // namespace Fred::Backend::PublicRequest::Type
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
