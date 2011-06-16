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

namespace Admin {

void
ObjectClient::runMethod()
{
    if (object_new_state_request//m_conf.hasOpt(OBJECT_NEW_STATE_REQUEST_NAME)
            ) {
        new_state_request();
    }   else if (object_update_states//m_conf.hasOpt(OBJECT_UPDATE_STATES_NAME)
            ) {
        update_states();
    }   else if (object_regular_procedure//m_conf.hasOpt(OBJECT_REGULAR_PROCEDURE_NAME)
            ) {
        regular_procedure();
    }
}

void ObjectClient::createObjectStateRequest(
        const std::string & object_name
        , unsigned long object_type
        , const std::string& object_state_name
        , const std::string& valid_from
        , const optional_string& valid_to
        )
{
    Logging::Manager::instance_ref().get(PACKAGE).debug(std::string(
        "ObjectClient::createObjectStateRequest object name: ") + object_name
        + " object type: " + boost::lexical_cast<std::string>(object_type)
        + " object state name: " + object_state_name
        + " valid from: " + valid_from
        + " valid to: " + valid_to.get_value());

    Database::Connection conn = Database::Manager::acquire();

    //get object
    Database::Result obj_id_res = conn.exec_params(
            "SELECT id FROM object_registry "
            " WHERE type=$1::integer AND name=$2::text "
            , Database::query_param_list
                (object_type)(object_name));

    if(obj_id_res.size() != 1)
        throw std::runtime_error("object not found");

    unsigned long long object_id = obj_id_res[0][0];

    //get object state
    Database::Result obj_state_res = conn.exec_params(
                "SELECT id FROM enum_object_states "
                " WHERE name=$1::text"
                , Database::query_param_list
                    (object_state_name));

    if(obj_state_res.size() !=1)
        throw std::runtime_error("object state not found");

    unsigned long long object_state_id = obj_state_res[0][0];

    //get existing state requests for object and state
    //assuming requests for different states of the same object may overlay
    Database::Result requests_result = conn.exec_params(
        "SELECT valid_from, valid_to, canceled FROM object_state_request "
        " WHERE object_id=$1::bigint AND state_id=$2::bigint "
        , Database::query_param_list(object_id)(object_state_id));

    //check time
    boost::posix_time::ptime new_valid_from
        = boost::posix_time::time_from_string(valid_from);
    boost::posix_time::ptime new_valid_to
        = boost::posix_time::time_from_string(valid_to.get_value());

    if(new_valid_from > new_valid_to )
        throw std::runtime_error("new_valid_from > new_valid_to");

    for(std::size_t i = 0 ; i < requests_result.size(); ++i)
    {
        boost::posix_time::ptime obj_valid_from
            = boost::posix_time::time_from_string(
                    std::string(requests_result[i][0]));

        boost::posix_time::ptime obj_valid_to
            = boost::posix_time::time_from_string(
                    std::string(requests_result[i][1]));

        //if obj_canceled is not null
        if(requests_result[i][2].isnull() == false)
        {
            boost::posix_time::ptime obj_canceled
                = boost::posix_time::time_from_string(
                        std::string(requests_result[i][2]));
            if (obj_canceled < obj_valid_to) obj_valid_to = obj_canceled;
        }//if obj_canceled is not null

        if(obj_valid_from > obj_valid_to )
            throw std::runtime_error("obj_valid_from > obj_valid_to");

        //check overlay
        if(((new_valid_from >= obj_valid_from) && (new_valid_from < obj_valid_to))
          || ((new_valid_to > obj_valid_from) && (new_valid_to <= obj_valid_to)))
            throw std::runtime_error("overlayed validity time intervals");
    }//for check with existing object state requests

    conn.exec_params(
        "INSERT INTO object_state_request "
        "(object_id,state_id,crdate, valid_from,valid_to) VALUES "
        "( $1::bigint , $2::bigint "
        ",CURRENT_TIMESTAMP, $3::timestamp, "
        "$4::timestamp )"
        , Database::query_param_list
            (object_id)(object_state_id)
            (new_valid_from)(new_valid_to.is_special()
                    ? Database::QPNull
                            : Database::QueryParam(new_valid_to) )
        );

    return;
}//ObjectClient::createObjectStateRequest

void
ObjectClient::new_state_request()
{

    createObjectStateRequest(
        object_new_state_request_params.object_name
        , object_new_state_request_params.object_type
        , object_new_state_request_params.object_state_name
        , object_new_state_request_params.valid_from.is_value_set()
            ? object_new_state_request_params.valid_from.get_value()
                :  boost::posix_time::to_iso_extended_string(microsec_clock::universal_time())  //valid_from default now
        , object_new_state_request_params.valid_to//valid_to
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
    sql << " ORDER BY CASE WHEN o.type = 3 THEN 1 ELSE 2 END ASC, s.id";
    unsigned int limit = 0;
    if (delete_objects_params.object_delete_limit.is_value_set()//m_conf.hasOpt(OBJECT_DELETE_LIMIT_NAME)
            ) {
        limit = delete_objects_params.object_delete_limit.get_value();//m_conf.get<unsigned int>(OBJECT_DELETE_LIMIT_NAME);
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

    if (debug) {
        *debug << "<objects>\n";
        for (unsigned int i = 0; i < (unsigned int)m_db->GetSelectRows(); i++) {
            *debug << "<object name='" << m_db->GetFieldValue(i, 0) << "'/>\n";
        }
        *debug << "</objects>\n";
        m_db->FreeSelect();
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
        for (unsigned int i = 0; i < (unsigned int)m_db->GetSelectRows(); i++) {
            std::string name = m_db->GetFieldValue(i, 0);
            std::string cltrid;
            std::string xml;
            xml = "<name>" + name + "</name>";

            ccReg::EppParams params;
            params.sessionID    = clientId;
            params.requestID    = 0;
            params.XML          = xml.c_str();

            try {
                switch (atoi(m_db->GetFieldValue(i, 1))) {
                    case 1:
                        cltrid = "delete_contact";
                        params.clTRID    = cltrid.c_str();
                        r = epp->ContactDelete(
                                name.c_str(), params);
                        break;
                    case 2:
                        cltrid = "delete_nsset";
                        params.clTRID    = cltrid.c_str();
                        r = epp->NSSetDelete(
                                name.c_str(), params);
                        break;
                    case 3:
                        cltrid = "delete_unpaid_zone_" + std::string(m_db->GetFieldValue(i, 2));
                        params.clTRID    = cltrid.c_str();
                        r = epp->DomainDelete(
                                name.c_str(), params);
                        break;
                    case 4:
                        cltrid = "delete_keyset";
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
        epp->ClientLogout(clientId, "system_delete_logout", "<system_delete_logout/>");
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
        if (object_regular_procedure_params.object_delete_types.is_value_set()//m_conf.hasOpt(OBJECT_DELETE_TYPES_NAME)
                ) {
            deleteTypes = object_regular_procedure_params.object_delete_types.get_value();//m_conf.get<std::string>(OBJECT_DELETE_TYPES_NAME);
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

} // namespace Admin;


