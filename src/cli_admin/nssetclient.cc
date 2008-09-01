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
        add_opt_type(CONTACT_ID_NAME, unsigned int)
        add_opt_type(CONTACT_HANDLE_NAME, std::string)
        add_opt_type(CONTACT_NAME_NAME, std::string)
        add_opt_type(FQDN_NAME, std::string)
        add_opt_type(IP_NAME, std::string);
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
    if (m_varMap.count(CONTACT_ID_NAME))
        nssFilter->addTechContact().addId().setValue(
                Database::ID(m_varMap[CONTACT_ID_NAME].as<unsigned int>()));
    if (m_varMap.count(CONTACT_HANDLE_NAME))
        nssFilter->addTechContact().addHandle().setValue(
                m_varMap[CONTACT_HANDLE_NAME].as<std::string>());
    if (m_varMap.count(CONTACT_NAME_NAME))
        nssFilter->addTechContact().addName().setValue(
                m_varMap[CONTACT_NAME_NAME].as<std::string>());
    if (m_varMap.count(FQDN_NAME))
        nssFilter->addHostFQDN().setValue(
                m_varMap[FQDN_NAME].as<std::string>());
    if (m_varMap.count(IP_NAME))
        nssFilter->addHostIP().setValue(
                m_varMap[IP_NAME].as<std::string>());

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
        << "    [--" << CONTACT_ID_NAME << "=<contact_id_number>] \\\n"
        << "    [--" << CONTACT_HANDLE_NAME << "=<contact_handle>] \\\n"
        << "    [--" << CONTACT_NAME_NAME << "=<contact_name>] \\\n"
        << "    [--" << FQDN_NAME << "=<fqdn>] \\\n"
        << "    [--" << IP_NAME << "=<ip>]\n"
        << std::endl;
}

} // namespace Admin;

