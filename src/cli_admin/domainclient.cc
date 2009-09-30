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

#include "simple.h"
#include "commonclient.h"
#include "domainclient.h"
#include "register/register.h"

#define addMethod(methods, name) \
    methods.insert(std::make_pair(name, DOMAIN_CLIENT))

namespace Admin {

const struct options *
DomainClient::getOpts()
{
    return m_opts;
}

void
DomainClient::runMethod()
{
    if (m_conf.hasOpt(DOMAIN_LIST_PLAIN_NAME)) {
        domain_list_plain();
    } else if (m_conf.hasOpt(DOMAIN_CREATE_NAME)) {
        domain_create();
    } else if (m_conf.hasOpt(DOMAIN_UPDATE_NAME)) {
        domain_update();
    } else if (m_conf.hasOpt(DOMAIN_INFO_NAME)) {
        domain_info();
    } else if (m_conf.hasOpt(DOMAIN_LIST_NAME)) {
        domain_list();
    } else if (m_conf.hasOpt(DOMAIN_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
DomainClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("Domain", getOpts(), getOptsCount());
}

void
DomainClient::domain_list()
{
    callHelp(m_conf, list_help);
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create());

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
    apply_CRDATE(domFilter);
    apply_DELDATE(domFilter);
    apply_TRANSDATE(domFilter);
    apply_UPDATE(domFilter);

    if (m_conf.hasOpt(ADMIN_ID_NAME))
        domFilter->addAdminContact().addId().setValue(
                Database::ID(m_conf.get<unsigned int>(ADMIN_ID_NAME)));
    if (m_conf.hasOpt(ADMIN_HANDLE_NAME))
        domFilter->addAdminContact().addHandle().setValue(
                m_conf.get<std::string>(ADMIN_HANDLE_NAME));
    if (m_conf.hasOpt(ADMIN_NAME_NAME))
        domFilter->addAdminContact().addName().setValue(
                m_conf.get<std::string>(ADMIN_NAME_NAME));

    apply_DATE(domFilter, DOMAIN_OUT_DATE_NAME, OutZone);
    apply_DATE(domFilter, DOMAIN_EXP_DATE_NAME, Expiration);
    apply_DATE(domFilter, DOMAIN_CANC_DATE_NAME, Cancel);

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(domFilter);

    apply_LIMIT(domList);
    domList->reload(*unionFilter);

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

void
DomainClient::domain_create()
{
    callHelp(m_conf, domain_create_help);
    std::string fqdn = m_conf.get<std::string>(DOMAIN_CREATE_NAME).c_str();
    std::string registrant = m_conf.get<std::string>(DOMAIN_REGISTRANT_NAME).c_str();
    std::string nsset = m_conf.get<std::string>(DOMAIN_NSSET_NAME).c_str();
    std::string keyset = m_conf.get<std::string>(DOMAIN_KEYSET_NAME).c_str();
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
    return;
}

void
DomainClient::domain_update()
{
    callHelp(m_conf, domain_update_help);
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
    return;
}
void
DomainClient::domain_info()
{
    callHelp(m_conf, no_help);
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
    return;
}
void
DomainClient::domain_list_plain()
{
    callHelp(m_conf, no_help);
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
    return;
}

void
DomainClient::domain_update_help()
{
    std::cout
        << "** Domain update **\n\n"
        << "  " << g_prog_name << " --" << DOMAIN_UPDATE_NAME << "=<domain_fqdn> \\\n"
        << "    [--" << REGISTRAR_HANDLE_NAME << "=<registrant_handle> \\\n"
        << "    [--" << NSSET_HANDLE_NAME << "=<new_nsset>] \\\n"
        << "    [--" << KEYSET_HANDLE_NAME << "=<new_keyset>] \\\n"
        << "    [--" << AUTH_PW_NAME << "=<new_authinfo_password>] \\\n"
        << "    [--" << ADMIN_ADD_NAME << "=<list_of_admins_to_add>] \\\n"
        << "    [--" << ADMIN_REM_NAME << "=<list_of_admins_to_rem>] \\\n"
        << "    [--" << ADMIN_REM_TEMP_NAME << "=<list_of_temp_admins_to_rem>]"
        << std::endl;
}

void
DomainClient::domain_create_help()
{
    std::cout
        << "** Domain create **\n\n"
        << "  " << g_prog_name << " --" << DOMAIN_CREATE_NAME << "=<domain_fqdn> \\\n"
        << "    --" << DOMAIN_REGISTRANT_NAME << "=<registrant_handle> \\\n"
        << "    [--" << NSSET_HANDLE_NAME << "=<nsset_handle>] \\\n"
        << "    [--" << KEYSET_HANDLE_NAME << "=<keyset_handle>] \\\n"
        << "    [--" << AUTH_PW_NAME << "=<authinfo_password>] \\\n"
        << "    --" << ADMIN_NAME_DESC << "=<list_of_admins_contact_handles> \\\n"
        << "    --" << DOMAIN_PERIOD_NAME << "=<period_in_months>\n\n"
        << "Domain creation example:\n"
        << "\t$ " << g_prog_name << " --" << DOMAIN_CREATE_NAME << "=example.cz "
        << "--" << ADMIN_NAME_DESC << "=\"CON::001 CON::005\" --"
        << NSSET_HANDLE_NAME << "\"NSS::137\" "
        << "--" << DOMAIN_PERIOD_NAME << "=24 --" << DOMAIN_REGISTRANT_NAME << "=\"CON::005\"\n"
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
        << "    [--" << CRDATE_NAME << "=<create_date>] \\\n"
        << "    [--" << DELDATE_NAME << "=<delete_date>] \\\n"
        << "    [--" << UPDATE_NAME << "=<update_date>] \\\n"
        << "    [--" << TRANSDATE_NAME << "=<transfer_date>] \\\n"
        << "    [--" << FULL_LIST_NAME << "]\n"
        << std::endl;
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_DOMAIN, name, name##_DESC, type, callable, visible}

const struct options
DomainClient::m_opts[] = {
    ADDOPT(DOMAIN_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(DOMAIN_LIST_PLAIN_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(DOMAIN_INFO_NAME, TYPE_STRING, true, true),
    ADDOPT(DOMAIN_CREATE_NAME, TYPE_STRING, true, true),
    ADDOPT(DOMAIN_UPDATE_NAME, TYPE_STRING, true, true),
    ADDOPT(DOMAIN_LIST_PLAIN_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(DOMAIN_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(DOMAIN_OUT_DATE_NAME, TYPE_STRING, false, false),
    ADDOPT(DOMAIN_EXP_DATE_NAME, TYPE_STRING, false, false),
    ADDOPT(DOMAIN_CANC_DATE_NAME, TYPE_STRING, false, false),
    add_ID,
    add_FQDN,
    add_NSSET_ID,
    add_NSSET_HANDLE,
    add_ANY_NSSET,
    add_KEYSET_ID,
    add_KEYSET_HANDLE,
    add_ANY_KEYSET,
    add_REGISTRANT_ID,
    add_REGISTRANT_NAME,
    add_REGISTRANT_HANDLE,
    add_ADMIN_ID,
    add_ADMIN_HANDLE,
    add_ADMIN_NAME,
    add_REGISTRAR_ID,
    add_REGISTRAR_HANDLE,
    add_REGISTRAR_NAME,
    add_ADMIN,
    add_ADMIN_ADD,
    add_ADMIN_REM,
    add_CRDATE,
    add_UPDATE,
    add_DELDATE,
    add_TRANSDATE
};

#undef ADDOPT

int 
DomainClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}
} // namespace Admin;
