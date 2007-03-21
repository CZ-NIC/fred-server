#include <vector>
#include <string>
#include <sstream>


#include "notifier.h"
#include "log.h"

// mailer manager
#include "register/mailer.h"

#define MAX_SQLSTRING 512


EPPNotifier::EPPNotifier( MailerManager *mailManager, DB *dbs , ID regid ,ID objectid)
{

mm=mailManager;
db=dbs;
enum_action=db->GetEPPAction(); // id of the EPP operation
objectID=objectid;
registrarID=regid;

LOG( DEBUG_LOG ,"EPPNotifier:  object %d  enum_action %d regID %d " ,  objectID , enum_action ,  registrarID );

   
LOG( DEBUG_LOG ,"EPPNotifier: add default contacts");   

AddDefault();  // add default contacts
}

EPPNotifier::~EPPNotifier()
{
enum_action=0;
objectID=0;
registrarID=0;
}


bool EPPNotifier::Send()
{
unsigned int i , num;
short  type , mod;
ID cID;
Register::Mailer::Parameters params;
Register::Mailer::Handles handles;
Register::Mailer::Attachments attach;
std::stringstream emails;



// 4 parameters  type of the object name  ticket svTRID and  handle of  registrar
params["ticket"] = db->GetsvTRID();
params["registrar"] =  db->GetValueFromTable( "registrar" , "handle" , "id" , registrarID ); // handle of registrar
params["handle"]  =  db->GetValueFromTable( "object_registry" , "name" , "id" , objectID ); // name of the object
params["type"] = db->GetValueFromTable( "object_registry" , "type" , "id" ,  objectID ); // type 1 contact 2 nsset 3 domain

LOG( DEBUG_LOG ,"EPPNotifier: Send object %d  enum_action %d regID %d ticket %s" ,  objectID , enum_action ,  registrarID ,   db->GetsvTRID()  );

num = notify.size();
for( i = 0 ; i < num ; i ++ )
{
cID =  notify[i].contactID;
type =   notify[i].type;
mod =   notify[i].modify;

LOG( DEBUG_LOG ,"EPPNotifier: sendTo  %s %s contactID %d [%s]" ,  
           GetContactType( type ) , GetContactModify( mod)  , cID  , 
           db->GetValueFromTable( "object_registry" ,  "name" , "id" ,  cID ) );

LOG( DEBUG_LOG ,"EPPNotifier:  email %s notifyEmail %s " , 
               db->GetValueFromTable( "contact" ,  "email" , "id" ,  cID ) ,
               db->GetValueFromTable( "contact" ,  "notifyemail" , "id" ,  cID )  ); 

         if ( i > 0 ) emails << " , ";         // add more
         emails <<   db->GetValueFromTable( "contact" ,  "notifyemail" , "id" ,  cID ) ;


}
LOG( DEBUG_LOG , "EPPNotifier: TO: %s" , emails.str().c_str()   );
// Mailer manager send emailes 
mm->sendEmail( "" ,  emails.str()  , "",  getTemplate() ,params,handles,attach );

return true;
}



void EPPNotifier::AddContactID( ID contactID , short type , short mod )
{
NotifyST n;

LOG( DEBUG_LOG ,"EPPNotifier: AddContactID %d typ[%d] mod[%d]" , contactID , type , mod );
n.contactID=contactID;
n.type=type;
n.modify=mod;
notify.push_back( n );
}

// addall  tech contact of  nsset  linked with domain with  domainID 
void EPPNotifier::AddNSSetTechByDomain( ID domainID )
{
// SQL select by NSSETID
AddNSSetTech( db->GetNumericFromTable( "domain"  , "nsset" , "id"  , domainID)  );
}

void EPPNotifier::AddNSSetTech(  ID nssetID )
{
int i , num;
char sqlString[128];

sprintf( sqlString , "SELECT  contactid  from nsset_contact_map where nssetid=%d", nssetID );

if( db->ExecSelect( sqlString ) )
 {
  num = db->GetSelectRows();
  for( i = 0 ; i  < num ; i ++ ) AddContactID( atoi( db->GetFieldValue( i , 0 ) ) , TECH_CONTACT  , 0 ); 
  db->FreeSelect(); 
 }
         
}
     
void EPPNotifier::AddDomainAdmin(ID domainID  )
{
int i , num;
char sqlString[128];

sprintf( sqlString , "SELECT  contactid  from domain_contact_map where domainid=%d", domainID );

if( db->ExecSelect( sqlString ) )
 {
  num = db->GetSelectRows();
  for( i = 0 ; i  < num ; i ++ ) AddContactID( atoi( db->GetFieldValue( i , 0 ) ) , ADMIN_CONTACT  , 0 );
  db->FreeSelect();
 }

}

// add owner of domain
void EPPNotifier::AddDomainRegistrant( ID domainID)
{
ID contactID;

contactID = db->GetNumericFromTable( "domain"  , "registrant" , "id"  , domainID);
LOG( DEBUG_LOG ,"EPPNotifier: AddDomainRegistrant domainID %d contactID %d" ,  domainID , contactID);   

AddContactID(  contactID  , REGISTRANT_CONTACT , 0 );
}

