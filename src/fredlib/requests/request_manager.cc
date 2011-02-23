/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pthread.h>

#include "config.h"

// FRED logging
#include "log/logger.h"
#include "log/context.h"

#include "request.h"
#include "request_manager.h"
#include "request_list.h"

#include "model_session.h"
#include "model_request_data.h"
#include "model_request.h"
#include "model_request_property_value.h"


namespace Fred {
namespace Logger {

using namespace Database;

using namespace boost::posix_time;
using namespace boost::gregorian;

#ifdef HAVE_LOGGER
class logd_ctx_init {
public:

    inline logd_ctx_init() {
    Logging::Context::clear();
        pthread_t tid = pthread_self();
        boost::format fmt = boost::format("logd-%1%") % tid;
        ctx.reset(new Logging::Context(fmt.str()));
    }

private:
    std::auto_ptr<Logging::Context> ctx;
};

#else

class logd_ctx_init { };

#endif


#ifdef LOGD_VERIFY_INPUT
#error "LOGD_VERIFY_INPUT should be off because of performance improvement."
#endif

/**
         *  Database access class designed for use in fred-logd with partitioned tables
         *  It ensures that the connection will have constraint_exclusion enabled
         *  and all queries will be performed in one transaction which
         *  retains the features of Database::Transaction - it has to be commited
         *  explicitly by the client, otherwise it's rollbacked.
         *  The class acquires database connection from the Database::Manager and explicitly
         *  releases in destructor.
         *  DEPENDENCIES:
         *   - It assumes that the system error logger is initialized (use of logger_error() function
         *   - there's only one connection per thread, so the DB framework receives
         *     the connection encapsulated here
         */
class logd_auto_db : public Connection {
public:

    explicit logd_auto_db() : Connection(Database::Manager::acquire()), tx(0) {
        // set constraint exclusion (needed for faster queries on partitioned tables)
        try {
            /* In postgres version older than 8.3 constraint_exclusion=on posed a risk - it could even
             * lead to incorrect results (cached execution plans)
             * But on new versions it can only mean a decrease of performance by few % if not used properly.
             * It was discussed that this option would be on by default
             */
            exec("set constraint_exclusion=on");
        } catch (Database::Exception &ex) {
            logger_error(boost::format("couldn't set constraint exclusion : %2%") % ex.what());
        }
        tx = new Database::Transaction(*this);
    }

    ~logd_auto_db() {
        delete tx;
        Database::Manager::release();
    }

    void commit() {
        tx->commit();
    }

private:
    Database::Transaction *tx;
};


Result ManagerImpl::i_getRequestTypesByService(ServiceType service)
{
        logd_ctx_init ctx;

        Connection conn = Database::Manager::acquire();

    TRACE("[CALL] Fred::Logger::ManagerImpl::i_getRequestTypesByService");

    boost::format query = boost::format("select id, name from request_type where service_id = %1%") % service;

        return conn.exec(query.str());

}

Result ManagerImpl::i_getServices()
{
    logd_ctx_init ctx;
    TRACE("[CALL] Fred::Logger::ManagerImpl::i_getServices()");

    Database::Connection conn = Database::Manager::acquire();
    std::string query = "SELECT id, name FROM service";
    return conn.exec(query);
}

Result ManagerImpl::i_getResultCodesByService(ServiceType service)
{
    logd_ctx_init ctx;
    Connection conn = Database::Manager::acquire();
    TRACE("[CALL] Fred::Logger::ManagerImpl::i_getResultCodesByService");
    boost::format query = boost::format("select result_code, name from result_code where service_id = %1%") % service;
    return conn.exec(query.str());
}

Result ManagerImpl::i_getObjectTypes()
{
    Database::Connection conn = Database::Manager::acquire();
    Result res = conn.exec("SELECT id, type FROM object_type");

    return res;
}


// ManagerImpl ctor: connect to the database and fill property_names map
ManagerImpl::ManagerImpl(const std::string &monitoring_hosts_file)
{
        std::ifstream file;

    logd_ctx_init ctx;

        Connection conn = Database::Manager::acquire();

    logger_notice("Logger startup - successfully connected to DATABASE ");

    if (!monitoring_hosts_file.empty()) {
        try {
                        std::string log_monitoring ("List of IP addresses of monitoring hosts: ");
            file.open(monitoring_hosts_file.c_str());
            // TODO
            // Error while reading config file test_log_monitoring.conf : basic_ios::clear
            // file.exceptions(std::ios::badbit | std::ios::failbit);

            while(file) {
                std::string input;
                file >> input;
                monitoring_ips.push_back(input);
                                log_monitoring = log_monitoring + input + " ";
            }

                        logger_notice(log_monitoring.c_str());

        } catch(std::exception &e) {
            LOGGER("fred-server").error(boost::format("Error while reading config file %1% : %2%") % monitoring_hosts_file % e.what());
        }
    }

    // now fill the property_names map:
    pcache.reset(new RequestPropertyNameCache(conn));

}

ManagerImpl::~ManagerImpl() {
  logd_ctx_init ctx;

  logger_notice("Logging destructor");
}

// check if a log record with the specified ID exists and if it can be modified (time_end isn't set yet)

#ifdef LOGD_VERIFY_INPUT
// optimization
bool ManagerImpl::record_check(ID id, Connection &conn)
{
    TRACE("[CALL] Fred::Logger::ManagerImpl::record_check");

    boost::format query = boost::format("select time_end from request where id=%1%") % id;
    Result res = conn.exec(query.str());

    // if there is no record with specified ID
    if(res.size() == 0) {
        logger_error(boost::format("record in request with ID %1% doesn't exist") % id);
        return false;
    }

    if(!res[0][0].isnull()) {
        logger_error(boost::format("record with ID %1% was already completed") % id);
        return false;
    }

    return true;
}
#endif //LOGD_VERIFY_INPUT



// insert properties for the given request record
void ManagerImpl::insert_props(DateTime request_time, ServiceType service, bool monitoring, ID request_id,  const Fred::Logger::RequestProperties& props, Connection &conn, bool output)
{
        TRACE("[CALL] Fred::Logger::ManagerImpl::insert_props");
    ID property_name_id, last_id = 0;

    if(props.size() == 0) {
        return;
    }

    // process the first record
    property_name_id = pcache->find_property_name_id(props[0].name, conn);

    if (props[0].child) {
        // the first property is set to child - this is an error
        logger_error(boost::format("entry ID %1%: first property marked as child. Ignoring this flag ") % request_id);
    }

    ModelRequestPropertyValue pv_first;
    pv_first.setRequestTimeBegin(request_time);
    pv_first.setRequestServiceId(service);
    pv_first.setRequestMonitoring(monitoring);
    pv_first.setRequestId(request_id);
    pv_first.setPropertyNameId(property_name_id);
    pv_first.setValue (props[0].value);
    pv_first.setOutput(output);

    pv_first.insert();
        last_id = pv_first.getId();

    // process the rest of the sequence
    for (unsigned i = 1; i < props.size(); i++) {
        property_name_id = pcache->find_property_name_id(props[i].name, conn);

        // create a new object for each iteration
        // because ParentId must alternate between NULL and some value
        ModelRequestPropertyValue pv;
        pv.setRequestTimeBegin(request_time);
        pv.setRequestServiceId(service);
        pv.setRequestMonitoring(monitoring);
        pv.setRequestId(request_id);
        pv.setPropertyNameId(property_name_id);
        pv.setValue (props[i].value);
        pv.setOutput(output);

        if(props[i].child) {
            pv.setParentId(last_id);
            pv.insert();
        } else {
            pv.insert();
                        last_id = pv.getId();
        }
    }

        logger_notice(boost::format("Inserted %1% properties") % props.size());

}

void ManagerImpl::insert_obj_ref(DateTime request_time, ServiceType service, bool monitoring, ID request_id,  const Fred::Logger::ObjectReferences &refs, Connection &conn)
{

    for (unsigned i = 0; i<refs.size(); i++) {
        const std::string & obj_type = refs[i].type;
        int type_id;

        boost::format find_type_query = boost::format("select id from request_object_type where name='%1%'") % obj_type;

        Result res_type = conn.exec(find_type_query.str());
        if (res_type.size() == 0) {
                boost::format msg = boost::format("Object type with name '%1%' does not exist") % obj_type;
        logger_error(msg);
                throw InternalServerError(msg.str());
        } else {
                type_id = res_type[0][0];
        }

        boost::format insert_query  = boost::format("INSERT INTO request_object_ref (request_time_begin, request_service_id, request_monitoring, request_id, object_type_id, object_id) VALUES ('%1%', %2%, %3%, %4%, %5%, %6%)") % request_time % service % (monitoring ? "true" : "false") % request_id % type_id % refs[i].id;

        conn.exec(insert_query.str());

    }

}

void ManagerImpl::insert_props_pub(DateTime request_time, ServiceType request_service_id, bool monitoring, ID request_id, const Fred::Logger::RequestProperties& props) {
        Connection conn = get_connection();
        // insert_props for migration is not going to be so simple, TODO - true here is just TEMP
    insert_props(request_time, request_service_id, monitoring, request_id, props, conn, true);
}


// log a new event, return the database ID of the record
ID ManagerImpl::i_createRequest(const char *sourceIP, ServiceType service, const char *content, const Fred::Logger::RequestProperties& props, const Fred::Logger::ObjectReferences &refs, RequestType request_type_id, ID session_id)
{
        logd_ctx_init ctx;
#ifdef HAVE_LOGGER
        boost::format sess_fmt = boost::format("session-%1%") % session_id;
        Logging::Context ctx_sess(sess_fmt.str());
#endif
        TRACE("[CALL] Fred::Logger::ManagerImpl::i_createRequest");
        std::auto_ptr<Logging::Context> ctx_entry;

        logd_auto_db db;

    ID request_id;

    // get UTC with microseconds
    DateTime time(microsec_clock::universal_time());

    std::list<std::string>::iterator it;

    ModelRequest req;
    bool monitoring = false;
    for (it = monitoring_ips.begin(); it != monitoring_ips.end(); it++) {
        if(sourceIP == *it) {
            monitoring = true;
            break;
        }
    }

    req.setIsMonitoring(monitoring);
    req.setTimeBegin(time);
    // watch out, these 2 values are passed by reference
    req.setServiceId(service);
    req.setRequestTypeId(request_type_id);

        if (session_id != 0){
                req.setSessionId(session_id);

                std::string user_name;
                Database::ID user_id;

                getSessionUser(db, session_id, &user_name, &user_id);

                if(user_name != std::string()) {
                    req.setUserName(user_name);
                }
                if(user_id != 0) {
                    req.setUserId(user_id);
                }
        }

    try {
        if(sourceIP != NULL && sourceIP[0] != '\0') {
            // make sure these values can be safely used in an SQL statement
            req.setSourceIp(sourceIP);
        }

        req.insert();
                request_id = req.getId();

#ifdef HAVE_LOGGER
                boost::format request_fmt = boost::format("request-%1%") % request_id;
                ctx_entry.reset(new Logging::Context(request_fmt.str()));

#endif
    } catch (Database::Exception &ex) {
        logger_error(ex.what());
        return 0;
    } catch (ConversionError &ex) {
        std::cout << "Exception text: " << ex.what() << std::endl;
                return 0;
    }

    try {
        ModelRequestData data;

        // insert into request_data
        if(content != NULL && content[0] != '\0') {
            data.setRequestTimeBegin(time);
            data.setRequestServiceId(service);
            data.setRequestMonitoring(monitoring);
            data.setRequestId(request_id);
            data.setContent(content);
            data.setIsResponse(false);

            data.insert();
        }

                if(props.size() > 0) {
                        insert_props(time, service, monitoring, request_id, props, db, false);
                }
                if(refs.size() > 0) {
                        insert_obj_ref(time, service, monitoring, request_id, refs, db);
                }

    } catch (Database::Exception &ex) {
        logger_error(ex.what());
                return 0;
    }

        db.commit();
    rcache.insert(req);
    return request_id;
}

// optimization
void ManagerImpl::getSessionUser(Connection &conn, ID session_id, std::string *user_name, Database::ID *user_id)
{
        TRACE("[CALL] Fred::Logger::ManagerImpl::getSessionUser");

    if (session_id != 0) {
                boost::format query = boost::format("select user_name, user_id from session where id = %1%") % session_id;
        Result res = conn.exec(query.str());

        if(res.size() == 0) {
                        boost::format msg = boost::format("Session with ID %1% does not exist.") % session_id;
            logger_error(msg);
                        throw InternalServerError(msg.str());
        }

                if(res[0][0].isnull()) *user_name = std::string();
                else *user_name = (std::string)res[0][0];

                if(res[0][1].isnull()) *user_id = 0;
                else *user_id = res[0][1];

    } else {
                *user_name = std::string();
                *user_id = 0;
        }

}

// update existing log record with given ID
bool ManagerImpl::i_addRequestProperties(ID id, const Fred::Logger::RequestProperties &props)
{
    logd_ctx_init ctx;
#ifdef HAVE_LOGGER
        boost::format request_fmt = boost::format("request-%1%") % id;
        Logging::Context ctx_entry(request_fmt.str());
#endif

        TRACE("[CALL] Fred::Logger::ManagerImpl::i_addRequestProperties");

        logd_auto_db db;

    try {
#ifdef LOGD_VERIFY_INPUT
        if (!record_check(id, db)) return false;
#endif

        DateTime request_time;
        ServiceType service_id;
        bool monitoring;

        try {
            const ModelRequest& mr = rcache.get(id);
            request_time = mr.getTimeBegin();
            service_id = mr.getServiceId();
            monitoring = mr.getIsMonitoring();
            // the record stays in cache for closeRequest
        }
        catch (RequestCache::NOT_EXISTS)
        {
            boost::format select = boost::format(
                    "SELECT time_begin, service_id, is_monitoring, "
                    "COALESCE(session_id,0) "
                    "FROM request where id = %1%") % id;
            Result res = db.exec(select.str());
            if (res.size() == 0) {
                logger_error(boost::format(
                        "Record with ID %1% not found in request table.") % id );
                return false;
            }
            request_time = res[0][0].operator ptime();
            service_id = (ServiceType)(int) res[0][1];
            monitoring = (bool)res[0][2];
        }

        insert_props(request_time, service_id, monitoring, id, props, db, true);
    } catch (Database::Exception &ex) {
        logger_error(ex.what());
        throw InternalServerError(ex.what());
    }

    db.commit();
    return true;

}

// close the record with given ID (end time is filled thus no further
// modification is possible after this call)
// TODO session_id is optional - refactor code
bool ManagerImpl::i_closeRequest(
        ID id,
        const char *content,
        const Fred::Logger::RequestProperties &props,
        const Fred::Logger::ObjectReferences &refs,
        const long result_code,
        ID session_id
)
{
    logd_ctx_init ctx;
    logd_auto_db db;
    DateTime request_time;
    ServiceType service_id;
    bool monitoring;
    ID old_session_id;
    try {
        const ModelRequest& mr = rcache.get(id);
        request_time = mr.getTimeBegin();
        service_id = mr.getServiceId();
        monitoring = mr.getIsMonitoring();
        old_session_id = mr.getSessionId();
        rcache.remove(id);
    }
    catch (RequestCache::NOT_EXISTS)
    {
        boost::format select = boost::format(
                "SELECT time_begin, service_id, is_monitoring, "
                "COALESCE(session_id,0) "
                "FROM request where id = %1%") % id;
        Result res = db.exec(select.str());
        if (res.size() == 0) {
            logger_error(boost::format(
                    "Record with ID %1% not found in request table.") % id );
            return false;
        }
        request_time = res[0][0].operator ptime();
        service_id = (ServiceType)(int) res[0][1];
        monitoring = (bool)res[0][2];
        old_session_id = (unsigned)res[0][3];
    }
#ifdef HAVE_LOGGER
    boost::format sess_fmt = boost::format("session-%1%") %
            (session_id ? session_id : old_session_id);
    Logging::Context ctx_sess(sess_fmt.str());
    boost::format request_fmt = boost::format("request-%1%") % id;
    Logging::Context ctx_entry(request_fmt.str());
#endif

    TRACE("[CALL] Fred::Logger::ManagerImpl::i_closeRequest");

    try {
#ifdef LOGD_VERIFY_INPUT
                boost::format query_check;
                query_check = boost::format("select session_id, time_end from request where id=%1%") % id;
                // TODO you really should update the session_id

        Result res_check = db.exec(query_check.str());

        // if there is no record with specified ID
        if(res_check.size() == 0) {
            logger_error(boost::format("record in request with ID %1% doesn't exist") % id);
            return false;
        }

                if(!res_check[0][1].isnull()) {
                        logger_error(boost::format("record with ID %1% was already completed") % id);
                        return false;
                }

        if(session_id != 0 && !res_check[0][0].isnull()) {
                ID filled = res_check[0][0];
                        if(filled != 0 && filled != session_id) {
                                logger_error(boost::format("record with ID %1% already has session_id filled") % id);
                                throw WrongUsageError(" session_id already set. ");
                        }
        }
#endif // LOGD_VERIFY_INPUT

    //set time_end
    std::string query("UPDATE request SET time_end = $1::timestamp");
    Database::QueryParams update_request_params;
    update_request_params.push_back(
            boost::posix_time::to_iso_extended_string(
                    microsec_clock::universal_time()));

    //preallocate
    query.reserve(512);
    update_request_params.reserve(20);

    if(session_id != 0) {
        //set session_id
        update_request_params.push_back(
                boost::lexical_cast<std::string>(session_id));
        query += ", session_id = $"
            + boost::lexical_cast<std::string>(update_request_params.size())
            +"::bigint";

        std::string user_name;
        Database::ID user_id;
        getSessionUser(db, session_id, &user_name, &user_id);

        if(!user_name.empty()) {
            //set user_name
            update_request_params.push_back(user_name);
            query += ", user_name = $"
                    + boost::lexical_cast<std::string>(update_request_params.size())
                    +"::text";
        }
        if(user_id != 0) {
            //set user_id
            update_request_params.push_back(
                    boost::lexical_cast<std::string>(user_id));
            query += ", user_id = $"
                    + boost::lexical_cast<std::string>(update_request_params.size())
                    + "::bigint";
        }
    }
    //set service_id
    update_request_params.push_back(
            boost::lexical_cast<std::string>(service_id));
    query += ", result_code_id=get_result_code_id( $"
            + boost::lexical_cast<std::string>(update_request_params.size())
            + "::integer";

    //set result_code
    update_request_params.push_back(
            boost::lexical_cast<std::string>(result_code));
    query += ", $"
            + boost::lexical_cast<std::string>(update_request_params.size())
            + "::integer" + " )";

    //by id
    update_request_params.push_back(
            boost::lexical_cast<std::string>(id));
    query += " WHERE id = $"
            + boost::lexical_cast<std::string>(update_request_params.size())
            + "::bigint";

    //by time_begin
    update_request_params.push_back(
            boost::posix_time::to_iso_extended_string(request_time));
    query += " and time_begin = $"
            + boost::lexical_cast<std::string>(update_request_params.size())
            + "::timestamp";

    //by service_id
    update_request_params.push_back(
                boost::lexical_cast<std::string>(service_id));
    query += " and service_id = $"
            + boost::lexical_cast<std::string>(update_request_params.size())
            + "::bigint";

    //by is_monitoring
    update_request_params.push_back(monitoring);
    query += " and is_monitoring = $"
                + boost::lexical_cast<std::string>(update_request_params.size())
                + "::boolean";

    db.exec_params(query, update_request_params);

    // insert output content
    if(content != NULL && content[0] != '\0') {
        ModelRequestData data;
        // insert into request_data
        data.setRequestTimeBegin(request_time);
        data.setRequestServiceId(service_id);
        data.setRequestMonitoring(monitoring);
        data.setRequestId(id);
        data.setContent(content);
        data.setIsResponse(true);
        data.insert();
    }

    // insert properties
    if(props.size() > 0)
        insert_props(request_time, service_id, monitoring, id, props, db, true);
    if(refs.size() > 0)
        insert_obj_ref(request_time, service_id, monitoring, id, refs, db);

    } catch (Database::Exception &ex) {
        logger_error(ex.what());
                throw InternalServerError(ex.what());
    }

        db.commit();
        return true;
}

ID ManagerImpl::i_createSession(ID user_id, const char *name)
{
    logd_ctx_init ctx;
        TRACE("[CALL] Fred::Logger::ManagerImpl::i_createSession");

        std::auto_ptr<Logging::Context> ctx_sess;

    ID session_id;

    logger_notice(boost::format("createSession: username-> [%1%] user_id-> [%2%]") % name %  user_id);

    DateTime time(microsec_clock::universal_time());

    ModelSession sess;
        sess.setLoginDate(time);

    if (name != NULL && *name != '\0') {
        sess.setUserName(name);
    } else {
        logger_error("createSession: name is empty!");
                throw WrongUsageError ("User name is empty");
    }

        if (user_id != 0) {
                sess.setUserId(user_id);
        }

    try {
                sess.insert();

                session_id = sess.getId();

#ifdef HAVE_LOGGER
                boost::format sess_fmt = boost::format("session-%1%") % session_id;
                ctx_sess.reset(new Logging::Context(sess_fmt.str()));
#endif

        } catch (Database::Exception &ex) {
                logger_error(ex.what());
                throw InternalServerError(ex.what());
    }
    return session_id;
}

bool ManagerImpl::i_closeSession(ID id)
{
    logd_ctx_init ctx;
#ifdef HAVE_LOGGER
        boost::format sess_fmt = boost::format("session-%1%") % id;
        Logging::Context ctx_sess(sess_fmt.str());
#endif

        TRACE("[CALL] Fred::Logger::ManagerImpl::i_closeSession");

        logd_auto_db db;
    std::string  time;

    logger_notice(boost::format("closeSession: session_id -> [%1%] ") % id );

    try {

#ifdef LOGD_VERIFY_INPUT
                boost::format query = boost::format("select logout_date from session where id=%1%") % id;
        Result res = db.exec(query.str());

        if(res.size() == 0) {
            logger_error(boost::format("record in session with ID %1% doesn't exist") % id);
            return false;
        }

        if(!res[0][0].isnull()) {
            logger_error(boost::format("record in session with ID %1% already closed") % id);
            return false;
        }
#endif //LOGD_VERIFY_INPUT

        boost::format update;
        time = boost::posix_time::to_iso_extended_string(microsec_clock::universal_time());

        update = boost::format("update session set logout_date = '%1%' where id=%2%") % time % id;

        db.exec(update.str());
    } catch (Database::Exception &ex) {
        logger_error(ex.what());
                throw InternalServerError(ex.what());
    }

        db.commit();
    return true;
}

Manager* Manager::create() {
    TRACE("[CALL] Fred::Logger::Manager::create()");
    return new ManagerImpl();
}

Manager *Manager::create(const std::string conn_db, const std::string &monitoring_hosts_file)
throw (Manager::DB_CONNECT_FAILED) {
    TRACE("[CALL] Fred::Logger::Manager::create(std::string, std::string)");
    return new ManagerImpl(monitoring_hosts_file);
}



} // namespace Logger
}
