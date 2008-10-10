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

#define RESOLVE_TRY 3

#include "simple.h"
#include "commonclient.h"
#include "objectclient.h"
#include "register/register.h"
#include "register/poll.h"
#include "log/logger.h"
#include "corba/nameservice.h"

namespace Admin {

ObjectClient::ObjectClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Object related options");
    m_options->add_options()
        addOptUInt(OBJECT_NEW_STATE_REQUEST_NAME)
        addOpt(OBJECT_REGULAR_PROCEDURE_NAME)
        addOpt(OBJECT_DELETE_CANDIDATES_NAME)
        addOpt(OBJECT_UPDATE_STATES_NAME)
        addOpt(OBJECT_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Object related sub options");
    m_optionsInvis->add_options()
        addOpt(OBJECT_DEBUG_NAME)
        addOptUInt(OBJECT_ID_NAME)
        addOptStr(OBJECT_NAME_NAME)
        addOptStrDef(OBJECT_DELETE_TYPES_NAME, "")
        addOptStrDef(OBJECT_NOTIFY_EXCEPT_TYPES_NAME, "")
        addOptStrDef(OBJECT_POLL_EXCEPT_TYPES_NAME, "")
        addOptUIntDef(OBJECT_DELETE_LIMIT_NAME, 0);
}
ObjectClient::ObjectClient(
        std::string connstring,
        std::string nsAddr):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_dbman = NULL;
    m_db.OpenDatabase(connstring.c_str());
    m_options = NULL;
    m_optionsInvis = NULL;
}

ObjectClient::~ObjectClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
ObjectClient::init(
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
ObjectClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
ObjectClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
ObjectClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
}

int
ObjectClient::createObjectStateRequest(
        Register::TID object,
        unsigned state)
{
      std::stringstream sql;
      sql << "SELECT COUNT(*) FROM object_state_request "
          << "WHERE object_id=" << object << " AND state_id=" << state
          << " AND canceled ISNULL "
          << " AND (valid_to ISNULL OR valid_to>CURRENT_TIMESTAMP) ";
      if (!m_db.ExecSelect(sql.str().c_str()))
          return -1;
      if (atoi(m_db.GetFieldValue(0,0)))
          return -2;
      m_db.FreeSelect();
      sql.str("");
      sql << "INSERT INTO object_state_request "
          << "(object_id,state_id,crdate, valid_from,valid_to) VALUES "
          << "(" << object << "," << state
          << ",CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, "
          << "CURRENT_TIMESTAMP + INTERVAL '7 days');";
      if (!m_db.ExecSQL(sql.str().c_str()))
          return -1;
      return 0;
}

int
ObjectClient::new_state_request()
{
    Register::TID id = m_conf.get<unsigned int>(OBJECT_ID_NAME);
    unsigned int state = m_conf.get<unsigned int>(OBJECT_NEW_STATE_REQUEST_NAME);
    int res = createObjectStateRequest(
            id, state
            );
    switch (res) {
        case -1:
            std::cerr << "SQL_ERROR" << std::endl;
            break;
        case -2:
            std::cerr << "Already exists" << std::endl;
            break;
        case 0:
            break;
        default:
            std::cerr << "Unknown error" << std::endl;
            break;
    }
    return 0;
}

void
ObjectClient::list()
{
}

int
ObjectClient::update_states()
{
    std::auto_ptr<Register::Manager> regMan(
            Register::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    regMan->updateObjectStates();
    return 0;
}

/// delete objects with status deleteCandidate
/** \return 0=OK -1=SQL ERROR -2=no system registrar -3=login failed */
int
ObjectClient::deleteObjects(
        const std::string& typeList)
{
    LOGGER("tracer").trace("ObjectClient::deleteObjects");
    ccReg::EPP_var epp = NULL;
    // temporary done by using EPP corba interface
    // should be instead somewhere in register library (object.cc?)
    // get login information for first system registrar
    if (!m_db.ExecSelect(
                "SELECT r.handle,ra.cert,ra.password "
                "FROM registrar r, registraracl ra "
                "WHERE r.id=ra.registrarid AND r.system='t' LIMIT 1 ")) {
        LOG(ERROR_LOG, "deleteObjects(): Error during ExecSelect");
        return -1;
    }
    if (!m_db.GetSelectRows()) {
        LOG(ERROR_LOG, "deleteObjects(): No rows returned (1)");
        return -1;
    }
    std::string handle = m_db.GetFieldValue(0, 0);
    std::string cert = m_db.GetFieldValue(0, 1);
    std::string password = m_db.GetFieldValue(0, 2);
    m_db.FreeSelect();

    // before connection load all objects, zones are needed to
    // put zone id into cltrid (used in statistics - need to fix)
    std::stringstream sql;
    sql <<
        "SELECT o.name, o.type, COALESCE(d.zone,0) "
        "FROM object_state s "
        "JOIN object_registry o ON ( "
        " o.erdate ISNULL AND o.id=s.object_id "
        " AND s.state_id=17 AND s.valid_to ISNULL) "
        "LEFT JOIN domain d ON (d.id=o.id)";
    if (!typeList.empty())
        sql << "WHERE o.type IN (" << typeList << ") ";
    sql << "ORDER BY s.id ";
    unsigned int limit = m_conf.get<unsigned int>(OBJECT_DELETE_LIMIT_NAME);
    if (limit > 0)
        sql << "LIMIT " << limit;
    if (!m_db.ExecSelect(sql.str().c_str())) {
        LOG(ERROR_LOG, "deleteObjects(): Error during ExecSelect");
        return -1;
    }
    if (!m_db.GetSelectRows()) {
        LOG(NOTICE_LOG, "deleteObjects(): No rows returned (2)");
        return 0;
    }

    std::ostream *debug;
    debug = m_conf.hasOpt(OBJECT_DEBUG_NAME) ? &std::cout : NULL;

    if (debug) {
        *debug << "<objects>\n";
        for (unsigned int i = 0; i < (unsigned int)m_db.GetSelectRows(); i++) {
            *debug << "<object name='" << m_db.GetFieldValue(i, 0) << "'/>\n";
        }
        *debug << "</objects>\n";
        m_db.FreeSelect();
        return 0;
    }
    try {
        CORBA::Long clientId = 0;

        for (int i = 0; i < RESOLVE_TRY; i++) {
            try {
                CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
                CORBA::Object_var o = cc.getNS()->resolve("EPP");
                if ((epp = ccReg::EPP::_narrow(o)) != NULL) {
                    break;
                }
            } catch (NameService::NOT_RUNNING) {
                LOG(ERROR_LOG, "deleteObjects(): resolve attempt %d of %d catching NOT_RUNNING", i + 1, RESOLVE_TRY);
            } catch (NameService::BAD_CONTEXT) {
                LOG(ERROR_LOG, "deleteObjects(): resolve attempt %d of %d catching BAD_CONTEXT", i + 1, RESOLVE_TRY);
            }
        }

        return 0;
        ccReg::Response_var r = epp->ClientLogin(
                handle.c_str(),password.c_str(),"","system_delete_login",
                "<system_delete_login/>",clientId,cert.c_str(),ccReg::EN);
        if (r->code != 1000 || !clientId) {
            LOG(ERROR_LOG, "Cannot connect: %d", r->code);
            std::cerr << "Cannot connect: " << r->code << std::endl;
            throw -3;
        }
        for (unsigned int i = 0; i < (unsigned int)m_db.GetSelectRows(); i++) {
            std::string name = m_db.GetFieldValue(i, 0);
            std::string cltrid;
            std::string xml;
            xml = "<name>" + name + "</name>";
            try {
                switch (atoi(m_db.GetFieldValue(i, 1))) {
                    case 1:
                        cltrid = "delete_contact";
                        r = epp->ContactDelete(
                                name.c_str(), clientId, cltrid.c_str(), xml.c_str());
                        break;
                    case 2:
                        cltrid = "delete_nsset";
                        r = epp->NSSetDelete(
                                name.c_str(), clientId, cltrid.c_str(), xml.c_str());
                        break;
                    case 3:
                        cltrid = "delete_unpaid_zone_" + std::string(m_db.GetFieldValue(i, 2));
                        r = epp->DomainDelete(
                                name.c_str(), clientId, cltrid.c_str(), xml.c_str());
                        break;
                    case 4:
                        cltrid = "delete_keyset";
                        r = epp->KeySetDelete(
                                name.c_str(), clientId, cltrid.c_str(), xml.c_str());
                        break;
                }
                if (r->code != 1000) {
                    LOG(ERROR_LOG, "deleteObjects(): cannot %s: %s", cltrid.c_str(), name.c_str());
                    std::cerr << "Cannot " << cltrid << ": " << name << " code: " << r->code;
                }
                else {
                    std::cerr << cltrid << ": " << name;
                }
                std::cerr << std::endl;
            }
            catch (...) {
                std::cerr << "Cannot " << cltrid << ": " << name << std::endl;
                LOG(ERROR_LOG, "deleteObjects(): cannot %s: %s", cltrid.c_str(), name.c_str());
                // proceed with next domain
            }
        }
        epp->ClientLogout(clientId, "system_delete_logout", "<system_delete_logout/>");
        m_db.FreeSelect();
        return 0;
    }
    catch (int& i) {
        LOG(ERROR_LOG, "deleteObjects(): Exception catched: %d", i);
        m_db.FreeSelect();
        return i;
    }
    catch (...) {
        LOG(ERROR_LOG, "deleteObjects(): Unknown exception catched");
        m_db.FreeSelect();
        return -4;
    }
}

int
ObjectClient::delete_candidates()
{
    return deleteObjects(m_conf.get<std::string>(OBJECT_DELETE_TYPES_NAME));
}

int
ObjectClient::regular_procedure()
{
    int i;
    CorbaClient *cc = NULL;
    try {
        std::auto_ptr<Register::Document::Manager> docMan(
                Register::Document::Manager::create(
                    m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                    m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                    m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                    m_nsAddr)
                );
        for (i = 0; i < RESOLVE_TRY; i++) {
            try {
                cc = new CorbaClient(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
                if (cc != NULL) {
                    break;
                }
            } catch (NameService::NOT_RUNNING) {
                LOG(ERROR_LOG, "regular_procedure(): resolve attempt %d of %d catching NOT_RUNNING", i + 1, RESOLVE_TRY);
            } catch (NameService::BAD_CONTEXT) {
                LOG(ERROR_LOG, "regular_procedure(): resolve attempt %d of %d catching BAD_CONTEXT", i + 1, RESOLVE_TRY);
            }
        }
        MailerManager mailMan(cc->getNS());

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
        std::auto_ptr<Register::Poll::Manager> pollMan(
                Register::Poll::Manager::create(
                    &m_db)
                );
        std::auto_ptr<Register::Manager> registerMan(
                Register::Manager::create(
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

        registerMan->updateObjectStates();
        registerMan->updateObjectStates();
        notifyMan->notifyStateChanges(
                m_conf.get<std::string>(OBJECT_NOTIFY_EXCEPT_TYPES_NAME),
                0, NULL, false);
        pollMan->createStateMessages(
                m_conf.get<std::string>(OBJECT_POLL_EXCEPT_TYPES_NAME),
                0, NULL);
        if ((i = deleteObjects(m_conf.get<std::string>(OBJECT_DELETE_TYPES_NAME))) != 0) {
            LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): Error has occured in deleteObject: %d", i);
        }

        pollMan->createLowCreditMessages();
        notifyMan->generateLetters();
    } catch (ccReg::Admin::SQL_ERROR) {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): SQL_ERROR catched");
    } catch (NameService::NOT_RUNNING) {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): NOT_RUNNING catched");
    } catch (NameService::BAD_CONTEXT) {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): BAD_CONTEXT catched");
    } catch (CORBA::Exception &e) {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): CORBA exception catched");
    } catch (...) {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): unknown exception catched");
    }

    try {
        delete cc;
    } catch (CORBA::Exception &e) {
        ;
    }
    return 0;
}

} // namespace Admin;


