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
#include "bankclient.h"
#include "commonclient.h"
#include "register/bank.h"
#include "register/register.h"

namespace Admin {

BankClient::BankClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Bank related options");
    m_options->add_options()
        addOpt(BANK_ONLINE_LIST_NAME)
        addOpt(BANK_STATEMENT_LIST_NAME)
        addOpt(BANK_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Bank related sub options");
    m_optionsInvis->add_options();
}
BankClient::BankClient(
        std::string connstring,
        std::string nsAddr):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_options = NULL;
    m_optionsInvis = NULL;
}

BankClient::~BankClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
BankClient::init(
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
BankClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
BankClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
BankClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
}

int
BankClient::online_list()
{
    std::ofstream stdout("/dev/stdout",std::ios::out);   

    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create(&m_db));
    std::auto_ptr<Register::Banking::OnlinePaymentList> statList(
            bankMan->createOnlinePaymentList());
    statList->reload();
    statList->exportXML(stdout);
    return 0;
}

int
BankClient::statement_list()
{
    std::ofstream stdout("/dev/stdout",std::ios::out);   

    std::auto_ptr<Register::Banking::Manager> bankMan(
            Register::Banking::Manager::create(&m_db));
    std::auto_ptr<Register::Banking::StatementList> statList(
            bankMan->createStatementList());
    statList->reload();
    statList->exportXML(stdout);

    return 0;
}

} // namespace Admin;

