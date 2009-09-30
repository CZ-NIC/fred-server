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
#include "nssetclient.h"

namespace Admin {

const struct options *
NssetClient::getOpts()
{
    return m_opts;
}

void
NssetClient::runMethod()
{
    if (m_conf.hasOpt(NSSET_LIST_NAME)) {
        list();
    } else if (m_conf.hasOpt(NSSET_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
NssetClient::show_opts() 
{
    callHelp(m_conf, no_help);
    print_options("Nsset", getOpts(), getOptsCount());
}

void
NssetClient::list()
{
    callHelp(m_conf, list_help);
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create());
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::NSSet::List> nssList(
            nssMan->createList());
    Database::Filters::NSSet *nssFilter;
    nssFilter = new Database::Filters::NSSetHistoryImpl();

    apply_ID(nssFilter);
    apply_HANDLE(nssFilter);
    apply_REGISTRAR_ID(nssFilter);
    apply_REGISTRAR_HANDLE(nssFilter);
    apply_REGISTRAR_NAME(nssFilter);
    apply_CRDATE(nssFilter);
    apply_DELDATE(nssFilter);
    apply_UPDATE(nssFilter);
    apply_TRANSDATE(nssFilter);

    if (m_conf.hasOpt(ADMIN_ID_NAME))
        nssFilter->addTechContact().addId().setValue(
                Database::ID(m_conf.get<unsigned int>(ADMIN_ID_NAME)));
    if (m_conf.hasOpt(ADMIN_HANDLE_NAME))
        nssFilter->addTechContact().addHandle().setValue(
                m_conf.get<std::string>(ADMIN_HANDLE_NAME));
    if (m_conf.hasOpt(ADMIN_NAME_NAME))
        nssFilter->addTechContact().addName().setValue(
                m_conf.get<std::string>(ADMIN_NAME_NAME));

    if (m_conf.hasOpt(FQDN_NAME))
        nssFilter->addHostFQDN().setValue(
                m_conf.get<std::string>(FQDN_NAME));

    if (m_conf.hasOpt(IP_NAME))
        nssFilter->addHostIP().setValue(
                m_conf.get<std::string>(IP_NAME));


    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(nssFilter);
    nssList->setLimit(m_conf.get<unsigned int>(LIMIT_NAME));

    nssList->reload(*unionFilter);

    std::cout << "<objects>\n";
    for (unsigned int i = 0; i < nssList->getCount(); i++) {
        Register::NSSet::NSSet *nsset = nssList->getNSSet(i);
        std::cout
            << "\t<nsset>\n"
            << "\t\t<id>" << nsset->getId() << "</id>\n"
            << "\t\t<handle>" << nsset->getHandle() << "</handle>\n"
            << "\t\t<check_level>" << nsset->getCheckLevel() << "</check_level>\n";
        for (unsigned int j = 0; j < nsset->getAdminCount(); j++) {
            std::cout
                << "\t\t<admin>\n"
                << "\t\t\t<id>" << nsset->getAdminIdByIdx(j) << "</id>\n"
                << "\t\t\t<handle>" << nsset->getAdminHandleByIdx(j) << "</handle>\n"
                << "\t\t</admin>\n";
        }
        for (unsigned int j = 0; j < nsset->getHostCount(); j++) {
            Register::NSSet::Host *host = (Register::NSSet::Host *)nsset->getHostByIdx(j);
            std::cout
                << "\t\t<host>\n"
                << "\t\t\t<name>" << host->getName() << "</name>\n"
                << "\t\t\t<name_idn>" << host->getNameIDN() << "</name_idn>\n";
            for (unsigned int n = 0; n < host->getAddrCount(); n++) {
                std::cout
                    << "\t\t\t<address>" << host->getAddrByIdx(n) << "</address>\n";
            }
            std::cout
                << "\t\t</host>\n";
        }
        if (m_conf.hasOpt(FULL_LIST_NAME)) {
            std::cout
                << "\t\t<create_date>" << nsset->getCreateDate() << "</create_date>\n"
                << "\t\t<transfer_date>" << nsset->getTransferDate() << "</transfer_date>\n"
                << "\t\t<update_date>" << nsset->getUpdateDate() << "</update_date>\n"
                << "\t\t<delete_date>" << nsset->getDeleteDate() << "</delete_date>\n"
                << "\t\t<registrar>\n"
                << "\t\t\t<id>" << nsset->getRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << nsset->getRegistrarHandle() << "</handle>\n"
                << "\t\t</registrar>\n"
                << "\t\t<create_registrar>\n"
                << "\t\t\t<id>" << nsset->getCreateRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << nsset->getCreateRegistrarHandle() << "</handle>\n"
                << "\t\t</create_registrar>\n"
                << "\t\t<update_registrar>\n"
                << "\t\t\t<id>" << nsset->getUpdateRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << nsset->getUpdateRegistrarHandle() << "</handle>\n"
                << "\t\t</update_registrar>\n"
                << "\t\t<auth_password>" << nsset->getAuthPw() << "</auth_password>\n"
                << "\t\t<ROID>" << nsset->getROID() << "</ROID>\n";
            for (unsigned int j = 0; j < nsset->getStatusCount(); j++) {
                Register::Status *status = (Register::Status *)nsset->getStatusByIdx(j);
                std::cout
                    << "\t\t<status>\n"
                    << "\t\t\t<id>" << status->getStatusId() << "</id>\n"
                    << "\t\t\t<from>" << status->getFrom() << "</from>\n"
                    << "\t\t\t<to>" << status->getTo() << "</to>\n"
                    << "\t\t</status>\n";
            }
        }
        std::cout
            << "\t</nsset>\n";
    }

    std::cout << "<objects>" << std::endl;
    unionFilter->clear();
    delete unionFilter;
}

void
NssetClient::list_help()
{
    std::cout
        << "** NSSet list **\n\n"
        << "  $ " << g_prog_name << " --" << NSSET_LIST_NAME << " \\\n"
        << "    [--" << ID_NAME << "=<id_nubmer>] \\\n"
        << "    [--" << HANDLE_NAME << "=<handle>] \\\n"
        << "    [--" << ADMIN_ID_NAME << "=<admin_id_number>] \\\n"
        << "    [--" << ADMIN_HANDLE_NAME << "=<admin_handle>] \\\n"
        << "    [--" << ADMIN_NAME_NAME << "=<admin_name>] \\\n"
        << "    [--" << REGISTRAR_ID_NAME << "=<registrar_id_number>] \\\n"
        << "    [--" << REGISTRAR_HANDLE_NAME << "=<registrar_handle>] \\\n"
        << "    [--" << REGISTRAR_NAME_NAME << "=<registrar_name>] \\\n"
        << "    [--" << FQDN_NAME << "=<fqdn>] \\\n"
        << "    [--" << IP_NAME << "=<ip>] \\\n"
        << "    [--" << CRDATE_NAME << "=<create_date>] \\\n"
        << "    [--" << DELDATE_NAME << "=<delete_date>] \\\n"
        << "    [--" << UPDATE_NAME << "=<update_date>] \\\n"
        << "    [--" << TRANSDATE_NAME << "=<transfer_date>] \\\n"
        << "    [--" << FULL_LIST_NAME << "]\n"
        << std::endl;
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_NSSET, name, name##_DESC, type, callable, visible}

const struct options
NssetClient::m_opts[] = {
    ADDOPT(NSSET_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(NSSET_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    add_ID,
    add_HANDLE,
    add_FQDN,
    add_IP,
    add_ADMIN_ID,
    add_ADMIN_HANDLE,
    add_ADMIN_NAME,
    add_REGISTRAR_ID,
    add_REGISTRAR_HANDLE,
    add_REGISTRAR_NAME,
    add_CRDATE,
    add_UPDATE,
    add_TRANSDATE,
    add_DELDATE,
};

#undef ADDOPT

int 
NssetClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

