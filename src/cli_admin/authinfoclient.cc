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
#include "authinfoclient.h"
#include "commonclient.h"
#include "corba/mailer_manager.h"
#include "register/register.h"

namespace Admin {

const struct options *
AuthInfoClient::getOpts()
{
    return m_opts;
}

void
AuthInfoClient::runMethod()
{
    if (m_conf.hasOpt(AUTHINFO_PDF_NAME)) {
        pdf();
    } else if (m_conf.hasOpt(AUTHINFO_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
AuthInfoClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("AuthInfo", getOpts(), getOptsCount());
}

void
AuthInfoClient::pdf()
{
    callHelp(m_conf, pdf_help);
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
    return;
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

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_AUTHINFO, name, name##_DESC, type, callable, visible}

const struct options
AuthInfoClient::m_opts[] = {
    ADDOPT(AUTHINFO_PDF_NAME, TYPE_UINT, true, true),
    ADDOPT(AUTHINFO_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
};

#undef ADDOPT

int 
AuthInfoClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}
} // namespace Admin;
