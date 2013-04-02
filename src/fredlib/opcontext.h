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
 *  @file opcontext.h
 *  operation context
 */

#ifndef OPCONTEXT_H_
#define OPCONTEXT_H_

#include <string>
#include <boost/utility.hpp>

#include "fredlib/db_settings.h"
#include "util/log/log.h"

namespace Fred
{
    class OperationContext
    : boost::noncopyable
    {
        const std::auto_ptr<Database::StandaloneConnection> conn_;
        bool in_transaction_;
        Logging::Log& log_;

    public:
        virtual ~OperationContext();
        OperationContext();
        Database::StandaloneConnection& get_conn();
        Logging::Log& get_log();
        void commit_transaction();
    };//class OperationContext

}//namespace Fred
#endif //OPCONTEXT_H_
