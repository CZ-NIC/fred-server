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

#ifndef DOMAINCLIENT_HH_3EE1CCDFD40D4553A52C187C8D328B0C
#define DOMAINCLIENT_HH_3EE1CCDFD40D4553A52C187C8D328B0C

#include <boost/program_options.hpp>
#include <iostream>

#include "src/bin/corba/admin/admin_impl.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/bin/cli/baseclient.hh"
#include "src/util/types/optional.hh"
#include "src/bin/cli/domain_params.hh"
#include "src/deprecated/libfred/logger_client.hh"

namespace Admin {

class DomainClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;

    //commands
    bool domain_list_;

    //domain_list options
    optional_string login_registrar_;
    std::string nameservice_context_;
    optional_id domain_id_;
    optional_string fqdn_;
    optional_string domain_handle_;
    optional_id nsset_id_;
    optional_string nsset_handle_;
    bool any_nsset_;
    optional_id keyset_id_;
    optional_string keyset_handle_;
    bool any_keyset_;
    optional_id zone_id_;
    optional_id registrant_id_;
    optional_string registrant_handle_;
    optional_string registrant_name_;
    optional_id admin_id_;
    optional_string admin_handle_;
    optional_string admin_name_;
    optional_id registrar_id_;
    optional_string registrar_handle_;
    optional_string registrar_name_;
    optional_string crdate_;
    optional_string deldate_;
    optional_string update_;
    optional_string transdate_;
    bool full_list_;
    optional_string out_zone_date_;
    optional_string exp_date_;
    optional_string cancel_date_;
    optional_ulonglong limit_;

    static const struct options m_opts[];
public:
    DomainClient()
    : domain_list_(false)
    , any_nsset_(false)
    , any_keyset_(false)
    , full_list_(false)
    { }
    DomainClient(
            const std::string &connstring
            , const std::string &nsAddr
            , bool domain_list
            , const std::string& nameservice_context
            , const DomainListArgs& domain_list_args
            )
    : BaseClient(connstring, nsAddr)
    , domain_list_(domain_list)
    , login_registrar_(domain_list_args.login_registrar)
    , nameservice_context_(nameservice_context)
    , domain_id_(domain_list_args.domain_id)
    , fqdn_(domain_list_args.fqdn)
    , domain_handle_(domain_list_args.domain_handle)
    , nsset_id_(domain_list_args.nsset_id)
    , nsset_handle_(domain_list_args.nsset_handle)
    , any_nsset_(domain_list_args.any_nsset)
    , keyset_id_(domain_list_args.keyset_id)
    , keyset_handle_(domain_list_args.keyset_handle)
    , any_keyset_(domain_list_args.any_keyset)
    , zone_id_(domain_list_args.zone_id)
    , registrant_id_(domain_list_args.registrant_id)
    , registrant_handle_(domain_list_args.registrant_handle)
    , registrant_name_(domain_list_args.registrant_name)
    , admin_id_(domain_list_args.admin_id)
    , admin_handle_(domain_list_args.admin_handle)
    , admin_name_(domain_list_args.admin_name)
    , registrar_id_(domain_list_args.registrar_id)
    , registrar_handle_(domain_list_args.registrar_handle)
    , registrar_name_(domain_list_args.registrar_name)
    , crdate_(domain_list_args.crdate)
    , deldate_(domain_list_args.deldate)
    , update_(domain_list_args.update)
    , transdate_(domain_list_args.transdate)
    , full_list_(domain_list_args.full_list)
    , out_zone_date_(domain_list_args.out_zone_date)
    , exp_date_(domain_list_args.exp_date)
    , cancel_date_(domain_list_args.cancel_date)
    , limit_(domain_list_args.limit)
    {
        Database::Connection conn = Database::Manager::acquire();
        m_db.reset(new DB(conn));
    }

    static const struct options *getOpts();
    static int getOptsCount();

    void runMethod();
    void domain_list();
}; // class DomainClient

} // namespace Admin;

#endif
