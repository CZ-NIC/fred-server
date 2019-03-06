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
#ifndef REGISTRARCLIENT_HH_A5F71ACF1ADA4B709D0A3E04C04AC7C7
#define REGISTRARCLIENT_HH_A5F71ACF1ADA4B709D0A3E04C04AC7C7

#include <boost/program_options.hpp>
#include <iostream>

#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/libfred/registry.hh"

#include "src/bin/corba/admin/admin_impl.hh"
#include "src/bin/cli/baseclient.hh"

#include "src/bin/cli/registrar_params.hh"

namespace Admin {

class RegistrarClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;
    static const struct options m_opts[];

    std::string nameservice_context;

    bool zone_add_;
    ZoneAddArgs zone_add_params_;
    bool registrar_add_;
    RegistrarAddArgs registrar_add_params_;
    bool registrar_add_zone_;
    RegistrarAddZoneArgs registrar_add_zone_params_;
    bool registrar_create_certification_;
    RegistrarCreateCertificationArgs registrar_create_certification_params_;
    bool registrar_create_group_;
    RegistrarCreateGroupArgs registrar_create_group_params_;
    bool registrar_into_group_;
    RegistrarIntoGroupArgs registrar_into_group_params_;
    bool registrar_list_;
    RegistrarListArgs registrar_list_params_;
    bool registrar_show_opts_;
    bool zone_ns_add_;
    ZoneNsAddArgs zone_ns_add_params_;
    bool registrar_acl_add_;
    RegistrarAclAddArgs registrar_acl_add_params_;
    bool price_add_;
    PriceAddArgs price_add_params_;

public:
    RegistrarClient()
    :  zone_add_(false)
    ,  registrar_add_(false)
    ,  registrar_add_zone_(false)
    ,  registrar_create_certification_(false)
    ,  registrar_create_group_(false)
    ,  registrar_into_group_(false)
    ,  registrar_list_(false)
    ,  registrar_show_opts_(false)
    ,  zone_ns_add_(false)
    ,  registrar_acl_add_(false)
    ,  price_add_(false)
    { }
    RegistrarClient(
            const std::string &connstring, const std::string &nsAddr
            , const std::string& _nameservice_context
            , bool _zone_add
            , const ZoneAddArgs& _zone_add_params
            , bool _registrar_add
            , const RegistrarAddArgs& _registrar_add_params
            , bool _registrar_add_zone
            , const RegistrarAddZoneArgs& _registrar_add_zone_params
            , bool _registrar_create_certification
            , const RegistrarCreateCertificationArgs& _registrar_create_certification_params
            , bool _registrar_create_group
            , const RegistrarCreateGroupArgs& _registrar_create_group_params
            , bool _registrar_into_group
            , const RegistrarIntoGroupArgs& _registrar_into_group_params
            , bool _registrar_list
            , const RegistrarListArgs& _registrar_list_params
            , bool _registrar_show_opts
            , bool _zone_ns_add
            , const ZoneNsAddArgs& _zone_ns_add_params
            , bool _registrar_acl_add
            , const RegistrarAclAddArgs& _registrar_acl_add_params
            , bool _price_add
            , const PriceAddArgs& _price_add_params
            )
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , zone_add_(_zone_add)
    , zone_add_params_(_zone_add_params)
    , registrar_add_(_registrar_add)
    , registrar_add_params_(_registrar_add_params)
    , registrar_add_zone_(_registrar_add_zone)
    , registrar_add_zone_params_(_registrar_add_zone_params)
    , registrar_create_certification_(_registrar_create_certification)
    , registrar_create_certification_params_(_registrar_create_certification_params)
    , registrar_create_group_(_registrar_create_group)
    , registrar_create_group_params_(_registrar_create_group_params)
    , registrar_into_group_(_registrar_into_group)
    , registrar_into_group_params_(_registrar_into_group_params)
    , registrar_list_(_registrar_list)
    , registrar_list_params_(_registrar_list_params)
    , registrar_show_opts_(_registrar_show_opts)
    , zone_ns_add_(_zone_ns_add)
    , zone_ns_add_params_(_zone_ns_add_params)
    , registrar_acl_add_(_registrar_acl_add)
    , registrar_acl_add_params_(_registrar_acl_add_params)
    , price_add_(_price_add)
    , price_add_params_(_price_add_params)
    {
        Database::Connection conn = Database::Manager::acquire();
        m_db.reset(new DB(conn));
    }
    ~RegistrarClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void list();
    void zone_add();
    void zone_ns_add();
    void registrar_add();
    void registrar_acl_add();
    void registrar_add_zone();
    void registrar_create_certification();
    void registrar_create_group();
    void registrar_into_group();
    void price_add();

    void zone_add_help();
    void zone_ns_add_help();
    void registrar_add_help();
    void registrar_acl_add_help();
    void registrar_add_zone_help();
    void registrar_create_certification_help();
    void registrar_create_group_help();
    void registrar_into_group_help();
    void price_add_help();
}; // class RegistrarClient

} // namespace Admin;

#endif
