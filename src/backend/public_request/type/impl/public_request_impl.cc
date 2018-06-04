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

#include "src/backend/public_request/type/personalinfo/public_request_personalinfo.hh"
#include "src/backend/public_request/type/impl/public_request_impl.hh"
#include "src/libfred/object/get_id_of_registered.hh"
#include "src/backend/public_request/type/personalinfo/public_request_personalinfo.hh"
#include "src/backend/public_request/type/block_unblock/public_request_blockunblock.hh"
#include "src/backend/public_request/type/authinfo/public_request_authinfo.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {

unsigned long long get_id_of_registered_object(
        LibFred::OperationContext& ctx,
        PublicRequestImpl::ObjectType::Enum object_type,
        const std::string& handle)
{
    switch (object_type)
    {
        case PublicRequestImpl::ObjectType::contact:
            return LibFred::get_id_of_registered<LibFred::Object_Type::contact>(ctx, handle);
        case PublicRequestImpl::ObjectType::nsset:
            return LibFred::get_id_of_registered<LibFred::Object_Type::nsset>(ctx, handle);
        case PublicRequestImpl::ObjectType::domain:
            return LibFred::get_id_of_registered<LibFred::Object_Type::domain>(ctx, handle);
        case PublicRequestImpl::ObjectType::keyset:
            return LibFred::get_id_of_registered<LibFred::Object_Type::keyset>(ctx, handle);
    }
    throw std::logic_error("unexpected PublicRequestImpl::ObjectType::Enum value");
}

unsigned long long get_id_of_contact(LibFred::OperationContext& ctx, const std::string& contact_handle)
{
    return get_id_of_registered_object(ctx, PublicRequestImpl::ObjectType::contact, contact_handle);
}

std::map<std::string, unsigned char> get_public_request_type_to_post_type_dictionary()
{
    std::map<std::string, unsigned char> dictionary;
    if (dictionary.insert(std::make_pair(get_auth_info_post_iface().get_public_request_type(), 1)).second &&
        dictionary.insert(std::make_pair(get_block_transfer_post_iface().get_public_request_type(), 2)).second &&
        dictionary.insert(std::make_pair(get_unblock_transfer_post_iface().get_public_request_type(), 3)).second &&
        dictionary.insert(std::make_pair(get_block_changes_post_iface().get_public_request_type(), 4)).second &&
        dictionary.insert(std::make_pair(get_unblock_changes_post_iface().get_public_request_type(), 5)).second &&
        dictionary.insert(std::make_pair(get_personal_info_post_iface().get_public_request_type(), 6)).second)
    {
        return dictionary;
    }
    throw std::logic_error("duplicate public request type");
}

short public_request_type_to_post_type(const std::string& public_request_type)
{
    typedef std::map<std::string, unsigned char> Dictionary;
    static const Dictionary dictionary = get_public_request_type_to_post_type_dictionary();
    const Dictionary::const_iterator result_ptr = dictionary.find(public_request_type);
    const bool key_found = result_ptr != dictionary.end();
    if (key_found)
    {
        return result_ptr->second;
    }
    throw PublicRequestImpl::InvalidPublicRequestType();
}

std::string language_to_lang_code(PublicRequestImpl::Language::Enum lang)
{
    switch (lang)
    {
        case PublicRequestImpl::Language::cs:
            return "cs";
        case PublicRequestImpl::Language::en:
            return "en";
    }
    throw std::invalid_argument("language code not found");
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
