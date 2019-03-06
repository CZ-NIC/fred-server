/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef KEYSETCLIENT_HH_B6793A8414C34B8E8BED9CB3B92C898D
#define KEYSETCLIENT_HH_B6793A8414C34B8E8BED9CB3B92C898D

#include <boost/program_options.hpp>
#include <iostream>

#include "src/bin/corba/admin/admin_impl.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/libfred/registry.hh"
#include "src/bin/cli/baseclient.hh"

#include "src/bin/cli/keyset_params.hh"

namespace Admin {

class KeysetClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;
    std::string nameservice_context;
    bool keyset_list;
    KeysetListArgs m_list_args;

    static const struct options m_opts[];
public:
    KeysetClient()
    : keyset_list(false)
    { }
    KeysetClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const std::string& _nameservice_context,
            bool _keyset_list,
            const KeysetListArgs &_list_args)
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , keyset_list(_keyset_list)
    , m_list_args(_list_args)
    {
        Database::Connection conn = Database::Manager::acquire();
        m_db.reset(new DB(conn));

    }

    static const struct options *getOpts();
    static int getOptsCount();

    void runMethod();
    void list();

}; // class KeysetClient

} // namespace Admin;

#endif
