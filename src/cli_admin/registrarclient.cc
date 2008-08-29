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
#include "registrarclient.h"

namespace Admin {

#define LOGIN_REGISTRARCLIENT \
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

#define LOGOUT_REGISTRARCLIENT \
    epp->ClientLogout(clientId,"system_delete_logout","<system_delete_logout/>");

RegistrarClient::RegistrarClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Registrar related options");
    m_options->add_options()
        ADD_OPT_TYPE(REGISTRAR_ZONE_ADD_NAME, "Add new zone", std::string)
        ADD_OPT_TYPE(REGISTRAR_REGISTRAR_ADD_NAME, "Add new registrar (make a copy of REG-FRED_A)", std::string)
        ADD_OPT(REGISTRAR_REGISTRAR_ADD_ZONE_NAME, "Add access rights right for registrar to zone")
        ADD_OPT(REGISTRAR_ZONE_ADD_HELP_NAME, "")
        ADD_OPT(REGISTRAR_REGISTRAR_ADD_HELP_NAME, "")
        ADD_OPT(REGISTRAR_REGISTRAR_ADD_ZONE_HELP_NAME, "");

    m_optionsInvis = new boost::program_options::options_description(
            "Registrar related invisible options");
    m_optionsInvis->add_options()
        ADD_OPT_TYPE(ZONE_HANDLE_NAME, "Zone handle", std::string)
        ADD_OPT_TYPE(REGISTRAR_HANDLE_NAME, "Registrar handle", std::string);
}
RegistrarClient::RegistrarClient(
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

RegistrarClient::~RegistrarClient()
{
}

void
RegistrarClient::init(
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
RegistrarClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
RegistrarClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

int
RegistrarClient::zone_add()
{
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::string fqdn = m_varMap[REGISTRAR_ZONE_ADD_NAME].as<std::string>();
    try {
        zoneMan->addZone(fqdn);
    } catch (Register::ALREADY_EXISTS) {
        std::cerr << "Zone '" << fqdn << "' already exists in configuratin" << std::endl;
    }
    return 0;
}
int
RegistrarClient::registrar_add()
{
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    regMan->addRegistrar(m_varMap[REGISTRAR_REGISTRAR_ADD_NAME].as<std::string>());
    return 0;
}
int
RegistrarClient::registrar_add_zone()
{
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    std::string zone = m_varMap[ZONE_HANDLE_NAME].as<std::string>();
    std::string registrar = m_varMap[REGISTRAR_HANDLE_NAME].as<std::string>();
    regMan->addRegistrarZone(registrar, zone);
    return 0;
}

void
RegistrarClient::zone_add_help()
{
    std::cout <<
        "** Add new zone **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_ZONE_ADD_NAME << "=<zone_fqdn>\n"
        << std::endl;
}

void 
RegistrarClient::registrar_add_help()
{
    std::cout <<
        "** Add new registrar **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_REGISTRAR_ADD_NAME << "=<registrar_handle>\n"
        << std::endl;
}
void 
RegistrarClient::registrar_add_zone_help()
{
    std::cout <<
        "** Add registrar to zone **\n\n"
        "  $ " << g_prog_name << " --" << REGISTRAR_REGISTRAR_ADD_ZONE_NAME << " \\\n"
        "    --" << ZONE_HANDLE_NAME << "=<zone_fqdn> \\\n"
        "    --" << REGISTRAR_HANDLE_NAME << "=<registrar_handle>\n"
        << std::endl;
}

} // namespace Admin;

