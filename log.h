#ifdef SYSLOG

#include<syslog.h>
  #define ERROR_LOG     LOG_ERR
  #define WARNING_LOG   LOG_WARNING
  #define SQL_LOG       LOG_DEBUG
  #define NOTICE_LOG    LOG_NOTICE
  #define EMERG_LOG     LOG_EMERG 
#else
  #define ERROR_LOG     stderr
  #define WARNING_LOG   stderr
  #define SQL_LOG       stdout
  #define NOTICE_LOG    stdout
  #define EMERG_LOG     stderr
#endif

#ifdef SYSLOG
  #define LOG syslog
#else
  #define LOG  fprintf
#endif

