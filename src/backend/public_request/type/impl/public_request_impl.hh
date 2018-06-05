/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef PUBLIC_REQUEST_HH_CEE815BC12D247A297FBBE6E96897259
#define PUBLIC_REQUEST_HH_CEE815BC12D247A297FBBE6E96897259

#include "src/libfred/public_request/public_request_type_iface.hh"
#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/util/random.hh"
#include "src/libfred/opcontext.hh"
#include "src/backend/public_request/public_request.hh"
#include "src/util/log/context.hh"

#include <boost/format.hpp>

#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {

template <class T>
struct ImplementedBy
{
    template <const char* name>
    class Named : public LibFred::PublicRequestTypeIface
    {
    public:
        typedef T Type;
        Named()
            : implementation_()
        {
        }
        std::string get_public_request_type() const
        {
            return name;
        }

    private:
        PublicRequestTypes get_public_request_types_to_cancel_on_create() const
        {
            return implementation_.template get_public_request_types_to_cancel_on_create<Named>();
        }
        PublicRequestTypes get_public_request_types_to_cancel_on_update(
                LibFred::PublicRequest::Status::Enum _old_status,
                LibFred::PublicRequest::Status::Enum _new_status) const
        {
            return implementation_.template get_public_request_types_to_cancel_on_update<Named>(
                    _old_status,
                    _new_status);
        }
        LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(
                LibFred::PublicRequest::Status::Enum _status) const
        {
            return implementation_.template get_on_status_action<Named>(_status);
        }
        const Type implementation_;
    };
};

} // namespace Fred::Backend::PublicRequest::Type

unsigned long long get_id_of_registered_object(
        LibFred::OperationContext& ctx,
        PublicRequestImpl::ObjectType::Enum object_type,
        const std::string& handle);

unsigned long long get_id_of_contact(LibFred::OperationContext& ctx, const std::string& contact_handle);

std::map<std::string, unsigned char> get_public_request_type_to_post_type_dictionary();

short public_request_type_to_post_type(const std::string& public_request_type);

std::string language_to_lang_code(PublicRequestImpl::Language::Enum lang);

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
