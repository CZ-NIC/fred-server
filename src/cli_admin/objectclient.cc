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

#include <boost/lexical_cast.hpp>
#include <omniORB4/CORBA.h>
#include "commonclient.h"
#include "objectclient.h"
#include "fredlib/registry.h"
#include "fredlib/poll.h"
#include "log/logger.h"
#include "corba/nameservice.h"
#include <stdexcept>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>

#include "fredlib/object_states.h"

namespace Admin {

void
ObjectClient::runMethod()
{
    if (object_new_state_request//m_conf.hasOpt(OBJECT_NEW_STATE_REQUEST_NAME)
            ) {
        new_state_request();
    }   else if (object_new_state_request_name
            ) {
        new_state_request_name();
    }   else if (object_update_states//m_conf.hasOpt(OBJECT_UPDATE_STATES_NAME)
            ) {
        update_states();
    }   else if (object_regular_procedure//m_conf.hasOpt(OBJECT_REGULAR_PROCEDURE_NAME)
            ) {
        regular_procedure();
    }   else if (object_delete_candidates//m_conf.hasOpt(OBJECT_DELETE_CANDIDATE)
            ) {
        delete_candidates();
    }
}

int
ObjectClient::createObjectStateRequest(
        Fred::TID object,
        unsigned state)
{
    Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("ObjectClient::createObjectStateRequest Fred::TID object: ")
     + boost::lexical_cast<std::string>(object) + " unsigned state: " + boost::lexical_cast<std::string>(state));
      std::stringstream sql;
      sql << "SELECT COUNT(*) FROM object_state_request "
          << "WHERE object_id=" << object << " AND state_id=" << state
          << " AND (canceled ISNULL OR canceled > CURRENT_TIMESTAMP) "
          << " AND (valid_to ISNULL OR valid_to > CURRENT_TIMESTAMP) ";
      Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("ObjectClient::createObjectStateRequest sql: ") +sql.str());
      if (!m_db->ExecSelect(sql.str().c_str()))
          return -1;
      if (atoi(m_db->GetFieldValue(0,0)))
          return -2;
      m_db->FreeSelect();
      sql.str("");
      sql << "INSERT INTO object_state_request "
          << "(object_id,state_id,crdate, valid_from,valid_to) VALUES "
          << "(" << object << "," << state
          << ",CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, "
          << "CURRENT_TIMESTAMP + INTERVAL '7 days');";
      Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("ObjectClient::createObjectStateRequest sql: ") +sql.str());
      if (!m_db->ExecSQL(sql.str().c_str()))
          return -1;
      return 0;
}

void
ObjectClient::new_state_request()
{
    //callHelp(m_conf, no_help);
    Fred::TID id = object_new_state_request_params.object_id;// m_conf.get<unsigned long long>(OBJECT_ID_NAME);
    unsigned int state = object_new_state_request_params.object_new_state_request;//m_conf.get<unsigned int>(OBJECT_NEW_STATE_REQUEST_NAME);
    int res = createObjectStateRequest(
            id, state
            );
    switch (res) {
        case -1:
            Logging::Manager::instance_ref().get(PACKAGE).error("SQL_ERROR" );
            std::cerr << "SQL_ERROR" << std::endl;
            break;
        case -2:
            Logging::Manager::instance_ref().get(PACKAGE).error("Already exists" );
            std::cerr << "Already exists" << std::endl;
            break;
        case 0:
            break;
        default:
            Logging::Manager::instance_ref().get(PACKAGE).error("Unknown error");
            std::cerr << "Unknown error" << std::endl;
            break;
    }
    return;
}



void
ObjectClient::new_state_request_name()
{
    Fred::createObjectStateRequestName(
        object_new_state_request_name_params.object_name
        , object_new_state_request_name_params.object_type
        , object_new_state_request_name_params.object_state_name
        , object_new_state_request_name_params.valid_from.is_value_set()
            ? object_new_state_request_name_params.valid_from.get_value()
                :  boost::posix_time::to_iso_extended_string(microsec_clock::universal_time())  //valid_from default now
        , object_new_state_request_name_params.valid_to//valid_to
        , m_db
        , restricted_handles
        , object_new_state_request_name_params.update_object_state
        );

    return;
}

void
ObjectClient::list()
{
    std::cout << "not implemented" << std::endl;
}

void
ObjectClient::update_states()
{
    std::auto_ptr<Fred::Manager> regMan(
            Fred::Manager::create(
                m_db,
                restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                )
            );
    unsigned long long id = 0;
    if (object_update_states_params.object_id.is_value_set()//m_conf.hasOpt(OBJECT_ID_NAME)
            ) {
        id = object_update_states_params.object_id.get_value();//m_conf.get<unsigned long long>(OBJECT_ID_NAME);
    }
    Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("regMan->updateObjectStates id: ")
        +boost::lexical_cast<std::string>(id));
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
    // should be instead somewhere in registry library (object.cc?)
    // get login information for first system registrar
    if (!m_db->ExecSelect(
                "SELECT r.handle,ra.cert,ra.password "
                "FROM registrar r, registraracl ra "
                "WHERE r.id=ra.registrarid AND r.system='t' LIMIT 1 ")) {
        LOG(ERROR_LOG, "deleteObjects(): Error during ExecSelect");
        return -1;
    }
    if (!m_db->GetSelectRows()) {
        LOG(ERROR_LOG, "deleteObjects(): No rows returned (1)");
        return -1;
    }
    std::string handle = m_db->GetFieldValue(0, 0);
    std::string cert = m_db->GetFieldValue(0, 1);
    std::string password = m_db->GetFieldValue(0, 2);
    m_db->FreeSelect();

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
    sql << " ORDER BY CASE WHEN o.type = 3 THEN 1 ELSE 2 END ASC, random()";
    unsigned int limit = 0;
    if (delete_objects_params.object_delete_limit.is_value_set()//m_conf.hasOpt(OBJECT_DELETE_LIMIT_NAME)
            ) {
        limit = delete_objects_params.object_delete_limit.get_value();//m_conf.get<unsigned int>(OBJECT_DELETE_LIMIT_NAME);
    }
    unsigned int parts = 0;
    if (delete_objects_params.object_delete_parts.is_value_set()
            ) {
        parts = delete_objects_params.object_delete_parts.get_value();
    }
    if (limit > 0)
        sql << "LIMIT " << limit;
    if (!m_db->ExecSelect(sql.str().c_str())) {
        LOG(ERROR_LOG, "deleteObjects(): Error during ExecSelect");
        return -1;
    }
    if (!m_db->GetSelectRows()) {
        LOG(NOTICE_LOG, "deleteObjects(): No rows returned (2)");
        return 0;
    }

    std::ostream *debug;
    debug = delete_objects_params.object_delete_debug//m_conf.hasOpt(OBJECT_DEBUG_NAME)
            ? &std::cout : NULL;

    unsigned int totalCount = (unsigned int)m_db->GetSelectRows();
    // limit number of object to just first part from total
    // could be done by COUNT(*) and LIMIT in SQL but this would need to
    // issue another SQL
    if (parts > 0) totalCount = totalCount/parts;

    if (debug) {
        *debug << "<objects>\n";
        for (unsigned int i = 0; i < totalCount; i++) {
            *debug << "<object name='" << m_db->GetFieldValue(i, 0) << "'/>\n";
        }
        *debug << "</objects>\n";
        m_db->FreeSelect();
        return 0;
    }
    try {
        CORBA::ULongLong clientId = 0;

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
                handle.c_str(),password.c_str(),"","system_delete_login", "<system_delete_login/>"
                ,clientId, 0 ,cert.c_str(),ccReg::EN);
        if (r->code != 1000 || !clientId) {
            LOG(ERROR_LOG, "Cannot connect: %d", r->code);
            std::cerr << "Cannot connect: " << r->code << std::endl;
            throw -3;
        }

        for (unsigned int i = 0; i < totalCount; i++) {
            std::string name = m_db->GetFieldValue(i, 0);
            /* we don't want to notify delete commands - add configured cltrid prefix */
            std::string cltrid = disable_epp_notifier_cltrid_prefix;
            std::string xml;
            xml = "<name>" + name + "</name>";

            ccReg::EppParams params;
            params.loginID    = clientId;
            params.requestID    = 0;
            params.XML          = xml.c_str();

            try {
                switch (atoi(m_db->GetFieldValue(i, 1))) {
                    case 1:
                        cltrid += "_delete_contact";
                        params.clTRID    = cltrid.c_str();
                        r = epp->ContactDelete(
                                name.c_str(), params);
                        break;
                    case 2:
                        cltrid += "_delete_nsset";
                        params.clTRID    = cltrid.c_str();
                        r = epp->NSSetDelete(
                                name.c_str(), params);
                        break;
                    case 3:
                        cltrid += "_delete_unpaid_zone_" + std::string(m_db->GetFieldValue(i, 2));
                        params.clTRID    = cltrid.c_str();
                        r = epp->DomainDelete(
                                name.c_str(), params);
                        break;
                    case 4:
                        cltrid += "_delete_keyset";
                        params.clTRID    = cltrid.c_str();
                        r = epp->KeySetDelete(
                                name.c_str(), params);
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

        ccReg::EppParams par_logout;
        par_logout.clTRID = "system_delete_logout";
        par_logout.XML = "<system_delete_logout/>";
        par_logout.loginID = clientId;

        epp->ClientLogout(par_logout);
        m_db->FreeSelect();
        return 0;
    }
    catch (int& i) {
        LOG(ERROR_LOG, "deleteObjects(): Exception catched: %d", i);
        m_db->FreeSelect();
        return i;
    }
    catch (CORBA::Exception& e) {
        LOG(ERROR_LOG, "deleteObjects(): Exception catched: %s", e._name());
        m_db->FreeSelect();
        return -4;
    }
    catch (std::exception& e) {
        LOG(ERROR_LOG, "deleteObjects(): Exception catched: %s", e.what());
        m_db->FreeSelect();
        return -4;
    }
    catch (...) {
        LOG(ERROR_LOG, "deleteObjects(): Unknown exception catched");
        m_db->FreeSelect();
        return -4;
    }
}

void
ObjectClient::regular_procedure()
{
    int i;
    std::auto_ptr<CorbaClient> cc;
    try {
        std::auto_ptr<Fred::Document::Manager> docMan(
                Fred::Document::Manager::create(
                    docgen_path.get_value()//m_conf.get<std::string>(REG_DOCGEN_PATH_NAME)
                    ,docgen_template_path.get_value()//m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME)
                    ,fileclient_path.get_value()//m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME)
                    ,m_nsAddr)
                );
        for (i = 0; i < RESOLVE_TRY; i++) {
            try {
                cc.reset(new CorbaClient(0, NULL, m_nsAddr, nameservice_context//m_conf.get<std::string>(NS_CONTEXT_NAME)
                        ));
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

        std::auto_ptr<Fred::Zone::Manager> zoneMan(
                Fred::Zone::Manager::create());

        Fred::Messages::ManagerPtr msgMan
            = Fred::Messages::create_manager();


        std::auto_ptr<Fred::Domain::Manager> domMan(
                Fred::Domain::Manager::create(m_db, zoneMan.get()));

        std::auto_ptr<Fred::Contact::Manager> conMan(
                Fred::Contact::Manager::create(
                    m_db,
                    restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                    )
                );
        std::auto_ptr<Fred::NSSet::Manager> nssMan(
                Fred::NSSet::Manager::create(
                    m_db,
                    zoneMan.get(),
                    restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                    )
                );
        std::auto_ptr<Fred::KeySet::Manager> keyMan(
                Fred::KeySet::Manager::create(
                    m_db,
                    restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                    )
                );
        std::auto_ptr<Fred::Poll::Manager> pollMan(
                Fred::Poll::Manager::create(
                    m_db)
                );
        std::auto_ptr<Fred::Manager> registryMan(
                Fred::Manager::create(
                    m_db,
                    restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                    )
                );
        std::auto_ptr<Fred::Registrar::Manager> regMan(
                Fred::Registrar::Manager::create(m_db));
        std::auto_ptr<Fred::Notify::Manager> notifyMan(
                Fred::Notify::Manager::create(
                    m_db,
                    &mailMan,
                    conMan.get(),
                    nssMan.get(),
                    keyMan.get(),
                    domMan.get(),
                    docMan.get(),
                    regMan.get(),
                    msgMan));

        registryMan->updateObjectStates();
        registryMan->updateObjectStates();
        std::string pollExcept("");
        if (object_regular_procedure_params.poll_except_types.is_value_set()//m_conf.hasOpt(OBJECT_POLL_EXCEPT_TYPES_NAME)
                ) {
            pollExcept = object_regular_procedure_params.poll_except_types.get_value();//m_conf.get<std::string>(OBJECT_POLL_EXCEPT_TYPES_NAME);
        }
        pollMan->createStateMessages(pollExcept, 0, NULL);

        std::string deleteTypes("");
        if (delete_objects_params.object_delete_types.is_value_set()//m_conf.hasOpt(OBJECT_DELETE_TYPES_NAME)
                ) {
            deleteTypes = delete_objects_params.object_delete_types.get_value();//m_conf.get<std::string>(OBJECT_DELETE_TYPES_NAME);
        }
        if ((i = deleteObjects(deleteTypes, *(cc.get()))) != 0) {
            LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): Error has occured in deleteObject: %d", i);
            return;
        }

        std::string notifyExcept("");
        if (object_regular_procedure_params.notify_except_types.is_value_set()//m_conf.hasOpt(OBJECT_NOTIFY_EXCEPT_TYPES_NAME)
                ) {
            notifyExcept = object_regular_procedure_params.notify_except_types.get_value();//m_conf.get<std::string>(OBJECT_NOTIFY_EXCEPT_TYPES_NAME);
        }
        notifyMan->notifyStateChanges(notifyExcept, 0, NULL, true);

        pollMan->createLowCreditMessages();
        notifyMan->generateLetters(docgen_domain_count_limit//m_conf.get<unsigned>(REG_DOCGEN_DOMAIN_COUNT_LIMIT)
                );
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

void
ObjectClient::delete_candidates()
{
    int i;
    std::auto_ptr<CorbaClient> cc;
    try {
        for (i = 0; i < RESOLVE_TRY; i++) {
            try {
                cc.reset(new CorbaClient(0, NULL, m_nsAddr, nameservice_context//m_conf.get<std::string>(NS_CONTEXT_NAME)
                        ));
                if (cc.get() != NULL) {
                    break;
                }
            } catch (NameService::NOT_RUNNING) {
                LOG(ERROR_LOG, "regular_procedure(): resolve attempt %d of %d catching NOT_RUNNING", i + 1, RESOLVE_TRY);
            } catch (NameService::BAD_CONTEXT) {
                LOG(ERROR_LOG, "regular_procedure(): resolve attempt %d of %d catching BAD_CONTEXT", i + 1, RESOLVE_TRY);
            }
        }
        std::string deleteTypes("");
        if (delete_objects_params.object_delete_types.is_value_set()//m_conf.hasOpt(OBJECT_DELETE_TYPES_NAME)
                ) {
            deleteTypes = delete_objects_params.object_delete_types.get_value();//m_conf.get<std::string>(OBJECT_DELETE_TYPES_NAME);
        }
        if ((i = deleteObjects(deleteTypes, *(cc.get()))) != 0) {
            LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): Error has occured in deleteObject: %d", i);
            return;
        }

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
} // ObjectClient::delete_candidates

} // namespace Admin;


