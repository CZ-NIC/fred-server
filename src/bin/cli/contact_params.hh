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
 *  @contact_params.h
 *  header of contact client implementation
 */

#ifndef CONTACT_PARAMS_HH_A3CD326D003549D8835664103DB3DC47
#define CONTACT_PARAMS_HH_A3CD326D003549D8835664103DB3DC47

#include "src/util/types/optional.hh"
#include "src/backend/admin/contact/verification/fill_check_queue.hh"

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


/**
 * \class ContactMergeDuplicateAutoArgs
 * \brief contact merge automatic procedure args
 */
struct ContactMergeDuplicateAutoArgs
{
    std::vector<std::string> registrar;
    std::vector<std::string> except_registrar;
    bool dry_run;
    std::vector<std::string> selection_filter_order;
    optional_ushort verbose;

    ContactMergeDuplicateAutoArgs()
    : dry_run(true)
    {
    }

    ContactMergeDuplicateAutoArgs(
            const optional_string &_registrar,
            const std::vector<std::string> &_except_registrar,
            const optional_ulonglong &_limit,
            bool _dry_run,
            const std::vector<std::string> &_selection_filter_order,
            const optional_ushort &_verbose)
        : registrar(_registrar),
          except_registrar(_except_registrar),
          dry_run(_dry_run),
          selection_filter_order(_selection_filter_order),
          verbose(_verbose)
    {
    }
};

/**
 * \class ContactMergeArgs
 * \brief manual two contact merge command args
 */
struct ContactMergeArgs
{
    std::string src;
    std::string dst;
    bool dry_run;
    optional_ushort verbose;

    ContactMergeArgs()
    {
    }

    ContactMergeArgs(
            const std::string &_src,
            const std::string &_dst,
            bool _dry_run,
            const optional_ushort &_verbose)
        : src(_src),
          dst(_dst),
          dry_run(_dry_run),
          verbose(_verbose)
    {
    }
};


/**
 * parameters for appropriate command options Handle#CLASS_NAME#Grp
 */

struct ContactVerificationFillQueueArgs {
    std::string             testsuite_handle;
    unsigned                max_queue_length;
    std::string             country_code;
    std::vector<std::string>   contact_states;
    std::vector<std::string>   contact_roles;

    ContactVerificationFillQueueArgs( )
        : max_queue_length(0)
    { }

    ContactVerificationFillQueueArgs(
        std::string             _testsuite_name,
        unsigned                _max_queue_length,
        std::string             _country_code,
        std::vector<std::string>   _contact_states,
        std::vector<std::string>   _contact_roles
    ) :
        testsuite_handle(_testsuite_name),
        max_queue_length(_max_queue_length),
        country_code(_country_code),
        contact_states(_contact_states),
        contact_roles(_contact_roles)
    { }
};


/**
 * parameters for appropriate command options Handle#CLASS_NAME#Grp
 */

struct ContactVerificationEnqueueCheckArgs {
    long long contact_id;
    std::string testsuite_handle;

    ContactVerificationEnqueueCheckArgs( )
        : contact_id(0)
    { }

    ContactVerificationEnqueueCheckArgs( long long _contact_id, const std::string& _testsuite_name )
        : contact_id(_contact_id), testsuite_handle(_testsuite_name)
    { }
};


/**
 * parameters for appropriate command options Handle#CLASS_NAME#Grp
 */

struct ContactVerificationStartEnqueuedChecksArgs {
    std::string cz_address_mvcr_xml_path;

    ContactVerificationStartEnqueuedChecksArgs()
    { }

    ContactVerificationStartEnqueuedChecksArgs( const std::string& _cz_address_mvcr_xml_path )
        : cz_address_mvcr_xml_path(_cz_address_mvcr_xml_path)
    { }
};


#endif
