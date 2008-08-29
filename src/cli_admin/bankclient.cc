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
#include "register/bank.h"
#include "commonclient.h"
#include "bankclient.h"

namespace Admin {

#define LOGIN_BANKCLIENT \
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

#define LOGOUT_BANKCLIENT \
    epp->ClientLogout(clientId,"system_delete_logout","<system_delete_logout/>");

BankClient::BankClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Bank related options");
    m_options->add_options()
        ADD_OPT(BANK_ONLINE_LIST_NAME, "list of online payments")
        ADD_OPT(BANK_STATEMENT_LIST_NAME, "list of bank statements");

    m_optionsInvis = new boost::program_options::options_description(
            "Bank related invisible options");
    m_optionsInvis->add_options();
}
BankClient::BankClient(
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

BankClient::~BankClient()
{
}

void
BankClient::init(
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
BankClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
BankClient::getInvisibleOptions() const
{
    return m_optionsInvis;
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

