#ifndef UTILS_H_
#define UTILS_H_

#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <math.h>
#include <memory>
#include <iomanip>
#include <corba/ccReg.hh>

#include "register/register.h"
#include "register/invoice.h"

using namespace boost::posix_time;
using namespace boost::gregorian;

std::string formatDate(date d);
std::string formatTime(ptime p, bool date);
std::string formatMoney(Register::Invoicing::Money m);
ptime makeBoostTime(const ccReg::DateTimeType& t);
date makeBoostDate(const ccReg::DateType& t);
ccReg::DateTimeType makeCorbaTime(ptime p);
ccReg::DateType makeCorbaDate(date p);
time_period setPeriod(const ccReg::DateTimeInterval& _v);
time_period setPeriod(const ccReg::DateInterval& _v);
void clearPeriod(ccReg::DateTimeInterval& _v);

#define DUPSTR(s) CORBA::string_dup(s)
#define DUPSTRC(s) CORBA::string_dup(s.c_str())
#define DUPSTRFUN(f) DUPSTRC(f())
#define DUPSTRDATE(f) DUPSTRC(formatTime(f(),true))
#define DUPSTRDATED(f) DUPSTRC(formatDate(f()))
#define DUPSTRDATESHORT(f) DUPSTRC(formatTime(f(),false))
#define COLHEAD(x,i,title,tp) \
  (*x)[i].name = DUPSTR(title); \
  (*x)[i].type = ccReg::Table::tp 

#define FILTER_IMPL(FUNC,PTYPEGET,PTYPESET,MEMBER,MEMBERG, SETF) \
PTYPEGET FUNC() { return MEMBERG; } \
void FUNC(PTYPESET _v) { MEMBER = _v; SETF; }

#define FILTER_IMPL_L(FUNC,MEMBER,SETF) \
  FILTER_IMPL(FUNC,ccReg::TID,ccReg::TID,MEMBER,MEMBER, SETF)

#define FILTER_IMPL_S(FUNC,MEMBER,SETF) \
  FILTER_IMPL(FUNC,char *,const char *,MEMBER,DUPSTRC(MEMBER), SETF)


// #define DUPSTRDATE(f) DUPSTR(to_simple_string(f()).c_str())

#endif /*UTILS_H_*/
