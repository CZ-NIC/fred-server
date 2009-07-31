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

#ifndef _REGISTRARCLIENT_H_
#define _REGISTRARCLIENT_H_

#define REGISTRAR_SHOW_OPTS_NAME            "registrar_show_opts"
#define REGISTRAR_SHOW_OPTS_NAME_DESC       "show all registrar command line options"
#define REGISTRAR_LIST_NAME                 "registrar_list"
#define REGISTRAR_LIST_NAME_DESC            "list all registrars (via filters)"
#define REGISTRAR_ZONE_ADD_NAME             "zone_add"
#define REGISTRAR_ZONE_ADD_NAME_DESC        "add new zone"
#define REGISTRAR_ZONE_NS_ADD_NAME          "zone_ns_add"
#define REGISTRAR_ZONE_NS_ADD_NAME_DESC     "add new nameserver to the zone"
#define REGISTRAR_REGISTRAR_ADD_NAME        "registrar_add"
#define REGISTRAR_REGISTRAR_ADD_NAME_DESC   "add new registrar (make a copy of REG-FRED_A)"
#define REGISTRAR_REGISTRAR_ACL_ADD_NAME        "registrar_acl_add"
#define REGISTRAR_REGISTRAR_ACL_ADD_NAME_DESC   "add new certificate for registrar"
#define REGISTRAR_REGISTRAR_ADD_ZONE_NAME       "registrar_add_zone"
#define REGISTRAR_REGISTRAR_ADD_ZONE_NAME_DESC  "add access right for registrar to zone"
#define REGISTRAR_PRICE_ADD_NAME            "price_add"
#define REGISTRAR_PRICE_ADD_NAME_DESC       "add price"

#define REGISTRAR_ZONE_ADD_HELP_NAME                "zone_add_help"
#define REGISTRAR_ZONE_ADD_HELP_NAME_DESC           "help for zone_add"
#define REGISTRAR_REGISTRAR_ADD_HELP_NAME           "registrar_add_help"
#define REGISTRAR_REGISTRAR_ADD_HELP_NAME_DESC      "help for registrar_add"
#define REGISTRAR_REGISTRAR_ADD_ZONE_HELP_NAME      "registrar_add_zone_help"
#define REGISTRAR_REGISTRAR_ADD_ZONE_HELP_NAME_DESC "help for registrar_add_zone"

#define REGISTRAR_ZONE_FQDN_NAME        "zone_fqdn"
#define REGISTRAR_ZONE_FQDN_NAME_DESC   "fqdn of new zone"

#define REGISTRAR_ZONE_ID_NAME          "zone_id"
#define REGISTRAR_ZONE_ID_NAME_DESC     "zone id"

#define REGISTRAR_ICO_NAME              "ico"
#define REGISTRAR_ICO_NAME_DESC         "organization identifier number"
#define REGISTRAR_DIC_NAME              "dic"
#define REGISTRAR_DIC_NAME_DESC         "tax identifier number"
#define REGISTRAR_VAR_SYMB_NAME         "varsymb"
#define REGISTRAR_VAR_SYMB_NAME_DESC    "registrar variable symbol"
#define REGISTRAR_NO_VAT_NAME           "no_vat"
#define REGISTRAR_NO_VAT_NAME_DESC      "no vat"
#define REGISTRAR_ADD_HANDLE_NAME           "handle"
#define REGISTRAR_ADD_HANDLE_NAME_DESC      "registrar handle"
#define REGISTRAR_ADD_NAME_NAME             "reg_name"
#define REGISTRAR_ADD_NAME_NAME_DESC        "registrar name"
#define REGISTRAR_ORGANIZATION_NAME     "organization"
#define REGISTRAR_ORGANIZATION_NAME_DESC "registrar organization"
#define REGISTRAR_STREET1_NAME          "street1"
#define REGISTRAR_STREET1_NAME_DESC     "registrar street #1"
#define REGISTRAR_STREET2_NAME          "street2"
#define REGISTRAR_STREET2_NAME_DESC     "registrar street #2"
#define REGISTRAR_STREET3_NAME          "street3"
#define REGISTRAR_STREET3_NAME_DESC     "registrar street #3"
#define REGISTRAR_CITY_NAME             "city"
#define REGISTRAR_CITY_NAME_DESC        "registrar city"
#define REGISTRAR_STATEORPROVINCE_NAME  "stateorprovince"
#define REGISTRAR_STATEORPROVINCE_NAME_DESC "registrar state or province"
#define REGISTRAR_POSTALCODE_NAME       "postalcode"
#define REGISTRAR_POSTALCODE_NAME_DESC  "registrar postal code"
#define REGISTRAR_COUNTRY_NAME          "country"
#define REGISTRAR_COUNTRY_NAME_DESC     "registrar two letter country code"
#define REGISTRAR_TELEPHONE_NAME        "telephone"
#define REGISTRAR_TELEPHONE_NAME_DESC   "registrar telephone"
#define REGISTRAR_FAX_NAME              "fax"
#define REGISTRAR_FAX_NAME_DESC         "registrar fax"
#define REGISTRAR_EMAIL_NAME            "email"
#define REGISTRAR_EMAIL_NAME_DESC       "registrar email"
#define REGISTRAR_URL_NAME              "url"
#define REGISTRAR_URL_NAME_DESC         "registrar url"
#define REGISTRAR_SYSTEM_NAME           "system"
#define REGISTRAR_SYSTEM_NAME_DESC      "if registrar is system"
#define REGISTRAR_CERT_NAME             "certificate"
#define REGISTRAR_CERT_NAME_DESC        "list of registrar MD5 certificate (splitted with ``;'')"
#define REGISTRAR_PASSWORD_NAME         "password"
#define REGISTRAR_PASSWORD_NAME_DESC    "list of registrar password"
#define REGISTRAR_FROM_DATE_NAME        "from_date"
#define REGISTRAR_FROM_DATE_NAME_DESC   "from date (default today)"
#define REGISTRAR_TO_DATE_NAME          "to_date"
#define REGISTRAR_TO_DATE_NAME_DESC     "to date (default not filled)"

#define REGISTRAR_EX_PERIOD_MIN_NAME       "ex_period_min"
#define REGISTRAR_EX_PERIOD_MIN_NAME_DESC  "ex_period_min"
#define REGISTRAR_EX_PERIOD_MAX_NAME       "ex_period_max"
#define REGISTRAR_EX_PERIOD_MAX_NAME_DESC  "ex_period_max"
#define REGISTRAR_TTL_NAME                      "ttl"
#define REGISTRAR_TTL_NAME_DESC                 "time to live"
#define REGISTRAR_HOSTMASTER_NAME               "hostmaster"
#define REGISTRAR_HOSTMASTER_NAME_DESC          "hostmaster"
#define REGISTRAR_UPDATE_RETR_NAME              "update_retr"
#define REGISTRAR_UPDATE_RETR_NAME_DESC         "update_retr"
#define REGISTRAR_REFRESH_NAME                  "refresh"
#define REGISTRAR_REFRESH_NAME_DESC             "refresh"
#define REGISTRAR_EXPIRY_NAME                   "expiry"
#define REGISTRAR_EXPIRY_NAME_DESC              "expiry"
#define REGISTRAR_MINIMUM_NAME                  "minimum"
#define REGISTRAR_MINIMUM_NAME_DESC             "minimum"
#define REGISTRAR_NS_FQDN_NAME                  "ns_fqdn"
#define REGISTRAR_NS_FQDN_NAME_DESC             "ns_fqdn"
#define REGISTRAR_ADDR_NAME                     "addr"
#define REGISTRAR_ADDR_NAME_DESC                "address"
#define REGISTRAR_CREATE_OPERATION_NAME         "create"
#define REGISTRAR_CREATE_OPERATION_NAME_DESC    "domain create operation"
#define REGISTRAR_RENEW_OPERATION_NAME          "renew"
#define REGISTRAR_RENEW_OPERATION_NAME_DESC     "domain renew operation"
#define REGISTRAR_VALID_FROM_NAME               "valid_from"
#define REGISTRAR_VALID_FROM_NAME_DESC          "price valid from datetime"
#define REGISTRAR_VALID_TO_NAME                 "valid_to"
#define REGISTRAR_VALID_TO_NAME_DESC            "price valid to datetime"
#define REGISTRAR_PRICE_NAME                    "operation_price"
#define REGISTRAR_PRICE_NAME_DESC               "operation price"
#define REGISTRAR_PERIOD_NAME                   "period"
#define REGISTRAR_PERIOD_NAME_DESC              "period"

#include <boost/program_options.hpp>
#include <iostream>

#include "old_utils/dbsql.h"
#include "register/register.h"

#include "corba/admin/admin_impl.h"
#include "baseclient.h"


namespace Admin {

class RegistrarClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    static const struct options m_opts[];
public:
    RegistrarClient()
    { }
    RegistrarClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        m_db.OpenDatabase(connstring.c_str());
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
    void price_add();

    void zone_add_help();
    void zone_ns_add_help();
    void registrar_add_help();
    void registrar_acl_add_help();
    void registrar_add_zone_help();
    void price_add_help();
}; // class RegistrarClient

} // namespace Admin;

#endif // _REGISTRARCLIENT_H_
