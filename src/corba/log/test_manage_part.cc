/*
 * test_manage_part.cc
 *
 *  Created on: Mar 25, 2009
 *      Author: jvicenik
 */

#include "manage_part_table.h"


int main()
{
	try {
		Database::Connection conn(DB_CONN_STR);
/*
		create_table (conn, "log_entry", 2009, 3, LC_UNIX_WHOIS);
		std::cout << std::endl;
		create_table (conn, "log_raw_content", 2009, 3, LC_UNIX_WHOIS);
		std::cout << std::endl;
		create_table (conn, "log_property_value", 2009, 3, LC_UNIX_WHOIS);
		*/

/*
		create_table (conn, "log_entry", 2009, 25);
		create_table (conn, "log_entry", 2009, 26);

		create_table (conn, "log_property_value", 2009, 25);
		create_table (conn, "log_property_value", 2009, 26);

		create_table (conn, "log_raw_content", 2009, 25);
		create_table (conn, "log_raw_content", 2009, 26);
		*/

/*
		create_table_set(conn, 2009, 27);
		create_table_set(conn, 2009, 28);
		*/
		for(int day=2; day<26; day++) {
			create_table_set(conn, 2009, day);
		}


	} catch (Database::Exception &ex) {
		std::cout << (boost::format("error when working with database (%1%) : %2%") % DB_CONN_STR % ex.what()).str();
	}

}

