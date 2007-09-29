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

class Logger 
{
 private:
  Logger() : level(DEBUG_LOG), facility(2) {} ///< ctor hidden
  Logger(Logger const&) {} ///< copy ctor hidden
  Logger& operator=(Logger const&) { return *this; } ///< assign op. hidden
  ~Logger() {} ///< dtor hidden
  int level; ///< minimal level of logging
  int facility; ///< syslog facility 
 public:
  static Logger &get()
  {
    static Logger l;
    return l;
  }
  void log(int prio, const char* t, ...) 
  {
    if (level < prio) return;
    va_list arglist;
    va_start(arglist, t);
    vsyslog(prio | ((LOG_FAC(LOG_LOCAL0)+facility) << 3), t, arglist);
    va_end(arglist);
  }
  void setFacility(int _facility)
  {
    facility = _facility;
  }
  void setLevel(int _level)
  {
    level = _level;
  }
};

#define LOG Logger::get().log
