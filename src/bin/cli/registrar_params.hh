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
 *  @registrar_params.h
 *  header of registrar client implementation
 */

#ifndef REGISTRAR_PARAMS_HH_6EAEB728E88F4AB2A62C010EE41CF630
#define REGISTRAR_PARAMS_HH_6EAEB728E88F4AB2A62C010EE41CF630

#include "src/util/types/optional.hh"
#include "src/util/decimal/decimal.hh"

#include <boost/asio.hpp>
#include <vector>

/**
 * \class ZoneAddArgs
 * \brief admin client zone_add params
 */
struct ZoneAddArgs
{
    std::string zone_fqdn;
    unsigned short ex_period_min;
    unsigned short ex_period_max;
    unsigned long ttl;
    std::string hostmaster;
    unsigned long update_retr;
    unsigned long refresh;
    unsigned long expiry;
    unsigned long minimum;
    std::string ns_fqdn;

    ZoneAddArgs()
    {}//ctor
    ZoneAddArgs( const std::string& _zone_fqdn
            , const unsigned short _ex_period_min
            , const unsigned short _ex_period_max
            , const unsigned long _ttl
            , const std::string& _hostmaster
            , const unsigned long _update_retr
            , const unsigned long _refresh
            , const unsigned long _expiry
            , const unsigned long _minimum
            , const std::string& _ns_fqdn
            )
    : zone_fqdn(_zone_fqdn)
    , ex_period_min(_ex_period_min)
    , ex_period_max(_ex_period_max)
    , ttl(_ttl)
    , hostmaster(_hostmaster)
    , update_retr(_update_retr)
    , refresh(_refresh)
    , expiry(_expiry)
    , minimum(_minimum)
    , ns_fqdn(_ns_fqdn)
    {}//init ctor
};//struct ZoneAddArgs

/**
 * \class RegistrarAddArgs
 * \brief admin client registrar_add params
 */
struct RegistrarAddArgs
{
    std::string handle;
    std::string country;
    optional_string ico;
    optional_string dic;
    optional_string varsymb;
    optional_string reg_name;
    optional_string organization;
    optional_string street1;
    optional_string street2;
    optional_string street3;
    optional_string city;
    optional_string stateorprovince;
    optional_string postalcode;
    optional_string telephone;
    optional_string fax;
    optional_string email;
    optional_string url;
    bool system;
    bool no_vat;

    RegistrarAddArgs()
    :system(false)
    , no_vat(false)
    {}//ctor

    RegistrarAddArgs(
    const std::string& _handle
    , const std::string& _country
    , const optional_string& _ico
    , const optional_string& _dic
    , const optional_string& _varsymb
    , const optional_string& _reg_name
    , const optional_string& _organization
    , const optional_string& _street1
    , const optional_string& _street2
    , const optional_string& _street3
    , const optional_string& _city
    , const optional_string& _stateorprovince
    , const optional_string& _postalcode
    , const optional_string& _telephone
    , const optional_string& _fax
    , const optional_string& _email
    , const optional_string& _url
    , bool _system
    , bool _no_vat
            )
    : handle(_handle)
    , country(_country)
    , ico(_ico)
    , dic(_dic)
    , varsymb(_varsymb)
    , reg_name(_reg_name)
    , organization(_organization)
    , street1(_street1)
    , street2(_street2)
    , street3(_street3)
    , city(_city)
    , stateorprovince(_stateorprovince)
    , postalcode(_postalcode)
    , telephone(_telephone)
    , fax(_fax)
    , email(_email)
    , url(_url)
    , system(_system)
    , no_vat(_no_vat)
    {}//init ctor
};//struct RegistrarAddArgs

/**
 * \class RegistrarAddZoneArgs
 * \brief admin client registrar_add_zone params
 */
struct RegistrarAddZoneArgs
{
    std::string zone_fqdn;
    std::string handle;
    optional_string from_date;
    optional_string to_date;

    RegistrarAddZoneArgs()
    {}//ctor
    RegistrarAddZoneArgs(
            const std::string& _zone_fqdn
            , const std::string& _handle
            , const optional_string& _from_date
            , const optional_string& _to_date
            )
    : zone_fqdn(_zone_fqdn)
    , handle(_handle)
    , from_date(_from_date)
    , to_date(_to_date)
    {}//init ctor
};//struct RegistrarAddZoneArgs

/**
 * \class RegistrarCreateCertificationArgs
 * \brief admin client registrar_create_certification params
 */
struct RegistrarCreateCertificationArgs
{
    std::string certification_evaluation;
    std::string certification_evaluation_mime_type;
    long certification_score;
    std::string handle;
    optional_string from_date;
    optional_string to_date;

    RegistrarCreateCertificationArgs()
    : certification_score(0)
    {}//ctor
    RegistrarCreateCertificationArgs(
            const std::string& _certification_evaluation
            , const std::string& _certification_evaluation_mime_type
            , const long _certification_score
            , const std::string& _handle
            , const optional_string& _from_date
            , const optional_string& _to_date
            )
    : certification_evaluation(_certification_evaluation)
    , certification_evaluation_mime_type(_certification_evaluation_mime_type)
    , certification_score(_certification_score)
    , handle(_handle)
    , from_date(_from_date)
    , to_date(_to_date)
    {}//init ctor
};//struct RegistrarCreateCertificationArgs

/**
 * \class RegistrarCreateGroupArgs
 * \brief admin client registrar_create_group params
 */
struct RegistrarCreateGroupArgs
{
    std::string registrar_group;

    RegistrarCreateGroupArgs()
    {}//ctor
    RegistrarCreateGroupArgs(
            const std::string& _registrar_group
            )
    : registrar_group(_registrar_group)
    {}//init ctor
};//struct RegistrarCreateGroupArgs

/**
 * \class RegistrarIntoGroupArgs
 * \brief admin client registrar_into_group params
 */
struct RegistrarIntoGroupArgs
{
    std::string handle;
    optional_string from_date;
    optional_string to_date;
    std::string registrar_group;

    RegistrarIntoGroupArgs()
    {}//ctor
    RegistrarIntoGroupArgs(
            const std::string& _handle
            , const optional_string& _from_date
            , const optional_string& _to_date
            , const std::string _registrar_group
            )
    : handle(_handle)
    , from_date(_from_date)
    , to_date(_to_date)
    , registrar_group(_registrar_group)
    {}//init ctor
};//struct RegistrarIntoGroupArgs

/**
 * \class RegistrarListArgs
 * \brief admin client registrar_list params
 */
struct RegistrarListArgs
{
    optional_id id;
    optional_string handle;
    optional_string name_name;
    optional_string organization;
    optional_string city;
    optional_string email;
    optional_string country;

    RegistrarListArgs()
    {}//ctor
    RegistrarListArgs(
            const optional_id& _id
            , const optional_string& _handle
            , const optional_string& _name_name
            , const optional_string& _organization
            , const optional_string& _city
            , const optional_string& _email
            , const optional_string& _country
            )
    : id(_id)
    , handle(_handle)
    , name_name(_name_name)
    , organization(_organization)
    , city(_city)
    , email(_email)
    , country(_country)
    {}//init ctor
};//struct RegistrarListArgs

/**
 * \class ZoneNsAddArgs
 * \brief admin client zone_ns_add params
 */
struct ZoneNsAddArgs
{
    std::string zone_fqdn;
    std::string ns_fqdn;
    std::vector<boost::asio::ip::address> addrs;

    ZoneNsAddArgs()
    {}//ctor

    ZoneNsAddArgs(
            const std::string& _zone_fqdn
            , const std::string& _ns_fqdn
            , const std::vector<boost::asio::ip::address>& _addrs
            )
    : zone_fqdn(_zone_fqdn)
    , ns_fqdn(_ns_fqdn)
    , addrs(_addrs)
    {}//init ctor
};//struct ZoneNsAddArgs

/**
 * \class RegistrarAclAddArgs
 * \brief admin client registrar_acl_add params
 */
struct RegistrarAclAddArgs
{
    std::string handle;
    std::string certificate;
    std::string password;

    RegistrarAclAddArgs()
    {}//ctor

    RegistrarAclAddArgs(
            const std::string& _handle
            , const std::string& _certificate
            , const std::string& _password
            )
    : handle(_handle)
    , certificate(_certificate)
    , password(_password)
    {}//init ctor
};//struct RegistrarAclAddArgs

/**
 * \class PriceAddArgs
 * \brief admin client price_add params
 */
struct PriceAddArgs
{
    optional_string valid_from;
    optional_string valid_to;
    optional_string operation_price;
    optional_ulong period;
    optional_string zone_fqdn;
    optional_id zone_id;
    optional_string operation;
    bool enable_postpaid_operation;

    PriceAddArgs()
    : enable_postpaid_operation(false)
    {}//ctor
    PriceAddArgs(
            const optional_string& _valid_from
            , const optional_string& _valid_to
            , const optional_string& _operation_price
            , const optional_ulong& _period
            , const optional_string& _zone_fqdn
            , const optional_id& _zone_id
            , const optional_string& _operation
            , bool _enable_postpaid_operation
            )
    : valid_from(_valid_from)
    , valid_to(_valid_to)
    , operation_price(_operation_price)
    , period(_period)
    , zone_fqdn(_zone_fqdn)
    , zone_id(_zone_id)
    , operation(_operation)
    , enable_postpaid_operation(_enable_postpaid_operation)
    {}//init ctor
};//struct PriceAddArgs

#endif
