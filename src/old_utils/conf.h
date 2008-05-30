#ifndef __CONF_H__
#define __CONF_H__

#ifdef XMLCONF
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

#ifndef LOG_LOCAL0
#include <syslog.h>
#endif

#define KEY_dbname      1
#define KEY_user        2
#define KEY_pass        3  
#define KEY_host        4
#define KEY_port        5
#define KEY_timeout     6
#define KEY_log_mask    7
#define KEY_log_level   8
#define KEY_log_local   9
#define KEY_nameservice 10
#define KEY_session_max  11
#define KEY_session_wait  12
#define KEY_docgen_path  13
#define KEY_fileclient_path  14
#define KEY_ebanka_url  15
#define KEY_docgen_template_path  16
#define KEY_nsset_level  17
#define KEY_restricted_handles  18
#define KEY_disable_epp_notifier  19
#define KEY_session_registrar_max 20
#define KEY_lock_epp_commands  21
#define MAX_KEYS  21

#include <string>

class Conf
{
public:

  Conf()
  {
    // default  values 
    port[0]=0;
    timeout[0] = 0;
    host[0] = 0;
    user[0]=0;
    password[0] = 0;
    host[0]=0;
    log_level = 0;
    conninfo[0] =0;
    log_local=0;
    nssetLevel="3";
    restrictedHandles=1;
    disableEPPNotifier=0;
    session_max=20;
    session_wait=300;
    session_registrar_max=5;
    lockEPPCommands=1;
  }
  ~Conf()
  {
  }
  ; // empty

  bool ReadConfigFile(
    const char *filename)
  {
#ifdef XMLCONF
    return ReadConfigFileXML( filename );
#else
    return ReadConfigFileTXT(filename);
#endif
  }
  ;

#ifdef XMLCONF
  void get_element_names(xmlNode * a_node);
  bool ReadConfigFileXML(const char *filename );
#endif
  bool ReadConfigFileTXT(
    const char *filename);

  int GetSessionMax()
  {
    return session_max;
  }
  ;
  int GetSessionRegistrarMax()
  {
    return session_registrar_max;
  }
  ;
  int GetSessionWait()
  {
    return session_wait;
  }
  ;

  const char *GetDBhost()
  {
    if (host[0] == 0)
      return NULL;
    else
      return host;
  }
  ;
  const char *GetDBname()
  {
    if (dbname[0] == 0)
      return NULL;
    else
      return dbname;
  }
  ;
  const char *GetDBuser()
  {
    if (user[0] == 0)
      return NULL;
    else
      return user;
  }
  ;
  const char *GetDBpass()
  {
    if (password[0] == 0)
      return NULL;
    else
      return password;
  }
  ;
  const char *GetDBport()
  {
    if (port[0] == 0)
      return NULL;
    else
      return port;
  }
  ;
  const char *GetDBtimeout()
  {
    if (timeout[0] == 0)
      return NULL;
    else
      return timeout;
  }
  ;
  const char *GetDBconninfo();
  const char *GetNameServiceHost()
  {
    return nameService.c_str();
  }
  const char *GetDocGenPath()
  {
    return docGenPath.c_str();
  }
  const char *GetDocGenTemplatePath()
  {
    return docGenTemplatePath.c_str();
  }
  const char *GetFileClientPath()
  {
    return fileClientPath.c_str();
  }
  const char *GetEBankaURL()
  {
    return eBankaURL.c_str();
  }
  const char *GetNSSetLevel()
  {
    return nssetLevel.c_str();
  }
  unsigned GetRestrictedHandles()
  {
    return restrictedHandles;
  }
  unsigned GetDisableEPPNotifier()
  {
    return disableEPPNotifier;
  }

  int GetSYSLOGlevel()
  {
    return log_level;
  }
  ;
  int GetSYSLOGlocal()
  {
    return log_local;
  }
  ;
  int GetSYSLOGfacility()
  {
    switch (log_local) {
      case 0:
        return LOG_LOCAL0;
      case 1:
        return LOG_LOCAL1;
      case 2:
        return LOG_LOCAL2;
      case 3:
        return LOG_LOCAL3;
      case 4:
        return LOG_LOCAL4;
      case 5:
        return LOG_LOCAL5;
      case 6:
        return LOG_LOCAL6;
      case 7:
        return LOG_LOCAL7;
      default:
        return LOG_LOCAL0;
    }

  }
  ;
  unsigned GetLockEPPCommands()
  {
    return lockEPPCommands;
  }
  int GetLocal(
    char *value);
  int GetLevel(
    char *value);

private:
  char dbname[32];
  char user[32];
  char password[32];
  char host[64];
  char port[16];
  char timeout[10];
  char conninfo[256];
  int log_level;
  int log_local;
  int session_max;
  int session_wait;
  int session_registrar_max;
  std::string nameService;
  std::string docGenPath;
  std::string docGenTemplatePath;
  std::string fileClientPath;
  std::string eBankaURL;
  std::string nssetLevel;
  unsigned restrictedHandles;
  unsigned disableEPPNotifier;
  unsigned lockEPPCommands;
};

#endif
