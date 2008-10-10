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
#include "authinfoclient.h"
#include "commonclient.h"
#include "corba/mailer_manager.h"
#include "register/register.h"

namespace Admin {

AuthInfoClient::AuthInfoClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Authinfo related options");
    m_options->add_options()
        addOptUInt(AUTHINFO_PDF_NAME)
        addOpt(AUTHINFO_PDF_HELP_NAME)
        addOpt(AUTHINFO_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Authinfo related sub options");
    m_optionsInvis->add_options();
}
AuthInfoClient::AuthInfoClient(
        std::string connstring,
        std::string nsAddr):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_options = NULL;
    m_optionsInvis = NULL;
}

AuthInfoClient::~AuthInfoClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
AuthInfoClient::init(
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
AuthInfoClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
AuthInfoClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
AuthInfoClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
}

int
AuthInfoClient::pdf()
{
    std::ofstream stdout("/dev/stdout",std::ios::out);   

    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                m_nsAddr)
            );

    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    MailerManager mailMan(cc.getNS());

    std::auto_ptr<Register::AuthInfoRequest::Manager> authMan(
            Register::AuthInfoRequest::Manager::create(
                &m_db,
                &mailMan,
                docMan.get())
            );
    authMan->getRequestPDF(
            m_conf.get<unsigned int>(AUTHINFO_PDF_NAME),
            m_conf.get<std::string>(CLI_LANGUAGE_NAME),
            stdout);
    return 0;
}

void
AuthInfoClient::pdf_help()
{
    std::cout <<
        "** Generate PDF for authorization requests **\n\n"
        "  $ " << g_prog_name << " --" << AUTHINFO_PDF_NAME << "=<number> \\\n"
        "    --" << CLI_LANGUAGE_NAME << "=<lang_code> \n"
        << std::endl;
}

} // namespace Admin;
