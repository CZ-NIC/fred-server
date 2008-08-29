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

#include "commonclient.h"
#include "domainclient.h"
#include "register/register.h"

namespace Admin {

#define LOGIN_DOMAINCLIENT \
CorbaClient cc(0, NULL, m_nsAddr.c_str()); \
CORBA::Object_var o = cc.getNS()->resolve("EPP"); \
ccReg::EPP_var epp; \
epp = ccReg::EPP::_narrow(o); \
CORBA::Long clientId = 0; \
ccReg::Response_var r; \
if (!m_db.ExecSelect( \
            "SELECT r.handle,ra.cert,ra.password " \
            "FROM registrar r, registraracl ra " \
            "WHERE r.id=ra.registrarid AND r.system='t' LIMIT 1 ") \
        ) \
    return -1; \
if (!m_db.GetSelectRows()) \
    return -1; \
std::string handle = m_db.GetFieldValue(0,0); \
std::string cert = m_db.GetFieldValue(0,1); \
std::string password = m_db.GetFieldValue(0,2); \
m_db.FreeSelect(); \
r = epp->ClientLogin(handle.c_str(),password.c_str(),"","system_delete_login","<system_delete_login/>", \
        clientId,cert.c_str(),ccReg::EN); \
if (r->code != 1000 || !clientId) { \
    std::cerr << "Cannot connect: " << r->code << std::endl; \
    return -1; \
}

#define LOGOUT_DOMAINCLIENT \
    epp->ClientLogout(clientId,"system_delete_logout","<system_delete_logout/>");

DomainClient::DomainClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Domain related options");
    m_options->add_options()
        ADD_OPT(DOMAIN_LIST_NAME, "list of all domains (via filters)")
        ADD_OPT(DOMAIN_LIST_PLAIN_NAME, "list of all domains (via ccReg_i)")
        ADD_OPT_TYPE(DOMAIN_INFO_NAME, "keyset info (via epp_impl)", std::string)
        ADD_OPT_TYPE(DOMAIN_CREATE_NAME, "create domain", std::string)
        ADD_OPT_TYPE(DOMAIN_UPDATE_NAME, "update domain", std::string)
        ADD_OPT(DOMAIN_CREATE_HELP_NAME, "help for domain creating")
        ADD_OPT(DOMAIN_UPDATE_HELP_NAME, "help for domain updating");

    m_optionsInvis = new boost::program_options::options_description(
            "Domain related invisible options");
    m_optionsInvis->add_options()
        ADD_OPT_TYPE(DOMAIN_REGISTRANT_NAME, "domain registrant", std::string)
        ADD_OPT_DEF(DOMAIN_NSSET_NAME, "domain NSSet", std::string, "")
        ADD_OPT_DEF(DOMAIN_KEYSET_NAME, "domain Keyset", std::string, "")
        ADD_OPT_DEF(DOMAIN_PERIOD_NAME, "period (in months)", unsigned int, 0);
}

DomainClient::DomainClient(
        std::string connstring,
        std::string nsAddr,
        boost::program_options::variables_map varMap):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_varMap = varMap;
    m_options = NULL;
    m_optionsInvis = NULL;
}
DomainClient::~DomainClient()
{
}

void
DomainClient::init(
        std::string connstring,
        std::string nsAddr,
        boost::program_options::variables_map varMap)
{
    m_connstring = connstring;
    m_nsAddr = nsAddr;
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_varMap = varMap;
}

boost::program_options::options_description *
DomainClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
DomainClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

int
DomainClient::domain_list()
{
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));

    std::auto_ptr<Register::Domain::Manager> domMan(
            Register::Domain::Manager::create(&m_db, zoneMan.get()));
    std::auto_ptr<Register::Domain::List> domList(
            domMan->createList());

    Database::Filters::Domain *domFilter;
    domFilter = new Database::Filters::DomainHistoryImpl();

    if (m_varMap.count(KEYSET_ID_NAME))
        domFilter->addKeySetId().setValue(
                Database::ID(m_varMap[KEYSET_ID_NAME].as<unsigned int>()));

    if (m_varMap.count(KEYSET_HANDLE_NAME))
        domFilter->addKeySet().addHandle().setValue(
                m_varMap[KEYSET_HANDLE_NAME].as<std::string>());

    if (m_varMap.count(NSSET_ID_NAME))
        domFilter->addNSSetId().setValue(
                Database::ID(m_varMap[NSSET_ID_NAME].as<unsigned int>()));

    if (m_varMap.count(ZONE_ID_NAME))
        domFilter->addZoneId().setValue(
                Database::ID(m_varMap[ZONE_ID_NAME].as<unsigned int>()));

    if (m_varMap.count(REGISTRANT_ID_NAME))
        domFilter->addRegistrantId().setValue(
                Database::ID(m_varMap[REGISTRANT_ID_NAME].as<unsigned int>()));

    if (m_varMap.count(ID_NAME))
        domFilter->addId().setValue(
                Database::ID(m_varMap[ID_NAME].as<unsigned int>()));

    if (m_varMap.count(FQDN_NAME))
        domFilter->addFQDN().setValue(
                m_varMap[FQDN_NAME].as<std::string>());
    
    if (m_varMap.count(ANY_KEYSET_NAME))
        domFilter->addKeySet();

    // TODO implement addNSSet in backend (?)
    // if (m_varMap.count(ANY_NSSET_NAME))
        // domFilter->addNSSet();

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(domFilter);

    domList->setLimit(m_varMap["limit"].as<unsigned int>());
    domList->reload(*unionFilter, m_dbman);
    std::cout << "<objects>" << std::endl;
    for (unsigned int i = 0; i < domList->getCount(); i++) {
        std::cout
            << "\t<domain>\n"
            << "\t\t<id>" << domList->getDomain(i)->getId() << "</id>\n"
            << "\t\t<fqdn>" << domList->getDomain(i)->getFQDN() << "</fqdn>\n"
            << "\t\t<zone>" << domList->getDomain(i)->getZoneId() << "</zone>\n"
            << "\t\t<nsset>" << domList->getDomain(i)->getNSSetId() << "</nsset>\n"
            << "\t\t<nsset_h>" << domList->getDomain(i)->getNSSetHandle() << "</nsset_h>\n"
            << "\t\t<keyset>" << domList->getDomain(i)->getKeySetId() << "</keyset>\n"
            << "\t\t<keyset_h>" << domList->getDomain(i)->getKeySetHandle() << "</keyset_h>\n"
            << "\t\t<registrant>" << domList->getDomain(i)->getRegistrantId() << "</registrant>\n"
            << "\t\t<registrant_h>" << domList->getDomain(i)->getRegistrantHandle() << "</registrant_h>\n";
        for (unsigned int j = 0; j < domList->getDomain(i)->getAdminCount(); j++)
            std::cout
                << "\t\t<admin>" << domList->getDomain(i)->getAdminIdByIdx(j) << "</admin>\n"
                << "\t\t<admin_h>" << domList->getDomain(i)->getAdminHandleByIdx(j) << "</admin_h>\n";
        std::cout
            << "\t\t<exp_date>" << domList->getDomain(i)->getExpirationDate() << "</exp_date>\n"
            << "\t</domain>\n";
    }
    std::cout << "</object>" << std::endl;
    return 0;
}

int
DomainClient::domain_create()
{
    std::string fqdn = m_varMap[DOMAIN_CREATE_NAME].as<std::string>().c_str();
    std::string registrant = m_varMap[DOMAIN_REGISTRANT_NAME].as<std::string>().c_str();
    std::string nsset = m_varMap[DOMAIN_NSSET_NAME].as<std::string>().c_str();
    std::string keyset = m_varMap[DOMAIN_KEYSET_NAME].as<std::string>().c_str();
    std::string authInfoPw = m_varMap[AUTH_INFO_PW_NAME].as<std::string>().c_str();
    std::string admins = m_varMap[ADMIN_NAME].as<std::string>().c_str();
    unsigned int period = m_varMap[DOMAIN_PERIOD_NAME].as<unsigned int>();

    bool empty_param = false;
    if (registrant.empty()) {
        std::cerr << "Parameter ``registrant'' is not allowed to be empty!" << std::endl;
        empty_param = true;
    }
    if (period == 0) {
        std::cerr << "Parameter ``period'' must be number bigger (and not equal) than zero!" << std::endl;
        empty_param = true;
    }
    if (admins.empty()) {
        std::cerr << "Parameter ``admins'' is not allowed to be empty!" << std::endl;
        empty_param = true;
    }
    if (empty_param)
        exit(1);

    /* lets get separated admin contact from its list */
    std::vector<std::string> admins_list;
    char *tok;
    char *str;
    str = (char *)std::malloc(sizeof(char) * (admins.length()));
    std::strcpy(str, admins.c_str());
    /* list of admin contact is seperated with spaces */
    tok = std::strtok(str, " ");
    while (tok != NULL) {
        admins_list.push_back(std::string(tok));
        tok = std::strtok(NULL, " ");
    }
    std::free(str);

    ccReg::AdminContact admin;
    admin.length(admins_list.size());
    for (int i = 0; i < (int)admins_list.size(); i++)
        admin[i] = CORBA::string_dup(admins_list[i].c_str());


    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + fqdn + "</fqdn>";
    cltrid = "domain_create";

    ccReg::Period_str period_str;
    period_str.count = (short int)period;
    period_str.unit = ccReg::unit_month;

    ccReg::timestamp crDate;
    ccReg::timestamp exDate;

    ccReg::ExtensionList extList;
    extList.length(0);

    LOGIN_DOMAINCLIENT;

    r = epp->DomainCreate(fqdn.c_str(), registrant.c_str(),
            nsset.c_str(), keyset.c_str(), authInfoPw.c_str(),
            period_str, admin, crDate, exDate,
            clientId, cltrid.c_str(), xml.c_str(), extList);

    std::cout << "return code: " << r->code << std::endl;

    LOGOUT_DOMAINCLIENT;
    return 0;
}

int
DomainClient::domain_update()
{
    std::string fqdn = m_varMap[DOMAIN_UPDATE_NAME].as<std::string>();
    std::string registrant = m_varMap[REGISTRANT_HANDLE_NAME].as<std::string>();
    std::string nsset = m_varMap[NSSET_HANDLE_NAME].as<std::string>();
    std::string keyset = m_varMap[KEYSET_HANDLE_NAME].as<std::string>();
    std::string authinfopw = m_varMap[AUTH_INFO_PW_NAME].as<std::string>();
    std::string admins_add = m_varMap[ADMIN_ADD_NAME].as<std::string>();
    std::string admins_rem = m_varMap[ADMIN_REM_NAME].as<std::string>();
    std::string admins_rem_temp = m_varMap[ADMIN_REM_TEMP_NAME].as<std::string>();

    std::vector<std::string> admins_add_list;
    std::vector<std::string> admins_rem_list;
    std::vector<std::string> admins_rem_temp_list;
    /* lets get separated admin contact from its list */
    char *tok;
    char *str;
    str = (char *)std::malloc(sizeof(char) * (admins_add.length()));
    std::strcpy(str, admins_add.c_str());
    /* list of admin contact is seperated with spaces */
    tok = std::strtok(str, " ");
    while (tok != NULL) {
        admins_add_list.push_back(std::string(tok));
        tok = std::strtok(NULL, " ");
    }
    std::free(str);

    str = (char *)std::malloc(sizeof(char) * (admins_rem.length()));
    std::strcpy(str, admins_rem.c_str());
    /* list of admin contact is seperated with spaces */
    tok = std::strtok(str, " ");
    while (tok != NULL) {
        admins_rem_list.push_back(std::string(tok));
        tok = std::strtok(NULL, " ");
    }
    std::free(str);

    str = (char *)std::malloc(sizeof(char) * (admins_rem_temp.length()));
    std::strcpy(str, admins_rem_temp.c_str());
    /* list of admin contact is seperated with spaces */
    tok = std::strtok(str, " ");
    while (tok != NULL) {
        admins_rem_temp_list.push_back(std::string(tok));
        tok = std::strtok(NULL, " ");
    }
    std::free(str);

    ccReg::AdminContact admin_add;
    ccReg::AdminContact admin_rem;
    ccReg::AdminContact admin_rem_temp;

    admin_add.length(admins_add_list.size());
    for (int i = 0; i < (int)admins_add_list.size(); i++)
        admin_add[i] = CORBA::string_dup(admins_add_list[i].c_str());
    admin_rem.length(admins_rem_list.size());
    for (int i = 0; i < (int)admins_rem_list.size(); i++)
        admin_rem[i] = CORBA::string_dup(admins_rem_list[i].c_str());
    admin_rem_temp.length(admins_rem_temp_list.size());
    for (int i = 0; i < (int)admins_rem_temp_list.size(); i++)
        admin_rem_temp[i] = CORBA::string_dup(admins_rem_temp_list[i].c_str());

    ccReg::ExtensionList extList;
    extList.length(0);

    LOGIN_DOMAINCLIENT;

    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + fqdn + "</fqdn>";
    cltrid = "domain_update";

    r = epp->DomainUpdate(fqdn.c_str(), registrant.c_str(),
            authinfopw.c_str(), nsset.c_str(), keyset.c_str(), 
            admin_add, admin_rem, admin_rem_temp,
            clientId, cltrid.c_str(), xml.c_str(), extList);

    std::cout << "return code: " << r->code << std::endl;
    LOGOUT_DOMAINCLIENT;
    return 0;
}
int
DomainClient::domain_info()
{
    std::string name = m_varMap[DOMAIN_INFO_NAME].as<std::string>();

    LOGIN_DOMAINCLIENT;
    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + name + "</fqdn>";
    cltrid = "info_keyset";

    ccReg::Domain *k = new ccReg::Domain;
    epp->DomainInfo(name.c_str(), k, clientId, cltrid.c_str(), xml.c_str());

    std::cout << k->name << std::endl;
    std::cout << k->AuthInfoPw << std::endl;
    std::cout << k->ROID << std::endl;
    std::cout << k->keyset << std::endl;

    LOGOUT_DOMAINCLIENT;
    return 0;
}
int
DomainClient::domain_list_plain()
{
    LOGIN_DOMAINCLIENT;
    std::string name = m_varMap[DOMAIN_LIST_PLAIN_NAME].as<std::string>().c_str();
    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + name + "</fqdn>";
    cltrid = "list_domains";

    ccReg::Lists *k;

    r = epp->DomainList(k, clientId, cltrid.c_str(), xml.c_str());

    for (int i = 0; i < (int)k->length(); i++)
        std::cout << (*k)[i] << std::endl;

    LOGOUT_DOMAINCLIENT;
    return 0;
}

void
DomainClient::domain_update_help()
{
    std::cout
        << "** Domain update **\n\n"
        << "  " << g_prog_name << " --domain-update=<domain_fqdn> \\\n"
        << "    [--registrant-handle=<registrant_handle> \\\n"
        << "    [--nsset-handle<new_nsset>] \\\n"
        << "    [--keyset-handle=<new_keyset>] \\\n"
        << "    [--auth-info-pw=<new_authinfo_password>] \\\n"
        << "    [--admins-add=<list_of_admins_to_add>] \\\n"
        << "    [--admins-rem=<list_of_admins_to_rem>] \\\n"
        << "    [--admins-rem-temp=<list_of_temp_admins_to_rem>]"
        << std::endl;
}

void
DomainClient::domain_create_help()
{
    std::cout
        << "** Domain create **\n\n"
        << "  " << g_prog_name << " --domain-create=<domain_fqdn> \\\n"
        << "    --registrant-handle=<registrant_handle> \\\n"
        << "    [--nsset-handle=<nsset_handle>] \\\n"
        << "    [--keyset-handle=<keyset_handle>] \\\n"
        << "    [--auth-info-pw=<authinfo_password>] \\\n"
        << "    --admins=<list_of_admins_contact_handles> \\\n"
        << "    --period=<period_in_months>\n\n"
        << "Domain creation example:\n"
        << "\t$ " << g_prog_name << " --domain-create=example.cz "
        << "--admins=\"CON::001 CON::005\" --nsset-handle=\"NSS::137\" "
        << "--period=24 --registrar-handle=\"CON::005\"\n"
        << "will create domain with FQDN ``example.cz'', administrator contacts "
        << "``CON::001'' and ``CON::005'', with NSSet ``NSS::137'', valid for "
        << "2 years and without any keyset."
        << std::endl;
}

} // namespace Admin;
