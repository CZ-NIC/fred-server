/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/admin/contact/merge_contact.hh"
#include "src/backend/admin/contact/merge_contact_logger.hh"


namespace Admin {

std::string get_system_registrar(LibFred::OperationContext &_ctx)
{
    Database::Result system_registrar_result = _ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system is True");
    if (system_registrar_result.size() == 0) {
        throw std::runtime_error("no system registrar found");
    }
    return static_cast<std::string>(system_registrar_result[0][0]);
}



MergeContact::MergeContact(
        const std::string &_src_contact_handle,
        const std::string &_dst_contact_handle)
    : src_contact_handle_(_src_contact_handle),
      dst_contact_handle_(_dst_contact_handle)
{
}



MergeContact::MergeContact(
        const std::string &_src_contact_handle,
        const std::string &_dst_contact_handle,
        const Optional<std::string> &_registrar)
    : src_contact_handle_(_src_contact_handle),
      dst_contact_handle_(_dst_contact_handle),
      registrar_(_registrar)
{
}



LibFred::MergeContactOutput MergeContact::exec(LibFred::Logger::LoggerClient &_logger_client)
{
    LibFred::OperationContextCreator ctx;

    if (!registrar_.isset()) {
        registrar_ = get_system_registrar(ctx);
    }

    LibFred::MergeContactOutput merge_data;
    unsigned long long req_id = 0;
    try {
        req_id = logger_merge_contact_create_request(_logger_client, src_contact_handle_, dst_contact_handle_);

        merge_data = LibFred::MergeContact(src_contact_handle_, dst_contact_handle_, registrar_.get_value())
                            .set_logd_request_id(req_id)
                            .exec(ctx);

        LibFred::create_poll_messages(merge_data, ctx);
        ctx.commit_transaction();

        logger_merge_contact_close_request_success(_logger_client, req_id, merge_data);
    }
    catch (...) {
        logger_merge_contact_close_request_fail(_logger_client, req_id);
        throw;
    }
    return merge_data;
}


LibFred::MergeContactOutput MergeContact::exec_dry_run()
{
    LibFred::OperationContextCreator ctx;

    if (!registrar_.isset()) {
        registrar_ = get_system_registrar(ctx);
    }

    return LibFred::MergeContact(src_contact_handle_, dst_contact_handle_, registrar_.get_value()).exec_dry_run(ctx);
}


}

