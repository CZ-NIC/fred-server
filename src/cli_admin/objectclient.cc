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

#define RESOLVE_TRY 3

#include "simple.h"
#include "commonclient.h"
#include "objectclient.h"
#include "register/register.h"
#include "register/poll.h"
#include "log/logger.h"
#include "corba/nameservice.h"

namespace Admin {

const struct options *
ObjectClient::getOpts()
{
    return m_opts;
}

void
ObjectClient::runMethod()
{
    if (m_conf.hasOpt(OBJECT_NEW_STATE_REQUEST_NAME)) {
        new_state_request();
    } else if (m_conf.hasOpt(OBJECT_LIST_NAME)) {
        list();
    } else if (m_conf.hasOpt(OBJECT_UPDATE_STATES_NAME)) {
        update_states();
    } else if (m_conf.hasOpt(OBJECT_DELETE_CANDIDATES_NAME)) {
        delete_candidates();
    } else if (m_conf.hasOpt(OBJECT_REGULAR_PROCEDURE_NAME)) {
        regular_procedure();
    } else if (m_conf.hasOpt(OBJECT_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
ObjectClient::show_opts() 
{
    callHelp(m_conf, no_help);
    print_options("Object", getOpts(), getOptsCount());
}

int
ObjectClient::createObjectStateRequest(
        Register::TID object,
        unsigned state)
{
      std::stringstream sql;
      sql << "SELECT COUNT(*) FROM object_state_request "
          << "WHERE object_id=" << object << " AND state_id=" << state
          << " AND (canceled ISNULL OR canceled > CURRENT_TIMESTAMP) "
          << " AND (valid_to ISNULL OR valid_to > CURRENT_TIMESTAMP) ";
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

void
ObjectClient::new_state_request()
{
    callHelp(m_conf, no_help);
    Register::TID id = m_conf.get<unsigned long long>(OBJECT_ID_NAME);
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
    return;
}

void
ObjectClient::list()
{
    callHelp(m_conf, no_help);
    std::cout << "not implemented" << std::endl;
}

void
ObjectClient::update_states()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Manager> regMan(
            Register::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    unsigned long long id = 0;
    if (m_conf.hasOpt(OBJECT_ID_NAME)) {
        id = m_conf.get<unsigned long long>(OBJECT_ID_NAME);
    }
    regMan->updateObjectStates(id);
    return;

}

/// delete objects with status deleteCandidate
/** \return 0=OK -1=SQL ERROR -2=no system registrar -3=login failed */
int
ObjectClient::deleteObjects(
        const std::string& typeList, CorbaClient &cc)
{
    LOGGER("tracer").trace("ObjectClient::deleteObjects");
    ccReg::EPP_var epp = NULL;
    // CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
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
    sql << " ORDER BY CASE WHEN o.type = 3 THEN 1 ELSE 2 END ASC, s.id";
    unsigned int limit = 0;
    if (m_conf.hasOpt(OBJECT_DELETE_LIMIT_NAME)) {
        limit = m_conf.get<unsigned int>(OBJECT_DELETE_LIMIT_NAME);
    }
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
                CORBA::Object_var o = cc.getNS()->resolve("EPP");
                if ((epp = ccReg::EPP::_narrow(o)) != NULL) {
                    break;
                }
            } catch (NameService::NOT_RUNNING) {
                LOG(ERROR_LOG, "deleteObjects(): resolve attempt %d of %d catching NOT_RUNNING", i + 1, RESOLVE_TRY);
                return -1;
            } catch (NameService::BAD_CONTEXT) {
                LOG(ERROR_LOG, "deleteObjects(): resolve attempt %d of %d catching BAD_CONTEXT", i + 1, RESOLVE_TRY);
                return -1;
	    } catch (std::exception& ex) {
    	        LOG(ERROR_LOG, "deleteObjects(): resolve attempt %d of %d catching %s", ex.what());
		return -1;
	    } catch (...) {
    	        LOG(ERROR_LOG, "deleteObjects(): resolve attempt catching unknown exception");
		return -1;
	    }

        }

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
    catch (CORBA::Exception& e) {
        LOG(ERROR_LOG, "deleteObjects(): Exception catched: %s", e._name());
        m_db.FreeSelect();
        return -4;
    }
    catch (std::exception& e) {
        LOG(ERROR_LOG, "deleteObjects(): Exception catched: %s", e.what());
        m_db.FreeSelect();
        return -4;
    }
    catch (...) {
        LOG(ERROR_LOG, "deleteObjects(): Unknown exception catched");
        m_db.FreeSelect();
        return -4;
    }
}

void
ObjectClient::delete_candidates()
{
    callHelp(m_conf, no_help);
    std::string deleteTypes("");
    if (m_conf.hasOpt(OBJECT_DELETE_TYPES_NAME)) {
        deleteTypes = m_conf.get<std::string>(OBJECT_DELETE_TYPES_NAME);
    }
    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    deleteObjects(deleteTypes, cc);
}

void
ObjectClient::regular_procedure()
{
    callHelp(m_conf, no_help);
    int i;
    std::auto_ptr<CorbaClient> cc;
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
                cc.reset(new CorbaClient(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME)));
                if (cc.get() != NULL) {
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
        std::string pollExcept("");
        if (m_conf.hasOpt(OBJECT_POLL_EXCEPT_TYPES_NAME)) {
            pollExcept = m_conf.get<std::string>(OBJECT_POLL_EXCEPT_TYPES_NAME);
        }
        pollMan->createStateMessages(pollExcept, 0, NULL);

        std::string deleteTypes("");
        if (m_conf.hasOpt(OBJECT_DELETE_TYPES_NAME)) {
            deleteTypes = m_conf.get<std::string>(OBJECT_DELETE_TYPES_NAME);
        }
        if ((i = deleteObjects(deleteTypes, *(cc.get()))) != 0) {
            LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): Error has occured in deleteObject: %d", i);
            return;
        }

        std::string notifyExcept("");
        if (m_conf.hasOpt(OBJECT_NOTIFY_EXCEPT_TYPES_NAME)) {
            notifyExcept = m_conf.get<std::string>(OBJECT_NOTIFY_EXCEPT_TYPES_NAME);
        }
        notifyMan->notifyStateChanges(notifyExcept, 0, NULL, true);

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

    return;
} // ObjectClient::regular_procedure

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_OBJECT, name, name##_DESC, type, callable, visible}

const struct options
ObjectClient::m_opts[] = {
    ADDOPT(OBJECT_NEW_STATE_REQUEST_NAME, TYPE_UINT, true, true),
    ADDOPT(OBJECT_REGULAR_PROCEDURE_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(OBJECT_DELETE_CANDIDATES_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(OBJECT_UPDATE_STATES_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(OBJECT_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(OBJECT_DEBUG_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(OBJECT_ID_NAME, TYPE_ULONGLONG, false, false),
    ADDOPT(OBJECT_NAME_NAME, TYPE_STRING, false, false),
    ADDOPT(OBJECT_DELETE_TYPES_NAME, TYPE_STRING, false, false),
    ADDOPT(OBJECT_NOTIFY_EXCEPT_TYPES_NAME, TYPE_STRING, false, false),
    ADDOPT(OBJECT_POLL_EXCEPT_TYPES_NAME, TYPE_STRING, false, false),
    ADDOPT(OBJECT_DELETE_LIMIT_NAME, TYPE_UINT, false, false),
};

#undef ADDOPT

int 
ObjectClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;


