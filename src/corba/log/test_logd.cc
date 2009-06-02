
#include <iostream>
#define BOOST_TEST_MODULE Test fred-logd
/*
 * for non-header library version:
 * #include <boost/test/unit_test.hpp>
 * */

#include <boost/test/included/unit_test.hpp>
#include <stdio.h>

#include "log_impl.h"
#include "manage_part_table.h"

using namespace Database;

namespace TestLogd {

const int MONTHS_COUNT  = 12;

// value from database table log_action_type
const int UNKNOWN_ACTION = 1000;

const std::string DB_CONN_STR("host=localhost port=22345 dbname=fred user=fred password=password connect_timeout=2");

struct MyFixture {
	static std::list<ID> id_list_entry;
	static std::list<ID> id_list_session;

	MyFixture() {
		std::cout << "This is INIT func !!!!!!!!!!!!!" << std::endl;

		Logging::Manager::instance_ref().get(PACKAGE).addHandler(Logging::Log::LT_FILE, std::string("log_test_logd.txt"));
		Logging::Manager::instance_ref().get(PACKAGE).setLevel(Logging::Log::LL_TRACE);
		LOGGER(PACKAGE).info("Logging initialized");
	
		// initialize database connection manager
		Manager::init(new ConnectionFactory(DB_CONN_STR));
	}

	~MyFixture() {
		try {
			std::list<ID>::iterator it = id_list_entry.begin();
			Connection conn = Manager::acquire();

			std::cout << "Deleting database records from log_entry and related. " << std::endl;
			while(it != id_list_entry.end()) {
				conn.exec( (boost::format("delete from log_raw_content where entry_id=%1%") % *it).str() );
				conn.exec( (boost::format("delete from log_property_value where entry_id=%1%") % *it).str() );
				conn.exec( (boost::format("delete from log_entry where id=%1%") % *it).str() );
				it++;
			}
			
			std::cout << "Deleting database records from log_session." << std::endl;	
			for(it = id_list_session.begin(); it != id_list_session.end();it++) {
				conn.exec( (boost::format("delete from log_session where id=%1%") % *it).str() );
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
	Impl_Log logd;
	Connection conn;
	static LogProperties no_props;

public:
	TestImplLog (const std::string connection_string) : logd(connection_string), conn(Manager::acquire()) {
	};

	TestImplLog (const std::string connection_string, const std::string monitoring_file) : logd(connection_string, monitoring_file), 
			conn(Manager::acquire()) {
	};

	Connection &get_conn() {
		return conn;
	};

	Database::ID new_session(Languages lang, const char *name, const char *clTRID = "Test-clTRID");
	bool         end_session(Database::ID id, const char *clTRID = "Test-clTRID-end");

	Database::ID new_event(const char * ip_addr, const LogServiceType serv, const char * content_in,  const LogProperties &props = TestImplLog::no_props);
	bool update_event(const Database::ID id, const LogProperties &props = TestImplLog::no_props);
	bool update_event_close(const Database::ID id, const char * content_out, const LogProperties &props = TestImplLog::no_props);
	std::auto_ptr<LogProperties> create_generic_properties(int number, int value_id);

	void check_db_properties_subset(ID rec_id, const LogProperties &props);
	bool property_match(const Row r, const LogProperty &p) ;

// different tests
	void check_db_properties(ID rec_id, const LogProperties & props);

};

inline bool has_content(const char *str) {
	return (str && *str!= '\0');
}

boost::format get_table_postfix_for_now()
{
	boost::posix_time::ptime utime = microsec_clock::universal_time();
	tm str_time = boost::posix_time::to_tm(utime);

	// months in tm are numbered from 0
	str_time.tm_mon++;

	return get_table_postfix((1900 + str_time.tm_year), str_time.tm_mon);

}

struct MyFixture;

LogProperties TestImplLog::no_props;


Database::ID TestImplLog::new_session(Languages lang, const char *name, const char *clTRID)
{
	Database::ID ret = logd.i_new_session(lang, name, clTRID);

	if(ret == 0) return 0;

	// first check if the correct partition was used ...
	boost::format test = boost::format("select login_date from log_session_%1% where id = %2%") % get_table_postfix_for_now() % ret;
	Result res = conn.exec(test.str());	

	if(res.size() == 0) {
		BOOST_ERROR(" Record not found in the correct partition ");
	}

	// now do a regular select from log_session
	res = conn.exec((boost::format("select lang, name, login_TRID from log_session where id=%1%") % ret).str());

	if (res.size() != 1) {
		if (res.size() == 0) {
			BOOST_ERROR(boost::format(" Record created with new_session with id %1% doesn't exist in the database! ") % ret);
		} else if(res.size() > 1) {
			BOOST_ERROR(boost::format(" Multiple records with id %1% after call to new_session! ") % ret);
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
		BOOST_CHECK(std::string(clTRID) == (std::string)res[0][2]);
	}

	MyFixture::id_list_session.push_back(ret);		

	return ret;
}

bool TestImplLog::end_session(Database::ID id, const char *clTRID)
{
	bool ret = logd.i_end_session(id, clTRID);

	if (!ret) return ret;

	Result res = conn.exec( (boost::format("select logout_date, logout_TRID from log_session where id=%1%") % id).str() );
	
	if (res.size() != 1) {
		if (res.size() == 0) {
			BOOST_ERROR(boost::format(" Record created with new_event with id %1% doesn't exist in the database! ") % id);
		} else if(res.size() > 1) {
			BOOST_ERROR(boost::format(" Multiple records with id %1% after call to new_event! ") % id);
		}
	} else {
		BOOST_CHECK((std::string)res[0][1] == std::string(clTRID));
	}

	return ret;
}

Database::ID TestImplLog::new_event(const char *ip_addr, const LogServiceType serv, const char * content_in, const LogProperties &props)
{
	Database::ID ret = logd.i_new_event(ip_addr, serv, content_in, props, UNKNOWN_ACTION);
	boost::format query;

	if(ret == 0) return 0;

	// first check if the correct partition was used ...

	boost::format test = boost::format("select time_begin from log_entry_%1% where id = %2%") % get_table_postfix_for_now() % ret;
	Result res = conn.exec(test.str());	

	if(res.size() == 0) {
		BOOST_ERROR(" Record not found in the correct partition ");
	}

	// now a regular select

	query = boost::format ( "select source_ip, service, raw.content from log_entry join log_raw_content raw on raw.entry_id=id where id=%1%") % ret;
	res = conn.exec(query.str());

	if (res.size() != 1) {
		res = conn.exec( (boost::format("select source_ip, service from log_entry where id=%1%") % ret).str() );

		if (res.size() != 1) {
			if (res.size() == 0) {
				BOOST_ERROR(boost::format(" Record created with new_event with id %1% doesn't exist in the database! ") % ret);
			} else if(res.size() > 1) {
				BOOST_ERROR(boost::format(" Multiple records with id %1% after call to new_event! ") % ret);
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

bool TestImplLog::update_event(const Database::ID id, const LogProperties &props)
{
	bool result = logd.i_update_event(id, props);

	if (!result) return result;

	check_db_properties_subset(id, props);

	return result;
}

bool TestImplLog::update_event_close(const Database::ID id, const char *content_out, const LogProperties &props)
{
	bool result = logd.i_update_event_close(id, content_out, props);

	if(!result) return result;

	boost::format query = boost::format ( "select time_end, raw.content from log_entry join log_raw_content raw on raw.entry_id=id where raw.is_response=true and id=%1%") % id;
	Result res = conn.exec(query.str());

	if (res.size() != 1) {
		res = conn.exec( (boost::format("select time_end from log_entry where id=%1%") % id).str() );

		if (res.size() != 1) {
			if (res.size() == 0) {
				BOOST_ERROR(boost::format(" Record created with new_event with id %1% doesn't exist in the database! ") % id);
			} else if(res.size() > 1) {
				BOOST_ERROR(boost::format(" Multiple records with id %1% after call to new_event! ") % id);
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

std::auto_ptr<LogProperties> TestImplLog::create_generic_properties(int number, int value_id)
{
	std::auto_ptr<LogProperties> ret(new LogProperties(number));
	LogProperties &ref = *ret;

	for(int i=0;i<number;i++) {
		ref[i].name = "handle";
		ref[i].value = (boost::format("val%1%.%2%") % value_id % i).str();

		ref[i].child = false;
		ref[i].output = false;
	}

	return ret;
}

// r is row produced by:
// boost::format query = boost::format("select name, value, parent_id, output from log_property_value pv join log_property_name pn on pn.id=pv.name_id where pv.entry_id = %1% order by pv.id") % rec_id;
// p is single property
// these two are compared :)
bool TestImplLog::property_match(const Row r, const LogProperty &p)
{

	if ( (std::string)r[0] != p.name.substr(0, Impl_Log::MAX_NAME_LENGTH))  return false;
	if ( (std::string)r[1] != p.value) return false;
	if ( (bool)r[3] != p.output) return false;

	if ( p.child ) {
		if (r[2].isnull()) return false;
	} else {
		if (!r[2].isnull()) return false;
	}

	return true;
}

void TestImplLog::check_db_properties_subset(ID rec_id, const LogProperties &props)
{
	if (props.size() == 0) return;

	boost::format query = boost::format("select name, value, parent_id, output from log_property_value pv join log_property_name pn on pn.id=pv.name_id where pv.entry_id = %1% order by pv.id") % rec_id;

	Result res = conn.exec(query.str());

	// this is expected for a *_subset function...
	int pind = 0;
	if(res.size() > props.size()) {
		for(int i=0; i<res.size(); i++) {
			if(property_match(res[i], props[pind])) {
				// property pind found in the sql result, proceed to another item in the list
				pind++;
			} else {
				// doesn't match, continue to the next row of the result
				continue;
			}
		}

		if(pind < props.size()) {
			BOOST_ERROR(boost::format(" Some properties were not found in database for record %1") % rec_id);
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
void TestImplLog::check_db_properties(ID rec_id, const LogProperties & props)
{
	boost::format query = boost::format("select name, value, parent_id, output from log_property_value pv join log_property_name pn on pn.id=pv.name_id where pv.entry_id = %1% order by pv.id") % rec_id;

	Result res = conn.exec(query.str());

	if (res.size() != props.size() ) {
		BOOST_ERROR(" Not all properties have been loaded into the database");
	}

	for(int i=0; i<res.size(); i++) {
		BOOST_CHECK( property_match(res[i], props[i]));
	}
}

static int global_call_count = 0;

void test_monitoring_ip(const std::string &ip, TestImplLog &t, bool result)
{
	Connection conn = Manager::acquire();
	Database::ID id;

	id = t.new_event(ip.c_str(), LC_EPP, "AAA");

	std::cout << " Recent ID: " << id << std::endl;

	boost::format query = boost::format ( "select is_monitoring from log_entry where id=%1%") % id;
	Result res = conn.exec(query.str());

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

	Database::ID id = test.new_session(CS, "regid01", "TestclTRID-session1");

	BOOST_CHECK(id != 0);
	BOOST_CHECK(test.end_session(id, "TestclTRID-session1-end"));

	id = test.new_session(EN, "regid02", "TestclTRID-session2");

	BOOST_CHECK(id != 0);
	BOOST_CHECK(test.end_session(id, "TestclTRID-session2-end"));
}

BOOST_AUTO_TEST_CASE( test_con_sessions )
{
	BOOST_TEST_MESSAGE("Create two concurrent sessions and close them");	

	TestImplLog test(DB_CONN_STR);

	Database::ID id  = test.new_session(CS, "regid03", "TestclTRID-session3");
	Database::ID id1 = test.new_session(EN, "regid04", "TestclTRID-session3");

	BOOST_CHECK(id  != 0);
	BOOST_CHECK(id1 != 0);

	BOOST_CHECK(test.end_session(id,  "TestclTRID-session3-end"));
	BOOST_CHECK(test.end_session(id1, "TestclTRID-session3-end"));
}

BOOST_AUTO_TEST_CASE( close_session_twice )
{
	BOOST_TEST_MESSAGE("Try to close a session which is already closed");	

	TestImplLog test(DB_CONN_STR);

	Database::ID id = test.new_session(CS, "regid01", "TestclTRID-session5");

	BOOST_CHECK(id != 0);
	BOOST_CHECK(test.end_session(id, "TestclTRID-session5-end"));

	BOOST_CHECK(!test.end_session(id, "TestclTRID-session5-end2"));
}

BOOST_AUTO_TEST_CASE( session_without_name )
{
	BOOST_TEST_MESSAGE("Try to create a session without providing the name of registrar/user");	

	TestImplLog test(DB_CONN_STR);

	Database::ID id = test.new_session(CS, NULL, "TestclTRID-session5");
	BOOST_CHECK(id == 0);

	id = test.new_session(CS, "", "TestclTRID-session5");
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
	Connection conn = Manager::acquire();
	boost::format time;

	// i is also used as minutes in time
	for(int i=1;i<MONTHS_COUNT;i++) {
		std::string date = create_date_str(2009, i);		

		try {
			time = boost::format("insert into log_entry (time_begin, service, is_monitoring) values ('%1% 9:%2%:00', 99, true)") % date % i;	

			conn.exec(time.str());

			Result res = conn.exec(Impl_Log::LAST_ENTRY_ID);

			if(res.size() == 0) {
				BOOST_FAIL(" Couldn't obtain ID of the last insert. ");
			}

			id = res[0][0];

			boost::format test = boost::format("select time_begin from log_entry_%1% where id = %2%") % get_table_postfix(2009, i) % id;
			res = conn.exec(test.str());	

			if(res.size() == 0) {
				BOOST_ERROR(" Record not found in the correct partition ");
			}

		} catch (Database::Exception &ex) {
			std::cout << (boost::format("error when working with database (%1%) : %2%") % DB_CONN_STR % ex.what()).str();
		}
	}	
		
}

BOOST_AUTO_TEST_CASE( long_property_name)
{
	BOOST_TEST_MESSAGE("Try to log a property with a very long name (which isn't in the database, also) and very long value");

	Database::ID id;
	TestImplLog test(DB_CONN_STR);
	std::auto_ptr<LogProperties> props;

	props = test.create_generic_properties(2, global_call_count++);

	(*props)[1].name = std::string(100, 'N');
	(*props)[1].value = "val - long name";

	(*props)[0].name = "name - very long value";
	(*props)[0].value = std::string(8000, 'X');

	id = test.new_event("100.100.100.100", LC_EPP, "AAA", *props);
	BOOST_CHECK(id != 0);
}

BOOST_AUTO_TEST_CASE( zero_property_name)
{
	BOOST_TEST_MESSAGE(" Try to add property with zero-length name or value");

	Database::ID id;
	TestImplLog test(DB_CONN_STR);
	std::auto_ptr<LogProperties> props;

	props = test.create_generic_properties(2, global_call_count++);

	(*props)[1].name = "";
	(*props)[1].value = "";

	(*props)[0].name = "name zero length value";
	(*props)[0].value = "";

	id = test.new_event("100.100.100.100", LC_EPP, "CCC", *props);

	BOOST_CHECK(id != 0);
}

BOOST_AUTO_TEST_CASE( without_properties )
{

	//////
	BOOST_TEST_MESSAGE(" Create a simple message, update it multiple times, and close it without using any properties. ");


	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	id1 = test.new_event("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD");
	BOOST_CHECK(id1 != 0);
	BOOST_CHECK(test.update_event(id1));
	BOOST_CHECK(test.update_event_close(id1, "ZZZZZZZZZZZZZZZZZZZZZ"));
}

BOOST_AUTO_TEST_CASE( service_types)
{
	BOOST_TEST_MESSAGE(" Try to use all possible service types");

	TestImplLog test(DB_CONN_STR);
	for (int i=0;i<30;i++) {
		BOOST_CHECK(test.new_event("111.222.111.222", (LogServiceType)i, "aaa"));
	}
}

BOOST_AUTO_TEST_CASE( invalid_ip)
{
	BOOST_TEST_MESSAGE(" Try to send an invalid IP address");

	TestImplLog test(DB_CONN_STR);
	BOOST_CHECK(test.new_event("ABC", LC_PUBLIC_REQUEST, "AA") == 0);
	BOOST_CHECK(test.new_event("127.0.0.256", LC_PUBLIC_REQUEST, "AA") == 0);
}

BOOST_AUTO_TEST_CASE( zero_length_strings )
{
	BOOST_TEST_MESSAGE(" Try using zero length strings in content and ip address. ");

	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	std::auto_ptr<LogProperties> props, props1;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.new_event("", LC_PUBLIC_REQUEST, "", *props);
	BOOST_CHECK(id1 != 0);

	props1 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.update_event(id1, *props1));
	props = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.update_event_close(id1, "", *props));

}

BOOST_AUTO_TEST_CASE( null_strings )
{
	BOOST_TEST_MESSAGE(" Try using nulls length strings in content and ip address. ");

	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	std::auto_ptr<LogProperties> props, props1;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.new_event(NULL, LC_PUBLIC_REQUEST, NULL, *props);
	BOOST_CHECK(id1 != 0);

	props1 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.update_event(id1, *props1));
	props = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.update_event_close(id1, NULL, *props));
}

BOOST_AUTO_TEST_CASE( long_strings )
{
	BOOST_TEST_MESSAGE(" Try using very long strings in content and ip address. ");

	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	std::auto_ptr<LogProperties> props, props1;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.new_event(std::string(100, 'X').c_str(), LC_PUBLIC_REQUEST, std::string(5000, 'X').c_str(), *props);
	BOOST_CHECK(id1 == 0);

	id1 = test.new_event("122.123.124.125", LC_PUBLIC_REQUEST, std::string(5000, 'X').c_str(), *props);
	BOOST_CHECK(id1 != 0);

	props1 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.update_event(id1, *props1));


	props = test.create_generic_properties(1, global_call_count++);

	BOOST_CHECK(test.update_event_close(id1, std::string(5000, 'X').c_str(), *props));
}

BOOST_AUTO_TEST_CASE( normal_event )
{
	BOOST_TEST_MESSAGE(" Create a simple message, update it multiple times, and close it. ");

	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	std::auto_ptr<LogProperties> props, props1, props2;
	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.new_event("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props);
	BOOST_CHECK(id1 != 0);

	props1 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.update_event(id1, *props1));
	props2 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(test.update_event(id1, *props2));

	props = test.create_generic_properties(1, global_call_count++);

	BOOST_CHECK(test.update_event_close(id1, "ZZZZZZZZZZZZZZZZZZZZZ", *props));

}

BOOST_AUTO_TEST_CASE( no_props )
{
	BOOST_TEST_MESSAGE(" Create an event without any properties");

	LogProperties no_props;
	TestImplLog test(DB_CONN_STR);
	Database::ID id1;

	id1 = test.new_event("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", no_props);
	BOOST_CHECK(id1 != 0);

	BOOST_CHECK(test.update_event(id1, no_props));

	BOOST_CHECK(test.update_event(id1, no_props));


	BOOST_CHECK(test.update_event_close(id1, "ZZZZZZZZZZZZZZZZZZZZZ", no_props));
}

BOOST_AUTO_TEST_CASE( _2_events )
{
	BOOST_TEST_MESSAGE(" Create an event, create a second event, close the second, close the first...");
	TestImplLog test(DB_CONN_STR);
	Database::ID id1, id2;

	std::auto_ptr<LogProperties> props1, props2, props;
	props1 = test.create_generic_properties(3, global_call_count++);
	props2 = test.create_generic_properties(3, global_call_count++);

	id1 = test.new_event("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props1);
	BOOST_CHECK(id1 != 0);

	id2 = test.new_event("101.101.101.101", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props2);
	BOOST_CHECK(id2 != 0);

	props = test.create_generic_properties(1, global_call_count++);

	BOOST_CHECK(test.update_event_close(id2, "ZZZZZZZZZZZZZZZZZZZZZ", *props));

	BOOST_CHECK(test.update_event_close(id1, "YYYYYYYYYYYYYYYY", *props));
}


BOOST_AUTO_TEST_CASE( already_closed )
{
	BOOST_TEST_MESSAGE(" Try to update and close already closed event. ");

	TestImplLog test(DB_CONN_STR);
	Database::ID id1, id2;
	std::auto_ptr<LogProperties> props, props1;

	props = test.create_generic_properties(3, global_call_count++);

	id1 = test.new_event("100.100.100.100", LC_PUBLIC_REQUEST, "AAABBBBCCCCCDDDDDD", *props);
	BOOST_CHECK(id1 != 0);
	BOOST_CHECK(test.update_event_close(id1, "YYYYYYYYYYYYYYYY"));
	// record closed here

	props1 = test.create_generic_properties(1, global_call_count++);
	BOOST_CHECK(!test.update_event(id1, *props1));
	BOOST_CHECK(!test.update_event(id1, *props1));
	BOOST_CHECK(!test.update_event_close(id1, "ZZZZZZZZZZZZZZZZZZZZZ", *props1));

}

BOOST_AUTO_TEST_CASE( close_record_0 )
{
	BOOST_TEST_MESSAGE(" Try to close record with id 0");
	TestImplLog test (DB_CONN_STR);

	BOOST_CHECK(!test.update_event_close(0, "ZZZZ"));

}


}  // namespace TestLogd 

