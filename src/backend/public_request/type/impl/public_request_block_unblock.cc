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
#include "src/backend/public_request/confirmed_by.hh"
#include "src/backend/public_request/type/public_request_authinfo.hh"
#include "src/backend/public_request/type/public_request_block_unblock.hh"
#include "src/backend/public_request/type/public_request_personal_info.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "libfred/public_request/public_request_status.hh"
#include "libfred/public_request/public_request_on_status_action.hh"
#include "src/backend/public_request/type/impl/implemented_by.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {
namespace Impl {
namespace {

extern const char block_changes_email_pif[] = "block_changes_email_pif";
extern const char block_changes_post_pif[] = "block_changes_post_pif";
extern const char block_changes_government_pif[] = "block_changes_government_pif";

extern const char block_transfer_email_pif[] = "block_transfer_email_pif";
extern const char block_transfer_post_pif[] = "block_transfer_post_pif";
extern const char block_transfer_government_pif[] = "block_transfer_government_pif";

extern const char unblock_changes_email_pif[] = "unblock_changes_email_pif";
extern const char unblock_changes_post_pif[] = "unblock_changes_post_pif";
extern const char unblock_changes_government_pif[] = "unblock_changes_government_pif";

extern const char unblock_transfer_email_pif[] = "unblock_transfer_email_pif";
extern const char unblock_transfer_post_pif[] = "unblock_transfer_post_pif";
extern const char unblock_transfer_government_pif[] = "unblock_transfer_government_pif";

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
    LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(LibFred::PublicRequest::Status::Enum _status [[gnu::unused]]) const
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
        typedef PublicRequest::Named<block_changes_government_pif> ByGovernment;
    };

    struct Transfer
    {
        typedef PublicRequest::Named<block_transfer_email_pif> ByEmail;
        typedef PublicRequest::Named<block_transfer_post_pif> ByPost;
        typedef PublicRequest::Named<block_transfer_government_pif> ByGovernment;
    };
};

struct Unblock
{
    typedef Fred::Backend::PublicRequest::Type::Impl::ImplementedBy<BlockUnblockImplementation> PublicRequest;
    struct Changes
    {
        typedef PublicRequest::Named<unblock_changes_email_pif> ByEmail;
        typedef PublicRequest::Named<unblock_changes_post_pif> ByPost;
        typedef PublicRequest::Named<unblock_changes_government_pif> ByGovernment;
    };

    struct Transfer
    {
        typedef PublicRequest::Named<unblock_transfer_email_pif> ByEmail;
        typedef PublicRequest::Named<unblock_transfer_post_pif> ByPost;
        typedef PublicRequest::Named<unblock_transfer_government_pif> ByGovernment;
    };
};

LibFred::PublicRequestTypeIface::PublicRequestTypes get_block_unblock_public_request_types_to_cancel_on_create()
{
    LibFred::PublicRequestTypeIface::PublicRequestTypes res;
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Changes::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Changes::ByPost));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Changes::ByGovernment));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Transfer::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Transfer::ByPost));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Transfer::ByGovernment));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Changes::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Changes::ByPost));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Changes::ByGovernment));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Transfer::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Transfer::ByPost));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Transfer::ByGovernment));
    return res;
}

} // namespace Fred::Backend::PublicRequest::Type::Impl::{anonymous}
} // namespace Fred::Backend::PublicRequest::Type::Impl

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<BlockChanges<ConfirmedBy::email>>()
{
    static const Impl::Block::Changes::ByEmail singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<BlockChanges<ConfirmedBy::letter>>()
{
    static const Impl::Block::Changes::ByPost singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<BlockChanges<ConfirmedBy::government>>()
{
    static const Impl::Block::Changes::ByGovernment singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<BlockTransfer<ConfirmedBy::email>>()
{
    static const Impl::Block::Transfer::ByEmail singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<BlockTransfer<ConfirmedBy::letter>>()
{
    static const Impl::Block::Transfer::ByPost singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<BlockTransfer<ConfirmedBy::government>>()
{
    static const Impl::Block::Transfer::ByGovernment singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<UnblockChanges<ConfirmedBy::email>>()
{
    static const Impl::Unblock::Changes::ByEmail singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<UnblockChanges<ConfirmedBy::letter>>()
{
    static const Impl::Unblock::Changes::ByPost singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<UnblockChanges<ConfirmedBy::government>>()
{
    static const Impl::Unblock::Changes::ByGovernment singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<UnblockTransfer<ConfirmedBy::email>>()
{
    static const Impl::Unblock::Transfer::ByEmail singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<UnblockTransfer<ConfirmedBy::letter>>()
{
    static const Impl::Unblock::Transfer::ByPost singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<UnblockTransfer<ConfirmedBy::government>>()
{
    static const Impl::Unblock::Transfer::ByGovernment singleton;
    return singleton;
}

template<template<ConfirmedBy> class T>
const LibFred::PublicRequestTypeIface& get_iface_of(ConfirmedBy confirmation_method)
{
    switch (confirmation_method)
    {
        case ConfirmedBy::email:
        {
            return get_iface_of<T<ConfirmedBy::email>>();
        }
        case ConfirmedBy::letter:
        {
            return get_iface_of<T<ConfirmedBy::letter>>();
        }
        case ConfirmedBy::government:
        {
            return get_iface_of<T<ConfirmedBy::government>>();
        }
    }
    throw std::runtime_error("unexpected confirmation method");
}

template const LibFred::PublicRequestTypeIface&
get_iface_of<BlockChanges>(ConfirmedBy confirmation_method);
template const LibFred::PublicRequestTypeIface&
get_iface_of<BlockTransfer>(ConfirmedBy confirmation_method);
template const LibFred::PublicRequestTypeIface&
get_iface_of<UnblockChanges>(ConfirmedBy confirmation_method);
template const LibFred::PublicRequestTypeIface&
get_iface_of<UnblockTransfer>(ConfirmedBy confirmation_method);

} // namespace Fred::Backend::PublicRequest::Type
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
