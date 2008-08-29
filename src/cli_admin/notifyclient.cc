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
#include "notifyclient.h"
#include "register/bank.h"
#include "register/info_buffer.h"

namespace Admin {

NotifyClient::NotifyClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Notify related options");
    m_options->add_options()
        ADD_OPT(NOTIFY_STATE_CHANGES_NAME, "send emails to contacts about object state changes")
        ADD_OPT(NOTIFY_LETTERS_CREATE_NAME, "generate pdf with domain registration warning");

    m_optionsInvis = new boost::program_options::options_description(
            "Notify related invisible options");
    m_optionsInvis->add_options()
        ADD_OPT_DEF(NOTIFY_EXCEPT_TYPES_NAME, "list of notification types ignored in notification", std::string, "4,5")
        ADD_OPT(NOTIFY_USE_HISTORY_TABLES_NAME, "slower queries into history tables, but can handle deleted objects")
        ADD_OPT_DEF(NOTIFY_LIMIT_NAME, "limit for nubmer of emails generated in one pass (0=no limit)", unsigned int, 0);

}
NotifyClient::NotifyClient(
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

NotifyClient::~NotifyClient()
{
}

void
NotifyClient::init(
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
NotifyClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
NotifyClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

int
NotifyClient::state_changes()
{
    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_varMap[DOCGEN_PATH_NAME].as<std::string>(),
                m_varMap[DOCGEN_TEMPLATE_PATH_NAME].as<std::string>(),
                m_varMap[FILECLIENT_PATH_NAME].as<std::string>(),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr);
    MailerManager mailMan(cc.getNS());
    // std::auto_ptr<Register::Invoicing::Manager> invMan(
            // Register::Invoicing::Manager::create(&m_db, docMan.get(), &mailMan));
    // std::auto_ptr<Register::AuthInfoRequest::Manager> authMan(
            // Register::AuthInfoRequest::Manager::create(&m_db, &mailMan, docMan.get()));
    // std::auto_ptr<Register::Banking::Manager> bankMan(
            // Register::Banking::Manager::create(&m_db));
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::auto_ptr<Register::Domain::Manager> domMan(
            Register::Domain::Manager::create(&m_db, zoneMan.get()));
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(
                &m_db,
                (bool)m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                (bool)m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                (bool)m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    // std::auto_ptr<Register::InfoBuffer::Manager> infoBufMan(
            // Register::InfoBuffer::Manager::create(
                // &m_db,
                // domMan.get(),
                // nssMan.get(),
                // conMan.get(),
                // keyMan.get())
            // );
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
            m_varMap[NOTIFY_EXCEPT_TYPES_NAME].as<std::string>(),
            m_varMap[NOTIFY_LIMIT_NAME].as<unsigned int>(),
            m_varMap.count(DEBUG_NAME) ? &std::cout : NULL,
            m_varMap.count(NOTIFY_USE_HISTORY_TABLES_NAME));
    return 0;
}
int
NotifyClient::letters_create()
{
    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_varMap[DOCGEN_PATH_NAME].as<std::string>(),
                m_varMap[DOCGEN_TEMPLATE_PATH_NAME].as<std::string>(),
                m_varMap[FILECLIENT_PATH_NAME].as<std::string>(),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr);
    MailerManager mailMan(cc.getNS());
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::auto_ptr<Register::Domain::Manager> domMan(
            Register::Domain::Manager::create(&m_db, zoneMan.get()));
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(
                &m_db,
                (bool)m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                (bool)m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                (bool)m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
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
    return 0;
}

} // namespace Admin;

