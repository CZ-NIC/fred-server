/*
 *  Copyright (C) 2008  CZ.NIC, z.s.p.o.
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

#ifndef _COMMONCLIENT_H_
#define _COMMONCLIENT_H_

#include <iostream>
#include <vector>

#include "corba/nameservice.h"

#define ID_NAME                 "id"
#define ID_NAME_DESC            "filter records with specific id nubmer"
#define NAME_NAME               "name-name"
#define NAME_NAME_DESC          "filter records with specific name"
#define HANDLE_NAME             "handle"
#define HANDLE_NAME_DESC        "filter records with specific handle"
#define FQDN_NAME               "fqdn"
#define FQDN_NAME_DESC          "filter records with specific fqdn"
#define IP_NAME                 "ip"
#define IP_NAME_DESC            "show only records with specific ip address"

#define ORGANIZATION_NAME       "organization"
#define ORGANIZATION_NAME_DESC  "show only records with specific organization name"
#define CITY_NAME               "city"
#define CITY_NAME_DESC          "show only records with specific city"
#define EMAIL_NAME              "email"
#define EMAIL_NAME_DESC         "show only records with specific email address"
#define NOTIFY_EMAIL_NAME       "notify-email"
#define NOTIFY_EMAIL_NAME_DESC  "show only records with specific notify email address"
#define VAT_NAME                "vat"
#define VAT_NAME_DESC           "show only records with ..."
#define SSN_NAME                "ssn"
#define SSN_NAME_DESC           "show only records with ..."

#define CONTACT_ID_NAME         "contact-id"
#define CONTACT_ID_NAME_DESC    "filter records with specific contact id number"
#define CONTACT_HANDLE_NAME     "contact-handle"
#define CONTACT_HANDLE_NAME_DESC "filter records with specific contact handle"
#define CONTACT_NAME_NAME       "contact-name"
#define CONTACT_NAME_NAME_DESC  "filter records with specific contact name"

#define DOMAIN_ID_NAME          "domain-id"
#define DOMAIN_ID_NAME_DESC     "show only records with specific domain id number"
#define DOMAIN_FQDN_NAME        "domain-fqdn"
#define DOMAIN_FQDN_NAME_DESC   "show only records with specific domain fqdn"
#define DOMAIN_HANDLE_NAME      DOMAIN_FQDN_NAME
#define DOMAIN_HANDLE_NAME_DESC DOMAIN_FQDN_NAME_DESC

#define KEYSET_ID_NAME          "keyset-id"
#define KEYSET_ID_NAME_DESC     "show only records with specific keyset id number"
#define KEYSET_HANDLE_NAME      "keyset-handle"
#define KEYSET_HANDLE_NAME_DESC "show only records with specific keyset handle"

#define NSSET_ID_NAME           "nsset-id"
#define NSSET_ID_NAME_DESC      "show only records with specific nsset id number"
#define NSSET_HANDLE_NAME       "nsset-handle"
#define NSSET_HANDLE_NAME_DESC  "show only records with specific nsset handle"

#define REGISTRAR_ID_NAME           "registrar-id"
#define REGISTRAR_ID_NAME_DESC      "show only records with specific registrar id number"
#define REGISTRAR_HANDLE_NAME       "registrar-handle"
#define REGISTRAR_HANDLE_NAME_DESC  "show only records with specific registrar handle"
#define REGISTRAR_NAME_NAME         "registrar-name"
#define REGISTRAR_NAME_NAME_DESC    "show only records with specific registrar name"

#define REGISTRANT_ID_NAME          "registrant-id"
#define REGISTRANT_ID_NAME_DESC     "show only records with specific registrant id number"
#define REGISTRANT_HANDLE_NAME      "registrant-handle"
#define REGISTRANT_HANDLE_NAME_DESC "show only records with specific registrant handle"
#define REGISTRANT_NAME_NAME        "registrant-name"
#define REGISTRANT_NAME_NAME_DESC   "show only recorrds with specific registrant name"

#define ADMIN_ID_NAME               "admin-id"
#define ADMIN_ID_NAME_DESC          "show only records with specific admin contact id number"
#define ADMIN_HANDLE_NAME           "admin-handle"
#define ADMIN_HANDLE_NAME_DESC      "show only records with specific admin contact handle"
#define ADMIN_NAME_NAME             "admin-name"
#define ADMIN_NAME_NAME_DESC        "show only records with specific admin contact name"

#define LIMIT_NAME                  "limit"
#define LIMIT_NAME_DESC             "set output limit"
#define FULL_LIST_NAME              "full-list"
#define FULL_LIST_NAME_DESC         "when do list show all available information"

#define ZONE_ID_NAME                "zone-id"
#define ZONE_ID_NAME_DESC           "show only records with specific zone id number"
#define ZONE_HANDLE_NAME            "zone-handle"
#define ZONE_HANDLE_NAME_DESC       "bla bla"

#define AUTH_PW_NAME                "auth-pw"
#define AUTH_PW_NAME_DESC           "show only records with specific authorization password"

#define AUTH_INFO_PW_NAME       "auth-info-pw"

#define ADMIN_NAME             "admin"
#define ADMIN_ADD_NAME         "admin-add"
#define ADMIN_REM_NAME         "admin-rem"
#define ADMIN_REM_TEMP_NAME    "admin-rem-temp"

#define ANY_KEYSET_NAME         "any-keyset"
#define ANY_KEYSET_NAME_DESC    "show only records with assigned keyset"
#define ANY_NSSET_NAME          "any-nsset"
#define ANY_NSSET_NAME_DESC     "show only records with assigned nsset"

#define DEBUG_NAME              "debug"
#define LANGUAGE_NAME           "lang"
#define DBNAME_NAME             "name"
#define DBUSER_NAME             "user"
#define DBPASS_NAME             "password"
#define DBHOST_NAME             "host"
#define DBPORT_NAME             "port"
#define NSHOST_NAME             "nshost"
#define NSPORT_NAME             "nsport"
#define NSSERVICE_NAME          "nameservice"
#define LOG_LEVEL_NAME          "log-level"
#define LOG_LOCAL_NAME          "log-local"
#define DOCGEN_PATH_NAME        "docgen-path"
#define DOCGEN_TEMPLATE_PATH_NAME "docgen-template-path"
#define FILECLIENT_PATH_NAME    "fileclient-path"
#define RESTRICTED_HANDLES_NAME "restricted-handles"

#define ADD_OPT(name, desc)                 (name, desc)
#define ADD_OPT_TYPE(name, desc, type)      (name, boost::program_options::value<type>(), desc)
#define ADD_OPT_DEF(name, desc, type, def)  (name, boost::program_options::value<type>()->default_value(def), desc)

#define add_opt(name)                       ADD_OPT(name, name##_DESC)
#define add_opt_type(name, type)            ADD_OPT_TYPE(name, name##_DESC, type)
#define add_opt_def(name, type, def)        ADD_OPT_DEF(name, name##_DESC, type, def)

extern const char *corbaOpts[][2];
extern const char g_prog_name[];
extern const char g_version[];

class CorbaClient {
    CORBA::ORB_var orb;
    std::auto_ptr<NameService> ns;
public:
    CorbaClient(int argc, char **argv, const std::string &nshost)
    {
        orb = CORBA::ORB_init(argc, argv, "", corbaOpts);
        ns.reset(new NameService(orb, nshost));
    }
    ~CorbaClient()
    {
        orb->destroy();
    }
    NameService *getNS()
    {
        return ns.get();
    }
};

std::vector<std::string> separateSpaces(const char *string);

void print_version();
void print_moo();

#endif // _COMMONCLIENT_H_
