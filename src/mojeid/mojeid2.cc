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
 *  mojeid2 implementation
 */

#include "src/mojeid/mojeid2.h"
#include "src/mojeid/mojeid_checkers.h"
#include "util/random.h"
#include "util/log/context.h"

namespace Registry {
namespace MojeID {

namespace {

std::string create_ctx_name(const std::string &_name)
{
    return str(boost::format("%1%-<%2%>")% _name % Random::integer(0, 10000));
}

}//Registry::MojeID::{anonymous}

MojeID2Impl::MojeID2Impl(const std::string &_server_name)
:   server_name_(_server_name)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx("init");
}//MojeID2Impl::MojeID2Impl

MojeID2Impl::~MojeID2Impl()
{
}

const std::string& MojeID2Impl::get_server_name()const
{
    return server_name_;
}

HandleList& MojeID2Impl::get_unregistrable_contact_handles(
        Fred::OperationContext &_ctx,
        ::size_t _chunk_size,
        ContactId &_start_from,
        HandleList &_result)const
{
    Logging::Context ctx_server(create_ctx_name(get_server_name()));
    Logging::Context ctx("get-unregistrable-contact-handles");

    try {
        const Database::Result dbres = _ctx.get_conn().exec_params(
            "WITH static_data AS ("
                "SELECT eot.id AS type_id,"
                       "NOW()::DATE-(ep.val||'MONTH')::INTERVAL AS handle_protected_to "
                "FROM enum_object_type eot,enum_parameters ep "
                "WHERE eot.name='contact' AND "
                      "ep.name='handle_registration_protection_period') "
            "SELECT obr.id,obr.name "
            "FROM static_data sd "
            "JOIN object_registry obr ON obr.type=sd.type_id AND "
                                        "COALESCE(sd.handle_protected_to<obr.erdate,TRUE) "
            "WHERE LOWER(obr.name)~$1::TEXT AND "
                  "$3::BIGINT<obr.id "
            "ORDER BY obr.id "
            "LIMIT $2::BIGINT",
            Database::query_param_list
                (Fred::Contact::Verification::USERNAME_PATTERN.str())
                (_chunk_size + 1)
                (_start_from));
        const bool data_continues = _chunk_size < dbres.size();
        const ::size_t items = data_continues ? _chunk_size
                                              : dbres.size();
        _result.clear();
        _result.reserve(items);
        enum
        {
            CONTACT_ID_IDX = 0,
            CONTACT_HANDLE_IDX = 1,
        };
        for (::size_t idx = 0; idx < items; ++idx) {
            _result.push_back(static_cast< std::string >(dbres[idx][CONTACT_HANDLE_IDX]));
        }
        _start_from = data_continues ? static_cast< ContactId >(dbres[_chunk_size - 1][CONTACT_ID_IDX])
                                     : contact_handles_end_reached;
        return _result;
    }
    catch (const std::exception &e) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % e.what());
        throw;
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw;
    }
}

ContactId MojeID2Impl::create_contact_prepare(
        Fred::OperationContext &_ctx,
        const Fred::MojeID::CreateContact &_contact,
        const std::string &_trans_id,
        LogRequestId _log_request_id,
        std::string &_ident)
{
    return 0;
}

}//namespace Registry::MojeID
}//namespace Registry
