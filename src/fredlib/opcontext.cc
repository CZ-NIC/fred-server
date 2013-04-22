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

#include "fredlib/opcontext.h"

#include <string>

#include "fredlib/db_settings.h"
#include "util/log/log.h"

namespace Fred
{
    OperationContextTransaction::OperationContextTransaction()
    : conn_(Database::StandaloneManager(new Database::StandaloneConnectionFactory("host=/data/fred/fred/scripts/root/nofred/pg_sockets port=22345 dbname=fred user=fred connect_timeout=2")).acquire())
    , log_(LOGGER(PACKAGE))
    {
        this->get_conn().exec("START TRANSACTION ISOLATION LEVEL READ COMMITTED");
    }

    OperationContextTransaction::~OperationContextTransaction()
    {
        if (transaction_in_progress())
        {
            try
            {
                conn_->exec("ROLLBACK TRANSACTION");
            }
            catch(...)
            {
                try
                {
                    log_.error("OperationContextTransaction::~OperationContextTransaction: rollback failed");
                }
                catch(...){}
            }
            try
            {
                conn_.release();
            }
            catch(...)
            {
                try
                {
                    log_.error("OperationContextTransaction::~OperationContextTransaction: connection release failed");
                }
                catch(...){}
            }
        }
    }

    Database::StandaloneConnection& OperationContextTransaction::get_conn()
    {
        if (transaction_in_progress()) {
            return *conn_.get();
        }
        throw std::runtime_error("no transaction in progress");
    }

    Logging::Log& OperationContextTransaction::get_log()
    {
        return log_;
    }

    void OperationContextTransaction::commit_transaction()
    {
        this->get_conn().exec("COMMIT TRANSACTION");
        conn_.release();
    }

    bool OperationContextTransaction::transaction_in_progress()const
    {
        return conn_.get() != NULL;
    }
}
