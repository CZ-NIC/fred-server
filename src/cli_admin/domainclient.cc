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

#include "simple.h"
#include "commonclient.h"
#include "domainclient.h"
#include "register/register.h"

namespace Admin {

DomainClient::DomainClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Domain related options");
    m_options->add_options()
        addOpt(DOMAIN_LIST_NAME)
        addOpt(DOMAIN_LIST_PLAIN_NAME)
        addOptStr(DOMAIN_INFO_NAME)
        addOptStr(DOMAIN_CREATE_NAME)
        addOptStr(DOMAIN_UPDATE_NAME)
        addOpt(DOMAIN_CREATE_HELP_NAME)
        addOpt(DOMAIN_UPDATE_HELP_NAME)
        addOpt(DOMAIN_LIST_PLAIN_NAME)
        addOpt(DOMAIN_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Domain related sub options");
    m_optionsInvis->add_options()
        add_ID()
        add_FQDN()
        add_NSSET_ID()
        add_NSSET_HANDLE()
        add_ANY_NSSET()
        add_KEYSET_ID()
        add_KEYSET_HANDLE()
        add_ANY_KEYSET()
        add_REGISTRANT_ID()
        add_REGISTRANT_NAME()
        add_REGISTRANT_HANDLE()
        add_ADMIN_ID()
        add_ADMIN_HANDLE()
        add_ADMIN_NAME()
        add_REGISTRAR_ID()
        add_REGISTRAR_HANDLE()
        add_REGISTRAR_NAME()
        add_ADMIN()
        add_ADMIN_ADD()
        add_ADMIN_REM()
        add_CRDATE()
        add_UPDATE()
        add_DELDATE()
        add_TRANSDATE();
    // TODO updateregistrar, createregistrar, authpw, type, state, ...
}

DomainClient::DomainClient(
        std::string connstring,
        std::string nsAddr):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_options = NULL;
    m_optionsInvis = NULL;
}
DomainClient::~DomainClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
DomainClient::init(
        std::string connstring,
        std::string nsAddr,
        Config::Conf &conf)
{
    m_connstring = connstring;
    m_nsAddr = nsAddr;
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_conf = conf;
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

void
DomainClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
}

void
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

    apply_ID(domFilter);
    apply_FQDN(domFilter);
    apply_HANDLE(domFilter);
    apply_NSSET_ID(domFilter);
    apply_NSSET_HANDLE(domFilter);
    apply_ANY_NSSET(domFilter);
    apply_KEYSET_ID(domFilter);
    apply_KEYSET_HANDLE(domFilter);
    apply_ANY_KEYSET(domFilter);
    apply_ZONE_ID(domFilter);
    apply_REGISTRANT_ID(domFilter);
    apply_REGISTRANT_HANDLE(domFilter);
    apply_REGISTRANT_NAME(domFilter);

    if (m_conf.hasOpt(ADMIN_ID_NAME))
        domFilter->addAdminContact().addId().setValue(
                Database::ID(m_conf.get<unsigned int>(ADMIN_ID_NAME)));
    if (m_conf.hasOpt(ADMIN_HANDLE_NAME))
        domFilter->addAdminContact().addHandle().setValue(
                m_conf.get<std::string>(ADMIN_HANDLE_NAME));
    if (m_conf.hasOpt(ADMIN_NAME_NAME))
        domFilter->addAdminContact().addName().setValue(
                m_conf.get<std::string>(ADMIN_NAME_NAME));

    if (m_conf.hasOpt(DOMAIN_EXP_DATE_FROM_NAME) || m_conf.hasOpt(DOMAIN_EXP_DATE_TO_NAME)) {
        Database::Date expDateFrom(NEG_INF);
        Database::Date expDateTo(POS_INF);
        if (m_conf.hasOpt(DOMAIN_EXP_DATE_FROM_NAME))
            expDateFrom.from_string(
                    m_conf.get<std::string>(DOMAIN_EXP_DATE_FROM_NAME));
        if (m_conf.hasOpt(DOMAIN_EXP_DATE_TO_NAME))
            expDateTo.from_string(
                    m_conf.get<std::string>(DOMAIN_EXP_DATE_TO_NAME));
        domFilter->addExpirationDate().setValue(
                Database::DateInterval(expDateFrom, expDateTo));
    }
    if (m_conf.hasOpt(DOMAIN_OUT_DATE_FROM_NAME) || m_conf.hasOpt(DOMAIN_OUT_DATE_TO_NAME)) {
        Database::Date outDateFrom(NEG_INF);
        Database::Date outDateTo(POS_INF);
        if (m_conf.hasOpt(DOMAIN_OUT_DATE_FROM_NAME))
            outDateFrom.from_string(
                    m_conf.get<std::string>(DOMAIN_OUT_DATE_FROM_NAME));
        if (m_conf.hasOpt(DOMAIN_OUT_DATE_TO_NAME))
            outDateTo.from_string(
                    m_conf.get<std::string>(DOMAIN_OUT_DATE_TO_NAME));
        domFilter->addOutZoneDate().setValue(
                Database::DateInterval(outDateFrom, outDateTo));
    }
    if (m_conf.hasOpt(DOMAIN_CANC_DATE_FROM_NAME) || m_conf.hasOpt(DOMAIN_CANC_DATE_TO_NAME)) {
        Database::Date cancDateFrom(NEG_INF);
        Database::Date cancDateTo(POS_INF);
        if (m_conf.hasOpt(DOMAIN_CANC_DATE_FROM_NAME))
            cancDateFrom.from_string(
                    m_conf.get<std::string>(DOMAIN_CANC_DATE_FROM_NAME));
        if (m_conf.hasOpt(DOMAIN_CANC_DATE_TO_NAME))
            cancDateTo.from_string(
                    m_conf.get<std::string>(DOMAIN_CANC_DATE_TO_NAME));
        domFilter->addCancelDate().setValue(
                Database::DateInterval(cancDateFrom, cancDateTo));
    }

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(domFilter);

    apply_LIMIT(domList);
    domList->reload(*unionFilter, m_dbman);

    std::cout << "<objects>\n";
    for (unsigned int i = 0; i < domList->getCount(); i++) {
        Register::Domain::Domain *domain = domList->getDomain(i);
        std::cout
            << "\t<domain>\n"
            << "\t\t<id>" << domain->getId() << "</id>\n"
            << "\t\t<fqdn>" << domain->getFQDN() << "</fqdn>\n"
            << "\t\t<fqdn_idn>" << domain->getFQDNIDN() << "</fqdn_idn>\n"
            << "\t\t<zone>" << domain->getZoneId() << "</zone>\n"
            << "\t\t<nsset>\n"
            << "\t\t\t<id>" << domain->getNSSetId() << "</id>\n"
            << "\t\t\t<handle>" << domain->getNSSetHandle() << "</handle>\n"
            << "\t\t</nsset>\n"
            << "\t\t<keyset>\n"
            << "\t\t\t<id>" << domain->getKeySetId() << "</id>\n"
            << "\t\t\t<handle>" << domain->getKeySetHandle() << "</handle>\n"
            << "\t\t</keyset>\n"
            << "\t\t<registrant>\n"
            << "\t\t\t<id>" << domain->getRegistrantId() << "</id>\n"
            << "\t\t\t<handle>" << domain->getRegistrantHandle() << "</handle>\n"
            << "\t\t\t<name>" << domain->getRegistrantName() << "</name>\n"
            << "\t\t</registrant>\n";
        for (unsigned int j = 0; j < domain->getAdminCount(); j++)
            std::cout
                << "\t\t<admin>\n"
                << "\t\t\t<id>" << domain->getAdminIdByIdx(j) << "</id>\n"
                << "\t\t\t<handle>" << domain->getAdminHandleByIdx(j) << "</handle>\n"
                << "\t\t</admin>\n";
        if (m_conf.hasOpt(FULL_LIST_NAME)) {
            std::cout
                << "\t\t<exp_date>" << domain->getExpirationDate() << "</exp_date>\n"
                << "\t\t<val_ex_date>" << domain->getValExDate() << "</val_ex_date>\n"
                << "\t\t<zone_stat_time>" << domain->getZoneStatusTime() << "</zone_stat_time>\n"
                << "\t\t<out_zone_date>" << domain->getOutZoneDate() << "</out_zone_date>\n"
                << "\t\t<cancel_date>" << domain->getCancelDate() << "</cancel_date>\n"
                << "\t\t<create_date>" << domain->getCreateDate() << "</create_date>\n"
                << "\t\t<transfer_date>" << domain->getTransferDate() << "</transfer_date>\n"
                << "\t\t<update_date>" << domain->getUpdateDate() << "</update_date>\n"
                << "\t\t<delete_date>" << domain->getDeleteDate() << "</delete_date>\n"
                << "\t\t<registrar>\n"
                << "\t\t\t<id>" << domain->getRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << domain->getRegistrarHandle() << "</handle>\n"
                << "\t\t</registrar>\n"
                << "\t\t<create_registrar>\n"
                << "\t\t\t<id>" << domain->getCreateRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << domain->getCreateRegistrarHandle() << "</handle>\n"
                << "\t\t</create_registrar>\n"
                << "\t\t<update_registrar>\n"
                << "\t\t\t<id>" << domain->getUpdateRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << domain->getUpdateRegistrarHandle() << "</handle>\n"
                << "\t\t</update_registrar>\n"
                << "\t\t<auth_password>" << domain->getAuthPw() << "</auth_password>\n"
                << "\t\t<ROID>" << domain->getROID() << "</ROID>\n";
            for (unsigned int j = 0; j < domain->getStatusCount(); j++) {
                Register::Status *status = (Register::Status *)domain->getStatusByIdx(j);
                std::cout
                    << "\t\t<status>\n"
                    << "\t\t\t<id>" << status->getStatusId() << "</id>\n"
                    << "\t\t\t<from>" << status->getFrom() << "</from>\n"
                    << "\t\t\t<to>" << status->getTo() << "</to>\n"
                    << "\t\t</status>\n";
            }
        }
        std::cout
            << "\t</domain>\n";
    }
    std::cout << "</objects>" << std::endl;

    unionFilter->clear();
    delete unionFilter;
}

int
DomainClient::domain_create()
{
    std::string fqdn = m_conf.get<std::string>(DOMAIN_CREATE_NAME).c_str();
    std::string registrant = m_conf.get<std::string>(DOMAIN_REGISTRANT_NAME).c_str();
    std::string nsset = m_conf.get<std::string>(DOMAIN_NSSET_NAME).c_str();
    std::string keyset = m_conf.get<std::string>(DOMAIN_KEYSET_NAME).c_str();
    //std::string authInfoPw = m_conf.get<std::string>(AUTH_INFO_PW_NAME).c_str();
    std::string authInfoPw = m_conf.get<std::string>(AUTH_PW_NAME).c_str();
    std::string admins = m_conf.get<std::string>(ADMIN_NAME).c_str();
    unsigned int period = m_conf.get<unsigned int>(DOMAIN_PERIOD_NAME);

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

    CLIENT_LOGIN;

    r = epp->DomainCreate(fqdn.c_str(), registrant.c_str(),
            nsset.c_str(), keyset.c_str(), authInfoPw.c_str(),
            period_str, admin, crDate, exDate,
            clientId, cltrid.c_str(), xml.c_str(), extList);

    std::cout << "return code: " << r->code << std::endl;

    CLIENT_LOGOUT;
    return 0;
}

int
DomainClient::domain_update()
{
    std::string fqdn = m_conf.get<std::string>(DOMAIN_UPDATE_NAME);
    std::string registrant = m_conf.get<std::string>(REGISTRANT_HANDLE_NAME);
    std::string nsset = m_conf.get<std::string>(NSSET_HANDLE_NAME);
    std::string keyset = m_conf.get<std::string>(KEYSET_HANDLE_NAME);
    std::string authinfopw = m_conf.get<std::string>(AUTH_PW_NAME);
    std::string admins_add = m_conf.get<std::string>(ADMIN_ADD_NAME);
    std::string admins_rem = m_conf.get<std::string>(ADMIN_REM_NAME);
    std::string admins_rem_temp = m_conf.get<std::string>(ADMIN_REM_TEMP_NAME);

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

    CLIENT_LOGIN;

    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + fqdn + "</fqdn>";
    cltrid = "domain_update";

    r = epp->DomainUpdate(fqdn.c_str(), registrant.c_str(),
            authinfopw.c_str(), nsset.c_str(), keyset.c_str(), 
            admin_add, admin_rem, admin_rem_temp,
            clientId, cltrid.c_str(), xml.c_str(), extList);

    std::cout << "return code: " << r->code << std::endl;
    CLIENT_LOGOUT;
    return 0;
}
int
DomainClient::domain_info()
{
    std::string name = m_conf.get<std::string>(DOMAIN_INFO_NAME);

    CLIENT_LOGIN;
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

    CLIENT_LOGOUT;
    return 0;
}
int
DomainClient::domain_list_plain()
{
    CLIENT_LOGIN;
    std::string name = m_conf.get<std::string>(DOMAIN_LIST_PLAIN_NAME).c_str();
    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + name + "</fqdn>";
    cltrid = "list_domains";

    ccReg::Lists *k;

    r = epp->DomainList(k, clientId, cltrid.c_str(), xml.c_str());

    for (int i = 0; i < (int)k->length(); i++)
        std::cout << (*k)[i] << std::endl;

    CLIENT_LOGOUT;
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

void
DomainClient::list_help()
{
    std::cout
        << "** Domain list **\n\n"
        << "  $ " << g_prog_name << " --" << DOMAIN_LIST_NAME << " \\\n"
        << "    [--" << ID_NAME << "=<id_nubmer>] \\\n"
        << "    [--" << HANDLE_NAME << "=<handle>] \\\n"
        << "    [--" << NSSET_ID_NAME << "=<id_number>] \\\n"
        << "    [--" << NSSET_HANDLE_NAME << "=<handle>] \\\n"
        << "    [--" << ANY_NSSET_NAME << "] \\\n"
        << "    [--" << KEYSET_ID_NAME << "=<id_number>] \\\n"
        << "    [--" << KEYSET_HANDLE_NAME << "=<handle>] \\\n"
        << "    [--" << ANY_KEYSET_NAME << "] \\\n"
        << "    [--" << ZONE_ID_NAME << "=<id_number>] \\\n"
        << "    [--" << REGISTRANT_ID_NAME << "=<id_number>] \\\n"
        << "    [--" << REGISTRANT_HANDLE_NAME << "=<handle>] \\\n"
        << "    [--" << REGISTRANT_NAME_NAME << "=<name>] \\\n"
        << "    [--" << ADMIN_ID_NAME << "=<admin_id_number>] \\\n"
        << "    [--" << ADMIN_HANDLE_NAME << "=<admin_handle>] \\\n"
        << "    [--" << ADMIN_NAME_NAME << "=<admin_name>] \\\n"
        << "    [--" << REGISTRAR_ID_NAME << "=<registrar_id_number>] \\\n"
        << "    [--" << REGISTRAR_HANDLE_NAME << "=<registrar_handle>] \\\n"
        << "    [--" << REGISTRAR_NAME_NAME << "=<registrar_name>] \\\n"
        << "    [--" << FULL_LIST_NAME << "]\n"
        << std::endl;
}

} // namespace Admin;
