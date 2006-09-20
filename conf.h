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
#define KEY_log_mask    6
#define KEY_log_level   7
#define KEY_log_local   8
#define KEY_nameservice 9

#define MAX_KEYS  9 

#include <string>

class Conf {
public:

Conf() { port[0]=0; host[0] = 0 ; user[0]=0 ; password[0] = 0 ; host[0]=0; log_level = 0 ; conninfo[0] =0 ; log_local=0; }; // empty
~Conf(){}; // empty

bool  ReadConfigFile(const char *filename )
{
#ifdef XMLCONF
return  ReadConfigFileXML( filename );
#else
return ReadConfigFileTXT( filename );
#endif
};

#ifdef XMLCONF
void get_element_names(xmlNode * a_node);
bool  ReadConfigFileXML(const char *filename );
#endif
bool  ReadConfigFileTXT(const char *filename );



char * GetDBhost(){ if( host[0] == 0 ) return NULL ; else  return host; };
char * GetDBname(){ if( dbname[0] == 0 ) return NULL ; else  return dbname; };
char * GetDBuser(){ if( user[0] == 0 ) return NULL ; else return user; };
char * GetDBpass(){ if( password[0] == 0 ) return NULL ; else return password; };
char * GetDBport(){ if(port[0] == 0 ) return NULL ; else  return port; };
char * GetDBconninfo();
const char *GetNameService(){ return nameServiceIOR.c_str(); } 


int GetSYSLOGlevel(){ return log_level; };
int GetSYSLOGlocal(){ return log_local; };
int GetSYSLOGfacility()
{ 
switch( log_local )
      {
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

};        


int GetLocal(char *value);
int GetLevel(char *value);

private:
char dbname[32];
char user[32];
char password[32];
char host[64];
char port[16];
char conninfo[256];
int log_level;
int log_local;
std::string nameServiceIOR;
};

