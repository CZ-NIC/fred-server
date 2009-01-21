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
#include "notifyclient.h"
#include "register/bank.h"
#include "register/info_buffer.h"

namespace Admin {

NotifyClient::NotifyClient()
{
    m_options = new boost::program_options::options_description(
            "Notify related options");
    m_options->add_options()
        addOpt(NOTIFY_STATE_CHANGES_NAME)
        addOpt(NOTIFY_LETTERS_CREATE_NAME)
        addOpt(NOTIFY_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Notify related sub options");
    m_optionsInvis->add_options()
        addOpt(NOTIFY_DEBUG_NAME)
        addOptStrDef(NOTIFY_EXCEPT_TYPES_NAME, "")
        addOpt(NOTIFY_EXCEPT_TYPES_NAME)
        addOptUIntDef(NOTIFY_LIMIT_NAME, 0);
}
NotifyClient::NotifyClient(
        std::string connstring,
        std::string nsAddr) : BaseClient(connstring, nsAddr)
{
    m_db.OpenDatabase(connstring.c_str());
    m_options = NULL;
    m_optionsInvis = NULL;
}

NotifyClient::~NotifyClient()
{
    delete m_options;
    delete m_optionsInvis;
}

void
NotifyClient::init(
        std::string connstring,
        std::string nsAddr,
        Config::Conf &conf)
{
    BaseClient::init(connstring, nsAddr);
    m_db.OpenDatabase(connstring.c_str());
    m_conf = conf;
}

boost::program_options::options_description *
NotifyClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
NotifyClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
NotifyClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
}

void
NotifyClient::state_changes()
{
    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    MailerManager mailMan(cc.getNS());
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::auto_ptr<Register::Domain::Manager> domMan(
            Register::Domain::Manager::create(&m_db, zoneMan.get()));
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    std::auto_ptr<Register::Notify::Manager> notifyMan(
            Register::Notify::Manager::create(
                &m_db,
                &mailMan,
                conMan.get(),
                nssMan.get(),
                keyMan.get(),
                domMan.get(),
                docMan.get(),
                regMan.get())
            );
    notifyMan->notifyStateChanges(
            m_conf.get<std::string>(NOTIFY_EXCEPT_TYPES_NAME),
            m_conf.get<unsigned int>(NOTIFY_LIMIT_NAME),
            m_conf.hasOpt(NOTIFY_DEBUG_NAME) ? &std::cout : NULL,
            m_conf.hasOpt(NOTIFY_USE_HISTORY_TABLES_NAME));
}

void
NotifyClient::letters_create()
{
    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    MailerManager mailMan(cc.getNS());
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::auto_ptr<Register::Domain::Manager> domMan(
            Register::Domain::Manager::create(&m_db, zoneMan.get()));
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    std::auto_ptr<Register::Notify::Manager> notifyMan(
            Register::Notify::Manager::create(
                &m_db,
                &mailMan,
                conMan.get(),
                nssMan.get(),
                keyMan.get(),
                domMan.get(),
                docMan.get(),
                regMan.get())
            );
    notifyMan->generateLetters();
}

} // namespace Admin;

