#define KEY_dbname      1
#define KEY_user        2
#define KEY_pass        3  
#define KEY_host        4
#define KEY_port        5
#define KEY_log_mask    6
#define KEY_log_level   7
#define KEY_log_local   8

#define MAX_KEYS  8 

class Conf {
public:

Conf() { port[0]=0; host[0] = 0 ; user[0]=0 ; password[0] = 0 ; host[0]=0; log_mask = 0 ; conninfo[0] =0 ; }; // empty
~Conf(){}; // empty

bool ReadConfigFile(const char *filename );

char * GetDBhost(){ if( host[0] == 0 ) return NULL ; else  return host; };
char * GetDBname(){ if( dbname[0] == 0 ) return NULL ; else  return dbname; };
char * GetDBuser(){ if( user[0] == 0 ) return NULL ; else return user; };
char * GetDBpass(){ if( password[0] == 0 ) return NULL ; else return password; };
char * GetDBport(){ if(port[0] == 0 ) return NULL ; else  return port; };
char * GetDBconninfo(){ return  conninfo; } ;

/*
int GetLOGmask(){ return log_mask; );
int GetLOGLocal(){ return 1; };
*/
private:
char dbname[32];
char user[32];
char password[32];
char host[64];
char port[16];
char conninfo[256];
int log_mask;
};

