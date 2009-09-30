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
#include "notifyclient.h"
#include "register/info_buffer.h"

namespace Admin {

const struct options *
NotifyClient::getOpts()
{
    return m_opts;
}


void
NotifyClient::runMethod()
{
    if (m_conf.hasOpt(NOTIFY_STATE_CHANGES_NAME)) {
        state_changes();
    } else if (m_conf.hasOpt(NOTIFY_LETTERS_CREATE_NAME)) {
        letters_create();
    } else if (m_conf.hasOpt(NOTIFY_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
NotifyClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("Notify", getOpts(), getOptsCount());
}

void
NotifyClient::state_changes()
{
    callHelp(m_conf, no_help);
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
            Register::Zone::Manager::create());
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
    std::string exceptTypes("");
    if (m_conf.hasOpt(NOTIFY_EXCEPT_TYPES_NAME)) {
        exceptTypes = m_conf.get<std::string>(NOTIFY_EXCEPT_TYPES_NAME);
    }
    int limit = 0;
    if (m_conf.hasOpt(NOTIFY_LIMIT_NAME)) {
        limit = m_conf.get<unsigned int>(NOTIFY_LIMIT_NAME);
    }
    notifyMan->notifyStateChanges(
            exceptTypes, limit,
            m_conf.hasOpt(NOTIFY_DEBUG_NAME) ? &std::cout : NULL,
            m_conf.hasOpt(NOTIFY_USE_HISTORY_TABLES_NAME));
}

void
NotifyClient::letters_create()
{
    callHelp(m_conf, no_help);
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
            Register::Zone::Manager::create());
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

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_NOTIFY, name, name##_DESC, type, callable, visible}

const struct options
NotifyClient::m_opts[] = {
    ADDOPT(NOTIFY_STATE_CHANGES_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(NOTIFY_LETTERS_CREATE_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(NOTIFY_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(NOTIFY_DEBUG_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(NOTIFY_EXCEPT_TYPES_NAME, TYPE_STRING, false, false),
    ADDOPT(NOTIFY_LIMIT_NAME, TYPE_UINT, false, false),
    ADDOPT(NOTIFY_USE_HISTORY_TABLES_NAME, TYPE_BOOL, false, false)
};

#undef ADDOPT

int 
NotifyClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

