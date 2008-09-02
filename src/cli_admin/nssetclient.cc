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
#include "nssetclient.h"

namespace Admin {

NssetClient::NssetClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "NSSet related options");
    m_options->add_options()
        add_opt(NSSET_LIST_NAME)
        add_opt(NSSET_LIST_HELP_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "NSSet related invisible options");
    m_optionsInvis->add_options()
        add_opt_type(ID_NAME, unsigned int)
        add_opt_type(HANDLE_NAME, std::string)
        add_opt_type(FQDN_NAME, std::string)
        add_opt_type(IP_NAME, std::string)
        add_opt_type(ADMIN_ID_NAME, unsigned int)
        add_opt_type(ADMIN_HANDLE_NAME, std::string)
        add_opt_type(ADMIN_NAME_NAME, std::string)
        add_opt_type(REGISTRAR_ID_NAME, unsigned int)
        add_opt_type(REGISTRAR_HANDLE_NAME, std::string)
        add_opt_type(REGISTRAR_NAME_NAME, std::string);
}

NssetClient::NssetClient(
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

NssetClient::~NssetClient()
{
    delete m_dbman;
}

void
NssetClient::init(
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
NssetClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
NssetClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

int
NssetClient::list()
{
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                (bool)m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::NSSet::List> nssList(
            nssMan->createList());
    Database::Filters::NSSet *nssFilter;
    nssFilter = new Database::Filters::NSSetHistoryImpl();

    if (m_varMap.count(ID_NAME))
        nssFilter->addId().setValue(
                Database::ID(m_varMap[ID_NAME].as<unsigned int>()));
    if (m_varMap.count(HANDLE_NAME))
        nssFilter->addHandle().setValue(
                m_varMap[HANDLE_NAME].as<std::string>());
    
    if (m_varMap.count(ADMIN_ID_NAME))
        nssFilter->addTechContact().addId().setValue(
                Database::ID(m_varMap[ADMIN_ID_NAME].as<unsigned int>()));
    if (m_varMap.count(ADMIN_HANDLE_NAME))
        nssFilter->addTechContact().addHandle().setValue(
                m_varMap[ADMIN_HANDLE_NAME].as<std::string>());
    if (m_varMap.count(ADMIN_NAME_NAME))
        nssFilter->addTechContact().addName().setValue(
                m_varMap[ADMIN_NAME_NAME].as<std::string>());

    if (m_varMap.count(FQDN_NAME))
        nssFilter->addHostFQDN().setValue(
                m_varMap[FQDN_NAME].as<std::string>());

    if (m_varMap.count(IP_NAME))
        nssFilter->addHostIP().setValue(
                m_varMap[IP_NAME].as<std::string>());

    if (m_varMap.count(REGISTRAR_ID_NAME))
        nssFilter->addRegistrar().addId().setValue(
                Database::ID(m_varMap[REGISTRAR_ID_NAME].as<unsigned int>()));
    if (m_varMap.count(REGISTRAR_HANDLE_NAME))
        nssFilter->addRegistrar().addHandle().setValue(
                m_varMap[REGISTRAR_HANDLE_NAME].as<std::string>());
    if (m_varMap.count(REGISTRAR_NAME_NAME))
        nssFilter->addRegistrar().addName().setValue(
                m_varMap[REGISTRAR_NAME_NAME].as<std::string>());

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(nssFilter);
    nssList->setLimit(m_varMap[LIMIT_NAME].as<unsigned int>());

    nssList->reload(*unionFilter, m_dbman);
    std::cout << "<objects>" << std::endl;

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
        if (m_varMap.count(FULL_LIST_NAME)) {
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

    return 0;
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
        << "    [--" << FULL_LIST_NAME << "]\n"
        << std::endl;
}

} // namespace Admin;

