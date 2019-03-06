/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#ifndef CONNECTION_RELEASER_HH_62CC5FC394E342D9B51CA77E1D213618
#define CONNECTION_RELEASER_HH_62CC5FC394E342D9B51CA77E1D213618

#include "src/deprecated/libfred/db_settings.hh"
#include "util/log/logger.hh"

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
        LOGGER.debug("CONNECTION RELEASER constructor");
    }

    ~ConnectionReleaser()
    {
        Database::Manager::release();
        LOGGER.debug("CONNECTION RELEASER destructor");
    }
};


#endif /*CONNECTION_RELEASER_H_*/

