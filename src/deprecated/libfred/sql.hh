/*
 * Copyright (C) 2007-2019  CZ.NIC, z. s. p. o.
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
#ifndef SQL_HH_A39EC0A3D09946AFA4580FB98CAE0749
#define SQL_HH_A39EC0A3D09946AFA4580FB98CAE0749

#include <boost/date_time/gregorian/formatters.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>

#define TIME_FILTER_BEG(f) (!f.begin().is_special())
#define TIME_FILTER_END(f) (!f.end().is_special())
#define TIME_FILTER_SET(f) (TIME_FILTER_BEG(f) || TIME_FILTER_END(f))

#define SQL_TIME_FILTER(x,colname,member) \
  if (TIME_FILTER_BEG(member)) \
     x << "AND " << colname << ">='"  \
       <<  to_iso_extended_string(member.begin()) \
       << "' "; \
  if (TIME_FILTER_END(member)) \
     x << "AND " << colname << "<='"  \
       <<  to_iso_extended_string(member.end()) \
       << "' "
#define SQL_DATE_FILTER(x,colname,member) \
  if (!member.begin().is_special()) \
     x << "AND " << colname << ">='"  \
       <<  to_iso_extended_string(member.begin().date()) \
       << "' "; \
  if (!member.end().is_special()) \
     x << "AND " << colname << "<='"  \
       <<  to_iso_extended_string(member.end().date()) \
       << " 23:59:59' "
#define SQL_WILDCARD_FILTER_FILL(x,colname,member) \
     x << "AND " \
       << colname << " ILIKE TRANSLATE('" << db->Escape2(member) << "','*?','%_') "
#define SQL_WILDCARD_FILTER(x,colname,member) \
  if (!member.empty()) SQL_WILDCARD_FILTER_FILL(x,colname,member)
#define SQL_HANDLE_FILTER(x,colname,member) \
  SQL_WILDCARD_FILTER(x,colname,member)
#define SQL_HANDLE_WILDCHECK_FILTER(x,colname,member,w,u) \
  if (!member.empty()) { \
    if (w && (member.find('*') != std::string::npos || \
              member.find('?') != std::string::npos)) \
      SQL_WILDCARD_FILTER_FILL(x,colname,member); \
    else x << "AND " << (u?"UPPER(":"") << colname << (u?")":"") \
           << "=" << (u?"UPPER(":"") \
           << "'" << db->Escape2(member) << "'" <<  (u?")":"") << " "; }
#define SQL_ID_FILTER_FILL(x,colname,member) \
     x << "AND " << colname << "=" << member << " "
#define SQL_ID_FILTER(x,colname,member) \
  if (member) SQL_ID_FILTER_FILL(x,colname,member)

#define MAKE_TIME_DEF(ROW,COL,DEF)  \
  (ptime(db->IsNotNull(ROW,COL) ? \
   time_from_string(db->GetFieldValue(ROW,COL)) : DEF))         
#define MAKE_TIME(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,ptime(not_a_date_time))         
#define MAKE_TIME_NEG(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,ptime(neg_infin))         
#define MAKE_TIME_POS(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,ptime(pos_infin))         
#define MAKE_DATE_DEF(ROW,COL,DEF)  \
 (date(db->IsNotNull(ROW,COL) ? from_string(db->GetFieldValue(ROW,COL)) : DEF))
#define MAKE_DATE(ROW,COL)  \
  MAKE_DATE_DEF(ROW,COL,date(not_a_date_time))
#define MAKE_DATE_NEG(ROW,COL) \
  MAKE_DATE_DEF(ROW,COL,date(neg_infin))         
#define MAKE_DATE_POS(ROW,COL) \
  MAKE_DATE_DEF(ROW,COL,date(pos_infin))         


#endif
