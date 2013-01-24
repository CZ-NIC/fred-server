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
    OperationContext::~OperationContext(){}

    OperationContext::OperationContext()
    : conn_(Database::Manager::acquire())
    , tx_(conn_)
    , log_(LOGGER(PACKAGE))
    {}

    Database::Connection& OperationContext::get_conn()
    {
        return conn_;
    }

    Logging::Log& OperationContext::get_log()
    {
        return log_;
    }

    void OperationContext::commit_transaction()
    {
        tx_.commit();
    }
}
