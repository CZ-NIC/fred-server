#ifndef __SQL_H__
#define __SQL_H__

#include <boost/date_time/gregorian/formatters.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>

#define SQL_TIME_FILTER(x,colname,member) \
  if (!member.begin().is_special()) \
     x << "AND " << colname << ">='"  \
       <<  to_iso_extended_string(member.begin()) \
       << "' "; \
  if (!member.end().is_special()) \
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
       << colname << " ILIKE TRANSLATE('" << member << "','*?','%_') "
#define SQL_WILDCARD_FILTER(x,colname,member) \
  if (!member.empty()) SQL_WILDCARD_FILTER_FILL(x,colname,member)
#define SQL_HANDLE_FILTER(x,colname,member) \
  SQL_WILDCARD_FILTER(x,colname,member)
#define SQL_ID_FILTER_FILL(x,colname,member) \
     x << "AND " << colname << "=" << member << " "
#define SQL_ID_FILTER(x,colname,member) \
  if (member) SQL_ID_FILTER_FILL(x,colname,member)

#endif
