#ifndef COMMON_HH_356FC41370474BDA82802F9A41BFD04F
#define COMMON_HH_356FC41370474BDA82802F9A41BFD04F

#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <math.h>
#include <memory>
#include <iomanip>
#include "src/bin/corba/Admin.hh"

#include "src/deprecated/libfred/registry.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"
#include "src/util/types/money.hh"

using namespace boost::posix_time;
using namespace boost::gregorian;

std::string formatDate(date d);
std::string formatTime(ptime p, bool date, bool _to_local = false);
std::string formatMoney(Money m);
ptime makeBoostTime(const ccReg::DateTimeType& t);
date makeBoostDate(const ccReg::DateType& t);
date makeBoostDate_throw(const ccReg::DateType& t);
ccReg::DateTimeType makeCorbaTime(ptime p, bool _to_local = false);
ccReg::DateType makeCorbaDate(const date& p);
time_period setPeriod(const ccReg::DateTimeInterval& _v);
time_period setPeriod(const ccReg::DateInterval& _v);
void clearPeriod(ccReg::DateTimeInterval& _v);

#define DUPSTR(s) CORBA::string_dup(s)
#define DUPSTRC(s) CORBA::string_dup(s.c_str())
#define DUPSTRFUN(f) DUPSTRC(f())
#define DUPSTRDATE(f) DUPSTRC(formatTime(f(),true,true))
#define DUPSTRDATE_NOLTCONVERT(f) DUPSTRC(formatTime(f(),true,false))
#define DUPSTRDATED(f) DUPSTRC(formatDate(f()))
#define DUPSTRDATESHORT(f) DUPSTRC(formatTime(f(),false,true))
#define COLHEAD(x,i,title,tp) \
  (*x)[i].name = DUPSTR(title); \
  (*x)[i].type = Registry::Table::tp 

#define FILTER_IMPL(FUNC,PTYPEGET,PTYPESET,MEMBER,MEMBERG, SETF) \
PTYPEGET FUNC() { return MEMBERG; } \
void FUNC(PTYPESET _v) { MEMBER = _v; SETF; }

#define FILTER_IMPL_L(FUNC,MEMBER,SETF) \
  FILTER_IMPL(FUNC,ccReg::TID,ccReg::TID,MEMBER,MEMBER, SETF)

#define FILTER_IMPL_S(FUNC,MEMBER,SETF) \
  FILTER_IMPL(FUNC,char *,const char *,MEMBER,DUPSTRC(MEMBER), SETF)


// #define DUPSTRDATE(f) DUPSTR(to_simple_string(f()).c_str())


#define C_STR(_val)  str_corbaout(_val).c_str()

/* detail library -> corba mappings macros */
#define MAKE_OID(_name, _id, _handle, _type) \
Registry::OID _name;                         \
_name.id     = _id;                          \
_name.handle = _handle;                      \
_name.type   = ccReg::_type;

#define CHANGED(method) (act->method() != prev->method()) || (act == prev)

#define ADD_NEW_HISTORY_RECORD(_field, _value)                                         \
unsigned i = detail->_field.length();                                                  \
detail->_field.length(i + 1);                                                          \
detail->_field[i].value  <<= _value;                                                   \
detail->_field[i].requestId = act->getRequestId();                                     \
detail->_field[i].from     = makeCorbaTime(act->getRequestStartTime(), true);          \
detail->_field[i].to       = (i > 0 ? makeCorbaTime(prev->getRequestStartTime(), true) \
                                    : makeCorbaTime(ptime(not_a_date_time)));    

#define MODIFY_LAST_HISTORY_RECORD(_field)                                            \
unsigned i = detail->_field.length();                                                 \
detail->_field[i - 1].requestId = act->getRequestId();                                \
detail->_field[i - 1].from     = makeCorbaTime(act->getRequestStartTime(), true);


#define MAP_HISTORY_VARIABLE(_field, _method, _conv)                                  \
if (CHANGED(_method)) {                                                               \
  ADD_NEW_HISTORY_RECORD(_field, _conv(act->_method()))                               \
}                                                                                     \
else {                                                                                \
  MODIFY_LAST_HISTORY_RECORD(_field)                                                  \
}

#define MAP_HISTORY_BOOL(_field, _method)                                             \
if (CHANGED(_method)) {                                                               \
  ADD_NEW_HISTORY_RECORD(_field, CORBA::Any::from_boolean(act->_method()))            \
}                                                                                     \
else {                                                                                \
  MODIFY_LAST_HISTORY_RECORD(_field)                                                  \
}

#define MAP_HISTORY_OID(_field, _get_id, _get_handle, _type)                          \
if (CHANGED(_get_id)) {                                                               \
  Registry::OID oid;                                                                  \
  oid.id = act->_get_id();                                                            \
  oid.handle = DUPSTRFUN(act->_get_handle);                                           \
  oid.type = _type;                                                                   \
  ADD_NEW_HISTORY_RECORD(_field, oid)                                                 \
}                                                                                     \
else {                                                                                \
  MODIFY_LAST_HISTORY_RECORD(_field)                                                  \
}

#define MAP_HISTORY_STRING(_field, _method)    MAP_HISTORY_VARIABLE(_field, _method, C_STR)
#define MAP_HISTORY_DATE(_field, _method)      MAP_HISTORY_VARIABLE(_field, _method, C_STR)
#define MAP_HISTORY_DATETIME(_field, _method)  MAP_HISTORY_VARIABLE(_field, _method, C_STR)


#endif /*UTILS_H_*/
