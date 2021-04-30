/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#include <iostream>

/*
 * for non-header library version:
 * #include <boost/test/unit_test.hpp>
 * */

#include <stdio.h>

#include "src/util/time_clock.hh"

#include "src/deprecated/libfred/requests/request_impl.hh"
#include "src/deprecated/libfred/requests/request.hh"
#include "src/deprecated/libfred/requests/request_manager.hh"

#include "src/util/corba_wrapper_decl.hh"
#include "src/bin/corba/Logger.hh"

#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/cfg/config_handler_decl.hh"

// these should be extras for threaded test
#include "src/util/concurrent_queue.hh"
#include "src/util/concurrent_set.hh"
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <utility>
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "test/setup/tests_common.hh"


using namespace Database;
using namespace ::LibFred::Logger;



namespace TestLogd {

//args processing config for custom main
// TODO this should be taken from the database
enum LogServiceType { LC_NO_SERVICE = -1, LC_UNIX_WHOIS=0, LC_WEB_WHOIS, LC_PUBLIC_REQUEST, LC_EPP, LC_WEBADMIN, LC_INTRANET, LC_MAX_SERVICE };

const int MONTHS_COUNT  = 2;

// value from database table request_type
const int UNKNOWN_ACTION = 1000;

//whether to test partitioning on request_property_value table
const bool PARTITIONS_TEST_PROPERTIES = true;

boost::format get_table_postfix(int year, int month, ServiceType service_num, bool monitoring);
boost::format get_table_postfix_for_now(ServiceType service_num, bool monitoring);
std::string create_date_str(int y, int m);

struct MyFixture {
	static std::list<ID> id_list_entry;
	static std::list<ID> id_list_session;
	static concurrent_set<ID>  id_list_property_name;

	MyFixture() {
	}

	~MyFixture() {
		try {
			std::list<ID>::iterator it = id_list_entry.begin();
			Connection conn = Database::Manager::acquire();

			BOOST_TEST_MESSAGE("Deleting database records from request and related. " );
			while(it != id_list_entry.end()) {
				conn.exec( (boost::format("delete from request_data where request_id=%1%") % *it).str() );
				conn.exec( (boost::format("delete from request_property_value where request_id=%1%") % *it).str() );
				conn.exec( (boost::format("delete from request where id=%1%") % *it).str() );
				it++;
			}

			BOOST_TEST_MESSAGE( "Deleting database records from session." );
			for(it = id_list_session.begin(); it != id_list_session.end();it++) {
				conn.exec( (boost::format("delete from session where id=%1%") % *it).str() );
			}

			BOOST_TEST_MESSAGE( "Deleting properties created as part of the test");
			for(std::set<ID>::iterator it = id_list_property_name.begin();
			        it != id_list_property_name.end();
			        it++) {
			    conn.exec((boost::format("delete from request_property_name where id = %1%") % *it).str());
			}
		} catch (Database::Exception &ex) {
		    BOOST_TEST_MESSAGE( (boost::format("error when working with database (%1%) : %2%") % CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info() % ex.what()).str());
		}
	}

};

std::list<ID> MyFixture::id_list_entry;
std::list<ID> MyFixture::id_list_session;
concurrent_set<ID>  MyFixture::id_list_property_name;

} // namespace TestLogd
BOOST_FIXTURE_TEST_SUITE(TestLogd, MyFixture)

//BOOST_AUTO_TEST_SUITE(TestLogd)
//BOOST_GLOBAL_FIXTURE( MyFixture );



class TestImplLog {
	// TODO this should follow the common ways with create(...)
        std::unique_ptr<::LibFred::Logger::ManagerImpl> logd;
        std::unique_ptr<::LibFred::Logger::RequestPropertyNameCache> pcache;


public:
    TestImplLog (const std::string /*connection_string*/) :
        logd(new ::LibFred::Logger::ManagerImpl())
    {
        Connection conn = Database::Manager::acquire();
        pcache.reset(new ::LibFred::Logger::RequestPropertyNameCache(conn));

    };

	TestImplLog (const std::string /*connection_string*/, const std::string monitoring_file) :
	    logd(new ::LibFred::Logger::ManagerImpl(monitoring_file))
    // , pcache(new ::LibFred::Logger::RequestPropertyNameCache(Database::Manager::acquire()))
    {};

	Database::ID createSession(Database::ID user_id, const char *user_name);
	bool         closeSession(Database::ID id);

	// TODO change this and test combination of session / request (session)
	Database::ID createRequest(const char *ip_addr, const ServiceType serv, const char * content, const ::LibFred::Logger::RequestProperties &props = TestImplLog::no_props, bool is_monitoring = false, const ::LibFred::Logger::ObjectReferences &refs = TestImplLog::no_objs, Database::ID session_id=0, Database::ID request_type_id = UNKNOWN_ACTION);

	bool closeRequest(const Database::ID id, const char * content, const ::LibFred::Logger::RequestProperties &props = TestImplLog::no_props, const ::LibFred::Logger::ObjectReferences &refs = TestImplLog::no_objs, long result_code = 1000, Database::ID session_id = 0);

	// to encapsulate some methods which are not part of the interface

	ID find_property_name(const std::string &name);

    unsigned long long getRequestCount( const boost::posix_time::ptime &from,
                                        const boost::posix_time::ptime &to,
                                        const std::string              &service,
                                        const std::string              &user
                                        ) {

        unsigned long long ret = logd->i_getRequestCount(from, to, service, user);

      // TODO proper test
        return ret;
    }

    std::unique_ptr<RequestCountInfo> getRequestCountUsers(const boost::posix_time::ptime &from,
                                                        const boost::posix_time::ptime &to,
                                                        const std::string              &service) {
        return logd->i_getRequestCountUsers(from, to , service);
    }

	// auxiliary testing functions
	std::unique_ptr<::LibFred::Logger::RequestProperties> create_generic_properties(int number, int value_id);
	std::unique_ptr<::LibFred::Logger::RequestProperties> create_properties_req_count(unsigned num_handles, unsigned num_others, int value_id);

        void check_obj_references(ID rec_id, const ::LibFred::Logger::ObjectReferences &refs);
        void check_obj_references_subset(ID rec_id, const ::LibFred::Logger::ObjectReferences &refs);
	void check_db_properties_subset(ID rec_id, const ::LibFred::Logger::RequestProperties &props, bool output);
	bool property_match(const Row r, const ::LibFred::Logger::RequestProperty &p) ;

	void insert_custom_request(ptime timestamp, const std::string &user);

// different tests
	void check_db_properties(ID rec_id, const ::LibFred::Logger::RequestProperties & props);

	static ::LibFred::Logger::RequestProperties no_props;
        static ::LibFred::Logger::ObjectReferences no_objs;
};

inline bool has_content(const char *str) {
	return (str && *str!= '\0');
}



boost::format get_table_postfix_for_now(ServiceType service_num, bool monitoring)
{
	boost::posix_time::ptime utime = microsec_clock::universal_time();
	tm str_time = boost::posix_time::to_tm(utime);

	// months in tm are numbered from 0
	str_time.tm_mon++;

	return get_table_postfix((1900 + str_time.tm_year), str_time.tm_mon, service_num, monitoring);

}

// TODO - rewrite - use strings from database
boost::format get_table_postfix(int year, int month, ServiceType service_num, bool monitoring)
{
	int shortyear = (year - 2000) % 100;
	std::string service_name("UNKNOWN");

	if (service_num == LC_NO_SERVICE) {
		// in this special case monitoring flag doesn't matter, postfix contains only the date part

		return boost::format("%2$02d_%3$02d") % service_name % shortyear % month;

	} else if (monitoring) {
		// special tables for all monitoring requests

		service_name = std::string("mon");

	} else {
		switch(service_num) {
			case LC_UNIX_WHOIS:
				service_name = std::string("whois");
				break;
			case LC_WEB_WHOIS:
				service_name = std::string("webwhois");
				break;
			case LC_PUBLIC_REQUEST:
				service_name = std::string("pubreq");
				break;
			case LC_EPP:
				service_name = std::string("epp");
				break;
			case LC_WEBADMIN:
				service_name = std::string("webadmin");
				break;
			case LC_INTRANET:
				service_name = std::string("intranet");
                                break;
		}
	}

	return boost::format("%1%_%2$02d_%3$02d") % service_name % shortyear % month;
}

std::string create_date_str(int y, int m)
{
	boost::format ret;

	if (m == 13) {
		ret  = boost::format("%1$02d-01-01") % (y+1);
	} else {
		ret  = boost::format("%1$02d-%2$02d-01") % y % m;
	}

	return ret.str();
}

Database::ID get_request_type_id(const std::string &request_name)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec_params("select id FROM request_type WHERE name = $1::text", Database::query_param_list(request_name) );

    if(res.size() != 1) {
        throw std::runtime_error("Couldn't find unique request type with given name. ");
    }

    return res[0][0];
}

Database::ID get_result_code_id(const std::string &result_code_name)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec_params("SELECT id FROM result_code WHERE name = $1::text", Database::query_param_list(result_code_name));

    if(res.size() != 1) {
        throw std::runtime_error("Couldn't find unique result code for the given name");
    }

    return res[0][0];
}

struct MyFixture;

LibFred::Logger::RequestProperties TestImplLog::no_props;
LibFred::Logger::ObjectReferences TestImplLog::no_objs;


Database::ID TestImplLog::createSession(Database::ID user_id, const char *user_name)
{
	Database::ID ret = logd->i_createSession(user_id, user_name);

	if(ret == 0) return 0;

        Database::Connection conn (Database::Manager::acquire());

	// first check if the correct partition was used ...
	boost::format test = boost::format("select login_date from session_%1% where id = %2%") % get_table_postfix_for_now(LC_NO_SERVICE, false) % ret;
	Result res = conn.exec(test.str());

	if(res.size() == 0) {
		BOOST_ERROR(" Record not found in the correct partition ");
	}

	// now do a regular select from session
        // res = conn.exec((boost::format("select user_id, name from session where id=%1%") % ret).str());
	res = conn.exec((boost::format("select user_name from session where id=%1%") % ret).str());

	if (res.size() != 1) {
		if (res.size() == 0) {
			BOOST_ERROR(boost::format(" Record created with createSession with id %1% doesn't exist in the database! ") % ret);
		} else if(res.size() > 1) {
			BOOST_ERROR(boost::format(" Multiple records with id %1% after call to createSession! ") % ret);
		}
	} else {
            // TODO
                BOOST_CHECK(std::string(user_name) == (std::string)res[0][0]);
	}

	MyFixture::id_list_session.push_back(ret);

	return ret;
}

void TestImplLog::insert_custom_request(ptime timestamp, const std::string &user)
{
    // request with different time
    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params("INSERT INTO request(time_begin, service_id, is_monitoring, user_name) VALUES "
            "(($1::timestamp AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC', 3, false, $2)", Database::query_param_list
                (timestamp)
                (user)
            );

    Result res = conn.exec("SELECT currval('request_id_seq') AS id");

    ID ret = res[0][0];

    MyFixture::id_list_entry.push_back(ret);

    conn.exec_params("UPDATE request SET result_code_id = 9 WHERE id = $1",
            Database::query_param_list (ret)
    );
}

bool TestImplLog::closeSession(Database::ID id)
{
	bool ret = logd->i_closeSession(id);

        Database::Connection conn (Database::Manager::acquire());

	if (!ret) return ret;

	Result res = conn.exec( (boost::format("select logout_date from session where id=%1%") % id).str() );

	if (res.size() != 1) {
		if (res.size() == 0) {
			BOOST_ERROR(boost::format(" Record created with createRequest with id %1% doesn't exist in the database! ") % id);
		} else if(res.size() > 1) {
			BOOST_ERROR(boost::format(" Multiple records with id %1% after call to createRequest! ") % id);
		}
	} else {
		BOOST_CHECK (!res[0][0].isnull());
	}

	return ret;
}

//TODO check object refs
Database::ID TestImplLog::createRequest(const char *ip_addr, const ServiceType serv, const char * content, const ::LibFred::Logger::RequestProperties &props, bool is_monitoring, const ::LibFred::Logger::ObjectReferences &refs, Database::ID session_id, Database::ID request_type_id)
{

	if(serv > LC_MAX_SERVICE) {
		BOOST_FAIL(boost::format (" ---------Invalid service num %1% ") % serv);
	}

	Database::ID ret = logd->i_createRequest(ip_addr, serv, content, props, refs, request_type_id, session_id); boost::format query;

	if(ret == 0) return 0;

	Connection conn = Database::Manager::acquire();
	// first check if the correct partition was used ...

	boost::format test = boost::format("select time_begin from request_%1% where id = %2%") % get_table_postfix_for_now(serv, is_monitoring) % ret;
	Result res = conn.exec(test.str());

	if(res.size() == 0) {
		BOOST_ERROR(" Record not found in the correct partition ");
	}

	// now a regular select

	query = boost::format ( "select source_ip, service_id, raw.content, session_id from request r join request_data raw on raw.request_id=r.id where r.id=%1%") % ret;
	res = conn.exec(query.str());

	if (res.size() != 1) {
		res = conn.exec( (boost::format("select source_ip, service_id, session_id from request where id=%1%") % ret).str() );

		if (res.size() != 1) {
			if (res.size() == 0) {
				BOOST_ERROR(boost::format(" Record created with createRequest with id %1% doesn't exist in the database! ") % ret);
			} else if(res.size() > 1) {
				BOOST_ERROR(boost::format(" Multiple records with id %1% after call to createRequest! ") % ret);
			}
		} else {
			if(has_content(ip_addr)) BOOST_CHECK(std::string(ip_addr) 	== (std::string)res[0][0]);
			BOOST_CHECK((int)serv 			== (int)res[0][1]);
                        Database::ID db_id = res[0][2];
                        BOOST_CHECK(session_id                  == db_id);
		}

	} else {
		if(has_content(ip_addr)) BOOST_CHECK(std::string(ip_addr) 	== (std::string)res[0][0]);
		BOOST_CHECK((int)serv 			== (int)res[0][1]);
		BOOST_CHECK(std::string(content) 	== (std::string)res[0][2]);
                Database::ID db_id = res[0][3];
                BOOST_CHECK(session_id                  == db_id);
	}

	check_db_properties(ret, props);
        // TODO
        check_obj_references(ret, refs);

	MyFixture::id_list_entry.push_back(ret);

	return ret;
}


bool TestImplLog::closeRequest(const Database::ID id, const char *content, const ::LibFred::Logger::RequestProperties &props, const ::LibFred::Logger::ObjectReferences &refs, long result_code, Database::ID session_id)
{
	bool result = logd->i_closeRequest(id, content, props, refs, result_code, session_id);

	if(!result) return result;

	Connection conn = Database::Manager::acquire();
	boost::format query = boost::format ( "select time_end, raw.content, session_id from request r join request_data raw on raw.request_id=r.id where raw.is_response=true and r.id=%1%") % id;
	Result res = conn.exec(query.str());

	if (res.size() != 1) {
		res = conn.exec( (boost::format("select time_end, session_id from request where id=%1%") % id).str() );

		if (res.size() != 1) {
			if (res.size() == 0) {
				BOOST_ERROR(boost::format(" Record created with createRequest with id %1% doesn't exist in the database! ") % id);
			} else if(res.size() > 1) {
				BOOST_ERROR(boost::format(" Multiple records with id %1% after call to createRequest! ") % id);
			}
		} else {
			BOOST_CHECK(!res[0][0].isnull());
			if(session_id != 0) {
                Database::ID db_id = res[0][1];
                BOOST_CHECK(session_id == db_id);
			}
		}

	} else {
		BOOST_CHECK(!res[0][0].isnull());
		BOOST_CHECK(std::string(content) == (std::string)res[0][1]);
		if(session_id != 0) {
            Database::ID db_id = res[0][2];
            BOOST_CHECK(session_id == db_id);
		}
	}

	check_db_properties_subset(id, props, true);
        // TODO
    check_obj_references_subset(id, refs);

	return result;
}

ID TestImplLog::find_property_name(const std::string &name) {

    Connection conn = Database::Manager::acquire();

    Database::Transaction tx(conn);
    ID ret_id = pcache->find_property_name_id(name, conn);

    // TODO concurrency
    MyFixture::id_list_property_name.insert(ret_id);

    tx.commit();

    return ret_id;
}

std::unique_ptr<::LibFred::Logger::RequestProperties> TestImplLog::create_generic_properties(int number, int value_id)
{
	std::unique_ptr<::LibFred::Logger::RequestProperties> ret(new ::LibFred::Logger::RequestProperties(number));
	LibFred::Logger::RequestProperties &ref = *ret;

	for(int i=0;i<number;i++) {
		ref[i].name = "handle";
		ref[i].value = (boost::format("val%1%.%2%") % value_id % i).str();

		ref[i].child = false;
	}

	return ret;
}

std::unique_ptr<::LibFred::Logger::RequestProperties> TestImplLog::create_properties_req_count(unsigned num_handles, unsigned num_others, int value_id)
{
    std::unique_ptr<::LibFred::Logger::RequestProperties> ret(new ::LibFred::Logger::RequestProperties(num_handles + num_others));
    ::LibFred::Logger::RequestProperties &ref = *ret;

    for(unsigned int i=0;i<num_handles;i++) {
        ref[i].name = "handle";
        ref[i].value = (boost::format("val%1%.%2%") % value_id % i).str();

        ref[i].child = false;
    }

    for(unsigned int i=num_handles;i< (num_others+num_handles);i++) {
        ref[i].name = "not_handle";
        ref[i].value = (boost::format("val%1%.%2%") % value_id % i).str();

        ref[i].child = false;
    }

    return ret;
}

// r is row produced by:
// boost::format query = boost::format("select name, value, parent_id, output from request_property_value pv join request_property_name pn on pn.id=pv.property_name_id where pv.request_id = %1% order by pv.id") % rec_id;
// p is single property
// these two are compared :)
bool TestImplLog::property_match(const Row r, const ::LibFred::Logger::RequestProperty &p)
{

	if ( (std::string)r[0] != p.name)  return false;
	if ( (std::string)r[1] != p.value) return false;

	if ( p.child ) {
		if (r[2].isnull()) return false;
	} else {
		if (!r[2].isnull()) return false;
	}

	return true;
}

void TestImplLog::check_db_properties_subset(ID rec_id, const ::LibFred::Logger::RequestProperties &props, bool)
{
	if (props.size() == 0) return;

	boost::format query = boost::format("select name, value, parent_id from request_property_value pv join request_property_name pn on pn.id=pv.property_name_id where pv.request_id = %1% order by pv.id") % rec_id;

	Connection conn = Database::Manager::acquire();
	Result res = conn.exec(query.str());

	// this is expected for a *_subset function...
	unsigned pind = 0;
	if(res.size() > props.size()) {
		for(unsigned i=0; i<res.size(); i++) {
			if(property_match(res[i], props[pind])) {
				// property pind found in the sql result, proceed to another item in the list
				pind++;
			} else {
				// doesn't match, continue to the next row of the result
				continue;
			}
		}

		if(pind < props.size()) {
			BOOST_ERROR(boost::format(" Some properties were not found in database for record %1%") % rec_id);
		}
	// but this is kinda' weird, something had to go wrong....
	} else if(res.size() < props.size()) {
		BOOST_ERROR(" Some properties were not stored... ");
	} else if(res.size() == props.size()) {
		// TODO something can be saved here - we have Result res already done
		check_db_properties(rec_id, props);
	}
}

// this func relies that the order of properties in the database
// (sorted by their ids) is the same as in the array
// the properties in the database with request_id=rec_id must match props exactly
void TestImplLog::check_db_properties(ID rec_id, const ::LibFred::Logger::RequestProperties & props)
{
	boost::format query = boost::format("select name, value, parent_id from request_property_value pv join request_property_name pn on pn.id=pv.property_name_id where pv.request_id = %1% order by pv.id") % rec_id;

	Connection conn = Database::Manager::acquire();

	Result res = conn.exec(query.str());

	if (res.size() != props.size() ) {
		BOOST_ERROR(" Not all properties have been loaded into the database");
	}

	for(unsigned i=0; i<res.size(); i++) {
		BOOST_CHECK( property_match(res[i], props[i]));
	}
}


void TestImplLog::check_obj_references_subset(ID, const ::LibFred::Logger::ObjectReferences &) {}
void TestImplLog::check_obj_references(ID, const ::LibFred::Logger::ObjectReferences &)
{
    // TODO check object_references
    /*
        boost::format query = boost::format(" ") % rec_id;

        Connection conn = Database::Manager::acquire();

        res = conn.exec(query.str());

        if(res.size() != refs.size()) {
            BOOST_ERROR( boost::format(" Number of object references for request ID %1% doesn't match.") % rec_id);
        }

        for (unsigned i =0; i<res.size(); i++) {
                BOOST_CHECK( res[i] ==  refs[i] );
        }
        */

}

static int global_call_count = 0;

void test_monitoring_ip(const std::string &ip, TestImplLog &t, bool result)
{
	Database::ID id;

	id = t.createRequest(ip.c_str(), LC_EPP, "AAA", TestImplLog::no_props, result);

	Connection conn = Database::Manager::acquire();

	BOOST_TEST_MESSAGE( " Recent ID: " << id );

	boost::format query = boost::format ( "select is_monitoring from request where id=%1%") % id;
	Result res = conn.exec(query.str());

	// since the request table is partitioned according to monitoring flag, it is already tested in createRequest
	if(res.size() == 0) {
		BOOST_ERROR(" Record wasn't found. ");
	} else {
		BOOST_CHECK((bool)res[0][0] == result);
	}

}

class ConfigFile {
private:
	std::ofstream conffile;

public:
	ConfigFile(const std::string &filename [[gnu::unused]], const std::string &content) {

		conffile.open("test_log_monitoring.conf");
		conffile << content;
		conffile.close();
	}

	~ConfigFile() {
		if (remove ("test_log_monitoring.conf") != 0) {
		    BOOST_TEST_MESSAGE( "Failed to delete a file ." );
		}
	}
};

BOOST_AUTO_TEST_CASE( test_session )
{
	BOOST_TEST_MESSAGE("Create and close single sessions with both available languages ");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

	Database::ID id = test.createSession(CS, "regid01");

	BOOST_CHECK(id != 0);
	BOOST_CHECK(test.closeSession(id));

	id = test.createSession(EN, "regid02");

	BOOST_CHECK(id != 0);
	BOOST_CHECK(test.closeSession(id));
}

BOOST_AUTO_TEST_CASE( test_con_sessions )
{
	BOOST_TEST_MESSAGE("Create two concurrent sessions and close them");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

	Database::ID id  = test.createSession(CS, "regid03");
	Database::ID id1 = test.createSession(EN, "regid04");

	BOOST_CHECK(id  != 0);
	BOOST_CHECK(id1 != 0);

	BOOST_CHECK(test.closeSession(id));
	BOOST_CHECK(test.closeSession(id1));
}

#ifdef LOGD_DEBUG_MODE
// this is not going to be detected unless we check for it in implementation
// under LOGD_DEBUG_MODE
BOOST_AUTO_TEST_CASE( close_session_twice )
{
	BOOST_TEST_MESSAGE("Try to close a session which is already closed");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

	Database::ID id = test.createSession(CS, "regid01");

	BOOST_CHECK(id != 0);
	BOOST_CHECK(test.closeSession(id));

	BOOST_CHECK(!test.closeSession(id));
}
#endif

BOOST_AUTO_TEST_CASE( session_without_name )
{
	BOOST_TEST_MESSAGE("Try to create a session without providing the name of registrar/user");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

        Database::ID id;
        bool caught=false;
        try {
            id = test.createSession(CS, NULL);
        } catch (WrongUsageError &e) {
            caught = true;
        }
	BOOST_CHECK(caught);

        caught = false;
        try {
            id = test.createSession(CS, "");
        } catch (WrongUsageError &e) {
            caught = true;
        }
	BOOST_CHECK(caught);

}

BOOST_AUTO_TEST_CASE( test_monitoring_flag )
{
	BOOST_TEST_MESSAGE("Test if the monitoring flag is set according to list of monitoring hosts");

	// create monitoring file first
	const std::string CONF_FILENAME("test_log_monitoring.conf");

	ConfigFile file(CONF_FILENAME,  "127.0.0.1      0.0.0.0   216.16.16.1 2001:db8:85a3:8d3:1319:8a2e:370:7348 ::fffe:c000:280 3001:da:43bd:dead::");
	// create an instance of TestImplLog

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info(), "test_log_monitoring.conf");

	test_monitoring_ip("127.0.0.1", test, true);
	test_monitoring_ip("127.0.0.2", test, false);
	test_monitoring_ip("216.16.16.1", test, true);
	test_monitoring_ip("216.16.16.2", test, false);
	test_monitoring_ip("155.120.1.1", test, false);
	test_monitoring_ip("2001:db8:85a3:8d3:1319:8a2e:370:7348", test, true);
	test_monitoring_ip("::fffe:c000:280", test, true);
	test_monitoring_ip("3001:da:43bd:dead::", test, true);
	test_monitoring_ip("3001:da:43bd:dead:123::", test, false);
	test_monitoring_ip("::fffe:c000:28", test, false);

}

BOOST_AUTO_TEST_CASE( partitions )
{
	BOOST_TEST_MESSAGE("Check if records with different dates are inserted into correct partitions. ");

	Database::ID id;
//	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

	// this gets the very same connection which is used by test object above since this program is single-threaded
	Connection conn = Database::Manager::acquire();
	boost::format insert;
	int service_id;

	// i is also used as minutes in time
	for(service_id = LC_UNIX_WHOIS; service_id < LC_MAX_SERVICE; service_id++) {
		for(int i=1;i<MONTHS_COUNT;i++) {
			std::string date = create_date_str(2009, i);
			try {
                                Database::ID request_id;

                                boost::format fmt = boost::format("%1% 9:%2%:00") % date % i;

                                ModelRequest req1;
                                req1.setTimeBegin(Database::DateTime(fmt.str()));
                                req1.setServiceId(service_id);
                                req1.setIsMonitoring(false);
                                req1.insert();
                                Database::ID rid1 = req1.getId();

                                MyFixture::id_list_entry.push_back(rid1);

                                boost::format test = boost::format("select time_begin from request_%1% where id = %2%") % get_table_postfix(2009, i, (ServiceType) service_id, false) % rid1;
                                Database::Result res = conn.exec(test.str());

                                if (res.size() == 0) {
                                    BOOST_ERROR(" Record not found in the correct partition ");
                                }

				// ----- now monitoring on

                                fmt = boost::format("%1% 9:%2%:00") % date % i;

                                ModelRequest req;
                                req.setTimeBegin(Database::DateTime(fmt.str()));
                                req.setServiceId(service_id);
                                req.setIsMonitoring(true);
                                req.insert();
                                Database::ID rid2 = req.getId();

				MyFixture::id_list_entry.push_back(rid2);

				test = boost::format("select time_begin from request_%1% where id = %2%") % get_table_postfix(2009, i, (ServiceType)service_id, true) % rid2;
				res = conn.exec(test.str());

				if(res.size() == 0) {
					BOOST_ERROR(" Record not found in the correct partition ");
				}

				if(PARTITIONS_TEST_PROPERTIES) {
                                        fmt = boost::format("%1% 9:%2%:00") % date % i;

                                        Database::ID prop_id1 = insert_property_record_impl(
                                            conn,
                                            fmt.str(),
                                            service_id,
                                            false,
                                            rid1,
                                            13,
                                            "valuevalue",
                                            false,
                                            0
                                        );

					boost::format test1 = boost::format("select request_time_begin from request_property_value_%1% where id = %2%") % get_table_postfix(2009, i, (ServiceType)service_id, false) % prop_id1;
					res = conn.exec(test1.str());

					if(res.size() == 0) {
						BOOST_ERROR(" Record not found in the correct partition ");
					}

                                        Database::ID prop_id2 = insert_property_record_impl(
                                            conn,
                                            fmt.str(),
                                            service_id,
                                            true,
                                            rid2,
                                            13,
                                            "valuevalue",
                                            false,
                                            0
                                        );

					boost::format test2 = boost::format("select request_time_begin from request_property_value_%1% where id = %2%") % get_table_postfix(2009, i, (ServiceType)service_id, true) % prop_id2;
					res = conn.exec(test2.str());

					if(res.size() == 0) {
						BOOST_ERROR(" Record not found in the correct partition ");
					}

				}

			} catch (Database::Exception &ex) {
			    BOOST_TEST_MESSAGE( (boost::format("error when working with database (%1%) : %2%") % CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info() % ex.what()).str());
			}
		}
	}

}


BOOST_AUTO_TEST_CASE( long_property_name)
{
	BOOST_TEST_MESSAGE("Try to log a property with a very long name (which isn't in the database, also) and very long value");

	Database::ID id;
	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	std::unique_ptr<::LibFred::Logger::RequestProperties> props;

	props = test.create_generic_properties(2, global_call_count++);

	(*props)[1].name = std::string(100, 'N');
	(*props)[1].value = "val - long name";

	(*props)[0].name = "name - very long value";
	(*props)[0].value = std::string(8000, 'X');

	id = test.createRequest("100.100.100.100", LC_EPP, "AAA", *props);
	BOOST_CHECK(id != 0);
}

BOOST_AUTO_TEST_CASE( zero_property_name)
{
	BOOST_TEST_MESSAGE(" Try to add property with zero-length name or value");

	Database::ID id;
	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	std::unique_ptr<::LibFred::Logger::RequestProperties> props;

	props = test.create_generic_properties(2, global_call_count++);

	(*props)[1].name = "";
	(*props)[1].value = "";

	(*props)[0].name = "name zero length value";
	(*props)[0].value = "";

	id = test.createRequest("100.100.100.100", LC_EPP, "CCC", *props);

	BOOST_CHECK(id != 0);
}

BOOST_AUTO_TEST_CASE( property_name_too_long )
{
    BOOST_TEST_MESSAGE("Try to log a property with a name which is too long.");

    Database::ID id;
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
    std::unique_ptr<::LibFred::Logger::RequestProperties> props
        = test.create_generic_properties(1, global_call_count++);

    (*props)[0].name = std::string(1000, 'N');
    (*props)[0].value = "property value - very long name";

    bool exception = false;
    // this should fail
    try {
        id = test.createRequest("100.100.100.100", LC_EPP, "AAA", *props);
    } catch (...) {
        exception = true;
    }
    BOOST_CHECK(exception);

    /*
     * check if the right exception was thrown
    bool correct_exception = false;
    try {
        id = test.createRequest("100.100.100.100", LC_EPP, "AAA", *props);
    } catch (const WrongUsageError &ex) {
        std::string message(ex.what());

        //if(message.find("") == std::string::npos) {
            //BOOST_FAIL("Either completely wrong exception or just a modified message");
        //}
        BOOST_FAIL(message);

        correct_exception = true;
    } catch (const Database::Exception &ex) {
        BOOST_FAIL("FAILED, Database::Exception: " + std::string(ex.what()));
    } catch (...) {
        BOOST_FAIL("Incorrect exception thrown");
    }

    if(!correct_exception) {
        // fail for incorrect exception would've been executed above
        BOOST_FAIL("No exception thrown.");
    }
    */
}


BOOST_AUTO_TEST_CASE( without_properties )
{

	//////
	BOOST_TEST_MESSAGE(" Create a simple message, update it multiple times, and close it without using any properties. ");


	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	Database::ID id1;

	id1 = test.createRequest("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD");
	BOOST_CHECK(id1 != 0);
	BOOST_CHECK(test.closeRequest(id1, "ZZZZZZZZZZZZZZZZZZZZZ"));
}


BOOST_AUTO_TEST_CASE ( test_double_close )
{
        BOOST_TEST_MESSAGE("Try to close one request two times - should yield an exception.");

        TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

        Database::ID idr = test.createRequest("100.100.100.100", LC_EPP, "AAA", TestImplLog::no_props, false, TestImplLog::no_objs, 0);
        BOOST_CHECK(idr != 0);
        BOOST_CHECK(test.closeRequest(idr, "ZZZ", TestImplLog::no_props, TestImplLog::no_objs, 1000, 0));

        bool correct_exception = false;
        try {
            test.closeRequest(idr, "ZZZ1", TestImplLog::no_props, TestImplLog::no_objs, 1001, 0);
        } catch (InternalServerError &ex) {
            std::string message(ex.what());
            if(message.find("was already completed") == std::string::npos) {
                BOOST_FAIL("Either completely wrong exception or just a modified message");
            }

            correct_exception = true;
        } catch (...) {
            BOOST_FAIL("Incorrect exception thrown");
        }

        BOOST_CHECK_MESSAGE(correct_exception, "No exception thrown when double close of request occured.");

}


BOOST_AUTO_TEST_CASE( service_types)
{
	BOOST_TEST_MESSAGE(" Try to use all possible service types");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	for (int i=LC_UNIX_WHOIS;i<LC_MAX_SERVICE;i++) {
		BOOST_CHECK(test.createRequest("111.222.111.222", (ServiceType)i, "aaa"));
	}
}

BOOST_AUTO_TEST_CASE( invalid_ip)
{
	BOOST_TEST_MESSAGE(" Try to send an invalid IP address");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
    bool exception = false;
    try {
        test.createRequest("ABC", LC_PUBLIC_REQUEST, "AA");
    } catch (...) {
        exception = true;
    }
    BOOST_CHECK(exception);

    exception = false;
    try {
        test.createRequest("127.0.0.256", LC_PUBLIC_REQUEST, "AA");
    } catch (...) {
        exception = true;
    }
    BOOST_CHECK(exception);
}

BOOST_AUTO_TEST_CASE( zero_length_strings )
{
	BOOST_TEST_MESSAGE(" Try using zero length strings in content and ip address. ");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	Database::ID id1;

	std::unique_ptr<::LibFred::Logger::RequestProperties> props;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.createRequest("", LC_PUBLIC_REQUEST, "", *props);
	BOOST_CHECK(id1 != 0);

	props = test.create_generic_properties(1, global_call_count++);
	props = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.closeRequest(id1, "", *props));

}

BOOST_AUTO_TEST_CASE( null_strings )
{
	BOOST_TEST_MESSAGE(" Try using nulls length strings in content and ip address. ");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	Database::ID id1;

	std::unique_ptr<::LibFred::Logger::RequestProperties> props;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.createRequest(NULL, LC_PUBLIC_REQUEST, NULL, *props);
	BOOST_CHECK(id1 != 0);

	props = test.create_generic_properties(1, global_call_count++);
	props = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.closeRequest(id1, NULL, *props));
}

BOOST_AUTO_TEST_CASE( long_strings )
{
	BOOST_TEST_MESSAGE(" Try using very long strings in content and ip address. ");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	Database::ID id1;

	std::unique_ptr<::LibFred::Logger::RequestProperties> props;
	props = test.create_generic_properties(3, global_call_count++);

    bool exception = false;
    // this should fail
    try {
        id1 = test.createRequest(std::string(100, 'X').c_str(), LC_PUBLIC_REQUEST, std::string(5000, 'X').c_str(), *props);
    } catch (...) {
        exception = true;
    }
    BOOST_CHECK(exception);

	id1 = test.createRequest("122.123.124.125", LC_PUBLIC_REQUEST, std::string(5000, 'X').c_str(), *props);
	BOOST_CHECK(id1 != 0);

	props = test.create_generic_properties(1, global_call_count++);

	props = test.create_generic_properties(1, global_call_count++);

	BOOST_CHECK(test.closeRequest(id1, std::string(5000, 'X').c_str(), *props));
}

BOOST_AUTO_TEST_CASE( normal_event )
{
	BOOST_TEST_MESSAGE(" Create a simple message, update it multiple times, and close it. ");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	Database::ID id1;

	std::unique_ptr<::LibFred::Logger::RequestProperties> props;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.createRequest("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props);
	BOOST_CHECK(id1 != 0);

	props = test.create_generic_properties(1, global_call_count++);
	props = test.create_generic_properties(1, global_call_count++);

	props = test.create_generic_properties(1, global_call_count++);

	BOOST_CHECK(test.closeRequest(id1, "ZZZZZZZZZZZZZZZZZZZZZ", *props));

}

BOOST_AUTO_TEST_CASE( no_props )
{
	BOOST_TEST_MESSAGE(" Create an event without any properties");

	LibFred::Logger::RequestProperties no_props;
	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	Database::ID id1;

	id1 = test.createRequest("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", no_props);
	BOOST_CHECK(id1 != 0);

	BOOST_CHECK(test.closeRequest(id1, "ZZZZZZZZZZZZZZZZZZZZZ", no_props));
}

BOOST_AUTO_TEST_CASE( _2_events )
{
	BOOST_TEST_MESSAGE(" Create an event, create a second event, close the second, close the first...");
	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	Database::ID id1, id2;

	std::unique_ptr<::LibFred::Logger::RequestProperties> props;
	props = test.create_generic_properties(3, global_call_count++);
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.createRequest("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props);
	BOOST_CHECK(id1 != 0);

	id2 = test.createRequest("101.101.101.101", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props);
	BOOST_CHECK(id2 != 0);

	props = test.create_generic_properties(1, global_call_count++);

	BOOST_CHECK(test.closeRequest(id2, "ZZZZZZZZZZZZZZZZZZZZZ", *props));

	BOOST_CHECK(test.closeRequest(id1, "YYYYYYYYYYYYYYYY", *props));
}

#ifdef LOGD_DEBUG_MODE
BOOST_AUTO_TEST_CASE( already_closed )
{
	BOOST_TEST_MESSAGE(" Try to update and close already closed event. ");

	TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());
	Database::ID id1, id2;
	std::unique_ptr<::LibFred::Logger::RequestProperties> props;

	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.createRequest("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props);
	BOOST_CHECK(id1 != 0);
	BOOST_CHECK(test.closeRequest(id1, "YYYYYYYYYYYYYYYY"));
	// record closed here

	props = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(!test.closeRequest(id1, "ZZZZZZZZZZZZZZZZZZZZZ", *props));

}
#endif

BOOST_AUTO_TEST_CASE( close_record_0 )
{
	BOOST_TEST_MESSAGE(" Try to close record with id 0");
	TestImplLog test (CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

	BOOST_CHECK(!test.closeRequest(0, "ZZZZ"));

}

BOOST_AUTO_TEST_CASE( getResultCodesByService )
{
//    try
//    {
        //CORBA init
        FakedArgs fa = CfgArgs::instance()->fa;
        HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
        CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
            , ns_args_ptr->nameservice_host
            , ns_args_ptr->nameservice_port
            , ns_args_ptr->nameservice_context);

        //BOOST_TEST_MESSAGE( "ccReg::Logger::_narrow" );
        ccReg::Logger_var logger_ref;
        logger_ref = ccReg::Logger::_narrow(CorbaContainer::get_instance()->nsresolve("Logger"));

        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //simple test
        ccReg::ResultCodeList_var result_codes_var(new ccReg::ResultCodeList);

        result_codes_var = logger_ref->getResultCodesByService(0);

        const ccReg::ResultCodeList& result_codes_ref = result_codes_var.in();

        BOOST_TEST_MESSAGE( "\ngetResultCodesByService" );
           for (CORBA::ULong i=0; i < result_codes_ref.length(); ++i)
           {
               const ccReg::ResultCodeListItem& rc = result_codes_ref[i];
               BOOST_TEST_MESSAGE( " result_code: " << rc.result_code << "\n"
                       << " name: " << rc.name << "\n");
           }


        BOOST_REQUIRE_EQUAL(1 , 1);
        CorbaContainer::destroy_instance();
/*
    }//try
    catch(CORBA::TRANSIENT&)
    {
        BOOST_TEST_MESSAGE( "getResultCodesByService Caught system exception TRANSIENT -- unable to contact the "
           << "server." );
    }
    catch(CORBA::SystemException& ex)
    {
        BOOST_TEST_MESSAGE( "getResultCodesByService Caught a CORBA::" << ex._name() );
    }
    catch(CORBA::Exception& ex)
    {
        BOOST_TEST_MESSAGE( "getResultCodesByService Caught CORBA::Exception: " << ex._name() );
    }
    catch(omniORB::fatalException& fe)
    {
        BOOST_TEST_MESSAGE( "getResultCodesByService Caught omniORB::fatalException:" );
        BOOST_TEST_MESSAGE( "  file: " << fe.file() );
        BOOST_TEST_MESSAGE( "  line: " << fe.line() );
        BOOST_TEST_MESSAGE( "  mesg: " << fe.errmsg() );
    }
*/
}



BOOST_AUTO_TEST_CASE ( test_rewrite_same_session )
{

        BOOST_TEST_MESSAGE("Try closing request with same session ID as used when creating");

        TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

        Database::ID ids = test.createSession(CS, "username");

        BOOST_CHECK(ids != 0);

        Database::ID idr = test.createRequest("100.99.98.97", LC_EPP, "AAA", TestImplLog::no_props, false, TestImplLog::no_objs, ids);


        BOOST_CHECK(test.closeRequest(idr, "ZZZ", TestImplLog::no_props, TestImplLog::no_objs, 1000, ids));

        test.closeSession(ids);
}

struct ThreadResult {
    int number;
    ID result;

    ThreadResult() : number(0), result(0) {};
};

typedef concurrent_queue<ThreadResult> ThreadResultQueue;

class TestPropsThreadWorker
{
public:
    TestPropsThreadWorker(
            unsigned n,
            boost::barrier* sb,
            std::size_t thread_group_divisor,
            TestImplLog *tb,
            const std::string &property_name,
            ThreadResultQueue* rq = 0)
    : number(n),
      sb_ptr(sb),
      divisor(thread_group_divisor),
      result_queue(rq),
      backend(tb),
      propname(property_name)
    { }

    void operator()()
    {
        ThreadResult res;
        res.number = number;
        res.result = 0;

        if (number % divisor)//if synchronized thread
        {
            //std::cout << "waiting: " << number_ << std::endl;
            if (sb_ptr != nullptr)
            {
                sb_ptr->wait();//wait for other synced threads
            }
        }
        else
        {//non-synchronized thread
            //std::cout << "NOwaiting: " << number_ << std::endl;
        }

        res.result = run();
        result_queue->guarded_access().push(res);
    }

    ID run()
    {
        // TODO make sure this name is really unique
        try
        {
            return backend->find_property_name(propname);
        }
        catch (const std::exception &e)
        {
            THREAD_BOOST_ERROR(e.what());
            return 0;
        }
        catch (...)
        {
            THREAD_BOOST_ERROR("Unknown exception caught");
            return 0;
        }
    }

private:
    int number;
    boost::barrier* sb_ptr;
    int divisor;
    ThreadResultQueue *result_queue;
    TestImplLog *backend;
    std::string propname;
};

BOOST_AUTO_TEST_CASE(threaded_property_add_test)
{
    const HandleThreadGroupArgs* const thread_args_ptr =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleThreadGroupArgs>();

    const std::size_t number_of_threads = thread_args_ptr->number_of_threads;
    const std::size_t thread_group_divisor = thread_args_ptr->thread_group_divisor;
    // int(number_of_threads - (number_of_threads % thread_group_divisor ? 1 : 0)
    // - number_of_threads / thread_group_divisor) is number of synced threads

    TestImplLog test_backend(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    const std::string unique_property_name("My test prop name");

    Connection conn = Database::Manager::acquire();

    boost::format check_query = boost::format("select id from request_property_name where name='%1%'")
            % unique_property_name;

    const Result res = conn.exec(check_query.str());
    if (res.size() > 0)
    {
        BOOST_FAIL(boost::format("Property %1% already present in the database, cannot perform the test") % unique_property_name);
    }

    ThreadResultQueue result_queue;

    //vector of thread functors
    std::vector<TestPropsThreadWorker> tw_vector;
    tw_vector.reserve(number_of_threads);

    BOOST_TEST_MESSAGE("thread barriers:: "
            << (number_of_threads - (number_of_threads % thread_group_divisor ? 1 : 0)
                    - number_of_threads/thread_group_divisor));

    //synchronization barriers instance
    boost::barrier sb(number_of_threads - (number_of_threads % thread_group_divisor ? 1 : 0)
            - number_of_threads/thread_group_divisor);

    //thread container
    boost::thread_group threads;
    for (unsigned i = 0; i < number_of_threads; ++i)
    {
        tw_vector.push_back(TestPropsThreadWorker(
                i,
                &sb,
                thread_group_divisor,
                &test_backend,
                unique_property_name,
                &result_queue));
        try
        {
            threads.create_thread(tw_vector.at(i));
        }
        catch (const std::exception &e)
        {
            BOOST_FAIL(e.what());
        }
    }

    threads.join_all();

    BOOST_TEST_MESSAGE("threads end result_queue.size(): " << result_queue.unguarded_access().size());

    bool first_result = true;
    ThreadResult result1;
    ID correct;

    while (!result_queue.unguarded_access().empty())
    {
        const auto thread_result = result_queue.unguarded_access().pop();
        if (first_result)
        {
            result1 = thread_result;
            correct = result1.result;
            first_result = false;
        }
        else if (thread_result.result != correct)
        {
            BOOST_TEST_MESSAGE(
                       " thread number: " << thread_result.number
                    << " return code: " << thread_result.result);
            BOOST_FAIL(" Incorrect property ID returned");
        }
    }
    if (first_result)
    {
        BOOST_FAIL("Result not found.");
    }
}

BOOST_AUTO_TEST_CASE(get_request_count_users_compare)
{
    TestImplLog test (CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    boost::posix_time::ptime begin(time_from_string("2011-01-01 00:00:00"));
    boost::posix_time::ptime end(time_from_string("2011-06-30 00:00:00"));

    std::unique_ptr<RequestCountInfo> info_ptr = test.getRequestCountUsers(begin, end, "EPP");

    for (RequestCountInfo::iterator it = info_ptr->begin(); it != info_ptr->end(); ++it)
    {
        unsigned long long check_count = test.getRequestCount(begin, end, "EPP", it->first);

        BOOST_REQUIRE_MESSAGE(check_count == it->second,
                "Count got from getRequestCount and getRequestCountUsers matches");
    }
}

BOOST_AUTO_TEST_CASE( get_request_count_wrong_date )
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    // "not-a-date-"
    BOOST_CHECK_THROW(test.getRequestCount(boost::posix_time::ptime(), time_from_string("2011-01-31"), "EPP", "REG-FRED_A"),
                    std::exception);
}

BOOST_AUTO_TEST_CASE( get_request_count_users_wrong_date )
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    BOOST_CHECK_THROW(test.getRequestCountUsers(boost::posix_time::ptime(), time_from_string("2011-01-31"), "EPP"),
                    std::exception);

}

BOOST_AUTO_TEST_CASE(test_request_count)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::unique_ptr<::LibFred::Logger::RequestProperties> props1;

    props1 = test.create_generic_properties(17, global_call_count++);

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    Database::ID session_id = test.createSession(0, reg_handle.c_str());

    ID r1 = test.createRequest("", LC_EPP, "", *props1, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r1, "");
    ID r2 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r2, "");

    ptime current_tstamp = boost::posix_time::microsec_clock::local_time();

    std::unique_ptr<RequestCountInfo> info = test.getRequestCountUsers(current_tstamp - hours(1), current_tstamp + hours(1), "EPP");

    RequestCountInfo::iterator it = info->find(reg_handle);

    BOOST_REQUIRE(it != info->end());

    BOOST_CHECK(it->second == 18);

    unsigned long long count = test.getRequestCount(current_tstamp - hours(1), current_tstamp + hours(1), "EPP", reg_handle);

    BOOST_CHECK(count == 18);
}

BOOST_AUTO_TEST_CASE(test_request_count_props_handle)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::unique_ptr<::LibFred::Logger::RequestProperties> props1;

    props1 = test.create_properties_req_count(7, 10, global_call_count++);

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    Database::ID session_id = test.createSession(0, reg_handle.c_str());

    ID r1 = test.createRequest("", LC_EPP, "", *props1, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r1, "");
    ID r2 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r2, "");

    ptime current_tstamp = boost::posix_time::microsec_clock::local_time();

    std::unique_ptr<RequestCountInfo> info = test.getRequestCountUsers(current_tstamp - hours(1), current_tstamp + hours(1), "EPP");

    RequestCountInfo::iterator it = info->find(reg_handle);

    BOOST_REQUIRE(it != info->end());

    BOOST_CHECK(it->second == 8);

    /// the other method (for 1 user)
    unsigned long long count = test.getRequestCount(current_tstamp - hours(1), current_tstamp + hours(1), "EPP", reg_handle);

    BOOST_CHECK(count == 8);
}

BOOST_AUTO_TEST_CASE(test_request_count_errors_simple)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    Database::ID session_id = test.createSession(0, reg_handle.c_str());

    ID r1 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r1, "", TestImplLog::no_props, TestImplLog::no_objs, get_result_code_id("CommandFailed") );

    ID r2 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r2, "", TestImplLog::no_props, TestImplLog::no_objs, get_result_code_id("CommandFailedServerClosingConnection") );

    ID r3 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r3, "");

    ptime current_tstamp = boost::posix_time::microsec_clock::local_time();

    std::unique_ptr<RequestCountInfo> info = test.getRequestCountUsers(current_tstamp - hours(1), current_tstamp + hours(1), "EPP");
    RequestCountInfo::iterator it = info->find(reg_handle);
    BOOST_REQUIRE(it != info->end());

    BOOST_CHECK(it->second == 1);

    unsigned long long count = test.getRequestCount(current_tstamp - hours(1), current_tstamp + hours(1), "EPP", reg_handle);
    BOOST_CHECK(count == 1);
}

BOOST_AUTO_TEST_CASE(test_request_count_errors_props)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::unique_ptr<::LibFred::Logger::RequestProperties> props_in;
    props_in = test.create_generic_properties(17, global_call_count++);

    std::unique_ptr<::LibFred::Logger::RequestProperties> props_out;
    props_out = test.create_generic_properties(5, global_call_count++);

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    Database::ID session_id = test.createSession(0, reg_handle.c_str());

    ID r1 = test.createRequest("", LC_EPP, "", *props_in, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r1, "", *props_out, TestImplLog::no_objs, get_result_code_id("CommandFailed") );

    ID r2 = test.createRequest("", LC_EPP, "", *props_in, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r2, "", *props_out, TestImplLog::no_objs, get_result_code_id("CommandFailedServerClosingConnection") );

    ID r3 = test.createRequest("", LC_EPP, "", *props_in, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r3, "", *props_out);

    ptime current_tstamp = boost::posix_time::microsec_clock::local_time();

    std::unique_ptr<RequestCountInfo> info = test.getRequestCountUsers(current_tstamp - hours(1), current_tstamp + hours(1), "EPP");

    RequestCountInfo::iterator it = info->find(reg_handle);

    BOOST_REQUIRE(it != info->end());

    BOOST_CHECK(it->second == 22);

    unsigned long long count = test.getRequestCount(current_tstamp - hours(1), current_tstamp + hours(1), "EPP", reg_handle);
    BOOST_CHECK(count == 22);
}

BOOST_AUTO_TEST_CASE(test_request_count_poll_simple)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    Database::ID session_id = test.createSession(0, reg_handle.c_str());

    Database::ID greeting_id = get_request_type_id("ClientGreeting");
    Database::ID contact_check_id = get_request_type_id("ContactCheck");
    Database::ID poll_ack_id = get_request_type_id("PollAcknowledgement");
    Database::ID poll_resp_id = get_request_type_id("PollResponse");

    // these requests should not be counted

    ID r1 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id, poll_ack_id);
    test.closeRequest(r1, "");
    ID r2 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id, poll_resp_id);
    test.closeRequest(r2, "");

    // these two count
    ID r3 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id, greeting_id);
    test.closeRequest(r3, "");
    ID r4 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id, contact_check_id);
    test.closeRequest(r4, "");

    ptime current_tstamp = boost::posix_time::microsec_clock::local_time();
    std::unique_ptr<RequestCountInfo> info = test.getRequestCountUsers(current_tstamp - minutes(1), current_tstamp + minutes(1), "EPP");
    RequestCountInfo::iterator it = info->find(reg_handle);

    BOOST_REQUIRE(it != info->end());
    BOOST_CHECK(it->second == 2);

    unsigned long long count = test.getRequestCount(current_tstamp - minutes(1), current_tstamp + minutes(1), "EPP", reg_handle);
    BOOST_CHECK(count == 2);
}

BOOST_AUTO_TEST_CASE(test_request_count_poll_props)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::unique_ptr<::LibFred::Logger::RequestProperties> props1;

    props1 = test.create_generic_properties(17, global_call_count++);

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    Database::ID session_id = test.createSession(0, reg_handle.c_str());

    Database::ID greeting_id = get_request_type_id("ClientGreeting");
    Database::ID contact_check_id = get_request_type_id("ContactCheck");
    Database::ID poll_ack_id = get_request_type_id("PollAcknowledgement");
    Database::ID poll_resp_id = get_request_type_id("PollResponse");

    // these requests should not be counted
    ID r1 = test.createRequest("", LC_EPP, "", *props1, false, TestImplLog::no_objs, session_id, poll_ack_id);
    test.closeRequest(r1, "");
    ID r2 = test.createRequest("", LC_EPP, "", *props1, false, TestImplLog::no_objs, session_id, poll_resp_id);
    test.closeRequest(r2, "");

    // these two count
    ID r3 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id, greeting_id);
    test.closeRequest(r3, "");
    ID r4 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id, contact_check_id);
    test.closeRequest(r4, "");

    ptime current_tstamp = boost::posix_time::microsec_clock::local_time();

    std::unique_ptr<RequestCountInfo> info = test.getRequestCountUsers(current_tstamp - minutes(1), current_tstamp + minutes(1), "EPP");

    RequestCountInfo::iterator it = info->find(reg_handle);

    BOOST_REQUIRE(it != info->end());

    BOOST_CHECK(it->second == 2);

    unsigned long long count = test.getRequestCount(current_tstamp - minutes(1), current_tstamp + minutes(1), "EPP", reg_handle);
    BOOST_CHECK(count == 2);
}

void test_registrar_request_count(TestImplLog &test, boost::posix_time::ptime p_from, boost::posix_time::ptime p_to, const std::string &reg_name, unsigned long long req_count)
{
    std::unique_ptr<RequestCountInfo> info = test.getRequestCountUsers(p_from, p_to, "EPP");

    RequestCountInfo::iterator it = info->find(reg_name);

    if(req_count > 0) {
        BOOST_REQUIRE(it != info->end());
        BOOST_CHECK(it->second == req_count);
    } else {
        BOOST_CHECK(it == info->end() || it->second == 0);
    }
}

BOOST_AUTO_TEST_CASE(test_request_count_past)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::unique_ptr<::LibFred::Logger::RequestProperties> props1;

    props1 = test.create_generic_properties(17, global_call_count++);

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    Database::ID session_id = test.createSession(0, reg_handle.c_str());

    ID r1 = test.createRequest("", LC_EPP, "", *props1, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r1, "");
    ID r2 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r2, "");

    ptime current_tstamp = boost::posix_time::microsec_clock::local_time();

    test.insert_custom_request(current_tstamp - days(2), reg_handle);

    std::unique_ptr<RequestCountInfo> info = test.getRequestCountUsers(current_tstamp - days(3), current_tstamp + days(3), "EPP");

    RequestCountInfo::iterator it = info->find(reg_handle);

    BOOST_REQUIRE(it != info->end());
    BOOST_CHECK(it->second == 19);

    unsigned long long count = test.getRequestCount(current_tstamp - days(3), current_tstamp + days(3), "EPP", reg_handle);
    BOOST_CHECK(count == 19);
}

BOOST_AUTO_TEST_CASE(test_request_count_cur_month)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    Database::ID session_id = test.createSession(0, reg_handle.c_str());

    ID r1 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r1, "");
    ID r2 = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id);
    test.closeRequest(r2, "");

    boost::gregorian::date current_date = boost::gregorian::day_clock::local_day();

    boost::gregorian::date from_date = boost::gregorian::date(current_date.year(), current_date.month(), 1);
    boost::gregorian::date to_date = from_date + boost::gregorian::months(1);

    test.insert_custom_request(boost::posix_time::ptime(from_date - months(1)), reg_handle);

    ptime p_from(from_date);
    ptime p_to(to_date);

    test_registrar_request_count(test, p_from, p_to, reg_handle, 2);
}

BOOST_AUTO_TEST_CASE(test_request_count_irregular)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    test.createSession(0, reg_handle.c_str());

    boost::gregorian::date current_date = boost::gregorian::day_clock::local_day();

    /// last month
    boost::gregorian::date to_date = boost::gregorian::date(current_date.year(), current_date.month(), 1);

    boost::gregorian::date from_date = to_date - boost::gregorian::months(1);

    ptime p_from(from_date);
    ptime p_to(to_date);

    // request outside of the interval:
    test.insert_custom_request(p_to + seconds(1), reg_handle);
    test_registrar_request_count(test, p_from, p_to, reg_handle, 0);

    test.insert_custom_request(p_from - seconds(1), reg_handle);
    test_registrar_request_count(test, p_from, p_to, reg_handle, 0);

    // requests inside the interval
    test.insert_custom_request(p_to - seconds(1), reg_handle);
    test_registrar_request_count(test, p_from, p_to, reg_handle, 1);
    test.insert_custom_request(p_from + seconds(1), reg_handle);
    test_registrar_request_count(test, p_from, p_to, reg_handle, 2);
    test.insert_custom_request(boost::posix_time::ptime(to_date - weeks(2)) + seconds(1), reg_handle);
    test_registrar_request_count(test, p_from, p_to, reg_handle, 3);

    // borders, `from' included, `to' not
    test.insert_custom_request(boost::posix_time::ptime(p_from), reg_handle);
    test_registrar_request_count(test, p_from, p_to, reg_handle, 4);

    test.insert_custom_request(boost::posix_time::ptime(p_to), reg_handle);
    test_registrar_request_count(test, p_from, p_to, reg_handle, 4);
}

BOOST_AUTO_TEST_CASE(test_request_count_short_period)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    Database::ID session_id = test.createSession(0, reg_handle.c_str());

    Database::ID contact_check_id = get_request_type_id("ContactCheck");

    ID r = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id, contact_check_id);
    test.closeRequest(r, "");

    ptime current_tstamp = boost::posix_time::microsec_clock::local_time();

    ptime day_start( current_tstamp.date() );

    if(current_tstamp - seconds(3) <= day_start) {
        sleep(3);
    }

    std::unique_ptr<RequestCountInfo> info = test.getRequestCountUsers(current_tstamp - seconds(3), current_tstamp - seconds(2), "EPP");

    RequestCountInfo::iterator it = info->find(reg_handle);

    BOOST_CHECK(it == info->end() || it->second == 0);

    unsigned long long count = test.getRequestCount(current_tstamp - seconds(3), current_tstamp - seconds(2), "EPP", reg_handle);
    BOOST_CHECK(count == 0);
}

BOOST_AUTO_TEST_CASE(test_request_count_timezone)
{
    TestImplLog test(CfgArgs::instance()->get_handler_ptr_by_type<HandleDatabaseArgs>()->get_conn_info());

    std::string time_string(TimeStamp::microsec());
    std::string reg_handle = "REG-"+ time_string;
    Database::ID session_id = test.createSession(0, reg_handle.c_str());

    Database::ID contact_check_id = get_request_type_id("ContactCheck");

    ID r = test.createRequest("", LC_EPP, "", TestImplLog::no_props, false, TestImplLog::no_objs, session_id, contact_check_id);
    test.closeRequest(r, "");

    ptime current_tstamp = boost::posix_time::microsec_clock::local_time();

    ptime day_start( current_tstamp.date() );

    ptime day_end = day_start + days(1) - seconds(1);

    // try to avoid problems
    if(current_tstamp == day_start) {
        sleep(1);
    }

    if(current_tstamp + seconds(2) >= day_end) {
        sleep (3);
    }

    std::unique_ptr<RequestCountInfo> info = test.getRequestCountUsers(day_start, current_tstamp - seconds(2), "EPP");
    RequestCountInfo::iterator it = info->find(reg_handle);
    BOOST_CHECK(it == info->end() || it->second == 0);

    std::unique_ptr<RequestCountInfo> info2 = test.getRequestCountUsers(current_tstamp + seconds(2), day_end, "EPP");
    RequestCountInfo::iterator it2 = info2->find(reg_handle);
    BOOST_CHECK(it2 == info2->end() || it2->second == 0);

    unsigned long long count = test.getRequestCount(day_start, current_tstamp - seconds(2), "EPP", reg_handle);
    BOOST_CHECK(count == 0);
    unsigned long long count2 = test.getRequestCount(current_tstamp + seconds(2), day_end, "EPP", reg_handle);
    BOOST_CHECK(count2 == 0);
}

/*
// TODO testcases
BOOST_AUTO_TEST_CASE(tryOverwriting session_id at the end)
BOOST_AUTO_TEST_CASE(tryOverwriting session_id at the end with the same value)
BOOST_AUTO_TEST_CASE(checkResultCodeSaving)
BOOST_AUTO_TEST_CASE(checkObjRefsSaving)
// in this testcase, do stuff from #4268 - property name race condition
BOOST_AUTO_TEST_CASE(...
*/


BOOST_AUTO_TEST_SUITE_END();


