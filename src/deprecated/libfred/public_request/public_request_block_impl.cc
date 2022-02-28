/*
 * Copyright (C) 2012-2022  CZ.NIC, z. s. p. o.
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

#include "src/deprecated/libfred/public_request/public_request_block_impl.hh"
#include "src/deprecated/libfred/public_request/public_request_impl.hh"

#include "libfred/public_request/public_request_on_status_action.hh"

#include <string>

namespace LibFred {
namespace PublicRequest {

namespace {

class BlockUnblockRequestImpl
        : public PublicRequestImpl
{
public:
    bool check() const override
    {
        return true;
    }

    std::string getTemplateName() const override
    {
        return std::string();
    }

    void fillTemplateParams(LibFred::Mailer::Parameters&) const override
    {
    }

    TID sendEmail() const override
    {
        return 0;
    }

    void save() override
    {
        if (this->getId() == 0)
        {
            throw std::runtime_error("insert new request disabled");
        }
        PublicRequestImpl::save();
        Database::Connection conn = Database::Manager::acquire();
        conn.exec_params(
            "UPDATE public_request SET on_status_action = $1::enum_on_status_action_type"
            " WHERE id = $2::bigint",
            Database::query_param_list
                (Conversion::Enums::to_db_handle(OnStatusAction::scheduled))
                (this->getId())
        );
    }
};

} // namespace LibFred::PublicRequest::{anonymous}

} // namespace LibFred::PublicRequest
} // namespace LibFred

using namespace LibFred::PublicRequest;

LibFred::PublicRequest::Factory& LibFred::PublicRequest::add_block_producers(Factory& factory)
{
    return factory
            .add_producer({PRT_BLOCK_TRANSFER_EMAIL_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_BLOCK_CHANGES_EMAIL_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_UNBLOCK_TRANSFER_EMAIL_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_UNBLOCK_CHANGES_EMAIL_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_BLOCK_TRANSFER_POST_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_BLOCK_CHANGES_POST_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_UNBLOCK_TRANSFER_POST_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_UNBLOCK_CHANGES_POST_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_BLOCK_TRANSFER_GOVERNMENT_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_BLOCK_CHANGES_GOVERNMENT_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_UNBLOCK_TRANSFER_GOVERNMENT_PIF, std::make_unique<BlockUnblockRequestImpl>()})
            .add_producer({PRT_UNBLOCK_CHANGES_GOVERNMENT_PIF, std::make_unique<BlockUnblockRequestImpl>()});
}
