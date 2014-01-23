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

#include <string>

#include "src/fredlib/db_settings.h"
#include "util/log/log.h"

namespace Fred
{
    OperationContext::~OperationContext()
    {
        if (in_transaction_)
        {
            try
            {
                conn_->exec("ROLLBACK TRANSACTION");
            }
            catch(...)
            {
                try
                {
                    log_.error("OperationContext::~OperationContext: rollback failed");
                }
                catch(...){}
            }
        }
    }

    OperationContext::OperationContext()
    : conn_(Database::StandaloneManager(new Database::StandaloneConnectionFactory(Database::Manager::getConnectionString())).acquire())
    , in_transaction_(true)
    , log_(LOGGER(PACKAGE))
    {
        conn_->exec("START TRANSACTION  ISOLATION LEVEL READ COMMITTED");
    }

    Database::StandaloneConnection& OperationContext::get_conn()
    {
        return *conn_.get();
    }

    Logging::Log& OperationContext::get_log()
    {
        return log_;
    }

    void OperationContext::commit_transaction()
    {
        conn_->exec("COMMIT TRANSACTION");
        in_transaction_ = false;
    }
}
