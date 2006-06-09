#ifdef SYSLOG

#include<syslog.h>
  #define ERROR_LOG     LOG_ERR
  #define WARNING_LOG   LOG_WARNING
  #define SQL_LOG       LOG_DEBUG
  #define NOTICE_LOG    LOG_NOTICE
#else
  #define ERROR_LOG     stderr
  #define WARNING_LOG   stderr
  #define SQL_LOG       stdout
  #define NOTICE_LOG    stdout
#endif

#ifdef SYSLOG
  #define LOG syslog
#else
  #define LOG  fprintf
#endif

