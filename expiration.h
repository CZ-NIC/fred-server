#include "mailer_manager.h"

#include "dbsql.h"
#include "action.h"



#define TYPE_EXDATE_AFTER  1 // notification after ExDate
#define TYPE_EXDATE_DNS   2 // remove domain from zone
#define TYPE_EXDATE_DEL   3 // definitly remove domain after protected interval

// for enum domain
#define TYPE_VALEXDATE_BEFORE  4 // notify 10 days before validity day
#define TYPE_VALEXDATE_AFTER  5 // notify after  validity day

#ifndef ID
#define ID unsigned  int
#endif

// expiration handling notification and remove domain
class Expiration {
  MailerManager *mm;
  DB *db;
  int type;

 public:
  Expiration( MailerManager *mailManager,   DB *dbs , int typ  ); // creator
  ~Expiration();
  int Process(); // make process wtite to object_notification_table

  int  NumDomains() { return numObjects; } ;
ID  GetObjectID( unsigned  int index ) 
 {
      if( (int ) index  < numObjects ) return  objectsID[index] ; 
      else return 0;
 }
char *GetDomainName( ID domainID );


  
private:
ID Save(ID objectID); // insert into table return id
int Message(ID objectID   ); // send EPP message vraci ID zpravy (-1) chyba (id)
bool Mailer(ID objectID , ID id ); // send emails  (id) z tabulky OSN pro vazbu
int numObjects;
ID *objectsID; // allocated selected domains


};
