#ifndef __LOG_H__
#define __LOG_H__

#include <syslog.h>
#include <stdarg.h>

#define EMERG_LOG     LOG_EMERG
#define ALERT_LOG     LOG_ALERT
#define ERROR_LOG     LOG_ERR
#define CRIT_LOG      LOG_CRIT
#define WARNING_LOG   LOG_WARNING
#define SQL_LOG       LOG_DEBUG
#define DEBUG_LOG     LOG_DEBUG
#define NOTICE_LOG    LOG_NOTICE
#define INFO_LOG      LOG_INFO

class SysLogger
{
private:
  SysLogger() :
    level(DEBUG_LOG), facility(2)
  {
  } ///< ctor hidden
  SysLogger(
    SysLogger const&)
  {
  } ///< copy ctor hidden
  SysLogger& operator=(
    SysLogger const&)
  {
    return *this;
  } ///< assign op. hidden
  ~SysLogger()
  {
  } ///< dtor hidden
  int level; ///< minimal level of logging
  int facility; ///< syslog facility 
public:
  static SysLogger &get()
  {
    static SysLogger l;
    return l;
  }
  void log(
    int prio, const char* t, ...)
  {
    if (level < prio)
      return;
    va_list arglist;
    va_start(arglist, t);
    vsyslog(prio | ((LOG_FAC(LOG_LOCAL0)+facility) << 3), t, arglist);
    va_end(arglist);
  }
  void setFacility(
    int _facility)
  {
    facility = _facility;
  }
  void setLevel(
    int _level)
  {
    level = _level;
  }
};

/**
 * commented out old style logger
 *
 *  #define LOG SysLogger::get().log
 */

#endif

