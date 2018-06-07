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

#include "src/backend/public_request/type/public_request_blockunblock.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/backend/public_request/type/impl/implemented_by.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {
namespace Impl {
namespace {

extern const char block_changes_email_pif[] = "block_changes_email_pif";
extern const char block_changes_post_pif[] = "block_changes_post_pif";

extern const char block_transfer_email_pif[] = "block_transfer_email_pif";
extern const char block_transfer_post_pif[] = "block_transfer_post_pif";

extern const char unblock_changes_email_pif[] = "unblock_changes_email_pif";
extern const char unblock_changes_post_pif[] = "unblock_changes_post_pif";

extern const char unblock_transfer_email_pif[] = "unblock_transfer_email_pif";
extern const char unblock_transfer_post_pif[] = "unblock_transfer_post_pif";

LibFred::PublicRequestTypeIface::PublicRequestTypes get_block_unblock_public_request_types_to_cancel_on_create();

struct BlockUnblockImplementation
{
    template <typename>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        return get_block_unblock_public_request_types_to_cancel_on_create();
    }
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum,
            LibFred::PublicRequest::Status::Enum) const
    {
        return LibFred::PublicRequestTypeIface::PublicRequestTypes();
    }
    template <typename T>
    LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(LibFred::PublicRequest::Status::Enum _status) const
    {
        return LibFred::PublicRequest::OnStatusAction::processed;
    }
};

struct Block
{
    typedef Fred::Backend::PublicRequest::Type::Impl::ImplementedBy<BlockUnblockImplementation> PublicRequest;
    struct Changes
    {
        typedef PublicRequest::Named<block_changes_email_pif> ByEmail;
        typedef PublicRequest::Named<block_changes_post_pif> ByPost;
    };

    struct Transfer
    {
        typedef PublicRequest::Named<block_transfer_email_pif> ByEmail;
        typedef PublicRequest::Named<block_transfer_post_pif> ByPost;
    };
};

struct Unblock
{
    typedef Fred::Backend::PublicRequest::Type::Impl::ImplementedBy<BlockUnblockImplementation> PublicRequest;
    struct Changes
    {
        typedef PublicRequest::Named<unblock_changes_email_pif> ByEmail;
        typedef PublicRequest::Named<unblock_changes_post_pif> ByPost;
    };

    struct Transfer
    {
        typedef PublicRequest::Named<unblock_transfer_email_pif> ByEmail;
        typedef PublicRequest::Named<unblock_transfer_post_pif> ByPost;
    };
};

LibFred::PublicRequestTypeIface::PublicRequestTypes get_block_unblock_public_request_types_to_cancel_on_create()
{
    LibFred::PublicRequestTypeIface::PublicRequestTypes res;
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Changes::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Changes::ByPost));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Transfer::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Transfer::ByPost));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Changes::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Changes::ByPost));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Transfer::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Transfer::ByPost));
    return res;
}

template <typename request>
const LibFred::PublicRequestTypeIface& get_public_request_type(PublicRequestImpl::ConfirmedBy::Enum confirmation_method)
{
    switch (confirmation_method)
    {
        case PublicRequestImpl::ConfirmedBy::email:
        {
            static const typename request::ByEmail public_request_type;
            return static_cast<const LibFred::PublicRequestTypeIface&>(public_request_type);
        }
        case PublicRequestImpl::ConfirmedBy::letter:
        {
            static const typename request::ByPost public_request_type;
            return static_cast<const LibFred::PublicRequestTypeIface&>(public_request_type);
        }
    }
    throw std::runtime_error("unexpected confirmation method");
}

} // namespace Fred::Backend::PublicRequest::Type::Impl::{anonymous}
} // namespace Fred::Backend::PublicRequest::Type::Impl

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<BlockChangesEmail>()
{
    static const Impl::Block::Changes::ByEmail singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<BlockChangesPost>()
{
    static const Impl::Block::Changes::ByPost singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<BlockTransferEmail>()
{
    static const Impl::Block::Transfer::ByEmail singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<BlockTransferPost>()
{
    static const Impl::Block::Transfer::ByPost singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<UnblockChangesEmail>()
{
    static const Impl::Unblock::Changes::ByEmail singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<UnblockChangesPost>()
{
    static const Impl::Unblock::Changes::ByPost singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<UnblockTransferEmail>()
{
    static const Impl::Unblock::Transfer::ByEmail singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<UnblockTransferPost>()
{
    static const Impl::Unblock::Transfer::ByPost singleton;
    return singleton;
}

const LibFred::PublicRequestTypeIface& get_block_transfer_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method)
{
    return Impl::get_public_request_type<Impl::Block::Transfer>(confirmation_method);
}

const LibFred::PublicRequestTypeIface& get_block_change_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method)
{
    return Impl::get_public_request_type<Impl::Block::Changes>(confirmation_method);
}

const LibFred::PublicRequestTypeIface& get_unblock_transfer_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method)
{
    return Impl::get_public_request_type<Impl::Unblock::Transfer>(confirmation_method);
}

const LibFred::PublicRequestTypeIface& get_unblock_changes_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method)
{
    return Impl::get_public_request_type<Impl::Unblock::Changes>(confirmation_method);
}

} // namespace Fred::Backend::PublicRequest::Type
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
