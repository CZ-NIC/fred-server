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

// FRED logging
#include "log/logger.h"
#include "log/context.h"

// TODO : to be removed
#define _TESTING_


using namespace boost;

void create_table(Database::Connection &conn, std::string table_base, int year, int month, LogServiceType type);


format get_table_postfix(int year, int month)
{
	int shortyear = (year - 2000) % 100;
	format table_postfix;

#ifdef _TESTING_
	// month is in fact day in march in this case :)
	// SERVICE format table_postfix = format("%1%_%2$02d_03_%3$02d") % service_name % shortyear % month;
	table_postfix = format("%1$02d_03_%2$02d") % shortyear % month;
#else
	table_postfix = format("%1%_%2$02d_%3$02d") % service_name % shortyear % month;
#endif

	return table_postfix;
}

bool exist_tables(Database::Connection &conn, int year, int month)
{
	format query = format("select * from pg_tables where tablename = 'log_entry_%1%'") % get_table_postfix(year, month);

	Result res = conn.exec(query.str());

	if(res.size() > 0) {
		return true;
	} else {
		return false;
	}
}

// create_table(Connection &conn, int year, int month, LogServiceType type)
void create_table(Database::Connection &conn, std::string table_base, int year, int month, LogServiceType type = LC_EPP)
{
	format lower, upper;

	/* SERVICE TODO
	 * std::string service_name;
	 * format service_condition;
	 */



	// TODO actually USE (full) year
#ifdef _TESTING_
	// month is in fact day in march in this case :)
	lower  = format("%1$02d-03-%2$02d") % year % month;

	if (month == 31) {
		upper  = format("%1$02d-04-01") % (year+1);
	} else {
		upper  = format("%1$02d-03-%2$02d") % year % (month+1);
	}
#else
	// lower  = format("%1%-%2%-01") % year % month;
	lower  = format("%1$02d-%2$02d-01") % year % month;

	if (month == 12) {
		upper  = format("%1$02d-01-01") % (year+1);
	} else {
		upper  = format("%1$02d-%2$02d-01") % year % (month+1);
	}

#endif

	/* SERVICE
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
	*/

	format table_postfix = get_table_postfix(year, month);

	format table_name = format("%1%_%2%") % table_base % table_postfix;
	format rule_name = format ("%1%_insert_%2%") % table_base % table_postfix;

	/* SERVICE
		format create_table  = format( "CREATE TABLE %1%    ("
								  "	CHECK (%2% and time_begin >= TIMESTAMP '%3%' and time_begin < TIMESTAMP '%4%') )"
								  " INHERITS (%5%) ") % table_name % service_condition % lower % upper % table_base;
		*/

	format create_table;
	format spec_alter_table;
	format create_rule;
	format create_indexes;

	if(table_base == "log_entry") {
		create_table  = format( "CREATE TABLE %1%    ("
								  "	CHECK (time_begin >= TIMESTAMP '%2%' and time_begin < TIMESTAMP '%3%') )"
								  " INHERITS (%4%) ") % table_name % lower % upper % table_base;

		// TODO insert will be usually done directly
		create_rule = format( "CREATE RULE %1% AS "
								  "	ON INSERT TO %4% WHERE (time_begin >= TIMESTAMP '%2%' and time_begin < TIMESTAMP '%3%') "
								  "DO INSTEAD "
								  "INSERT INTO %1% VALUES ( NEW.id, NEW.time_begin, NEW.time_end, NEW.source_ip, NEW.service, NEW.action_type, NEW.is_monitoring); " ) % table_name % lower % upper % table_base;


		spec_alter_table = format("ALTER TABLE %1% ADD PRIMARY KEY (id); ") % table_name;

		create_indexes   = format("CREATE INDEX %1%_time_begin_idx ON %1%(time_begin); "
								"CREATE INDEX %1%_time_end_idx ON %1%(time_end);"
								"CREATE INDEX %1%_source_ip_idx ON %1%(source_ip);"
								"CREATE INDEX %1%_service_idx ON %1%(service);"
								"CREATE INDEX %1%_action_type_idx ON %1%(action_type);"
								"CREATE INDEX %1%_monitoring_idx ON %1%(is_monitoring);") % table_name;

	} else if (table_base == "log_session") {

		create_table = format( "CREATE TABLE %1%    ("
				  "	CHECK (login_date >= TIMESTAMP '%2%' and login_date < TIMESTAMP '%3%') )"
				  " INHERITS (%4%) ") % table_name % lower % upper % table_base;

		create_rule = format( "CREATE RULE %1% AS "
										  "	ON INSERT TO %4% WHERE (login_date >= TIMESTAMP '%2%' and login_date < TIMESTAMP '%3%') "
										  "DO INSTEAD "
										"INSERT INTO %1% VALUES ( NEW.id, NEW.name, NEW.login_date, NEW.logout_date, NEW.login_TRID, NEW.logout_TRID, NEW.lang); " ) % table_name % lower % upper % table_base;

		spec_alter_table = format("ALTER TABLE %1% ADD PRIMARY KEY (id); ") % table_name;

		create_indexes = format("CREATE INDEX %1%_name_idx ON %1%(name);"
								"CREATE INDEX %1%_login_date_idx ON %1%(login_date);"
								"CREATE INDEX %1%_lang_idx ON %1%(lang);") % table_name;

	} else {
		// create table is different for log_entry and for other tables...
		create_table  = format( "CREATE TABLE %1%    ("
										  "	CHECK (entry_time_begin >= TIMESTAMP '%2%' and entry_time_begin < TIMESTAMP '%3%') )"
										  " INHERITS (%4%) ") % table_name % lower % upper % table_base;

		if(table_base == "log_raw_content") {

			// insert will be usually done directly
			create_rule = format( "CREATE RULE %1% AS "
									  "	ON INSERT TO %4% WHERE (entry_time_begin >= TIMESTAMP '%2%' and entry_time_begin < TIMESTAMP '%3%') "
									  "DO INSTEAD "
									  "INSERT INTO %1% VALUES ( NEW.entry_time_begin, NEW.entry_id, NEW.content, NEW.is_response); " ) % table_name % lower % upper % table_base;

			spec_alter_table = format("ALTER TABLE %1% ADD CONSTRAINT %1%_entry_id_fkey FOREIGN KEY (entry_id) REFERENCES log_entry_%2%(id); ") % table_name % table_postfix;

			create_indexes = format("CREATE INDEX %1%_entry_time_begin_idx ON %1%(entry_time_begin);"
			"CREATE INDEX %1%_entry_id_idx ON %1%(entry_id);"
			"CREATE INDEX %1%_content_idx ON %1%(content);"
			"CREATE INDEX %1%_is_response_idx ON %1%(is_response);") % table_name;

		} else if(table_base == "log_property_value") {
			// insert will be usually done directly
			create_rule = format( "CREATE RULE %1% AS "
									  "	ON INSERT TO %4% WHERE (entry_time_begin >= TIMESTAMP '%2%' and entry_time_begin < TIMESTAMP '%3%') "
									  "DO INSTEAD "
									  "INSERT INTO %1% VALUES ( NEW.entry_time_begin, NEW.id, NEW.entry_id, NEW.name_id, NEW.value, NEW.output, NEW.parent_id); " ) % table_name % lower % upper % table_base;

			spec_alter_table = format(	"ALTER TABLE %1% ADD PRIMARY KEY (id); "
					"ALTER TABLE %1% ADD CONSTRAINT %1%_entry_id_fkey FOREIGN KEY (entry_id) REFERENCES log_entry_%2%(id); "
					"ALTER TABLE %1% ADD CONSTRAINT %1%_name_id_fkey FOREIGN KEY (name_id) REFERENCES log_property_name(id); "
					"ALTER TABLE %1% ADD CONSTRAINT %1%_parent_id_fkey FOREIGN KEY (parent_id) REFERENCES %1%(id); ") % table_name % table_postfix;

			create_indexes = format("CREATE INDEX %1%_entry_time_begin_idx ON %1%(entry_time_begin);"
			"CREATE INDEX %1%_entry_id_idx ON %1%(entry_id);"
			"CREATE INDEX %1%_name_id_idx ON %1%(name_id);"
			"CREATE INDEX %1%_value_idx ON %1%(value);"
			"CREATE INDEX %1%_output_idx ON %1%(output);"
			"CREATE INDEX %1%_parent_id_idx ON %1%(parent_id);") % table_name;

		} else {
			// spec_alter_table = format(""); TODO is it ok?
#ifdef HAVE_LOGGER
			LOGGER("fred-server").info( boost::format("Specified table name %1% is not known.") % table_base);
#endif

		}
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
	conn.exec(create_indexes.str());
}

void create_table_set(Database::Connection &conn, int year, int month)
{
	create_table(conn, "log_entry", year, month);
	create_table(conn, "log_raw_content", year, month);
	create_table(conn, "log_property_value", year, month);
	create_table(conn, "log_session", year, month);
}

