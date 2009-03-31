
/* TODO :
 * property names should exist in the database - the test shouldn't create any new ones
 * database cleanup after testing
 * database querying during the test
 * - test child properties
 */

#include <iostream>
#define BOOST_TEST_MODULE Test fred-logd
/*
 * for non-header library version:
 * #include <boost/test/unit_test.hpp>
 * */

#include <boost/test/included/unit_test.hpp>


// TODO - change -- configure your own loger a la server.cc

#include "log_impl.h"

using namespace Database;

#ifdef PACKAGE
#undef PACKAGE
#endif

#define PACKAGE "test_logd"



const std::string DB_CONN_STR("host=localhost port=22345 dbname=fred user=fred password=password connect_timeout=2");


std::list<ID> id_list;

// TODO maybe it should inherit Impl_Log or be a friend class
class TestImplLog {
	Impl_Log logd;
	Connection conn;
	static LogProperties no_props;

public:
	TestImplLog (std::string connection_string) : logd(connection_string), conn(connection_string) {
	};

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

struct MyFixture;

LogProperties TestImplLog::no_props;

Database::ID TestImplLog::new_event(const char *ip_addr, const LogServiceType serv, const char * content_in, const LogProperties &props)
{
	// TODO replace constant 1000
	Database::ID ret = logd.i_new_event(ip_addr, serv, content_in, props, 1000);
	boost::format query;

	if(ret == 0) return 0;

	query = boost::format ( "select source_ip, service, raw.content from log_entry join log_raw_content raw on raw.entry_id=id where id=%1%") % ret;
	Result res = conn.exec(query.str());

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

	id_list.push_back(ret);
	// TODO
	// ::MyFixture::id_list.push_back(ret);

	return ret;
}

bool TestImplLog::update_event(const Database::ID id, const LogProperties &props)
{
	bool result = logd.i_update_event(id, props);

	if (!result) return result;

	// check_db_properties(id, props);
	// TODO compare with database, again

	return result;
}

bool TestImplLog::update_event_close(const Database::ID id, const char *content_out, const LogProperties &props)
{
	bool result = logd.i_update_event_close(id, content_out, props);

	if(!result) return result;

	check_db_properties_subset(id, props);

	return result;
	// TODO as usual
}

std::auto_ptr<LogProperties> TestImplLog::create_generic_properties(int number, int value_id)
{
	std::auto_ptr<LogProperties> ret(new LogProperties(number));
	LogProperties &ref = *ret;

	ret->length(number);

	for(int i=0;i<number;i++) {
		// TODO more property names....
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

	if ( (std::string)r[0] != p.name)  return false;
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
	if (props.length() == 0) return;

	boost::format query = boost::format("select name, value, parent_id, output from log_property_value pv join log_property_name pn on pn.id=pv.name_id where pv.entry_id = %1% order by pv.id") % rec_id;

	Result res = conn.exec(query.str());

	// this is expected for a *_subset function...
	int pind = 0;
	if(res.size() > props.length()) {
		for(int i=0; i<res.size(); i++) {
			// TODO this should definitely do something :)
			if(property_match(res[i], props[pind])) {
				// property pind found in the sql result, proceed to another item in the list
				pind++;
			} else {
				// doesn't match, continue to the next row of the result
				continue;
			}
		}

		if(pind < props.length()) {
			BOOST_ERROR(boost::format(" Some properties were not found in database for record %1") % rec_id);
		}
	// but this is kinda' weird, something had to go wrong....
	} else if(res.size() < props.length()) {
		BOOST_ERROR(" Some properties were not stored... ");
	} else if(res.size() == props.length()) {
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

	if (res.size() != props.length() ) {
		BOOST_ERROR(" Not all properties have been loaded into the database");
	}

	// TODO more specific messages:
	for(int i=0; i<res.size(); i++) {
		BOOST_CHECK( (std::string)res[i][0] 	== props[i].name );
		BOOST_CHECK( (std::string)res[i][1] 	== props[i].value );
		if(props[i].child) {
			BOOST_CHECK(!res[i][2].isnull());
		} else {
			BOOST_CHECK(res[i][2].isnull());
		}
		BOOST_CHECK( (bool)res[i][3]		== props[i].output );

		BOOST_CHECK( property_match(res[i], props[i]));
	}
}

static int global_call_count = 0;


struct MyFixture {
	MyFixture() {
		std::cout << "This is INIT func !!!!!!!!!!!!!" << std::endl;


		Logging::Manager::instance_ref().get(PACKAGE).addHandler(Logging::Log::LT_FILE, std::string("log_test_logd.txt"));
		Logging::Manager::instance_ref().get(PACKAGE).setLevel(Logging::Log::LL_TRACE);
		LOGGER(PACKAGE).info("Logging initialized");
	}

	~MyFixture() {
		std::list<ID>::iterator it = id_list.begin();

		try {
			Connection conn(DB_CONN_STR);

			std::cout << "Deleting database records. " << std::endl;
			while(it != id_list.end()) {
				conn.exec( (boost::format("delete from log_raw_content where entry_id=%1%") % *it).str() );
				conn.exec( (boost::format("delete from log_property_value where entry_id=%1%") % *it).str() );
				conn.exec( (boost::format("delete from log_entry where id=%1%") % *it).str() );
				std::cout << *it << std::endl;
				it++;
			}
		} catch (Database::Exception &ex) {
			std::cout << (boost::format("error when working with database (%1%) : %2%") % DB_CONN_STR % ex.what()).str();
		}
	}

//	static std::list<ID> id_list;
};

// std::list<ID> MyFixture::id_list;


BOOST_GLOBAL_FIXTURE( MyFixture );


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
	//TODO this should be different.... probably
	BOOST_TEST_MESSAGE(" Try to use all possible service types");

	TestImplLog test(DB_CONN_STR);
	for (int i=0;i<30;i++) {
		BOOST_CHECK(test.new_event("111.222.111.222", (LogServiceType)i, "aaa"));
	}
	// TODO check database
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

	// TODO the first argument is not a valid IP address
	id1 = test.new_event(std::string(100, 'X').c_str(), LC_PUBLIC_REQUEST, std::string(5000, 'X').c_str(), *props);
	BOOST_CHECK(id1 == 0);
	// the previous shoul've failed

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
	// TODO duplicity?
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

/* TODO

BOOST_MESSAGE(" Add some child properties. ");
BOOST_MESSAGE(" Distinct input and output properties. ");

*/

