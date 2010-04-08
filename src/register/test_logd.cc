/* Some test cases can fail at 00:00 AM because of get_table_postfix_for_now() function :)
 * after re-running the unit test it shouldn't happen again
 *
 */

#include <iostream>
#define BOOST_TEST_MODULE Test fred-logd
/*
 * for non-header library version:
 * #include <boost/test/unit_test.hpp>
 * */

#include <boost/test/included/unit_test.hpp>
#include <stdio.h>

#include "request_impl.h"

using namespace Database;
using namespace Register::Logger;

namespace TestLogd {

// TODO this should be taken from the database 
enum LogServiceType { LC_NO_SERVICE = -1, LC_UNIX_WHOIS=0, LC_WEB_WHOIS, LC_PUBLIC_REQUEST, LC_EPP, LC_WEBADMIN, LC_INTRANET, LC_MAX_SERVICE };

const int MONTHS_COUNT  = 2;

// value from database table request_type
const int UNKNOWN_ACTION = 1000;

//whether to test partitioning on request_property_value table
const bool PARTITIONS_TEST_PROPERTIES = true;

const std::string DB_CONN_STR("host=localhost port=22345 dbname=fred user=fred password=password connect_timeout=2");


boost::format get_table_postfix(int year, int month, RequestServiceType service_num, bool monitoring);
boost::format get_table_postfix_for_now(RequestServiceType service_num, bool monitoring);
std::string create_date_str(int y, int m);

struct MyFixture {
	static std::list<ID> id_list_entry;
	static std::list<ID> id_list_session;

	MyFixture() {
		Logging::Manager::instance_ref().get(PACKAGE).addHandler(Logging::Log::LT_FILE, std::string("log_test_logd.txt"));
		Logging::Manager::instance_ref().get(PACKAGE).setLevel(Logging::Log::LL_TRACE);
		LOGGER(PACKAGE).info("Logging initialized");

		// initialize database connection manager
		Database::Manager::init(new ConnectionFactory(DB_CONN_STR));
	}

	~MyFixture() {
		try {
			std::list<ID>::iterator it = id_list_entry.begin();
			Connection conn = Database::Manager::acquire();

			std::cout << "Deleting database records from request and related. " << std::endl;
			while(it != id_list_entry.end()) {
				conn.exec( (boost::format("delete from request_data where entry_id=%1%") % *it).str() );
				conn.exec( (boost::format("delete from request_property_value where entry_id=%1%") % *it).str() );
				conn.exec( (boost::format("delete from request where id=%1%") % *it).str() );
				it++;
			}

			std::cout << "Deleting database records from session." << std::endl;
			for(it = id_list_session.begin(); it != id_list_session.end();it++) {
				conn.exec( (boost::format("delete from session where id=%1%") % *it).str() );
			}
		} catch (Database::Exception &ex) {
			std::cout << (boost::format("error when working with database (%1%) : %2%") % DB_CONN_STR % ex.what()).str();
		}
	}

};

std::list<ID> MyFixture::id_list_entry;
std::list<ID> MyFixture::id_list_session;

BOOST_GLOBAL_FIXTURE( MyFixture );


class TestImplLog {
	// TODO this should follow the common ways with create(...) 
	Register::Logger::Manager *logd;
	Connection conn;


public:
	TestImplLog (const std::string connection_string) : logd(Register::Logger::Manager::create(connection_string)), conn(Database::Manager::acquire()) {
	};

	TestImplLog (const std::string connection_string, const std::string monitoring_file) : logd(Register::Logger::Manager::create(connection_string, monitoring_file)), conn(Database::Manager::acquire()) {
	};

	Connection &get_conn() {
		return conn;
	};

	Database::ID CreateSession(Languages lang, const char *name);
	bool         CloseSession(Database::ID id);

	// TODO change this and test combination of session / request (session)
	Database::ID CreateRequest(const char *ip_addr, const RequestServiceType serv, const char * content_in, const Register::Logger::RequestProperties &props = TestImplLog::no_props, bool is_monitoring = false);
	bool UpdateRequest(const Database::ID id, const Register::Logger::RequestProperties &props = TestImplLog::no_props);
	bool CloseRequest(const Database::ID id, const char * content_out, const Register::Logger::RequestProperties &props = TestImplLog::no_props);
	std::auto_ptr<Register::Logger::RequestProperties> create_generic_properties(int number, int value_id);

	void check_db_properties_subset(ID rec_id, const Register::Logger::RequestProperties &props);
	bool property_match(const Row r, const Register::Logger::RequestProperty &p) ;

// different tests
	void check_db_properties(ID rec_id, const Register::Logger::RequestProperties & props);

	static Register::Logger::RequestProperties no_props;
};

inline bool has_content(const char *str) {
	return (str && *str!= '\0');
}

boost::format get_table_postfix_for_now(RequestServiceType service_num, bool monitoring)
{
	boost::posix_time::ptime utime = microsec_clock::universal_time();
	tm str_time = boost::posix_time::to_tm(utime);

	// months in tm are numbered from 0
	str_time.tm_mon++;

	return get_table_postfix((1900 + str_time.tm_year), str_time.tm_mon, service_num, monitoring);

}

boost::format get_table_postfix(int year, int month, RequestServiceType service_num, bool monitoring)
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



struct MyFixture;

Register::Logger::RequestProperties TestImplLog::no_props;


Database::ID TestImplLog::CreateSession(Languages lang, const char *name)
{
	Database::ID ret = logd->i_CreateSession(lang, name);

	if(ret == 0) return 0;

	// first check if the correct partition was used ...
	boost::format test = boost::format("select login_date from session_%1% where id = %2%") % get_table_postfix_for_now(LC_NO_SERVICE, false) % ret;
	Result res = conn.exec(test.str());

	if(res.size() == 0) {
		BOOST_ERROR(" Record not found in the correct partition ");
	}

	// now do a regular select from session
	res = conn.exec((boost::format("select lang, name from session where id=%1%") % ret).str());

	if (res.size() != 1) {
		if (res.size() == 0) {
			BOOST_ERROR(boost::format(" Record created with CreateSession with id %1% doesn't exist in the database! ") % ret);
		} else if(res.size() > 1) {
			BOOST_ERROR(boost::format(" Multiple records with id %1% after call to CreateSession! ") % ret);
		}
	} else {
		if(lang == CS) {
			BOOST_CHECK("cs" == (std::string)res[0][0]);
		} else if(lang == EN) {
			BOOST_CHECK("en" == (std::string)res[0][0]);
		} else {
			BOOST_FAIL(" Unknown language given. ");
		}
		BOOST_CHECK(std::string(name) == (std::string)res[0][1]);
	}

	MyFixture::id_list_session.push_back(ret);

	return ret;
}

bool TestImplLog::CloseSession(Database::ID id)
{
	bool ret = logd->i_CloseSession(id);

	if (!ret) return ret;

	Result res = conn.exec( (boost::format("select logout_date from session where id=%1%") % id).str() );

	if (res.size() != 1) {
		if (res.size() == 0) {
			BOOST_ERROR(boost::format(" Record created with CreateRequest with id %1% doesn't exist in the database! ") % id);
		} else if(res.size() > 1) {
			BOOST_ERROR(boost::format(" Multiple records with id %1% after call to CreateRequest! ") % id);
		}
	} else {
		BOOST_CHECK (!res[0][0].isnull());
	}

	return ret;
}

Database::ID TestImplLog::CreateRequest(const char *ip_addr, const RequestServiceType serv, const char * content_in, const Register::Logger::RequestProperties &props, bool is_monitoring)
{

	if(serv > LC_MAX_SERVICE) {
		BOOST_FAIL(boost::format (" ---------Invalid service num %1% ") % serv);
	}

	// TODO generic session_id 99  - change
	Database::ID ret = logd->i_CreateRequest(ip_addr, serv, content_in, props, UNKNOWN_ACTION, 99);
	boost::format query;

	if(ret == 0) return 0;

	// first check if the correct partition was used ...

	boost::format test = boost::format("select time_begin from request_%1% where id = %2%") % get_table_postfix_for_now(serv, is_monitoring) % ret;
	Result res = conn.exec(test.str());

	if(res.size() == 0) {
		BOOST_ERROR(" Record not found in the correct partition ");
	}

	// now a regular select

	query = boost::format ( "select source_ip, service, raw.content from request join request_data raw on raw.entry_id=id where id=%1%") % ret;
	res = conn.exec(query.str());

	if (res.size() != 1) {
		res = conn.exec( (boost::format("select source_ip, service from request where id=%1%") % ret).str() );

		if (res.size() != 1) {
			if (res.size() == 0) {
				BOOST_ERROR(boost::format(" Record created with CreateRequest with id %1% doesn't exist in the database! ") % ret);
			} else if(res.size() > 1) {
				BOOST_ERROR(boost::format(" Multiple records with id %1% after call to CreateRequest! ") % ret);
			}
		} else {
			if(has_content(ip_addr)) BOOST_CHECK(std::string(ip_addr) 	== (std::string)res[0][0]);
			BOOST_CHECK((int)serv 			== (int)res[0][1]);
		}

	} else {
		if(has_content(ip_addr)) BOOST_CHECK(std::string(ip_addr) 	== (std::string)res[0][0]);
		BOOST_CHECK((int)serv 			== (int)res[0][1]);
		BOOST_CHECK(std::string(content_in) 	== (std::string)res[0][2]);
	}

	check_db_properties(ret, props);

	MyFixture::id_list_entry.push_back(ret);

	return ret;
}

bool TestImplLog::UpdateRequest(const Database::ID id, const Register::Logger::RequestProperties &props)
{
	bool result = logd->i_UpdateRequest(id, props);

	if (!result) return result;

	check_db_properties_subset(id, props);

	return result;
}

bool TestImplLog::CloseRequest(const Database::ID id, const char *content_out, const Register::Logger::RequestProperties &props)
{
	bool result = logd->i_CloseRequest(id, content_out, props);

	if(!result) return result;

	boost::format query = boost::format ( "select time_end, raw.content from request join request_data raw on raw.entry_id=id where raw.is_response=true and id=%1%") % id;
	Result res = conn.exec(query.str());

	if (res.size() != 1) {
		res = conn.exec( (boost::format("select time_end from request where id=%1%") % id).str() );

		if (res.size() != 1) {
			if (res.size() == 0) {
				BOOST_ERROR(boost::format(" Record created with CreateRequest with id %1% doesn't exist in the database! ") % id);
			} else if(res.size() > 1) {
				BOOST_ERROR(boost::format(" Multiple records with id %1% after call to CreateRequest! ") % id);
			}
		} else {
			BOOST_CHECK(!res[0][0].isnull());
		}

	} else {
		BOOST_CHECK(!res[0][0].isnull());
		BOOST_CHECK(std::string(content_out) == (std::string)res[0][1]);
	}

	check_db_properties_subset(id, props);

	return result;
}

std::auto_ptr<Register::Logger::RequestProperties> TestImplLog::create_generic_properties(int number, int value_id)
{
	std::auto_ptr<Register::Logger::RequestProperties> ret(new Register::Logger::RequestProperties(number));
	Register::Logger::RequestProperties &ref = *ret;

	for(int i=0;i<number;i++) {
		ref[i].name = "handle";
		ref[i].value = (boost::format("val%1%.%2%") % value_id % i).str();

		ref[i].child = false;
		ref[i].output = false;
	}

	return ret;
}

// r is row produced by:
// boost::format query = boost::format("select name, value, parent_id, output from request_property_value pv join request_property pn on pn.id=pv.name_id where pv.entry_id = %1% order by pv.id") % rec_id;
// p is single property
// these two are compared :)
bool TestImplLog::property_match(const Row r, const Register::Logger::RequestProperty &p)
{

	if ( (std::string)r[0] != p.name.substr(0, Register::Logger::ManagerImpl::MAX_NAME_LENGTH))  return false;
	if ( (std::string)r[1] != p.value) return false;
	if ( (bool)r[3] != p.output) return false;

	if ( p.child ) {
		if (r[2].isnull()) return false;
	} else {
		if (!r[2].isnull()) return false;
	}

	return true;
}

void TestImplLog::check_db_properties_subset(ID rec_id, const Register::Logger::RequestProperties &props)
{
	if (props.size() == 0) return;

	boost::format query = boost::format("select name, value, parent_id, output from request_property_value pv join request_property pn on pn.id=pv.name_id where pv.entry_id = %1% order by pv.id") % rec_id;

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
// the properties in the database with entry_id=rec_id must match props exactly
void TestImplLog::check_db_properties(ID rec_id, const Register::Logger::RequestProperties & props)
{
	boost::format query = boost::format("select name, value, parent_id, output from request_property_value pv join request_property pn on pn.id=pv.name_id where pv.entry_id = %1% order by pv.id") % rec_id;

	Result res = conn.exec(query.str());

	if (res.size() != props.size() ) {
		BOOST_ERROR(" Not all properties have been loaded into the database");
	}

	for(unsigned i=0; i<res.size(); i++) {
		BOOST_CHECK( property_match(res[i], props[i]));
	}
}

static int global_call_count = 0;

void test_monitoring_ip(const std::string &ip, TestImplLog &t, bool result)
{
	Connection conn = Database::Manager::acquire();
	Database::ID id;

	id = t.CreateRequest(ip.c_str(), LC_EPP, "AAA", TestImplLog::no_props, result);

	std::cout << " Recent ID: " << id << std::endl;

	boost::format query = boost::format ( "select is_monitoring from request where id=%1%") % id;
	Result res = conn.exec(query.str());

	// since the request table is partitioned according to monitoring flag, it is already tested in CreateRequest
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
	ConfigFile(const std::string &filename, const std::string &content) {

		conffile.open("test_log_monitoring.conf");
		conffile << content;
		conffile.close();
	}

	~ConfigFile() {
		if (remove ("test_log_monitoring.conf") != 0) {
			std::cout << "Failed to delete a file ." << std::endl;
		}
	}
};


BOOST_AUTO_TEST_CASE( test_session )
{
	BOOST_TEST_MESSAGE("Create and close single sessions with both available languages ");

	TestImplLog test(DB_CONN_STR);

	Database::ID id = test.CreateSession(CS, "regid01");

	BOOST_CHECK(id != 0);
	BOOST_CHECK(test.CloseSession(id));

	id = test.CreateSession(EN, "regid02");

	BOOST_CHECK(id != 0);
	BOOST_CHECK(test.CloseSession(id));
}

BOOST_AUTO_TEST_CASE( test_con_sessions )
{
	BOOST_TEST_MESSAGE("Create two concurrent sessions and close them");

	TestImplLog test(DB_CONN_STR);

	Database::ID id  = test.CreateSession(CS, "regid03");
	Database::ID id1 = test.CreateSession(EN, "regid04");

	BOOST_CHECK(id  != 0);
	BOOST_CHECK(id1 != 0);

	BOOST_CHECK(test.CloseSession(id));
	BOOST_CHECK(test.CloseSession(id1));
}

BOOST_AUTO_TEST_CASE( close_session_twice )
{
	BOOST_TEST_MESSAGE("Try to close a session which is already closed");

	TestImplLog test(DB_CONN_STR);

	Database::ID id = test.CreateSession(CS, "regid01");

	BOOST_CHECK(id != 0);
	BOOST_CHECK(test.CloseSession(id));

	BOOST_CHECK(!test.CloseSession(id));
}

BOOST_AUTO_TEST_CASE( session_without_name )
{
	BOOST_TEST_MESSAGE("Try to create a session without providing the name of registrar/user");

	TestImplLog test(DB_CONN_STR);

	Database::ID id = test.CreateSession(CS, NULL);
	BOOST_CHECK(id == 0);

	id = test.CreateSession(CS, "");
	BOOST_CHECK(id == 0);

}


BOOST_AUTO_TEST_CASE( test_monitoring_flag )
{
	BOOST_TEST_MESSAGE("Test if the monitoring flag is set according to list of monitoring hosts");

	// create monitoring file first
	const std::string CONF_FILENAME("test_log_monitoring.conf");

	ConfigFile file(CONF_FILENAME,  "127.0.0.1 0.0.0.0 216.16.16.1");
	// create an instance of TestImplLog

	TestImplLog test(DB_CONN_STR, "test_log_monitoring.conf");

	test_monitoring_ip("127.0.0.1", test, true);
	test_monitoring_ip("127.0.0.2", test, false);
	test_monitoring_ip("216.16.16.1", test, true);
	test_monitoring_ip("216.16.16.2", test, false);
	test_monitoring_ip("155.120.1.1", test, false);

}

BOOST_AUTO_TEST_CASE( partitions )
{
	BOOST_TEST_MESSAGE("Check if records with different dates are inserted into correct partitions. ");

	Database::ID id;
	TestImplLog test(DB_CONN_STR);

	// this gets the very same connection which is used by test object above since this program is single-threaded
	Connection conn = Database::Manager::acquire();
	boost::format insert;
	int service;

	// i is also used as minutes in time
	for(service = LC_UNIX_WHOIS; service < LC_MAX_SERVICE; service++) {
		for(int i=1;i<MONTHS_COUNT;i++) {
			std::string date = create_date_str(2009, i);
			try {
                                Database::ID entry_id;                                

                                boost::format fmt = boost::format("%1% 9:%2%:00") % date % i;

                                ModelRequest req1;
                                req1.setTimeBegin(Database::DateTime(fmt.str()));
                                req1.setServiceId(service);
                                req1.setIsMonitoring(false);
                                req1.insert();
                                id = req1.getId();
                                entry_id = id;

                                MyFixture::id_list_entry.push_back(id);

                                boost::format test = boost::format("select time_begin from request_%1% where id = %2%") % get_table_postfix(2009, i, (RequestServiceType) service, false) % id;
                                Database::Result res = conn.exec(test.str());

                                if (res.size() == 0) {
                                    BOOST_ERROR(" Record not found in the correct partition ");
                                }

				// ----- now monitoring on

                                fmt = boost::format("%1% 9:%2%:00") % date % i;

                                ModelRequest req;
                                req.setTimeBegin(Database::DateTime(fmt.str()));
                                req.setServiceId(service);
                                req.setIsMonitoring(true);
                                req.insert();
                                id = req.getId();
                                entry_id = id;

				MyFixture::id_list_entry.push_back(id);

				test = boost::format("select time_begin from request_%1% where id = %2%") % get_table_postfix(2009, i, (RequestServiceType)service, true) % id;
				res = conn.exec(test.str());

				if(res.size() == 0) {
					BOOST_ERROR(" Record not found in the correct partition ");
				}

				if(PARTITIONS_TEST_PROPERTIES) {

                                        fmt = boost::format("%1% 9:%2%:00") % date % i;

                                        ModelRequestPropertyValue pv;
                                        pv.setEntryTimeBegin(fmt.str());
                                        pv.setEntryService(service);
                                        pv.setEntryMonitoring(false);
                                        pv.setEntry(entry_id);
                                        pv.setName(13);
                                        pv.setValue ("valuevalue");
                                        pv.insert();
                                        id = pv.getId();

					MyFixture::id_list_entry.push_back(id);

					boost::format test = boost::format("select entry_time_begin from request_property_value_%1% where id = %2%") % get_table_postfix(2009, i, (RequestServiceType)service, false) % id;
					res = conn.exec(test.str());

					if(res.size() == 0) {
						BOOST_ERROR(" Record not found in the correct partition ");
					}

					/*
					// ----- now monitoring on
					insert = boost::format() % date % i % service;
					conn.exec(insert.str());

					res = conn.exec(Register::Logger::ManagerImpl::LAST_PROPERTY_VALUE_ID);
					if (res.size() == 0) {
						BOOST_FAIL(" Couldn't obtain ID of the last insert. ");
					}
					id = res[0][0];
					MyFixture::id_list_entry.push_back(id);

					test = boost::format("select time_begin from request_%1% where id = %2%") % get_table_postfix(2009, i, (RequestServiceType)service, true) % id;
					res = conn.exec(test.str());

					if(res.size() == 0) {
						BOOST_ERROR(" Record not found in the correct partition ");
					}
					*/
				}

			} catch (Database::Exception &ex) {
				std::cout << (boost::format("error when working with database (%1%) : %2%") % DB_CONN_STR % ex.what()).str();
			}
		}
	}

}


BOOST_AUTO_TEST_CASE( long_property_name)
{
	BOOST_TEST_MESSAGE("Try to log a property with a very long name (which isn't in the database, also) and very long value");

	Database::ID id;
	TestImplLog test(DB_CONN_STR);
	std::auto_ptr<Register::Logger::RequestProperties> props;

	props = test.create_generic_properties(2, global_call_count++);

	(*props)[1].name = std::string(100, 'N');
	(*props)[1].value = "val - long name";

	(*props)[0].name = "name - very long value";
	(*props)[0].value = std::string(8000, 'X');

	id = test.CreateRequest("100.100.100.100", LC_EPP, "AAA", *props);
	BOOST_CHECK(id != 0);
}

BOOST_AUTO_TEST_CASE( zero_property_name)
{
	BOOST_TEST_MESSAGE(" Try to add property with zero-length name or value");

	Database::ID id;
	TestImplLog test(DB_CONN_STR);
	std::auto_ptr<Register::Logger::RequestProperties> props;

	props = test.create_generic_properties(2, global_call_count++);

	(*props)[1].name = "";
	(*props)[1].value = "";

	(*props)[0].name = "name zero length value";
	(*props)[0].value = "";

	id = test.CreateRequest("100.100.100.100", LC_EPP, "CCC", *props);

	BOOST_CHECK(id != 0);
}


BOOST_AUTO_TEST_CASE( without_properties )
{

	//////
	BOOST_TEST_MESSAGE(" Create a simple message, update it multiple times, and close it without using any properties. ");


	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	id1 = test.CreateRequest("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD");
	BOOST_CHECK(id1 != 0);
	BOOST_CHECK(test.UpdateRequest(id1));
	BOOST_CHECK(test.CloseRequest(id1, "ZZZZZZZZZZZZZZZZZZZZZ"));
}

BOOST_AUTO_TEST_CASE( service_types)
{
	BOOST_TEST_MESSAGE(" Try to use all possible service types");

	TestImplLog test(DB_CONN_STR);
	for (int i=LC_UNIX_WHOIS;i<LC_MAX_SERVICE;i++) {
		BOOST_CHECK(test.CreateRequest("111.222.111.222", (RequestServiceType)i, "aaa"));
	}
}

BOOST_AUTO_TEST_CASE( invalid_ip)
{
	BOOST_TEST_MESSAGE(" Try to send an invalid IP address");

	TestImplLog test(DB_CONN_STR);
	BOOST_CHECK(test.CreateRequest("ABC", LC_PUBLIC_REQUEST, "AA") == 0);
	BOOST_CHECK(test.CreateRequest("127.0.0.256", LC_PUBLIC_REQUEST, "AA") == 0);
}


BOOST_AUTO_TEST_CASE( zero_length_strings )
{
	BOOST_TEST_MESSAGE(" Try using zero length strings in content and ip address. ");

	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	std::auto_ptr<Register::Logger::RequestProperties> props, props1;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.CreateRequest("", LC_PUBLIC_REQUEST, "", *props);
	BOOST_CHECK(id1 != 0);

	props1 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.UpdateRequest(id1, *props1));
	props = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.CloseRequest(id1, "", *props));

}

BOOST_AUTO_TEST_CASE( null_strings )
{
	BOOST_TEST_MESSAGE(" Try using nulls length strings in content and ip address. ");

	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	std::auto_ptr<Register::Logger::RequestProperties> props, props1;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.CreateRequest(NULL, LC_PUBLIC_REQUEST, NULL, *props);
	BOOST_CHECK(id1 != 0);

	props1 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.UpdateRequest(id1, *props1));
	props = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.CloseRequest(id1, NULL, *props));
}

BOOST_AUTO_TEST_CASE( long_strings )
{
	BOOST_TEST_MESSAGE(" Try using very long strings in content and ip address. ");

	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	std::auto_ptr<Register::Logger::RequestProperties> props, props1;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.CreateRequest(std::string(100, 'X').c_str(), LC_PUBLIC_REQUEST, std::string(5000, 'X').c_str(), *props);
	BOOST_CHECK(id1 == 0);

	id1 = test.CreateRequest("122.123.124.125", LC_PUBLIC_REQUEST, std::string(5000, 'X').c_str(), *props);
	BOOST_CHECK(id1 != 0);

	props1 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.UpdateRequest(id1, *props1));


	props = test.create_generic_properties(1, global_call_count++);

	BOOST_CHECK(test.CloseRequest(id1, std::string(5000, 'X').c_str(), *props));
}

BOOST_AUTO_TEST_CASE( normal_event )
{
	BOOST_TEST_MESSAGE(" Create a simple message, update it multiple times, and close it. ");

	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	std::auto_ptr<Register::Logger::RequestProperties> props, props1, props2;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.CreateRequest("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props);
	BOOST_CHECK(id1 != 0);

	props1 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.UpdateRequest(id1, *props1));
	props2 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.UpdateRequest(id1, *props2));

	props = test.create_generic_properties(1, global_call_count++);

	BOOST_CHECK(test.CloseRequest(id1, "ZZZZZZZZZZZZZZZZZZZZZ", *props));

}

BOOST_AUTO_TEST_CASE( no_props )
{
	BOOST_TEST_MESSAGE(" Create an event without any properties");

	Register::Logger::RequestProperties no_props;
	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	id1 = test.CreateRequest("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", no_props);
	BOOST_CHECK(id1 != 0);

	BOOST_CHECK(test.UpdateRequest(id1, no_props));

	BOOST_CHECK(test.UpdateRequest(id1, no_props));


	BOOST_CHECK(test.CloseRequest(id1, "ZZZZZZZZZZZZZZZZZZZZZ", no_props));
}

BOOST_AUTO_TEST_CASE( _2_events )
{
	BOOST_TEST_MESSAGE(" Create an event, create a second event, close the second, close the first...");
	TestImplLog test(DB_CONN_STR);
	Database::ID id1, id2;

	std::auto_ptr<Register::Logger::RequestProperties> props1, props2, props;
	props1 = test.create_generic_properties(3, global_call_count++);
	props2 = test.create_generic_properties(3, global_call_count++);

	id1 = test.CreateRequest("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props1);
	BOOST_CHECK(id1 != 0);

	id2 = test.CreateRequest("101.101.101.101", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props2);
	BOOST_CHECK(id2 != 0);

	props = test.create_generic_properties(1, global_call_count++);

	BOOST_CHECK(test.CloseRequest(id2, "ZZZZZZZZZZZZZZZZZZZZZ", *props));

	BOOST_CHECK(test.CloseRequest(id1, "YYYYYYYYYYYYYYYY", *props));
}


BOOST_AUTO_TEST_CASE( already_closed )
{
	BOOST_TEST_MESSAGE(" Try to update and close already closed event. ");

	TestImplLog test(DB_CONN_STR);
	Database::ID id1, id2;
	std::auto_ptr<Register::Logger::RequestProperties> props, props1;

	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.CreateRequest("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props);
	BOOST_CHECK(id1 != 0);
	BOOST_CHECK(test.CloseRequest(id1, "YYYYYYYYYYYYYYYY"));
	// record closed here

	props1 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(!test.UpdateRequest(id1, *props1));
	BOOST_CHECK(!test.UpdateRequest(id1, *props1));
	BOOST_CHECK(!test.CloseRequest(id1, "ZZZZZZZZZZZZZZZZZZZZZ", *props1));

}

BOOST_AUTO_TEST_CASE( close_record_0 )
{
	BOOST_TEST_MESSAGE(" Try to close record with id 0");
	TestImplLog test (DB_CONN_STR);

	BOOST_CHECK(!test.CloseRequest(0, "ZZZZ"));

}


}  // namespace TestLogd

