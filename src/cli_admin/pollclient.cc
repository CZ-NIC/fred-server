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
#include "pollclient.h"
#include "register/poll.h"

namespace Admin {

const struct options *
PollClient::getOpts()
{
    return m_opts;
}

void
PollClient::runMethod()
{
    if (m_conf.hasOpt(POLL_LIST_ALL_NAME)) {
        list_all();
    } else if (m_conf.hasOpt(POLL_LIST_NEXT_NAME)) {
        list_next();
    } else if (m_conf.hasOpt(POLL_LIST_NEXT_HANDLE_NAME)) {
        list_next();
    } else if (m_conf.hasOpt(POLL_CREATE_STATE_CHANGES_NAME) ||
            m_conf.hasOpt(POLL_CREATE_STATE_CHANGES_2_NAME)) {
        create_state_changes();
    } else if (m_conf.hasOpt(POLL_CREATE_LOW_CREDIT_NAME) ||
            m_conf.hasOpt(POLL_CREATE_LOW_CREDIT_2_NAME)) {
        create_low_credit();
    } else if (m_conf.hasOpt(POLL_SET_SEEN_NAME)) {
        set_seen();
    } else if (m_conf.hasOpt(POLL_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
PollClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("Poll", getOpts(), getOptsCount());
}

void
PollClient::list_all()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
    std::auto_ptr<Register::Poll::List> pmList(pollMan->createList());
    if (m_conf.hasOpt(POLL_TYPE_NAME))
        pmList->setTypeFilter(m_conf.get<unsigned int>(POLL_TYPE_NAME));
    if (m_conf.hasOpt(REGISTRAR_ID_NAME))
        pmList->setRegistrarFilter(m_conf.get<unsigned int>(REGISTRAR_ID_NAME));
    if (m_conf.hasOpt(POLL_NONSEEN_NAME))
        pmList->setNonSeenFilter(true);
    if (m_conf.hasOpt(POLL_NONEX_NAME))
        pmList->setNonExpiredFilter(true);
    pmList->reload();
    for (unsigned int i = 0; i < pmList->getCount(); i++) {
        Register::Poll::Message *msg = pmList->getMessage(i);
        if (msg) {
            msg->textDump(std::cout);
            std::cout << std::endl;
        }
    }
    return;
}
void
PollClient::list_next()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
    if (m_conf.hasOpt(POLL_LIST_NEXT_NAME)) {
        Register::TID reg = m_conf.get<unsigned int>(POLL_LIST_NEXT_NAME);
        unsigned int count = pollMan->getMessageCount(reg);
        if (!count) {
            std::cout << "No message" << std::endl;
        } else {
            std::auto_ptr<Register::Poll::Message> msg(pollMan->getNextMessage(reg));
            std::cout << "Messages:" << count << std::endl;
            msg->textDump(std::cout);
            std::cout << std::endl;
        }
    } else if (m_conf.hasOpt(POLL_LIST_NEXT_HANDLE_NAME)) {
        std::string reg = m_conf.get<std::string>(POLL_LIST_NEXT_HANDLE_NAME);
        unsigned int count = pollMan->getMessageCount(reg);
        if (!count) {
            std::cout << "No message" << std::endl;
        } else {
            std::auto_ptr<Register::Poll::Message> msg(pollMan->getNextMessage(reg));
            std::cout << "Messages:" << count << std::endl;
            msg->textDump(std::cout);
            std::cout << std::endl;
        }
    } else {
        std::cerr << "Registrar is not set, use ``--" << REGISTRAR_ID_NAME
            << "'' or ``--" << REGISTRAR_HANDLE_NAME << "''" << std::endl;
        return;
    }
}
void
PollClient::set_seen()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
    try {
        Register::TID messageId = m_conf.get<unsigned int>(POLL_SET_SEEN_NAME); 
        if (m_conf.hasOpt(REGISTRAR_ID_NAME)) {
            Register::TID reg = m_conf.get<unsigned int>(REGISTRAR_ID_NAME);
            pollMan->setMessageSeen(messageId, reg);
            std::cout << "NextId:" << pollMan->getNextMessageId(reg) << std::endl;
        } else if (m_conf.hasOpt(REGISTRAR_HANDLE_NAME)) {
            std::string reg = m_conf.get<std::string>(REGISTRAR_ID_NAME);
            pollMan->setMessageSeen(messageId, reg);
            std::cout << "NextId:" << pollMan->getNextMessageId(reg) << std::endl;
        } else {
            std::cerr << "Registrar is not set, use ``--" << REGISTRAR_ID_NAME
                << "'' or ``--" << REGISTRAR_HANDLE_NAME << "''" << std::endl;
            return;
        }
    } catch (...) {
        std::cout << "No message" << std::endl;
    }
}
void
PollClient::create_state_changes()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
    std::string exceptTypes("");
    if (m_conf.hasOpt(POLL_EXCEPT_TYPES_NAME)) {
        exceptTypes = m_conf.get<std::string>(POLL_EXCEPT_TYPES_NAME);
    }
    int limit = 0;
    if (m_conf.hasOpt(POLL_LIMIT_NAME)) {
        limit = m_conf.get<unsigned int>(POLL_LIMIT_NAME);
    }
    pollMan->createStateMessages(
            exceptTypes, limit,
            m_conf.hasOpt(POLL_DEBUG_NAME) ? &std::cout : NULL
    );
    return;
}
void
PollClient::create_low_credit()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
    pollMan->createLowCreditMessages();
    return;
}
#define ADDOPT(name, type, callable, visible) \
    {CLIENT_POLL, name, name##_DESC, type, callable, visible}

const struct options
PollClient::m_opts[] = {
    add_REGISTRAR_ID,
    add_REGISTRAR_HANDLE,
    ADDOPT(POLL_LIST_ALL_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(POLL_LIST_NEXT_NAME, TYPE_UINT, true, true),
    ADDOPT(POLL_LIST_NEXT_HANDLE_NAME, TYPE_STRING, true, true),
    ADDOPT(POLL_SET_SEEN_NAME, TYPE_UINT, true, true),
    ADDOPT(POLL_CREATE_STATE_CHANGES_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(POLL_CREATE_STATE_CHANGES_2_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(POLL_CREATE_LOW_CREDIT_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(POLL_CREATE_LOW_CREDIT_2_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(POLL_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(POLL_TYPE_NAME, TYPE_UINT, false, false),
    ADDOPT(POLL_REGID_NAME, TYPE_UINT, false, false),
    ADDOPT(POLL_NONSEEN_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(POLL_NONEX_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(POLL_DEBUG_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(POLL_EXCEPT_TYPES_NAME, TYPE_STRING, false, false),
    ADDOPT(POLL_LIMIT_NAME, TYPE_UINT, false, false),
};

#undef ADDOPT

int 
PollClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

