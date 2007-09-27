#ifndef __SQL_H__
#define __SQL_H__

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
