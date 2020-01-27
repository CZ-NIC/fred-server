/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <pthread.h>
#include <boost/algorithm/string.hpp>
#include <utility>

#include "config.h"


#include "src/deprecated/libfred/requests/request.hh"
#include "src/deprecated/libfred/requests/request_manager.hh"
#include "src/deprecated/libfred/requests/request_list.hh"

#include "src/deprecated/libfred/requests/model_session.hh"
#include "src/deprecated/libfred/requests/model_request.hh"


namespace LibFred {
namespace Logger {

using namespace Database;

using namespace boost::posix_time;
using namespace boost::gregorian;




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
            logger_error(boost::format("couldn't set constraint exclusion : %1%") % ex.what());
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


unsigned long long insert_property_record_impl(
    Database::Connection &_conn,
    const DateTime &_request_time,
    ServiceType _service,
    bool _monitoring,
    unsigned long long _request_id,
    unsigned long long _property_name_id,
    const std::string &_value,
    bool _output,
    unsigned long long _parent_id)
{
    _conn.exec_params(
        "INSERT INTO request_property_value"
        " (request_time_begin, request_service_id, request_monitoring, request_id, property_name_id, value, output, parent_id)"
        " VALUES"
        " ($1::timestamp, $2::bigint, $3::boolean, $4::bigint, $5::bigint, $6::text, $7::boolean, $8::bigint)",
        Database::query_param_list
            (_request_time)
            (_service)
            (_monitoring)
            (_request_id)
            (_property_name_id)
            (_value)
            (_output)
            ((_parent_id == 0) ? Database::QPNull : _parent_id)
    );
    Database::Result rprop_id = _conn.exec("SELECT currval('request_property_value_id_seq'::regclass)");
    if (rprop_id.size() != 1)
    {
        throw std::runtime_error("cannot get id of last inserted property");
    }
    return static_cast<unsigned long long>(rprop_id[0][0]);
}


void insert_request_data_impl(
    Database::Connection &_conn,
    const DateTime &_time,
    ServiceType _service,
    bool _monitoring,
    unsigned long long _request_id,
    const std::string &_content,
    bool _is_response
)
{
    _conn.exec_params("INSERT INTO request_data"
        " (request_time_begin, request_service_id, request_monitoring,"
        " request_id, content, is_response)"
        " VALUES"
        " ($1::timestamp, $2::bigint, $3::boolean, $4::bigint, $5::text, $6::boolean)",
        Database::query_param_list
            (_time)
            (_service)
            (_monitoring)
            (_request_id)
            (_content)
            (_is_response ? "True" : "False")
    );
}


Result ManagerImpl::i_getRequestTypesByService(ServiceType service) 
{
    logd_ctx_init ctx;
    TRACE("[CALL] LibFred::Logger::ManagerImpl::i_getRequestTypesByService");

        Connection conn = Database::Manager::acquire();

    boost::format query = boost::format("select id, name from request_type where service_id = %1%") % service;

        return conn.exec(query.str());

}

Result ManagerImpl::i_getServices()
{
    logd_ctx_init ctx;
    TRACE("[CALL] LibFred::Logger::ManagerImpl::i_getServices()");

    Database::Connection conn = Database::Manager::acquire();
    std::string query = "SELECT id, name FROM service";
    return conn.exec(query);
}

Result ManagerImpl::i_getResultCodesByService(ServiceType service)
{
    logd_ctx_init ctx;
    TRACE("[CALL] LibFred::Logger::ManagerImpl::i_getResultCodesByService");

    Connection conn = Database::Manager::acquire();
    boost::format query = boost::format("select result_code, name from result_code where service_id = %1%") % service;
    return conn.exec(query.str());
}

Result ManagerImpl::i_getObjectTypes()
{
    logd_ctx_init ctx;
    TRACE("[CALL] LibFred::Logger::ManagerImpl::i_getRequestTypesByService");

    Database::Connection conn = Database::Manager::acquire();
    Result res = conn.exec("SELECT id, type FROM object_type");

    return res;
}


// ManagerImpl ctor: connect to the database and fill property_names map
ManagerImpl::ManagerImpl(const std::string &monitoring_hosts_file)
    : scache(1000, 300)
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
                boost::algorithm::trim(input);
                if (!input.empty())
                {
                    monitoring_ips.push_back(input);
                                log_monitoring = log_monitoring + input + " ";
                }
            }

                        logger_notice(log_monitoring.c_str());

        } catch(std::exception &e) {
            LOGGER.error(boost::format("Error while reading config file %1% : %2%") % monitoring_hosts_file % e.what());
            LOGGER.info("Monitoring hosts might not be recognized now");
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
    TRACE("[CALL] LibFred::Logger::ManagerImpl::record_check");

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
void ManagerImpl::insert_props(DateTime request_time, ServiceType service, bool monitoring, ID request_id,  const LibFred::Logger::RequestProperties& props, Connection &conn, bool output)
{
    TRACE("[CALL] LibFred::Logger::ManagerImpl::insert_props");
    unsigned long long property_name_id, last_id = 0;

    if(props.size() == 0) {
        return;
    }

    // process the first record
    property_name_id = pcache->find_property_name_id(props[0].name, conn);

    if (props[0].child)
    {
        // the first property is set to child - this is an error
        logger_error(boost::format("entry ID %1%: first property marked as child. Ignoring this flag ") % request_id);
    }

    last_id = insert_property_record_impl(
        conn,
        request_time,
        service,
        monitoring,
        request_id,
        property_name_id,
        props[0].value,
        output,
        0
    );
    // process the rest of the sequence
    for (unsigned i = 1; i < props.size(); i++) {
        property_name_id = pcache->find_property_name_id(props[i].name, conn);

        unsigned long long aux_last_id = insert_property_record_impl(
            conn,
            request_time,
            service,
            monitoring,
            request_id,
            property_name_id,
            props[i].value,
            output,
            (props[i].child ? last_id : 0)
        );
        if (props[i].child == false)
        {
            last_id = aux_last_id;
        }
    }

    logger_notice(boost::format("Inserted %1% properties") % props.size());
}

void ManagerImpl::insert_obj_ref(DateTime request_time, ServiceType service, bool monitoring, ID request_id,  const LibFred::Logger::ObjectReferences &refs, Connection &conn)
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

void ManagerImpl::insert_props_pub(DateTime request_time, ServiceType request_service_id, bool monitoring, ID request_id, const LibFred::Logger::RequestProperties& props) {
        Connection conn = get_connection();
        // insert_props for migration is not going to be so simple, TODO - true here is just TEMP
    insert_props(request_time, request_service_id, monitoring, request_id, props, conn, true);
}


// log a new event, return the database ID of the record
// EXCEPTIONS MUST be handled in the caller
ID ManagerImpl::i_createRequest(const char *sourceIP, ServiceType service, const char *content, const LibFred::Logger::RequestProperties& props, const LibFred::Logger::ObjectReferences &refs, RequestType request_type_id, ID session_id)
{
    logd_ctx_init ctx;
#ifdef HAVE_LOGGER
    boost::format sess_fmt = boost::format("session-%1%") % session_id;
    Logging::Context ctx_sess(sess_fmt.str());
#endif
    TRACE("[CALL] LibFred::Logger::ManagerImpl::i_createRequest");
    std::unique_ptr<Logging::Context> ctx_entry;

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

    // insert into request_data
    if(content != NULL && content[0] != '\0')
    {
        insert_request_data_impl(
            db,
            time,
            service,
            monitoring,
            request_id,
            content,
            false
        );
    }

    if(props.size() > 0) {
            insert_props(time, service, monitoring, request_id, props, db, false);
    }
    if(refs.size() > 0) {
            insert_obj_ref(time, service, monitoring, request_id, refs, db);
    }

    db.commit();
    rcache.insert(req);
    return request_id;
}

// optimization
void ManagerImpl::getSessionUser(Connection &conn [[gnu::unused]], ID session_id, std::string *user_name, Database::ID *user_id)
{
        TRACE("[CALL] LibFred::Logger::ManagerImpl::getSessionUser");

    if(session_id == 0) {
        *user_name = std::string();
        *user_id = 0;
        return;
    }

    try {
        std::shared_ptr<ModelSession> sess = scache.get(session_id);

        *user_name = sess->getUserName();
        *user_id = sess->getUserId();

    } catch (const CACHE_MISS&) {
        std::shared_ptr<ModelSession> sess(new ModelSession());

        sess->setId(session_id);
        sess->reload();

        *user_name = sess->getUserName();
        *user_id   = sess->getUserId();

        scache.add(sess->getId(), sess);

        /*
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
        */
    }

}

// close the record with given ID (end time is filled thus no further
// modification is possible after this call)
// EXCEPTIONS MUST be handled in the caller
bool ManagerImpl::i_closeRequest(
        ID id,
        const char *content,
        const LibFred::Logger::RequestProperties &props,
        const LibFred::Logger::ObjectReferences &refs,
        const long result_code,
        ID session_id
)
{
    logd_ctx_init ctx;
    logd_auto_db db;
    DateTime request_time;
    DateTime request_time_end;
    ServiceType service_id;
    bool monitoring;
    ID old_session_id;
    try {
        const ModelRequest& mr = rcache.get(id);
        request_time = mr.getTimeBegin();
        request_time_end = mr.getTimeEnd();
        service_id = mr.getServiceId();
        monitoring = mr.getIsMonitoring();
        old_session_id = mr.getSessionId();
        // it must be removed here, if not, the cache leaks
        rcache.remove(id);
    }
    catch (const RequestCache::NOT_EXISTS&)
    {
        boost::format select = boost::format(
                "SELECT time_begin, time_end, service_id, is_monitoring, "
                "COALESCE(session_id,0) "
                "FROM request where id = %1%") % id;
        Result res = db.exec(select.str());
        if (res.size() == 0) {
            logger_error(boost::format(
                    "Record with ID %1% not found in request table.") % id );
            return false;
        }
        request_time = res[0][0].operator ptime();
        request_time_end = res[0][1].operator ptime();
        service_id = (ServiceType)(int) res[0][2];
        monitoring = (bool)res[0][3];
        old_session_id = (unsigned)res[0][4];
    }


#ifdef HAVE_LOGGER
    boost::format sess_fmt = boost::format("session-%1%") %
            (session_id ? session_id : old_session_id);
    Logging::Context ctx_sess(sess_fmt.str());
    boost::format request_fmt = boost::format("request-%1%") % id;
    Logging::Context ctx_entry(request_fmt.str());
#endif

    TRACE("[CALL] LibFred::Logger::ManagerImpl::i_closeRequest");

    if(!request_time_end.is_special()) {
        boost::format msg = boost::format("record with ID %1% was already completed") % id;
        logger_error(msg);
        // time_end already filled - error
        throw InternalServerError(msg.str());
    }

#ifdef LOGD_VERIFY_INPUT
    boost::format query_check;
    query_check = boost::format("select session_id, time_end from request where id=%1%") % id;

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
    if(content != NULL && content[0] != '\0')
    {
        insert_request_data_impl(
            db,
            request_time,
            service_id,
            monitoring,
            id,
            content,
            true
        );
    }

    // insert properties
    if(props.size() > 0)
        insert_props(request_time, service_id, monitoring, id, props, db, true);
    if(refs.size() > 0)
        insert_obj_ref(request_time, service_id, monitoring, id, refs, db);

    db.commit();
    return true;
}

// EXCEPTIONS MUST be handled in the caller
ID ManagerImpl::i_createSession(ID user_id, const char *name)
{
    logd_ctx_init ctx;
        TRACE("[CALL] LibFred::Logger::ManagerImpl::i_createSession");

        std::unique_ptr<Logging::Context> ctx_sess;

    ID session_id;

    logger_notice(boost::format("createSession: username-> [%1%] user_id-> [%2%]") % name %  user_id);

    DateTime time(microsec_clock::universal_time());

    std::shared_ptr<ModelSession> sess(new ModelSession());
        sess->setLoginDate(time);

    if (name != NULL && *name != '\0') {
        sess->setUserName(name);
    } else {
        logger_error("createSession: name is empty!");
                throw WrongUsageError ("User name is empty");
    }

        if (user_id != 0) {
                sess->setUserId(user_id);
        }

    sess->insert();

    session_id = sess->getId();

#ifdef HAVE_LOGGER
    boost::format sess_fmt = boost::format("session-%1%") % session_id;
    ctx_sess.reset(new Logging::Context(sess_fmt.str()));
#endif
    scache.add(session_id, sess);

    return session_id;
}

// EXCEPTIONS MUST be handled in the caller
bool ManagerImpl::i_closeSession(ID id)
{
    logd_ctx_init ctx;
#ifdef HAVE_LOGGER
    boost::format sess_fmt = boost::format("session-%1%") % id;
    Logging::Context ctx_sess(sess_fmt.str());
#endif

    TRACE("[CALL] LibFred::Logger::ManagerImpl::i_closeSession");

    // remove the record from the cache before anything can throw
    scache.remove(id);
        
        logd_auto_db db;
    std::string  time;

    logger_notice(boost::format("closeSession: session_id -> [%1%] ") % id );

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

    db.commit();
    return true;
}


Database::ID getServiceIdForName(const std::string &service_name)
{
    Database::Connection conn = Database::Manager::acquire();

    Database::Result res_servid = conn.exec_params("SELECT id FROM service WHERE name=$1",
         Database::query_param_list (service_name));

    if(res_servid.size()!=1 || res_servid[0][0].isnull()) {
        throw WrongUsageError(
           (boost::format("Couldn't find corresponding FRED service id in the database, service name given as argument: %1%")
            % service_name).str());
    }

    Database::ID ret = res_servid[0][0];
    return ret;
}

void convert_timestamps_to_utc(ptime &from, ptime &to, const ptime &from_localtime, const ptime &to_localtime)
{
    Database::Connection conn = Database::Manager::acquire();

    Result res_times = conn.exec_params( " SELECT "
        "($1::timestamp AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC', "
        "($2::timestamp AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC' " ,
        Database::query_param_list
        (from_localtime)
        (to_localtime)
        );

    from = res_times[0][0];
    to   = res_times[0][1];
}


/*
 * Request count during specified period for specified service and request.user_name
 * timestamps in local time
 */
unsigned long long ManagerImpl::i_getRequestCount(
        const boost::posix_time::ptime &from_localtime,
        const boost::posix_time::ptime &to_localtime,
        const std::string &service,
        const std::string &user)
{
    logd_ctx_init ctx;
    TRACE("[CALL] LibFred::Logger::ManagerImpl::i_getRequestCount()");

    Database::ID service_id = getServiceIdForName(service);

    ptime datetime_from, datetime_to;
    convert_timestamps_to_utc(datetime_from, datetime_to, from_localtime, to_localtime);

    date from_date = datetime_from.date();
    date to_date   = datetime_to.date();

    unsigned long long total = 0;

    // last day
    if(to_date != from_date) {
        // first day of 2 or more
        total += getRequestCountWorker(datetime_from, ptime(from_date+days(1)), service_id, user);
        // last day of 2 or more
        total += getRequestCountWorker(ptime(to_date), datetime_to, service_id, user);
    } else {
        // from and to are within one day
        total += getRequestCountWorker(datetime_from, datetime_to, service_id, user);
    }

    // the rest
    for(date d = from_date+days(1);
        d < to_date;
        d += days(1)) {

            total += getRequestCountWorker(ptime(d), ptime(d+days(1)), service_id, user);
    }

    return total;
}


std::unique_ptr<RequestCountInfo>
ManagerImpl::i_getRequestCountUsers(
        const boost::posix_time::ptime &datetime_from_local,
        const boost::posix_time::ptime &datetime_to_local,
        const std::string &service)
{
    logd_ctx_init ctx;
    TRACE("[CALL] LibFred::Logger::ManagerImpl::i_getRequestCountUsers()");

    Database::Connection conn = Database::Manager::acquire();

    std::unique_ptr<RequestCountInfo> info (new RequestCountInfo());
    Database::ID service_id = getServiceIdForName(service);

    ptime datetime_from, datetime_to;
    convert_timestamps_to_utc(datetime_from, datetime_to, datetime_from_local, datetime_to_local);

    date from_date = datetime_from.date();
    date to_date   = datetime_to.date();

    if(to_date != from_date) {

        // first day of 2 or more
        incrementRequestCounts(info.get(),
                getRequestCountUsersWorker(
                            datetime_from, ptime(from_date+days(1)), service_id
                        )
        );

        // last day of 2 or more
        incrementRequestCounts(info.get(), getRequestCountUsersWorker(
                ptime(to_date) , datetime_to, service_id
            )
        );
    } else {
        // within one day
        incrementRequestCounts(info.get(),
                getRequestCountUsersWorker(
                            datetime_from, datetime_to, service_id
                        )
        );
    }

    // the rest - in case there are several days,
    //   0 iterations if there are 1 or 2 days
    for(date d = from_date+days(1);
        d < to_date;
        d += days(1)) {
            incrementRequestCounts(info.get(), getRequestCountUsersWorker(
                    ptime(d), ptime(d + days(1)), service_id
                )
            );
    }

    return info;
}


// timestamps are in UTC
Result ManagerImpl::getRequestCountUsersWorker(ptime from, ptime to, int service_id)
{
    Database::Connection conn = Database::Manager::acquire();

    Result res = conn.exec_params(
        " SELECT r.user_name, count(*) FROM request r "
        " LEFT JOIN ("
        "   request_property_value rpv "
        "   JOIN request_property_name rpn ON rpn.id = rpv.property_name_id "
        "       AND rpn.name = 'handle' "
        " ) ON ( "
        "   rpv.request_id = r.id "
        "  AND rpv.request_service_id = $1::integer "
        "  AND rpv.request_time_begin >= $2::timestamp "
        "  AND rpv.request_time_begin < $3::timestamp "
        "  AND rpv.request_monitoring = false "
        " ) "
        " JOIN result_code rc ON rc.id = r.result_code_id "
        "  AND rc.name NOT IN ('CommandFailed', 'CommandFailedServerClosingConnection') "
        " JOIN request_type rt ON rt.id = r.request_type_id "
        "  AND rt.name NOT IN ('PollAcknowledgement', 'PollResponse') "
        "WHERE r.service_id = $1::integer "
        " AND r.time_begin >= $2::timestamp "
        " AND r.time_begin < $3::timestamp "
        " AND r.is_monitoring = false "
        "GROUP BY r.user_name "
        "ORDER BY r.user_name; ",
        Database::query_param_list
                        (service_id)
                        (from)
                        (to)
    );

    return res;
}

unsigned long long ManagerImpl::getRequestCountWorker(ptime from, ptime to, int service_id, std::string user_name)
{
    Database::Connection conn = Database::Manager::acquire();

    Database::Result res =
    conn.exec_params(
        " SELECT count(*) FROM request r "
        " LEFT JOIN ( "
        "   request_property_value rpv "
        "   JOIN request_property_name rpn ON rpn.id = rpv.property_name_id "
        "       AND rpn.name = 'handle' "
        " ) ON ( "
        "  rpv.request_id = r.id "
        " AND rpv.request_service_id = $3::integer "
        " AND rpv.request_time_begin >= $1::timestamp "
        " AND rpv.request_time_begin < $2::timestamp "
        " AND rpv.request_monitoring = false "
        " )  "
        " JOIN result_code rc ON rc.id = r.result_code_id "
        "  AND rc.name NOT IN ('CommandFailed', 'CommandFailedServerClosingConnection') "
        " JOIN request_type rt ON rt.id = r.request_type_id "
        "  AND rt.name NOT IN ('PollAcknowledgement', 'PollResponse') "
        " WHERE r.service_id = $3::integer "
        " AND r.time_begin >= $1::timestamp "
        " AND r.time_begin < $2::timestamp "
        " AND r.is_monitoring = false "
        " AND r.user_name = $4; ",
       Database::query_param_list
                        (from)
                        (to)
                        (service_id)
                        (user_name)
                        );

    // count cannot be NULL or nonexistent, so we can check it here
    if(res.size() != 1 || res[0][0].isnull()) {
        throw InternalServerError("Count of records not found");
    }

    return res[0][0];
}

void ManagerImpl::incrementRequestCounts(RequestCountInfo *inf_ptr, Result res)
{

    RequestCountInfo &inf = *inf_ptr;

    for(size_t i=0;i<res.size();++i) {
        std::string user_handle = (std::string)res[i][0];
        unsigned long long count = (unsigned long long)res[i][1];

        RequestCountInfo::iterator it = inf.find(user_handle);

        if(it != inf.end()) {
            inf[user_handle] = it->second + count;
        } else {
            inf[user_handle] = count;
        }
    }
}

Manager* Manager::create() {
    TRACE("[CALL] LibFred::Logger::Manager::create()");
    return new ManagerImpl();
}

Manager *Manager::create(const std::string conn_db [[gnu::unused]], const std::string &monitoring_hosts_file) {
    TRACE("[CALL] LibFred::Logger::Manager::create(std::string, std::string)");
    return new ManagerImpl(monitoring_hosts_file);
}



} // namespace Logger
}
