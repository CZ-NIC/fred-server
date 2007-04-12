#include<syslog.h>
#include <libdaemon/dlog.h>

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

#define LOG daemon_log
