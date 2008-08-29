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
#include "pollclient.h"
#include "register/poll.h"

namespace Admin {

PollClient::PollClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Poll related options");
    m_options->add_options()
        ADD_OPT(POLL_LIST_ALL_NAME, "List all poll messages")
        ADD_OPT_TYPE(POLL_LIST_NEXT_NAME, "List next message for given registrar", unsigned int)
        ADD_OPT_TYPE(POLL_SET_SEEN_NAME, "Set given message as seen", unsigned int)
        ADD_OPT(POLL_CREATE_STATE_CHANGES_NAME, "Create messages for state changes")
        ADD_OPT(POLL_CREATE_LOW_CREDIT_NAME, "Create message for low credit");

    m_optionsInvis = new boost::program_options::options_description(
            "Poll related invisible options");
    m_optionsInvis->add_options()
        ADD_OPT_TYPE(POLL_TYPE_NAME, "set filter for poll type", unsigned int)
        ADD_OPT_TYPE(REGISTRAR_ID_NAME, "set filter for registrar id", unsigned int)
        ADD_OPT(POLL_NONSEEN_NAME, "set filter for non seen messages")
        ADD_OPT(POLL_NONEX_NAME, "set filter for non expired messages")
        ADD_OPT_DEF(POLL_EXCEPT_TYPES_NAME, "list of poll message types ignored in creation (only states now)", std::string, "6,7");
}
PollClient::PollClient(
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

PollClient::~PollClient()
{
}

void
PollClient::init(
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
PollClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
PollClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

int
PollClient::list_all()
{
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
    std::auto_ptr<Register::Poll::List> pmList(pollMan->createList());
    if (m_varMap.count(POLL_TYPE_NAME))
        pmList->setTypeFilter(m_varMap[POLL_TYPE_NAME].as<unsigned int>());
    if (m_varMap.count(REGISTRAR_ID_NAME))
        pmList->setRegistrarFilter(m_varMap[REGISTRAR_ID_NAME].as<unsigned int>());
    if (m_varMap.count(POLL_NONSEEN_NAME))
        pmList->setNonSeenFilter(true);
    if (m_varMap.count(POLL_NONEX_NAME))
        pmList->setNonExpiredFilter(true);
    pmList->reload();
    for (unsigned int i = 0; i < pmList->getCount(); i++) {
        Register::Poll::Message *msg = pmList->getMessage(i);
        if (msg) {
            msg->textDump(std::cout);
            std::cout << std::endl;
        }
    }
    return 0;
}
int
PollClient::list_next()
{
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
    Register::TID reg = m_varMap[POLL_LIST_NEXT_NAME].as<unsigned int>();
    unsigned int count = pollMan->getMessageCount(reg);
    if (!count) {
        std::cout << "No message" << std::endl;
    } else {
        std::auto_ptr<Register::Poll::Message> msg(pollMan->getNextMessage(reg));
        std::cout << "Messages:" << count << std::endl;
        msg->textDump(std::cout);
        std::cout << std::endl;
    }
    return 0;
}
int
PollClient::set_seen()
{
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
    try {
        Register::TID reg = m_varMap[REGISTRAR_ID_NAME].as<unsigned int>();
        if (!reg) std::cout << "Owning registar must be set." << std::endl;
        Register::TID messageId = m_varMap[POLL_SET_SEEN_NAME].as<unsigned int>(); 
        pollMan->setMessageSeen(messageId, reg);
        std::cout << "NextId:" << pollMan->getNextMessageId(reg) << std::endl;
    } catch (...) {
        std::cout << "No message" << std::endl;
    }
    return 0;
}
int
PollClient::create_state_changes()
{
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
    pollMan->createStateMessages(
            m_varMap["poll_except_types"].as<std::string>(),
            m_varMap[LIMIT_NAME].as<unsigned int>(),
            m_varMap.count(DEBUG_NAME) ? &std::cout : NULL
    );
    return 0;
}
int
PollClient::create_low_credit()
{
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
    pollMan->createLowCreditMessages();
    return 0;
}

} // namespace Admin;

