/*
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

#ifndef CONNECTION_RELEASER_H_
#define CONNECTION_RELEASER_H_

#include "fredlib/db_settings.h"
#include "log/logger.h"

/*
 * Class for auto release connection from thread local storage to manager
 * (pool) at request end (idl method end). It's quite hack because of
 * omniorb thread pooling. This should be done in better way if have
 * better corba object architecture.
 */

class ConnectionReleaser
{
    public:
        ConnectionReleaser()
        {
            LOGGER(PACKAGE).debug("CONNECTION RELEASER constructor");
        }

        ~ConnectionReleaser()
        {
            Database::Manager::release();
            LOGGER(PACKAGE).debug("CONNECTION RELEASER destructor");
        }
};


#endif /*CONNECTION_RELEASER_H_*/

