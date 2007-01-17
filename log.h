#ifdef SYSLOG

#include<syslog.h>
#define EMERG_LOG     LOG_EMERG
#define ALERT_LOG     LOG_ALERT
#define ERROR_LOG     LOG_ERR
#define CRIT_LOG      LOG_CRIT
#define WARNING_LOG   LOG_WARNING
#define SQL_LOG       LOG_DEBUG
#define DEBUG_LOG     LOG_DEBUG
#define NOTICE_LOG    LOG_NOTICE
#define INFO_LOG      LOG_INFO
#define EMERG_LOG     LOG_EMERG 
#else
#define EMERG_LOG       0       /* system is unusable */
#define ALERT_LOG       1       /* action must be taken immediately */
#define CRIT_LOG        2       /* critical conditions */
#define ERROR_LOG       3       /* error conditions */
#define WARNING_LOG     4       /* warning conditions */
#define NOTICE_LOG      5       /* normal but significant condition */
#define INFO_LOG        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */
#define SQL_LOG      LOG_DEBUG
#define DEBUG_LOG LOG_DEBUG
#endif

#ifdef SYSLOG
  #define LOG syslog
#else
  void logprintf(int level ,  char *fmt, ... );
  #define LOG  logprintf
#endif

