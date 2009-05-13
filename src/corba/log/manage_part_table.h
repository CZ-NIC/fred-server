/*
 * manage_part_table.h
 *
 *  Created on: Mar 25, 2009
 *      Author: jvicenik
 */

#ifndef MANAGE_PART_TABLE_H_
#define MANAGE_PART_TABLE_H_

#include <boost/format.hpp>
#include <string>

#include "log_impl.h"
#include "register/db_settings.h"

/*
#define _TESTING_
*/

const std::string DB_CONN_STR("host=localhost port=22345 dbname=fred user=fred password=password connect_timeout=2");

std::string create_date_str(int y, int m);
boost::format get_table_postfix(int year, int month);

/*
bool exist_tables(Database::Connection &conn, int year, int month);
void create_table_set(Database::Connection &conn, int year, int month);
void check_and_create_all(Database::Connection &conn, int year, int month);
void create_table_if_not_present(Database::Connection &conn, std::string table_base, int year, int month);
void create_table(Database::Connection &conn, std::string table_base, int year, int month, LogServiceType type = LC_EPP);
*/

#endif /* MANAGE_PART_TABLE_H_ */

