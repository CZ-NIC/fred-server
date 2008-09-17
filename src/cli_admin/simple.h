#ifndef _SIMPLE_H_
#define _SIMPLE_H_

#define addOpt(name)                        (name, name##_DESC)
#define addOptStr(name)                     (name, boost::program_options::value<std::string>(), name##_DESC)
#define addOptInt(name)                     (name, boost::program_options::value<int>(), name##_DESC)
#define addOptUInt(name)                    (name, boost::program_options::value<unsigned int>(), name##_DESC)
#define addOptTID(name)                     (name, boost::program_options::value<Register::TID>(), name##_DESC)
#define addOptBool(name)                    (name, boost::program_options::value<bool>(), name##_DESC)
#define addOptType(name, type)              (name, boost::program_options::value<type>(), name##_DESC)

#define addOptStrDef(name, val)           (name, boost::program_options::value<std::string>()->default_value(val), name##_DESC)
#define addOptIntDef(name, val)           (name, boost::program_options::value<int>()->default_value(val), name##_DESC)
#define addOptUIntDef(name, val)          (name, boost::program_options::value<unsigned int>()->default_value(val), name##_DESC)
#define addOptTIDDef(name, val)           (name, boost::program_options::value<Register::TID>()->default_value(val), name##_DESC)
#define addOptBoolDef(name, val)          (name, boost::program_options::value<bool>()->default_value(val), name##_DESC)
#define addOptTypeDef(name, type, val)    (name, boost::program_options::value<type>()->default_value(val), name##_DESC)

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

#define apply_DATETIME(filter, name, nameFrom, nameTo, varFrom, varTo)      \
    if (m_conf.hasOpt(nameFrom) || m_conf.hasOpt(nameTo)) {                 \
        Database::DateTime varFrom("1901-01-01 00:00:00");                  \
        Database::DateTime varTo("2101-01-01 00:00:00");                    \
        if (m_conf.hasOpt(nameFrom))                                        \
            varFrom.from_string(m_conf.get<std::string>(nameFrom));         \
        if (m_conf.hasOpt(nameTo))                                          \
            varTo.from_string(m_conf.get<std::string>(nameTo));             \
        filter->add##name##Time().setValue(                                 \
                Database::DateTimeInterval(varFrom, varTo));                \
    }

#define CLI_HELP_NAME           "help,h"
#define CLI_VERSION_NAME        "version,V"
#define CLI_VERSION_NAME_DESC   "print version and licence information"
#define CLI_CONF_NAME           "conf,c"
#define CLI_CONF_NAME_DESC      "configuration file"
#define CLI_LANGUAGE_NAME       "lang,l"
#define CLI_LANGUAGE_NAME_DESC  "communication language"
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

// dates use almost all
#define CRDATE_FROM_NAME        "cr_date_from"
#define CRDATE_FROM_NAME_DESC   "show only records with create date and date after input value (date format: YYYY-MM-DD [hh:mm:ss])"
#define CRDATE_TO_NAME          "cr_date_to"
#define CRDATE_TO_NAME_DESC     "show only records with create date and date before input value (date format: YYYY-MM-DD [hh:mm:ss])"
#define add_CRDATE_FROM()       addOptStr(CRDATE_FROM_NAME)
#define add_CRDATE_TO()         addOptStr(CRDATE_TO_NAME)
#define add_CRDATE()            add_CRDATE_FROM() add_CRDATE_TO()
#define apply_CRDATE(filter)    apply_DATETIME(filter, Create, CRDATE_FROM_NAME, CRDATE_TO_NAME, crDateFrom, crDateTo)

#define DELDATE_FROM_NAME       "del_date_from"
#define DELDATE_FROM_NAME_DESC  "show only record with delete date and time after input value (date format: YYYY-MM-DD hh:mm:ss)"
#define DELDATE_TO_NAME         "del_date_to"
#define DELDATE_TO_NAME_DESC    "show only records with delete date and time before input value (date format: YYYY-MM-DD hh:mm:ss)"
#define add_DELDATE_FROM()      addOptStr(DELDATE_FROM_NAME)
#define add_DELDATE_TO()        addOptStr(DELDATE_TO_NAME)
#define add_DELDATE()           add_DELDATE_FROM() add_DELDATE_TO()           
#define apply_DELDATE(filter)   apply_DATETIME(filter, Delete, DELDATE_FROM_NAME, DELDATE_TO_NAME, delDateFrom, delDateTo)

#define TRANSDATE_FROM_NAME     "trans_date_from"
#define TRANSDATE_FROM_NAME_DESC "show only records with transfer date and time after input value (date format: YYYY-MM-DD hh:mm:ss)"
#define TRANSDATE_TO_NAME       "trans_date_to"
#define TRANSDATE_TO_NAME_DESC  "show only records with transfer date and time before input value (date format: YYYY-MM-DD hh:mm:ss)"
#define add_TRANSDATE_FROM()    addOptStr(TRANSDATE_FROM_NAME)
#define add_TRANSDATE_TO()      addOptStr(TRANSDATE_TO_NAME)
#define add_TRANSDATE()         add_TRANSDATE_FROM() add_TRANSDATE_TO()           
#define apply_TRANSDATE(filter) apply_DATETIME(filter, Transfer, TRANSDATE_FROM_NAME, TRANSDATE_TO_NAME, transDateFrom, transDateTo)

#define UPDATE_FROM_NAME        "up_date_from"
#define UPDATE_FROM_NAME_DESC   "show only records with transfer date and time after input value (date format: YYYY-MM-DD hh:mm:ss)"
#define UPDATE_TO_NAME          "up_date_to"
#define UPDATE_TO_NAME_DESC     "show only records with transfer date and time before input value (date format: YYYY-MM-DD hh:mm:ss)"
#define add_UPDATE_FROM()       addOptStr(UPDATE_FROM_NAME)
#define add_UPDATE_TO()         addOptStr(UPDATE_TO_NAME)
#define add_UPDATE()            add_UPDATE_FROM() add_UPDATE_TO()           
#define apply_UPDATE(filter)    apply_DATETIME(filter, Update, UPDATE_FROM_NAME, UPDATE_TO_NAME, upDatefrom, upDateTo)

// all
#define ID_NAME                 "id"
#define ID_NAME_DESC            "filter records with specific id nubmer"
#define add_ID()                addOptUInt(ID_NAME)
#define apply_ID(filter)          get_DID(filter, Id, ID_NAME)

// contact, domain, file, invoice, keyset, nsset, registrar
#define NAME_NAME               "name_name"
#define NAME_NAME_DESC          "filter records with specific name"
#define add_NAME()              addOptStr(NAME_NAME)
#define apply_NAME(filter)        get_Str(filter, Name, NAME_NAME)

// contact, domain, keyset, mail, nsset, registrar
#define HANDLE_NAME             "handle"
#define HANDLE_NAME_DESC        "filter records with specific handle"
#define add_HANDLE()            addOptStr(HANDLE_NAME)
#define apply_HANDLE(filter)      get_Str(filter, Handle, HANDLE_NAME)

// domain, nsset, object
#define FQDN_NAME               "fqdn"
#define FQDN_NAME_DESC          "filter records with specific fqdn"
#define add_FQDN()              addOptStr(FQDN_NAME)
#define apply_FQDN(filter)      get_Str(filter, FQDN, FQDN_NAME)

// contact, domain, infobuffer, invoice, keyset, nsset, poll
#define REGISTRAR_ID_NAME           "registrar_id"
#define REGISTRAR_ID_NAME_DESC      "show only records with specific registrar id number"
#define add_REGISTRAR_ID()          addOptUInt(REGISTRAR_ID_NAME)
#define apply_REGISTRAR_ID(filter)  get_DID(filter, Registrar().addId, REGISTRAR_ID_NAME)

// contact, domain, keyset, nsset, registrar
#define REGISTRAR_HANDLE_NAME       "registrar_handle"
#define REGISTRAR_HANDLE_NAME_DESC  "show only records with specific registrar handle"
#define add_REGISTRAR_HANDLE()      addOptStr(REGISTRAR_HANDLE_NAME)
#define apply_REGISTRAR_HANDLE(filter) get_Str(filter, Registrar().addHandle, REGISTRAR_HANDLE_NAME)

// contact,  domain, keyset, nsset
#define REGISTRAR_NAME_NAME         "registrar_name"
#define REGISTRAR_NAME_NAME_DESC    "show only records with specific registrar name"
#define add_REGISTRAR_NAME()        addOptStr(REGISTRAR_NAME_NAME)
#define apply_REGISTRAR_NAME(filter) get_Str(filter, Registrar().addName, REGISTRAR_NAME_NAME)

// domain, invoice
#define REGISTRANT_ID_NAME          "registrant_id"
#define REGISTRANT_ID_NAME_DESC     "show only records with specific registrant id number"
#define add_REGISTRANT_ID()         addOptUInt(REGISTRANT_ID_NAME)
#define apply_REGISTRANT_ID(filter) get_DID(filter, Registrant().addId, REGISTRANT_ID_NAME)

// domain
#define REGISTRANT_HANDLE_NAME      "registrant_handle"
#define REGISTRANT_HANDLE_NAME_DESC "show only records with specific registrant handle"
#define add_REGISTRANT_HANDLE()     addOptStr(REGISTRANT_HANDLE_NAME)
#define apply_REGISTRANT_HANDLE(filter) get_Str(filter, Registrant().addHandle, REGISTRANT_HANDLE_NAME)

// domain
#define REGISTRANT_NAME_NAME        "registrant_name"
#define REGISTRANT_NAME_NAME_DESC   "show only recorrds with specific registrant name"
#define add_REGISTRANT_NAME()       addOptStr(REGISTRANT_NAME_NAME)
#define apply_REGISTRANT_NAME(filter) get_Str(filter, Registrant().addName, REGISTRANT_NAME_NAME)

// domain, keyset, nsset
#define ADMIN_ID_NAME               "admin_id"
#define ADMIN_ID_NAME_DESC          "show only records with specific admin contact id number"
#define add_ADMIN_ID()              addOptUInt(ADMIN_ID_NAME)

// domain, keyset, nsset
#define ADMIN_HANDLE_NAME           "admin_handle"
#define ADMIN_HANDLE_NAME_DESC      "show only records with specific admin contact handle"
#define add_ADMIN_HANDLE()          addOptStr(ADMIN_HANDLE_NAME)

// domain, keyset, nsset
#define ADMIN_NAME_NAME             "admin_name"
#define ADMIN_NAME_NAME_DESC        "show only records with specific admin contact name"
#define add_ADMIN_NAME()            addOptStr(ADMIN_NAME_NAME)

// all
#define LIMIT_NAME                  "limit"
#define LIMIT_NAME_DESC             "set output limit"
#define add_LIMIT()                 addOptUInt(LIMIT_NAME)
#define apply_LIMIT(list)           list->setLimit(getOptUInt(LIMIT_NAME))

// domain, keyset, nsset, contact
#define FULL_LIST_NAME              "full_list"
#define FULL_LIST_NAME_DESC         "when do list show all available information"
#define add_FULL_LIST()             addOpt(FULL_LIST_NAME)

// domain, invoice
#define ZONE_ID_NAME                "zone_id"
#define ZONE_ID_NAME_DESC           "show only records with specific zone id number"
#define add_ZONE_ID()               addOptUInt(ZONE_ID_NAME)
#define apply_ZONE_ID(filter)       get_DID(filter, ZoneId, ZONE_ID_NAME)

// registrar
#define ZONE_HANDLE_NAME            "zone_handle"
#define ZONE_HANDLE_NAME_DESC       "bla bla"
#define add_ZONE_HANDLE()           addOptStr(ZONE_HANDLE_NAME)
#define apply_ZONE_HANDLE(filter)   get_Str(filter, Zone, ZONE_ID_NAME)

// almost all
#define AUTH_PW_NAME                "auth_pw"
#define AUTH_PW_NAME_DESC           "show only records with specific authorization password"
#define add_AUTH_PW()               addOptStr(AUTH_PW_NAME)

// domain, keyset, nsset
#define ADMIN_NAME                  "admins"
#define ADMIN_NAME_DESC             "list of admins"
#define add_ADMIN()                 addOptStr(ADMIN_NAME)

// domain, keyset
#define ADMIN_ADD_NAME              "admin_add"
#define ADMIN_ADD_NAME_DESC         "list of admins to add"
#define add_ADMIN_ADD()             addOptStr(ADMIN_ADD_NAME)

// domain, keyset
#define ADMIN_REM_NAME              "admin_rem"
#define ADMIN_REM_NAME_DESC         "list of admins to remove"
#define add_ADMIN_REM()             addOptStr(ADMIN_REM_NAME)

// domain
#define ADMIN_REM_TEMP_NAME         "admin_rem_temp"
#define ADMIN_REM_TEMP_NAME_DESC    "list of temporary admins to remove"
#define add_REM_TEMP_ADMIN()        addOptStr(ADMIN_REM_TEMP_NAME)

// nsset
#define IP_NAME                 "ip"
#define IP_NAME_DESC            "show only records with specific ip address"
#define add_IP()                addOptStr(IP_NAME)

// contact, registrar
#define ORGANIZATION_NAME       "organization"
#define ORGANIZATION_NAME_DESC  "show only records with specific organization name"
#define add_ORGANIZATION()      addOptStr(ORGANIZATION_NAME)
#define apply_ORGANIZATION(filter) get_Str(filter, Organization, ORGANIZATION_NAME)

// contact, registrar
#define CITY_NAME               "city"
#define CITY_NAME_DESC          "show only records with specific city"
#define add_CITY()              addOptStr(CITY_NAME)
#define apply_CITY(filter)      get_Str(filter, City, CITY_NAME)

// contact, registrar
#define EMAIL_NAME              "email"
#define EMAIL_NAME_DESC         "show only records with specific email address"
#define add_EMAIL()             addOptStr(EMAIL_NAME)
#define apply_EMAIL(filter)     get_Str(filter, Email, EMAIL_NAME)

// contcat
#define NOTIFY_EMAIL_NAME       "notify_email"
#define NOTIFY_EMAIL_NAME_DESC  "show only records with specific notify email address"
#define add_NOTIFY_EMAIL()      addOptStr(NOTIFY_EMAIL_NAME)
#define apply_NOTIFY_EMAIL(filter) get_Str(filter, NotifyEmail, NOTIFY_EMAIL_NAME)

// contact
#define VAT_NAME                "vat"
#define VAT_NAME_DESC           "show only records with ..."
#define add_VAT()               addOptStr(VAT_NAME)
#define apply_VAT(filter)       get_Str(filter, Vat, VAT_NAME)

// contact
#define SSN_NAME                "ssn"
#define SSN_NAME_DESC           "show only records with ..."
#define add_SSN()               addOptStr(SSN_NAME)
#define apply_SSN(filter)       get_Str(filter, Ssn, SSN_NAME)

// registrar
#define COUNTRY_NAME            "country"
#define COUNTRY_NAME_DESC       "show only records with specific country"
#define add_COUNTRY()           addOptStr(COUNTRY_NAME)
#define apply_COUNTRY(filter)   get_Str(filter, Country, COUNTRY_NAME)

// domain
#define KEYSET_ID_NAME          "keyset_id"
#define KEYSET_ID_NAME_DESC     "show only records with specific keyset id number"
#define add_KEYSET_ID()         addOptUInt(KEYSET_ID_NAME)
#define apply_KEYSET_ID(filter) get_DID(filter, KeySet().addId, KEYSET_ID_NAME)

// domain
#define KEYSET_HANDLE_NAME      "keyset_handle"
#define KEYSET_HANDLE_NAME_DESC "show only records with specific keyset handle"
#define add_KEYSET_HANDLE()     addOptStr(KEYSET_HANDLE_NAME)
#define apply_KEYSET_HANDLE(filter) get_Str(filter, KeySet().addHandle, KEYSET_HANDLE_NAME)

// domain
#define NSSET_ID_NAME           "nsset_id"
#define NSSET_ID_NAME_DESC      "show only records with specific nsset id number"
#define add_NSSET_ID()          addOptUInt(NSSET_ID_NAME)
#define apply_NSSET_ID(filter)  get_DID(filter, NSSet().addId, NSSET_ID_NAME)

// domain
#define NSSET_HANDLE_NAME       "nsset_handle"
#define NSSET_HANDLE_NAME_DESC  "show only records with specific nsset handle"
#define add_NSSET_HANDLE()      addOptStr(NSSET_HANDLE_NAME)
#define apply_NSSET_HANDLE(filter) get_Str(filter, NSSet().addHandle, NSSET_HANDLE_NAME)

// domain
#define ANY_KEYSET_NAME         "any_keyset"
#define ANY_KEYSET_NAME_DESC    "show only records with assigned keyset"
#define add_ANY_KEYSET()        addOpt(ANY_KEYSET_NAME)
#define apply_ANY_KEYSET(filter) checkGetOpt(filter->addKeySet(), ANY_KEYSET_NAME)

// domain
#define ANY_NSSET_NAME          "any_nsset"
#define ANY_NSSET_NAME_DESC     "show only records with assigned nsset"
#define add_ANY_NSSET()         addOpt(ANY_NSSET_NAME)
#define apply_ANY_NSSET(filter) checkGetOpt(filter->addNSSet(), ANY_NSSET_NAME)

#endif // _SIMPLE_H_
