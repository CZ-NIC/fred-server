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
 *  @keyset_params.h
 *  header of keyset list implementation
 */

#ifndef KEYSET_PARAMS_HH_5225AE0C627C4CFFB4DC9B7A3A2B623F
#define KEYSET_PARAMS_HH_5225AE0C627C4CFFB4DC9B7A3A2B623F

#include "src/util/types/optional.hh"

/**
 * \class KeysetListArgs
 * \brief admin client keyset_list params
 */
struct KeysetListArgs
{
    optional_string login_registrar;
    optional_id keyset_id;
    optional_string keyset_handle;
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
    optional_ulonglong limit;

    KeysetListArgs()
    : full_list(false)
    {}//ctor
    KeysetListArgs(
            const optional_string& _login_registrar
            , const optional_id& _keyset_id
            , const optional_string& _keyset_handle
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
            , const optional_ulonglong& _limit
            )
    : login_registrar(_login_registrar)
    , keyset_id(_keyset_id)
    , keyset_handle(_keyset_handle)
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
    , limit(_limit)
    {}//init ctor
};//struct KeysetListArgs

#endif
