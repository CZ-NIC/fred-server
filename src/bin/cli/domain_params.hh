/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @domain_params.h
 *  header of domain client implementation
 */

#ifndef DOMAIN_PARAMS_HH_F379BDA5DF2942259EE8FD97A91853F6
#define DOMAIN_PARAMS_HH_F379BDA5DF2942259EE8FD97A91853F6

#include "src/util/types/optional.hh"

/**
 * \class DomainListArgs
 * \brief admin client domain_list params
 */

struct DomainListArgs
{
    optional_string login_registrar;
    optional_id domain_id;
    optional_string fqdn;
    optional_string domain_handle;
    optional_id nsset_id;
    optional_string nsset_handle;
    bool any_nsset;
    optional_id keyset_id;
    optional_string keyset_handle;
    bool any_keyset;
    optional_id zone_id;
    optional_id registrant_id;
    optional_string registrant_handle;
    optional_string registrant_name;
    optional_id admin_id;
    optional_string admin_handle;
    optional_string admin_name;
    optional_id registrar_id;
    optional_string registrar_handle;
    optional_string registrar_name;
    optional_string crdate;
    optional_string deldate;
    optional_string update;
    optional_string transdate;
    bool full_list;
    optional_string out_zone_date;
    optional_string exp_date;
    optional_string cancel_date;
    optional_ulonglong limit;

    DomainListArgs()
    : any_nsset(false)
    , any_keyset(false)
    , full_list(false)
    {}//ctor
    DomainListArgs(
            const optional_string& _login_registrar
            , const optional_id& _domain_id
            , const optional_string& _fqdn
            , const optional_string& _domain_handle
            , const optional_id& _nsset_id
            , const optional_string& _nsset_handle
            , bool _any_nsset
            , const optional_id& _keyset_id
            , const optional_string& _keyset_handle
            , bool _any_keyset
            , const optional_id& _zone_id
            , const optional_id& _registrant_id
            , const optional_string& _registrant_handle
            , const optional_string& _registrant_name
            , const optional_id& _admin_id
            , const optional_string& _admin_handle
            , const optional_string& _admin_name
            , const optional_id& _registrar_id
            , const optional_string& _registrar_handle
            , const optional_string& _registrar_name
            , const optional_string& _crdate
            , const optional_string& _deldate
            , const optional_string& _update
            , const optional_string& _transdate
            , bool _full_list
            , const optional_string& _out_zone_date
            , const optional_string& _exp_date
            , const optional_string& _cancel_date
            , const optional_ulonglong& _limit
            )
    : login_registrar(_login_registrar)
    , domain_id(_domain_id)
    , fqdn(_fqdn)
    , domain_handle(_domain_handle)
    , nsset_id(_nsset_id)
    , nsset_handle(_nsset_handle)
    , any_nsset(_any_nsset)
    , keyset_id(_keyset_id)
    , keyset_handle(_keyset_handle)
    , any_keyset(_any_keyset)
    , zone_id(_zone_id)
    , registrant_id(_registrant_id)
    , registrant_handle(_registrant_handle)
    , registrant_name(_registrant_name)
    , admin_id(_admin_id)
    , admin_handle(_admin_handle)
    , admin_name(_admin_name)
    , registrar_id(_registrar_id)
    , registrar_handle(_registrar_handle)
    , registrar_name(_registrar_name)
    , crdate(_crdate)
    , deldate(_deldate)
    , update(_update)
    , transdate(_transdate)
    , full_list(_full_list)
    , out_zone_date(_out_zone_date)
    , exp_date(_exp_date)
    , cancel_date(_cancel_date)
    , limit(_limit)
    {}//init ctor

};//struct DomainListArgs


/**
 * \class CreateExpiredDomainArgs
 * \brief admin client create_expired_domain params
 */
struct CreateExpiredDomainArgs
{
    std::string fqdn;
    std::string registrant;
    std::string cltrid;
    bool delete_existing;

    CreateExpiredDomainArgs()
    : delete_existing(false)
    {}//ctor
    CreateExpiredDomainArgs(
             const std::string& _fqdn
             , const std::string& _registrant
             , const std::string& _cltrid
             , bool _delete_existing
            )
    : fqdn(_fqdn)
    , registrant(_registrant)
    , cltrid(_cltrid)
    , delete_existing(_delete_existing)
    {}//init ctor
};//struct CreateExpiredDomainArgs

#endif
