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

#include "register/db_settings.h"

const std::string DB_CONN_STR("host=localhost port=22345 dbname=fred user=fred password=password connect_timeout=2");

bool exist_tables(Database::Connection &conn, int year, int month);
void create_table_set(Database::Connection &conn, int year, int month);


#endif /* MANAGE_PART_TABLE_H_ */
