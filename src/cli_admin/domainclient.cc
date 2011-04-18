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

//#include "simple.h"
#include "commonclient.h"
#include "domainclient.h"
#include "fredlib/registry.h"

#define addMethod(methods, name) \
    methods.insert(std::make_pair(name, DOMAIN_CLIENT))

namespace Admin {


void
DomainClient::runMethod()
{
    if (domain_list_plain_)
    {
        domain_list_plain();
    }
        else if (domain_info_.is_value_set())
    {
        domain_info();
    }
        else if (domain_list_)
    {
        domain_list();
    }
        else if (domain_show_opts_)
    {
        //show_opts();
    }
}


void
DomainClient::domain_list()
{
    std::auto_ptr<Fred::Zone::Manager> zoneMan(
            Fred::Zone::Manager::create());

    std::auto_ptr<Fred::Domain::Manager> domMan(
            Fred::Domain::Manager::create(m_db, zoneMan.get()));
    std::auto_ptr<Fred::Domain::List> domList(
            domMan->createList());

    Database::Filters::Domain *domFilter;
    domFilter = new Database::Filters::DomainHistoryImpl();

    if (domain_id_.is_value_set())
        domFilter->addId().setValue(Database::ID(domain_id_.get_value()));
    if (fqdn_.is_value_set())
        domFilter->addFQDN().setValue(fqdn_.get_value());
    if (domain_handle_.is_value_set())
        domFilter->addHandle().setValue(domain_handle_.get_value());
    if (nsset_id_.is_value_set())
        domFilter->addNSSet().addId().setValue(Database::ID(nsset_id_.get_value()));
    if (nsset_handle_.is_value_set())
        domFilter->addNSSet().addHandle().setValue(nsset_handle_.get_value());
    if (any_nsset_) domFilter->addNSSet();
    if (keyset_id_.is_value_set())
        domFilter->addKeySet().addId().setValue(Database::ID(keyset_id_.get_value()));
    if (keyset_handle_.is_value_set())
        domFilter->addKeySet().addHandle().setValue(keyset_handle_.get_value());
    if (any_keyset_) domFilter->addKeySet();
    if (zone_id_.is_value_set())
        domFilter->addZoneId().setValue(Database::ID(zone_id_.get_value()));
    if (registrant_id_.is_value_set())
        domFilter->addRegistrant().addId().setValue(Database::ID(registrant_id_.get_value()));
    if (registrant_handle_.is_value_set())
        domFilter->addRegistrant().addHandle().setValue(registrant_handle_.get_value());
    if (registrant_name_.is_value_set())
        domFilter->addRegistrant().addName().setValue(registrant_name_.get_value());
    if (crdate_.is_value_set())
        domFilter->addCreateTime().setValue(*parseDateTime(crdate_.get_value()));
    if (deldate_.is_value_set())
        domFilter->addDeleteTime().setValue(*parseDateTime(deldate_.get_value()));
    if (transdate_.is_value_set())
        domFilter->addTransferTime().setValue(*parseDateTime(transdate_.get_value()));
    if (update_.is_value_set())
        domFilter->addUpdateTime().setValue(*parseDateTime(update_.get_value()));
    if (admin_id_.is_value_set())
        domFilter->addAdminContact().addId().setValue(Database::ID(admin_id_.get_value()));
    if (admin_handle_.is_value_set())
        domFilter->addAdminContact().addHandle().setValue(admin_handle_.get_value());
    if (admin_name_.is_value_set())
        domFilter->addAdminContact().addName().setValue(admin_name_.get_value());
    if (out_zone_date_.is_value_set())
        domFilter->addOutZoneDate().setValue(*parseDate(out_zone_date_.get_value()));
    if (exp_date_.is_value_set())
        domFilter->addExpirationDate().setValue(*parseDate(exp_date_.get_value()));
    if (cancel_date_.is_value_set())
        domFilter->addCancelDate().setValue(*parseDate(cancel_date_.get_value()));

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(domFilter);

    if(limit_.is_value_set())
        domList->setLimit(limit_.get_value());

    domList->reload(*unionFilter);

    std::cout << "<objects>\n";
    for (unsigned int i = 0; i < domList->getCount(); i++) {
        Fred::Domain::Domain *domain = domList->getDomain(i);
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
        if (full_list_) {
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
                Fred::Status *status = (Fred::Status *)domain->getStatusByIdx(j);
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
DomainClient::domain_info()
{
    std::string name = domain_info_.is_value_set()
                        ? domain_info_.get_value()
                        : "";

    epp_client_login_return epp_login = epp_client_login( m_db
            , m_nsAddr
            , nameservice_context_
            , login_registrar_.is_value_set()
                ? login_registrar_.get_value()
                : "");

    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + name + "</fqdn>";
    cltrid = "info_keyset";

    ccReg::Domain *k = new ccReg::Domain;
    epp_login.epp->DomainInfo(name.c_str(), k, epp_login.clientId, cltrid.c_str(), xml.c_str());

    std::cout << k->name << std::endl;
    std::cout << k->AuthInfoPw << std::endl;
    std::cout << k->ROID << std::endl;
    std::cout << k->keyset << std::endl;

    epp_login.epp->ClientLogout(epp_login.clientId,"system_delete_logout",                      \
                "<system_delete_logout/>");
    return;
}

void
DomainClient::domain_list_plain()
{
    epp_client_login_return epp_login = epp_client_login( m_db
            , m_nsAddr
            , nameservice_context_
            , login_registrar_.is_value_set()
                ? login_registrar_.get_value()
                : "");

    std::string name = "";//option domain_list_plain have no argument
    std::string cltrid;
    std::string xml;
    xml = "<fqdn>" + name + "</fqdn>";
    cltrid = "list_domains";

    ccReg::Lists *k;

    epp_login.r = epp_login.epp->DomainList(k, epp_login.clientId, cltrid.c_str(), xml.c_str());

    for (int i = 0; i < (int)k->length(); i++)
        std::cout << (*k)[i] << std::endl;

    epp_login.epp->ClientLogout(epp_login.clientId,"system_delete_logout",
                "<system_delete_logout/>");
    return;
}

/*

 const struct options *
DomainClient::getOpts()
{
    return m_opts;
}


void
DomainClient::show_opts()
{
    print_options("Domain", getOpts(), getOptsCount());
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
*/
} // namespace Admin;
