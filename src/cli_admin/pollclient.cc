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
#include "pollclient.h"
#include "register/poll.h"

namespace Admin {

PollClient::PollClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Poll related options");
    m_options->add_options()
        addOpt(POLL_LIST_ALL_NAME)
        addOptUInt(POLL_LIST_NEXT_NAME)
        addOptUInt(POLL_SET_SEEN_NAME)
        addOpt(POLL_CREATE_STATE_CHANGES_NAME)
        addOpt(POLL_CREATE_STATE_CHANGES_2_NAME)
        addOpt(POLL_CREATE_LOW_CREDIT_NAME)
        addOpt(POLL_CREATE_LOW_CREDIT_2_NAME)
        addOpt(POLL_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Poll related sub options");
    m_optionsInvis->add_options()
        addOptUInt(POLL_TYPE_NAME)
        addOptUInt(POLL_REGID_NAME)
        addOpt(POLL_NONSEEN_NAME)
        addOpt(POLL_NONEX_NAME)
        addOpt(POLL_DEBUG_NAME)
        addOptStrDef(POLL_EXCEPT_TYPES_NAME, "")
        addOptUIntDef(POLL_LIMIT_NAME, 0);
}
PollClient::PollClient(
        std::string connstring,
        std::string nsAddr):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_options = NULL;
    m_optionsInvis = NULL;
}

PollClient::~PollClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
PollClient::init(
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
PollClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
PollClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
PollClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
}

int
PollClient::list_all()
{
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
    return 0;
}
int
PollClient::list_next()
{
    std::auto_ptr<Register::Poll::Manager> pollMan(
            Register::Poll::Manager::create(
                &m_db)
            );
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
        Register::TID reg = m_conf.get<unsigned int>(REGISTRAR_ID_NAME);
        if (!reg) std::cout << "Owning registar must be set." << std::endl;
        Register::TID messageId = m_conf.get<unsigned int>(POLL_SET_SEEN_NAME); 
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
            m_conf.get<std::string>(POLL_EXCEPT_TYPES_NAME),
            m_conf.get<unsigned int>(POLL_LIMIT_NAME),
            m_conf.hasOpt(POLL_DEBUG_NAME) ? &std::cout : NULL
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

