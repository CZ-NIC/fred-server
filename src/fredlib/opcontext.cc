/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @file opcontext.cc
 *  operation context
 */

#include "src/fredlib/opcontext.h"

#include <stdexcept>

namespace Fred
{

namespace
{

const std::string& check_transaction_id(const std::string &_value)
{
    if (!_value.empty()) {
        return _value;
    }
    throw std::runtime_error("prepared transaction id mustn't be empty");
}

using namespace Database;

std::auto_ptr< StandaloneConnection > get_database_conn()
{
    StandaloneConnectionFactory *const factory =
        new StandaloneConnectionFactory(Manager::getConnectionString());
    StandaloneConnection *const conn_ptr = StandaloneManager(factory).acquire();
    return std::auto_ptr< StandaloneConnection >(conn_ptr);
}

void start_transaction(StandaloneConnection &_conn)
{
    _conn.exec("START TRANSACTION ISOLATION LEVEL READ COMMITTED");
}

}//Fred::{anonymous}

OperationContextDatabasePart::OperationContextDatabasePart()
:   transaction_id_(),
    conn_(get_database_conn())
{
    start_transaction(this->get_conn());
}

OperationContextDatabasePart::OperationContextDatabasePart(const std::string &_transaction_id)
:   transaction_id_(check_transaction_id(_transaction_id)),
    conn_(get_database_conn())
{
    start_transaction(this->get_conn());
}

Database::StandaloneConnection& OperationContextDatabasePart::get_conn()const
{
    Database::StandaloneConnection *const conn_ptr = conn_.get();
    if (conn_ptr != NULL) {
        return *conn_ptr;
    }
    throw std::runtime_error("database connection doesn't exist");
}

std::string OperationContextDatabasePart::get_transaction_id()const
{
    const bool process_two_phase_commit = !transaction_id_.empty();
    if (process_two_phase_commit) {
        return transaction_id_;
    }
    throw std::runtime_error("prepared transaction not in progress");
}

OperationContextLoggingPart::OperationContextLoggingPart()
:   log_(LOGGER(PACKAGE))
{
}

OperationContext::~OperationContext()
{
    Database::StandaloneConnection *const conn_ptr = conn_.get();
    if (conn_ptr == NULL) {
        return;
    }
    try {
        conn_ptr->exec("ROLLBACK");
    }
    catch(...) {
        try {
            this->get_log().error("OperationContext::~OperationContext: rollback failed");
        }
        catch(...) {
        }
    }
    try {
        conn_.reset();
    }
    catch(...) {
        try {
            this->get_log().error("OperationContext::~OperationContext: database connection destroying failed");
        }
        catch(...) {
        }
    }
}

void OperationContext::commit_transaction()
{
    Database::StandaloneConnection *const conn_ptr = conn_.get();
    if (conn_ptr == NULL) {
        throw std::runtime_error("no transaction in progress");
    }
    const bool process_two_phase_commit = !transaction_id_.empty();
    if (process_two_phase_commit) {
        conn_ptr->exec_params("PREPARE TRANSACTION $1::TEXT", Database::query_param_list(transaction_id_));
    }
    else {
        conn_ptr->exec("COMMIT");
    }
    conn_.reset();
}

void commit_transaction(const std::string &_transaction_id)
{
    check_transaction_id(_transaction_id);
    std::auto_ptr< Database::StandaloneConnection > conn_ptr = get_database_conn();
    conn_ptr->exec_params("COMMIT PREPARED $1::TEXT", Database::query_param_list(_transaction_id));
}

void rollback_transaction(const std::string &_transaction_id)
{
    check_transaction_id(_transaction_id);
    std::auto_ptr< Database::StandaloneConnection > conn_ptr = get_database_conn();
    conn_ptr->exec_params("ROLLBACK PREPARED $1::TEXT", Database::query_param_list(_transaction_id));
}

}//Fred
