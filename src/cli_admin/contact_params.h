/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @contact_params.h
 *  header of contact client implementation
 */

#ifndef CONTACT_PARAMS_H_
#define CONTACT_PARAMS_H_

#include "util/types/optional.h"

/**
 * \class ContactListArgs
 * \brief admin client contact_list params
 */
struct ContactListArgs
{
    optional_string login_registrar;
    optional_id contact_id;
    optional_string contact_handle;
    optional_string contact_name;
    optional_string contact_organization;
    optional_string contact_city;
    optional_string contact_email;
    optional_string contact_notify_email;
    optional_string contact_vat;
    optional_string contact_ssn;
    optional_id registrar_id;
    optional_string registrar_handle;
    optional_string registrar_name;
    optional_string crdate;
    optional_string deldate;
    optional_string update;
    optional_string transdate;
    bool full_list;
    optional_ulonglong limit;

    ContactListArgs()
    : full_list(false)
    {}//ctor
    ContactListArgs(
            const optional_string& _login_registrar
            , const optional_id& _contact_id
            , const optional_string& _contact_handle
            , const optional_string& _contact_name
            , const optional_string& _contact_organization
            , const optional_string& _contact_city
            , const optional_string& _contact_email
            , const optional_string& _contact_notify_email
            , const optional_string& _contact_vat
            , const optional_string& _contact_ssn
            , const optional_id& _registrar_id
            , const optional_string& _registrar_handle
            , const optional_string& _registrar_name
            , const optional_string& _crdate
            , const optional_string& _deldate
            , const optional_string& _update
            , const optional_string& _transdate
            , bool _full_list
            , const optional_ulonglong& _limit
            )
    : login_registrar(_login_registrar)
    , contact_id(_contact_id)
    , contact_handle(_contact_handle)
    , contact_name(_contact_name)
    , contact_organization(_contact_organization)
    , contact_city(_contact_city)
    , contact_email(_contact_email)
    , contact_notify_email(_contact_notify_email)
    , contact_vat(_contact_vat)
    , contact_ssn(_contact_ssn)
    , registrar_id(_registrar_id)
    , registrar_handle(_registrar_handle)
    , registrar_name(_registrar_name)
    , crdate(_crdate)
    , deldate(_deldate)
    , update(_update)
    , transdate(_transdate)
    , full_list(_full_list)
    , limit(_limit)
    {}//init ctor

};//struct ContactListArgs

#endif // CONTACT_PARAMS_H_
