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

#ifndef _SIMPLE_H_
#define _SIMPLE_H_

#include <map>
#include <string>

typedef std::map<std::string, int> METHODS;
typedef std::map<std::string, int>::iterator METHODS_IT;

#define CLIENT_COMMON           0
#define CLIENT_DOMAIN           1
#define CLIENT_KEYSET           2
#define CLIENT_CONTACT          3
#define CLIENT_INVOICE          4
#define CLIENT_AUTHINFO         5
#define CLIENT_BANK             6
#define CLIENT_POLL             7
#define CLIENT_REGISTRAR        8
#define CLIENT_NOTIFY           9
#define CLIENT_OBJECT           10
#define CLIENT_INFOBUFF         11
#define CLIENT_NSSET            12
#define CLIENT_FILE             13
#define CLIENT_MAIL             14
#define CLIENT_PUBLICREQUEST    15
#define CLIENT_ENUMPARAM        16

#define TYPE_NOTYPE     0
#define TYPE_STRING     1
#define TYPE_INT        2
#define TYPE_UINT       3
#define TYPE_ULONGLONG  4
#define TYPE_BOOL       5
#define TYPE_DOUBLE     6

struct options {
    int         client;
    const char  *name;
    const char  *description;
    int         type;
    bool        callable;
    bool        visible;
};

#define addOpt(name)                        (name, name##_DESC)
#define addOptStr(name)                     (name, boost::program_options::value<std::string>(), name##_DESC)
#define addOptInt(name)                     (name, boost::program_options::value<int>(), name##_DESC)
#define addOptUInt(name)                    (name, boost::program_options::value<unsigned int>(), name##_DESC)
#define addOptULongLong(name)               (name, boost::program_options::value<unsigned long long>(), name##_DESC)
#define addOptTID(name)                     (name, boost::program_options::value<Register::TID>(), name##_DESC)
#define addOptBool(name)                    (name, boost::program_options::value<bool>(), name##_DESC)
#define addOptType(name, type)              (name, boost::program_options::value<type>(), name##_DESC)
#define addOptDID(name)                     (name, boost::program_options::value<unsigned int>(), name##_DESC)
#define addOptDouble(name)                  (name, boost::program_options::value<double>(), name##_DESC)

#define addOptStrDef(name, val)           (name, boost::program_options::value<std::string>()->default_value(val), name##_DESC)
#define addOptIntDef(name, val)           (name, boost::program_options::value<int>()->default_value(val), name##_DESC)
#define addOptUIntDef(name, val)          (name, boost::program_options::value<unsigned int>()->default_value(val), name##_DESC)
#define addOptULongLongDef(name, val)     (name, boost::program_options::value<unsigned long long>()->default_value(val), name##_DESC)
#define addOptTIDDef(name, val)           (name, boost::program_options::value<Register::TID>()->default_value(val), name##_DESC)
#define addOptBoolDef(name, val)          (name, boost::program_options::value<bool>()->default_value(val), name##_DESC)
#define addOptTypeDef(name, type, val)    (name, boost::program_options::value<type>()->default_value(val), name##_DESC)
#define addOptDoubleDef(name, val)           (name, boost::program_options::value<double>()->default_value(val), name##_DESC)

#define getOptUInt(option)          m_conf.get<unsigned int>(option)
#define getOptInt(option)           m_conf.get<int>(option)
#define getOptStr(option)           m_conf.get<std::string>(option)
#define getOptBool(option)          m_conf.get<bool>(option)
#define getOptType(option, type)    m_conf.get<type>(option)

#define checkGetOpt(operation, option)      if (m_conf.hasOpt(option)) operation

#define get_DID(filter, name, option)       checkGetOpt(filter->add##name().setValue(Database::ID(getOptUInt(option))), option)
#define get_UInt(filter, name, option)      checkGetOpt(filter->add##name().setValue(getOptUInt(option)), option)
#define get_Int(filter, name, option)       checkGetOpt(filter->add##name().setValue(getOptInt(option)), option)
#define get_Str(filter, name, option)       checkGetOpt(filter->add##name().setValue(getOptStr(option)), option)
#define get_Bool(filter, name, option)      checkGetOpt(filter->add##name().setValue(getOptStr(option)), option)

#define apply_DATETIME(filter, name, innerName)                             \
    if (m_conf.hasOpt(name)) {                                              \
        filter->add##innerName##Time().setValue(                            \
                *parseDateTime(m_conf.get<std::string>(name)));             \
    }

#define apply_DATE(filter, name, innerName)                                 \
    if (m_conf.hasOpt(name)) {                                              \
        filter->add##innerName##Date().setValue(                            \
                *parseDate(m_conf.get<std::string>(name)));                 \
    }

#define callHelp(conf, help_func)                                           \
    if (conf.hasOpt("help")) {                                              \
        help_func();                                                        \
        return;                                                             \
    }

#define HELP                    "help"
#define CLI_HELP_NAME           "help,h"
#define CLI_VERSION_NAME        "version,V"
#define CLI_VERSION_NAME_DESC   "print version and licence information"
#define CLI_CONF_NAME           "conf,c"
#define CLI_CONF_NAME_DESC      "configuration file"
#define CLI_LANGUAGE_NAME       "lang,l"
#define CLI_LANGUAGE_NAME_DESC  "communication language"
#define CLI_HELP_DATES_NAME     "help_dates"
#define CLI_HELP_DATES_NAME_DESC "help on date/time format"
#define CLI_MOO_NAME            "moo"
#define CLI_MOO_NAME_DESC       "moo"

#define DB_NAME_NAME        "database.name"
#define DB_NAME_NAME_DESC   "database name"
#define DB_USER_NAME        "database.user"
#define DB_USER_NAME_DESC   "database user name"
#define DB_PASS_NAME        "database.password"
#define DB_PASS_NAME_DESC   "database password"
#define DB_HOST_NAME        "database.host"
#define DB_HOST_NAME_DESC   "database hostname"
#define DB_PORT_NAME        "database.port"
#define DB_PORT_NAME_DESC   "database port number"

#define NS_HOST_NAME        "nameservice.host"
#define NS_HOST_NAME_DESC   "CORBA nameservice hostname"
#define NS_PORT_NAME        "nameservice.port"
#define NS_PORT_NAME_DESC   "CORBA nameservice port number"
#define NS_CONTEXT_NAME     "nameservice.context"
#define NS_CONTEXT_NAME_DESC "CORBA nameservice context name"

#define LOG_TYPE_NAME       "log.type"
#define LOG_TYPE_NAME_DESC  "log type"
#define LOG_LEVEL_NAME      "log.level"
#define LOG_LEVEL_NAME_DESC "log level"
#define LOG_FILE_NAME       "log.file"
#define LOG_FILE_NAME_DESC  "log file path (for log.type==1)"
#define LOG_SYSLOG_NAME     "log.syslog_facility"
#define LOG_SYSLOG_NAME_DESC "syslog facility (for log.type==2)"

#define REG_RESTRICTED_HANDLES_NAME         "registry.restricted_handles"
#define REG_RESTRICTED_HANDLES_NAME_DESC    "force using restricted handles for NSSETs, KEYSETs and CONTACTs"
#define REG_DOCGEN_PATH_NAME                "registry.docgen_path"
#define REG_DOCGEN_PATH_NAME_DESC           "PDF generator path"
#define REG_DOCGEN_TEMPLATE_PATH_NAME       "registry.docgen_template_path"
#define REG_DOCGEN_TEMPLATE_PATH_NAME_DESC  "PDF generator template path"
#define REG_FILECLIENT_PATH_NAME            "registry.fileclient_path"
#define REG_FILECLIENT_PATH_NAME_DESC       "file manager client path"

#define LOGIN_REGISTRAR_NAME                "login_registrar"
#define LOGIN_REGISTRAR_NAME_DESC           "login registrar handle"

#define _ADDOPT(name, type) \
    {CLIENT_COMMON, name, name##_DESC, type, false, false}

#define add_CRDATE              _ADDOPT(CRDATE_NAME, TYPE_STRING)
#define add_DELDATE             _ADDOPT(DELDATE_NAME, TYPE_STRING)
#define add_TRANSDATE           _ADDOPT(TRANSDATE_NAME, TYPE_STRING)
#define add_UPDATE              _ADDOPT(UPDATE_NAME, TYPE_STRING)
#define add_ID                  _ADDOPT(ID_NAME, TYPE_UINT)
#define add_NAME                _ADDOPT(NAME_NAME, TYPE_STRING)
#define add_HANDLE              _ADDOPT(HANDLE_NAME, TYPE_STRING)
#define add_FQDN                _ADDOPT(FQDN_NAME, TYPE_STRING)
#define add_REGISTRAR_ID        _ADDOPT(REGISTRAR_ID_NAME, TYPE_UINT)
#define add_REGISTRAR_HANDLE    _ADDOPT(REGISTRAR_HANDLE_NAME, TYPE_STRING)
#define add_REGISTRAR_NAME      _ADDOPT(REGISTRAR_NAME_NAME, TYPE_STRING)
#define add_REGISTRANT_ID       _ADDOPT(REGISTRANT_ID_NAME, TYPE_UINT)
#define add_REGISTRANT_HANDLE   _ADDOPT(REGISTRANT_HANDLE_NAME, TYPE_STRING)
#define add_REGISTRANT_NAME     _ADDOPT(REGISTRANT_NAME_NAME, TYPE_STRING)
#define add_ADMIN_ID            _ADDOPT(ADMIN_ID_NAME, TYPE_UINT)
#define add_ADMIN_HANDLE        _ADDOPT(ADMIN_HANDLE_NAME, TYPE_STRING)
#define add_ADMIN_NAME          _ADDOPT(ADMIN_NAME_NAME, TYPE_STRING)
#define add_LIMIT               _ADDOPT(LIMIT_NAME, TYPE_UINT)
#define add_FULL_LIST           _ADDOPT(FULL_LIST_NAME, TYPE_NOTYPE)
#define add_ZONE_ID             _ADDOPT(ZONE_ID_NAME, TYPE_UINT)
#define add_ZONE_FQDN           _ADDOPT(ZONE_FQDN_NAME, TYPE_STRING)
#define add_AUTH_PW             _ADDOPT(AUTH_PW_NAME, TYPE_STRING)
#define add_ADMIN               _ADDOPT(ADMIN_NAME, TYPE_STRING)
#define add_ADMIN_ADD           _ADDOPT(ADMIN_ADD_NAME, TYPE_STRING)
#define add_ADMIN_REM           _ADDOPT(ADMIN_REM_NAME, TYPE_STRING)
#define add_ADMIN_REM_TEMP      _ADDOPT(ADMIN_REM_TEMP_NAME, TYPE_STRING)
#define add_IP                  _ADDOPT(IP_NAME, TYPE_STRING)
#define add_NSSET_ID            _ADDOPT(NSSET_ID_NAME, TYPE_UINT)
#define add_NSSET_HANDLE        _ADDOPT(NSSET_HANDLE_NAME, TYPE_STRING)
#define add_ANY_NSSET           _ADDOPT(ANY_NSSET_NAME, TYPE_NOTYPE)
#define add_KEYSET_ID           _ADDOPT(KEYSET_ID_NAME, TYPE_UINT)
#define add_KEYSET_HANDLE       _ADDOPT(KEYSET_HANDLE_NAME, TYPE_STRING)
#define add_ANY_KEYSET          _ADDOPT(ANY_KEYSET_NAME, TYPE_NOTYPE)
#define add_ORGANIZATION        _ADDOPT(ORGANIZATION_NAME, TYPE_STRING)
#define add_CITY                _ADDOPT(CITY_NAME, TYPE_STRING)
#define add_EMAIL               _ADDOPT(EMAIL_NAME, TYPE_STRING)
#define add_NOTIFY_EMAIL        _ADDOPT(NOTIFY_EMAIL_NAME, TYPE_STRING)
#define add_VAT                 _ADDOPT(VAT_NAME, TYPE_STRING)
#define add_SSN                 _ADDOPT(SSN_NAME, TYPE_STRING)
#define add_COUNTRY             _ADDOPT(COUNTRY_NAME, TYPE_STRING)

// almost all
#define CRDATE_NAME             "crdate"
#define CRDATE_NAME_DESC        "create date (type ``./fred-admin --help_dates'' for further date&time information)"
#define apply_CRDATE(filter)    apply_DATETIME(filter, CRDATE_NAME, Create)

#define DELDATE_NAME            "deldate"
#define DELDATE_NAME_DESC       "delete date (type ``./fred-admin --help_dates'' for further date&time information)"
#define apply_DELDATE(filter)   apply_DATETIME(filter, DELDATE_NAME, Delete)

#define TRANSDATE_NAME          "transdate"
#define TRANSDATE_NAME_DESC     "transfer date (type ``./fred-admin --help_dates'' for further date&time information)"
#define apply_TRANSDATE(filter) apply_DATETIME(filter, TRANSDATE_NAME, Transfer)

#define UPDATE_NAME             "update"
#define UPDATE_NAME_DESC        "update date (type ``./fred-admin --help_dates'' for further date&time information)"
#define apply_UPDATE(filter)    apply_DATETIME(filter, UPDATE_NAME, Update)

// all
#define ID_NAME                 "id"
#define ID_NAME_DESC            "filter records with specific id nubmer"
#define apply_ID(filter)          get_DID(filter, Id, ID_NAME)

// contact, domain, file, invoice, keyset, nsset, registrar
#define NAME_NAME               "name_name"
#define NAME_NAME_DESC          "filter records with specific name"
#define apply_NAME(filter)        get_Str(filter, Name, NAME_NAME)

// contact, domain, keyset, mail, nsset, registrar
#define HANDLE_NAME             "handle"
#define HANDLE_NAME_DESC        "filter records with specific handle"
#define apply_HANDLE(filter)      get_Str(filter, Handle, HANDLE_NAME)

// domain, nsset, object
#define FQDN_NAME               "fqdn"
#define FQDN_NAME_DESC          "filter records with specific fqdn"
#define apply_FQDN(filter)      get_Str(filter, FQDN, FQDN_NAME)

// contact, domain, infobuffer, invoice, keyset, nsset, poll
#define REGISTRAR_ID_NAME           "registrar_id"
#define REGISTRAR_ID_NAME_DESC      "show only records with specific registrar id number"
#define apply_REGISTRAR_ID(filter)  get_DID(filter, Registrar().addId, REGISTRAR_ID_NAME)

// contact, domain, keyset, nsset, registrar
#define REGISTRAR_HANDLE_NAME       "registrar_handle"
#define REGISTRAR_HANDLE_NAME_DESC  "show only records with specific registrar handle"
#define apply_REGISTRAR_HANDLE(filter) get_Str(filter, Registrar().addHandle, REGISTRAR_HANDLE_NAME)

// contact,  domain, keyset, nsset
#define REGISTRAR_NAME_NAME         "registrar_name"
#define REGISTRAR_NAME_NAME_DESC    "show only records with specific registrar name"
#define apply_REGISTRAR_NAME(filter) get_Str(filter, Registrar().addName, REGISTRAR_NAME_NAME)

// domain, invoice
#define REGISTRANT_ID_NAME          "registrant_id"
#define REGISTRANT_ID_NAME_DESC     "show only records with specific registrant id number"
#define apply_REGISTRANT_ID(filter) get_DID(filter, Registrant().addId, REGISTRANT_ID_NAME)

// domain
#define REGISTRANT_HANDLE_NAME      "registrant_handle"
#define REGISTRANT_HANDLE_NAME_DESC "show only records with specific registrant handle"
#define apply_REGISTRANT_HANDLE(filter) get_Str(filter, Registrant().addHandle, REGISTRANT_HANDLE_NAME)

// domain
#define REGISTRANT_NAME_NAME        "registrant_name"
#define REGISTRANT_NAME_NAME_DESC   "show only recorrds with specific registrant name"
#define apply_REGISTRANT_NAME(filter) get_Str(filter, Registrant().addName, REGISTRANT_NAME_NAME)

// domain, keyset, nsset
#define ADMIN_ID_NAME               "admin_id"
#define ADMIN_ID_NAME_DESC          "show only records with specific admin contact id number"

// domain, keyset, nsset
#define ADMIN_HANDLE_NAME           "admin_handle"
#define ADMIN_HANDLE_NAME_DESC      "show only records with specific admin contact handle"

// domain, keyset, nsset
#define ADMIN_NAME_NAME             "admin_name"
#define ADMIN_NAME_NAME_DESC        "show only records with specific admin contact name"

// all
#define LIMIT_NAME                  "limit"
#define LIMIT_NAME_DESC             "set output limit"
#define apply_LIMIT(list)           list->setLimit(getOptUInt(LIMIT_NAME))

// domain, keyset, nsset, contact
#define FULL_LIST_NAME              "full_list"
#define FULL_LIST_NAME_DESC         "when do list show all available information"

// domain, invoice
#define ZONE_ID_NAME                "zone_id"
#define ZONE_ID_NAME_DESC           "show only records with specific zone id number"
#define apply_ZONE_ID(filter)       get_DID(filter, ZoneId, ZONE_ID_NAME)

#define ZONE_FQDN_NAME              "zone_fqdn"
#define ZONE_FQDN_NAME_DESC         "zone fully qualified domain name"
#define apply_ZONE_FQDN(filter)     get_Str(filter, ZoneFqdn, ZONE_FQDN_NAME)

// almost all
#define AUTH_PW_NAME                "auth_pw"
#define AUTH_PW_NAME_DESC           "show only records with specific authorization password"

// domain, keyset, nsset
#define ADMIN_NAME                  "admins"
#define ADMIN_NAME_DESC             "list of admins"

// domain, keyset
#define ADMIN_ADD_NAME              "admin_add"
#define ADMIN_ADD_NAME_DESC         "list of admins to add"

// domain, keyset
#define ADMIN_REM_NAME              "admin_rem"
#define ADMIN_REM_NAME_DESC         "list of admins to remove"

// domain
#define ADMIN_REM_TEMP_NAME         "admin_rem_temp"
#define ADMIN_REM_TEMP_NAME_DESC    "list of temporary admins to remove"

// nsset
#define IP_NAME                 "ip"
#define IP_NAME_DESC            "show only records with specific ip address"

// contact, registrar
#define ORGANIZATION_NAME       "organization"
#define ORGANIZATION_NAME_DESC  "show only records with specific organization name"
#define apply_ORGANIZATION(filter) get_Str(filter, Organization, ORGANIZATION_NAME)

// contact, registrar
#define CITY_NAME               "city"
#define CITY_NAME_DESC          "show only records with specific city"
#define apply_CITY(filter)      get_Str(filter, City, CITY_NAME)

// contact, registrar
#define EMAIL_NAME              "email"
#define EMAIL_NAME_DESC         "show only records with specific email address"
#define apply_EMAIL(filter)     get_Str(filter, Email, EMAIL_NAME)

// contcat
#define NOTIFY_EMAIL_NAME       "notify_email"
#define NOTIFY_EMAIL_NAME_DESC  "show only records with specific notify email address"
#define apply_NOTIFY_EMAIL(filter) get_Str(filter, NotifyEmail, NOTIFY_EMAIL_NAME)

// contact
#define VAT_NAME                "vat"
#define VAT_NAME_DESC           "show only records with ..."
#define apply_VAT(filter)       get_Str(filter, Vat, VAT_NAME)

// contact
#define SSN_NAME                "ssn"
#define SSN_NAME_DESC           "show only records with ..."
#define apply_SSN(filter)       get_Str(filter, Ssn, SSN_NAME)

// registrar
#define COUNTRY_NAME            "country"
#define COUNTRY_NAME_DESC       "show only records with specific country"
#define apply_COUNTRY(filter)   get_Str(filter, Country, COUNTRY_NAME)

// domain
#define KEYSET_ID_NAME          "keyset_id"
#define KEYSET_ID_NAME_DESC     "show only records with specific keyset id number"
#define apply_KEYSET_ID(filter) get_DID(filter, KeySet().addId, KEYSET_ID_NAME)

// domain
#define KEYSET_HANDLE_NAME      "keyset_handle"
#define KEYSET_HANDLE_NAME_DESC "show only records with specific keyset handle"
#define apply_KEYSET_HANDLE(filter) get_Str(filter, KeySet().addHandle, KEYSET_HANDLE_NAME)

// domain
#define NSSET_ID_NAME           "nsset_id"
#define NSSET_ID_NAME_DESC      "show only records with specific nsset id number"
#define apply_NSSET_ID(filter)  get_DID(filter, NSSet().addId, NSSET_ID_NAME)

// domain
#define NSSET_HANDLE_NAME       "nsset_handle"
#define NSSET_HANDLE_NAME_DESC  "show only records with specific nsset handle"
#define apply_NSSET_HANDLE(filter) get_Str(filter, NSSet().addHandle, NSSET_HANDLE_NAME)

// domain
#define ANY_KEYSET_NAME         "any_keyset"
#define ANY_KEYSET_NAME_DESC    "show only records with assigned keyset"
#define apply_ANY_KEYSET(filter) checkGetOpt(filter->addKeySet(), ANY_KEYSET_NAME)

// domain
#define ANY_NSSET_NAME          "any_nsset"
#define ANY_NSSET_NAME_DESC     "show only records with assigned nsset"
#define apply_ANY_NSSET(filter) checkGetOpt(filter->addNSSet(), ANY_NSSET_NAME)

#define OUTPUT_NAME             "output"
#define OUTPUT_NAME_DESC        "output file name"

#define CLIENT_LOGIN                                                        \
CorbaClient cc(0, NULL, m_nsAddr.c_str(),                                   \
        m_conf.get<std::string>(NS_CONTEXT_NAME));                          \
CORBA::Object_var o = cc.getNS()->resolve("EPP");                           \
ccReg::EPP_var epp;                                                         \
epp = ccReg::EPP::_narrow(o);                                               \
CORBA::Long clientId = 0;                                                   \
ccReg::Response_var r;                                                      \
std::string get_registrar_query;                                            \
if (m_conf.hasOpt(LOGIN_REGISTRAR_NAME)) {                                  \
    std::stringstream sss;                                                  \
    sss << "SELECT r.handle, ra.cert, ra.password "                         \
        << "FROM registrar r, registraracl ra "                             \
        << "WHERE r.id=ra.registrarid AND r.handle='"                       \
        << m_conf.get<std::string>(LOGIN_REGISTRAR_NAME)                    \
        << "' LIMIT 1";                                                     \
    get_registrar_query = sss.str();                                        \
} else {                                                                    \
    get_registrar_query = "SELECT r.handle,ra.cert,ra.password "            \
        "FROM registrar r, registraracl ra "                                \
        "WHERE r.id=ra.registrarid AND r.system='t' LIMIT 1";               \
}                                                                           \
if (!m_db.ExecSelect(get_registrar_query.c_str())) {                        \
    std::cout << "error in exec" << std::endl;                              \
    return;                                                                 \
}                                                                           \
if (!m_db.GetSelectRows()) {                                                \
    std::cout << "No result" << std::endl;                                  \
    return;                                                                 \
}                                                                           \
std::string gg_handle = m_db.GetFieldValue(0,0);                            \
std::string gg_cert = m_db.GetFieldValue(0,1);                              \
std::string gg_password = m_db.GetFieldValue(0,2);                          \
m_db.FreeSelect();                                                          \
r = epp->ClientLogin(gg_handle.c_str(),gg_password.c_str(),"",              \
    "system_delete_login","<system_delete_login/>",                         \
    clientId,gg_cert.c_str(),ccReg::EN);                                    \
if (r->code != 1000 || !clientId) {                                         \
    std::cerr << "Cannot connect: " << r->code << std::endl;                \
    return;                                                                 \
}


#define CLIENT_LOGOUT                                                       \
    epp->ClientLogout(clientId,"system_delete_logout",                      \
            "<system_delete_logout/>");

#endif // _SIMPLE_H_
