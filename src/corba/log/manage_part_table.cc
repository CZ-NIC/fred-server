// TODO : cleanup of tables, remove _TESTING_ and so on ...
/*
 * manage_part_table.cc
 *
 *  Created on: Mar 10, 2009
 *      Author: jvicenik
 */


#include <boost/format.hpp>
#include <string>

// TODO this shouldn't be here
#include "log_impl.h"
/*
#include "db/connection.h"
*/
#include "register/db_settings.h"

// TODO : to be removed
#define _TESTING_

const std::string DB_CONN_STR("host=localhost port=22345 dbname=fred user=fred password=password connect_timeout=2");

using namespace boost;

void create_table(Database::Connection &conn, std::string table_base, int year, int month, LogServiceType type);

// create_table(Connection &conn, int year, int month, LogServiceType type)
void create_table(Database::Connection &conn, std::string table_base, int year, int month, LogServiceType type)
{
	int i(10);
	std::string service_name;
	format service_condition, lower, upper;

	shortyear = (year - 2000) % 100;

	// TODO actually USE (full) year
#ifdef _TESTING_
	// month is in fact day in march in this case :)
	lower  = format("%1$02d-03-%2$02d") % shortyear % month;

	if (month == 31) {
		upper  = format("%1$02d-04-01") % (shortyear+1);
	} else {
		upper  = format("%1$02d-03-%2$02d") % shortyear % (month+1);
	}
#else
	// lower  = format("%1%-%2%-01") % shortyear % month;
	lower  = format("%1$02d-%2$02d-01") % shortyear % month;

	if (month == 12) {
		upper  = format("%1$02d-01-01") % (shortyear+1);
	} else {
		upper  = format("%1$02d-%2$02d-01") % shortyear % (month+1);
	}

#endif

	std::cout << "Upper: " << upper << ", Lower: " << lower << std::endl;

	switch (type) {
		case LC_UNIX_WHOIS:
		case LC_WEB_WHOIS:
			service_condition = format("(service = %1% or service = %2% )") % LC_UNIX_WHOIS % LC_WEB_WHOIS;
			service_name = "whois";
			break;
		case LC_EPP:
			service_condition = format( "(service = %1%)" ) % LC_EPP;
			service_name = "epp";
			break;
		default:
			service_condition = format( "(service <> %1% AND service <> %2% AND service <> %3%)" ) % LC_EPP % LC_UNIX_WHOIS % LC_WEB_WHOIS;
			service_name = "other";
			break;
	}

#ifdef _TESTING_
	// month is in fact day in march in this case :)
	format table_postfix = format("%1%_%2$02d_03_%3$02d") % service_name % shortyear % month;

	format table_name = format("%1%_%2%") % table_base % table_postfix;
	format rule_name = format ("%1%_insert_%2%") % table_base % table_postfix;

#else
	format table_postfix = format("%1%_%2$02d_%3$02d") % service_name % shortyear % month;

	format table_name = format("%1%_%2%") % table_base % table_postfix;
	format rule_name = format ("%1%_insert_%2%") % table_base % table_postfix;
#endif

	format create_table  = format( "CREATE TABLE %1%    ("
						  "	CHECK (%2% and time_begin >= TIMESTAMP %3% and time_begin < TIMESTAMP %4%) )"
						  " INHERITS (%5%) ") % table_name % service_condition % lower % upper % table_base;


	format spec_alter_table;
	format create_rule;

	if(table_base == "log_entry") {
		// TODO insert will be usually done directly
		create_rule = format( "CREATE RULE %1% AS "
								  "	ON INSERT TO %4% WHERE (time_begin >= TIMESTAMP %2% and time_begin < TIMESTAMP %3%) "
								  "DO INSTEAD "
								  "INSERT INTO %1% VALUES ( NEW.id, NEW.time_begin, NEW.time_end, NEW.source_ip, NEW.service); " ) % table_name % lower % upper % table_base;


		spec_alter_table = format("ALTER TABLE %1% ADD PRIMARY KEY (id); ") % table_name;

	} else if(table_base == "log_raw_content") {
		// insert will be usually done directly
		create_rule = format( "CREATE RULE %1% AS "
								  "	ON INSERT TO %4% WHERE (time_begin >= TIMESTAMP %2% and time_begin < TIMESTAMP %3%) "
								  "DO INSTEAD "
								  "INSERT INTO %1% VALUES ( NEW.entry_time_begin, NEW.entry_id, NEW.content, NEW.is_response); " ) % table_name % lower % upper % table_base;

		spec_alter_table = format("ALTER TABLE %1% ADD CONSTRAINT %1%_entry_id_fkey FOREIGN KEY (entry_id) REFERENCES log_entry_%2%(id); ") % table_name % table_postfix;

	} else if(table_base == "log_property_value") {
		// insert will be usually done directly
		create_rule = format( "CREATE RULE %1% AS "
								  "	ON INSERT TO %4% WHERE (time_begin >= TIMESTAMP %2% and time_begin < TIMESTAMP %3%) "
								  "DO INSTEAD "
								  "INSERT INTO %1% VALUES ( NEW.entry_time_begin, NEW.id, NEW.entry_id, NEW.name_id, NEW.value, NEW.output, NEW.parent_id); " ) % table_name % lower % upper % table_base;

		spec_alter_table = format(	"ALTER TABLE %1% ADD PRIMARY KEY (id); "
				"ALTER TABLE %1% ADD CONSTRAINT %1%_entry_id_fkey FOREIGN KEY (entry_id) REFERENCES log_entry_%2%(id); "
				"ALTER TABLE %1% ADD CONSTRAINT %1%_name_id_fkey FOREIGN KEY (name_id) REFERENCES log_raw_content_%2%(id); "
				"ALTER TABLE %1% ADD CONSTRAINT %1%_parent_id_fkey FOREIGN KEY (parent_id) REFERENCES %1%(id); ") % table_name % table_postfix;

	} else {
		// spec_alter_table = format(""); TODO is it ok?
		std::cout << "WARNING: unknown table name " << table_base << std::endl;
	}

	format alter_table  = format("ALTER TABLE %1% DROP CONSTRAINT %2%_no_insert_root") % table_name % table_base;


	std::cout << "Create table: " << create_table << std::endl;
	std::cout << "Create rule: " << create_rule << std::endl;
	std::cout << "Alter table: " << alter_table << std::endl;
	std::cout << "Specific alter table: " << spec_alter_table << std::endl;

	conn.exec(create_table.str());
	conn.exec(create_rule.str());
	conn.exec(spec_alter_table.str());
	conn.exec(alter_table.str());
}

int main()
{
	try {
		Database::Connection conn(DB_CONN_STR);

		create_table (conn, "log_entry", 2009, 3, LC_UNIX_WHOIS);
		std::cout << std::endl;
		create_table (conn, "log_raw_content", 2009, 3, LC_UNIX_WHOIS);
		std::cout << std::endl;
		create_table (conn, "log_property_value", 2009, 3, LC_UNIX_WHOIS);

	} catch (Database::Exception &ex) {
		std::cerr <<  boost::format("cannot connect to database %1% : %2%") % DB_CONN_STR % ex.what();
	}

}





