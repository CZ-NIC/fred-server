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

#include "register/register.h"
#include "commonclient.h"
#include "authinfoclient.h"

namespace Admin {

#define LOGIN_AUTHINFOCLIENT \
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

#define LOGOUT_AUTHINFOCLIENT \
    epp->ClientLogout(clientId,"system_delete_logout","<system_delete_logout/>");

AuthInfoClient::AuthInfoClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Authinfo related options");
    m_options->add_options()
        ADD_OPT_TYPE(AUTHINFO_PDF_NAME, "generate pdf of auth info request", unsigned int)
        ADD_OPT(AUTHINFO_PDF_HELP_NAME, "help for authinfo pdf creation");

    m_optionsInvis = new boost::program_options::options_description(
            "Authinfo related invisible options");
    m_optionsInvis->add_options();
}
AuthInfoClient::AuthInfoClient(
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

AuthInfoClient::~AuthInfoClient()
{
}

void
AuthInfoClient::init(
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
AuthInfoClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
AuthInfoClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

int
AuthInfoClient::pdf()
{
    std::ofstream stdout("/dev/stdout",std::ios::out);   

    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_varMap[DOCGEN_PATH_NAME].as<std::string>(),
                m_varMap[DOCGEN_TEMPLATE_PATH_NAME].as<std::string>(),
                m_varMap[FILECLIENT_PATH_NAME].as<std::string>(),
                m_nsAddr)
            );

    CorbaClient cc(0, NULL, m_nsAddr);
    MailerManager mailMan(cc.getNS());

    std::auto_ptr<Register::AuthInfoRequest::Manager> authMan(
            Register::AuthInfoRequest::Manager::create(
                &m_db,
                &mailMan,
                docMan.get())
            );
    authMan->getRequestPDF(
            m_varMap[AUTHINFO_PDF_NAME].as<unsigned int>(),
            m_varMap[LANGUAGE_NAME].as<std::string>(),
            stdout);
    return 0;
}

void
AuthInfoClient::pdf_help()
{
    std::cout <<
        "** Generate PDF for authorization requests **\n\n"
        "  $ " << g_prog_name << " --" << AUTHINFO_PDF_NAME << "=<number> \\\n"
        "    --" << LANGUAGE_NAME << "=<lang_code> \n"
        << std::endl;
}

} // namespace Admin;
