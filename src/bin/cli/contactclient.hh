/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONTACTCLIENT_HH_62D312F551484FCFA363F90C7DE0190F
#define CONTACTCLIENT_HH_62D312F551484FCFA363F90C7DE0190F

#include <boost/program_options.hpp>
#include <iostream>

#include "src/bin/corba/admin/admin_impl.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/bin/cli/baseclient.hh"
#include "libfred/db_settings.hh"

#include "src/bin/cli/contact_params.hh"

namespace Admin {

class ContactClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;
    std::string nameservice_context;
    bool contact_list;
    ContactListArgs params;

    static const struct options m_opts[];
public:
    ContactClient()
    : contact_list(false)
    { }
    ContactClient(
            const std::string &connstring
            , const std::string &nsAddr
            , const std::string& _nameservice_context
            , bool _contact_list
            , const ContactListArgs& _params
            )
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , contact_list(_contact_list)
    , params(_params)
    {
        Database::Connection conn = Database::Manager::acquire();
        m_db.reset(new DB(conn));
    }

    static const struct options *getOpts();
    static int getOptsCount();

    void runMethod();
    void list();
}; // class ContactClient

} // namespace Admin;


#endif
