#include <stdio.h>
#include <string.h>
//#include <sstream>
#include <ccReg.hh>

#include <nameservice.h>


#include "expiration.h"
#include "log.h"
#include "conf.h"

#include "util.h"

// mailer manager
#include "mailer_manager.h"
#define MAX_SQLSTRING 512


Expiration::Expiration( MailerManager *mailManager, DB *dbs , int typ )
{
mm=mailManager;
db=dbs;
type = typ; // notify expiration type
numObjects=0; // number of selected object
LOG( DEBUG_LOG ,"Expiration: type %d " ,  type );

}

Expiration::~Expiration()
{
type = 0;
delete objectsID; // free 


}



// retrun number of notified objects
int Expiration::Process()
{
char sqlString[MAX_SQLSTRING];
int i , num , add ;
int mesgID;
ID historyID;
ID id;


switch( type )
{
case  TYPE_EXDATE_DEL: //  expiration domain after 45 days END of the protected period domain will be deleted after notification
      strncpy( sqlString , "SELECT id from domain where current_date  >= ExDate +  interval '45 days' ; ", sizeof(sqlString)-1 );
      break;
case  TYPE_EXDATE_DNS: // expiration domain after 30 days domains not generated to zone
      strncpy( sqlString , "SELECT id from domain where current_date  >= ExDate +  interval '30 days' ; ", sizeof(sqlString)-1 );
      break;      
case  TYPE_EXDATE_AFTER: // expirations domain 
      strncpy( sqlString , "SELECT id from domain where current_date  >= ExDate ;", sizeof(sqlString)-1 );
      break;
case  TYPE_EXDATE_BEFORE: // domain that have 30 days before expiration
      strncpy( sqlString , "SELECT id from domain where current_date  >= ExDate - interval '30 days' ;", sizeof(sqlString)-1 );
      break;
case  TYPE_VALEXDATE_AFTER: // enum domain after validity
      strncpy( sqlString , "SELECT domainid from enumval where current_date >= exdate;", sizeof(sqlString)-1 );
      break;
case  TYPE_VALEXDATE_BEFORE: // enum domains that have 30 days before end of validate day 
      strncpy( sqlString , "SELECT domainid from enumval where current_date >= exdate - interval '30 days' ;", sizeof(sqlString)-1 );
      break;
default:
    sqlString[0] = 0;
}

if( db->ExecSelect( sqlString ) )
 {
  num = db->GetSelectRows();
  numObjects=num; // number of domains
  objectsID = new ID[num];
  for( i = 0 ; i  < num ; i ++ )
     {
        objectsID[i] = atoi( db->GetFieldValue( i , 0 ) ); // ID of the objects 
        LOG( DEBUG_LOG , "  objectsID %d " ,  objectsID[i]  );        
     }
  db->FreeSelect();


  for( i = 0  , add =0 ; i  < num ; i ++ )
  {

   // get actual histyory ID
   historyID = db->GetNumericFromTable( "object_registry" , "historyID" , "id" ,  objectsID[i]);

   sprintf( sqlString , "SELECT id from object_status_notifications where notify=%d AND objectid=%d AND  historyid=%d;" , type , objectsID[i] , historyID );
   if( db->ExecSelect( sqlString ) )
     {
         if( db->GetSelectRows() == 1 ) 
           {
              id =  atoi( db->GetFieldValue( 0 , 0 ) );
           }
         else id=0;
        db->FreeSelect();
     }


    if( id == 0  )  // if not in the table 
      { 
        if( ( id= Save( objectsID[i] ) )  > 0 ) 
          {
            // make EPP message
           mesgID = Message( objectsID[i]  );     

           // update table and write  historyID  and messageID
           db->UPDATE("object_status_notifications");
           db->SET( "historyID" , ( int ) historyID );
           if(  mesgID >0 )   db->SET( "messageID" , (int ) mesgID );
           db->WHERE( "ID" , id );

           if( db->EXEC() ) 
           {
               if(  Mailer(  objectsID[i] , id ) ) add++;
               else  { add=-4; break ; } //  mailer error
               
           }
           else  { add=-3 ; break ; } //  update error


          }
        else  { add=-1; break;}  // other error
      }

  }


   return add;

 }
else return -2;

}

ID Expiration::Save(ID objectID) // save objectID
{
ID id;

id= db->GetSequenceID( "object_status_notifications" );

db->INSERT( "object_status_notifications" );
db->INTO( "id" );
db->INTO( "objectID" );
db->INTO( "crdate");
db->INTO( "notify");
db->VALUE( (int ) id );
db->VALUE( (int ) objectID );
db->VALUENOW();
db->VALUE( (int ) type );

if( db->EXEC() ) return id;
else return 0;
}

char * Expiration::GetDomainName( ID domainID )
{
return db->GetValueFromTable( "object_registry"  , "name" , "id" ,  (int ) domainID );
}

// Make EPP message
int Expiration::Message(ID objectID )
{
char xmlString[1024];
char schema_domain[] =  " xmlns:domain=\"http://www.nic.cz/xml/epp/domain-1.3\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.nic.cz/xml/epp/domain-1.3 domain-1.3.xsd\" ";
char schema_enumval[] =  " xmlns:enumval=\"http://www.nic.cz/xml/epp/enumval-1.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.nic.cz/xml/epp/enumval-1.1 enumval-1.1.xsd\" ";
char exDateStr[32];
char nameStr[64];
int regID;
ID id;

// client the registrator of the object 
regID = db->GetNumericFromTable( "object" , "ClID" , "id" , (int )  objectID );
strncpy( nameStr ,  db->GetValueFromTable( "object_registry"  , "name" , "id" ,  (int ) objectID ), sizeof(nameStr)-1);

switch( type )
{
case TYPE_EXDATE_BEFORE:
      strncpy( exDateStr, db->GetValueFromTable( "domain" , "ExDate" , "ID" , (int ) objectID ), sizeof(exDateStr)-1 );
      snprintf(xmlString, sizeof(xmlString), "<domain:impendingExpData %s ><domain:name>%s</domain:name><domain:exDate>%s</domain:exDate></domain:impendingExpData>", schema_domain, nameStr, exDateStr );
      break;
case TYPE_EXDATE_AFTER:
      strncpy( exDateStr, db->GetValueFromTable( "domain" , "ExDate" , "ID" , (int ) objectID ), sizeof(exDateStr)-1 );
      snprintf(xmlString, sizeof(xmlString), "<domain:expData %s ><domain:name>%s</domain:name><domain:exDate>%s</domain:exDate></domain:expData>", schema_domain, nameStr, exDateStr );
      break;
case TYPE_EXDATE_DNS:
      strncpy( exDateStr, db->GetValueFromTable( "domain" , "ExDate" , "ID" , (int ) objectID ), sizeof(exDateStr)-1 );
      snprintf(xmlString, sizeof(xmlString), "<domain:dnsOutageData  %s ><domain:name>%s</domain:name><domain:exDate>%s</domain:exDate></domain:dnsOutageData>", schema_domain, nameStr, exDateStr );
      break;
case TYPE_EXDATE_DEL:
      strncpy( exDateStr, db->GetValueFromTable( "domain" , "ExDate" , "ID" , (int ) objectID ), sizeof(exDateStr)-1 );
      snprintf(xmlString, sizeof(xmlString), "<domain:delData  %s ><domain:name>%s</domain:name><domain:exDate>%s</domain:exDate></domain:delData>" ,
      schema_domain,   nameStr , exDateStr  );
      break;
case  TYPE_VALEXDATE_BEFORE:
      strncpy( exDateStr, db->GetValueFromTable( "enumval"  , "ExDate" , "domainID" , (int ) objectID ), sizeof(exDateStr)-1 );
      snprintf(xmlString, sizeof(xmlString), "<enumval:impendingValExpData %s ><enumval:name>%s</enumval:name><enumval:valExDate>%s</enumval:valExDate></enumval:impendingValExpData>", schema_enumval, nameStr, exDateStr );
      break;
case  TYPE_VALEXDATE_AFTER:
      strncpy( exDateStr, db->GetValueFromTable( "enumval"  , "ExDate" , "domainID" , (int ) objectID ), sizeof(exDateStr)-1 );
      snprintf(xmlString, sizeof(xmlString), "<enumval:valExpData %s ><enumval:name>%s</enumval:name><enumval:valExDate>%s</enumval:valExDate></enumval:valExpData>", schema_enumval, nameStr, exDateStr );
      break;

}

LOG(DEBUG_LOG , "Expiration: xmlsString: %s" , xmlString  );
//  insert message into table message default ExDate + 1 month

id= db->GetSequenceID( "message" );

db->INSERT( "message" );
db->INTO( "id" );
db->INTO( "Clid" );
db->INTO( "crdate");
db->INTO( "exdate");
db->INTO( "seen");
db->INTO( "message" );
db->VALUE( (int) id );
db->VALUE( regID ); // id  registratora
db->VALUENOW();
db->VALUEPERIOD( 1 ); // acctual date plus one month
db->VALUE(  false );
db->VALUE( xmlString );
if( db->EXEC() ) return id;
else return -1; //error

}


bool Expiration::Mailer(ID objectID , ID id )
{
// this type of event is not announced
if (type == TYPE_EXDATE_BEFORE) return true;

char sqlString[1024];
char paramName[32];
char checkdateStr[16];
char exregdateStr[16];
char dnsdateStr[16];
int regID  ,  registrantID ,nssetID;
ID mailID , tech_mailID;
ID mail_type , tech_mail_type;
unsigned int i , num=0;
time_t t;
t = time(NULL);
Register::Mailer::Parameters params;
Register::Mailer::Handles handles;
Register::Mailer::Attachments attach;
std::stringstream emails;
std::stringstream tech_emails;

// actual local date in the format   YYYY-MM-DD
get_rfc3339_timestamp( t , checkdateStr , true );
params["checkdate"] = checkdateStr;
params["domain"]  =  db->GetValueFromTable( "object_registry" , "name" , "id" , objectID );

// domain owner 
registrantID = db->GetNumericFromTable( "domain" , "registrant" , "id" , objectID );
params["owner"] = db->GetValueFromTable( "object_registry" , "name" , "id" , registrantID );
// notify e-mail if the owner
emails <<  db->GetValueFromTable( "contact"  , "email" , "id" ,  registrantID );    



if( type == TYPE_VALEXDATE_AFTER || type == TYPE_EXDATE_DNS || type == TYPE_EXDATE_DEL )
{
nssetID =  db->GetNumericFromTable( "domain" , "nsset" ,  "id" , objectID );

if( nssetID > 0 ) // test if exist nsset for this domain
{
params["nsset"]  = db->GetValueFromTable( "object_registry" , "name" , "id" ,  nssetID );

// Tech emails 
sprintf( sqlString , "SELECT email FROM contact LEFT  JOIN nsset_contact_map  ON nsset_contact_map.nssetid=%d WHERE contact.id=nsset_contact_map.contactid;" , nssetID);

if( db->ExecSelect( sqlString ) )
 {
  num = db->GetSelectRows();
  for( i = 0 ; i  < num ; i ++ )
    {
       if( db->IsNotNull( i ,  0 ) )
          {
            std::string newEmail = db->GetFieldValue( i , 0 ) ;
            if (!newEmail.empty() && tech_emails.str().find(newEmail) == std::string::npos)
              {
                if(  !tech_emails.str().empty()  )  tech_emails << " , " ; // insert comma 
                tech_emails <<  newEmail;
              }
          }
     }
  db->FreeSelect();
  }
}
else params["nsset"]  = "" ; // empty nsset 


}



// registrar clietn of the domain
regID  = db->GetNumericFromTable( "object" , "clID" ,  "id" , objectID );
params["registrar"] = db->GetRegistrarHandle( regID );

// validity date for enum domain
if( type ==  TYPE_VALEXDATE_BEFORE )
{
params["valdate"] = db->GetDomainValExDate(  objectID );
}

if( type ==  TYPE_EXDATE_AFTER  )
{
params["exdate" ] = db->GetDomainExDate( objectID );
get_rfc3339_timestamp( t + SECSPERDAY *  30 , dnsdateStr , true ); // date aftre this is not generate this domain into zone
params["dnsdate"] = dnsdateStr;
get_rfc3339_timestamp( t + SECSPERDAY *  45 , exregdateStr , true ); // date of the delate domain
params["exregdate"] = exregdateStr;
}

if( type ==  TYPE_EXDATE_DNS )
{
//  delete domain after  15 days 
get_rfc3339_timestamp( t + SECSPERDAY *  15 ,  exregdateStr , true );
params["exregdate"] = exregdateStr;
}




if( type ==  TYPE_EXDATE_DEL )
{
// delete day remove today
get_rfc3339_timestamp( t + SECSPERDAY  ,  exregdateStr , true );
params["exregdate"] = exregdateStr;
}




// list  admin-c with  notify e-mail
sprintf(  sqlString , "SELECT  object_registry.name , contact.email  FROM object_registry JOIN domain_contact_map ON domain_contact_map.contactID=object_registry.id  JOIN contact ON object_registry.id=contact.id WHERe domain_contact_map.domainid=%d and contact.id = object_registry.id;" , objectID );

if( db->ExecSelect( sqlString ) )
 {
  num = db->GetSelectRows();
  for( i = 0 ; i  < num ; i ++ ) 
    {
     sprintf( paramName , "administrators.%d" , i+ 1 );
       params[paramName]  = db->GetFieldValue( i , 0 );
       if( db->IsNotNull( i ,  1 ) )
          {
            std::string newEmail = db->GetFieldValue( i , 1 ) ;
            if (!newEmail.empty() && emails.str().find(newEmail) == std::string::npos)
              {
                if( !emails.str().empty()  )  emails << " , " ; // insert comma
                emails << newEmail ;
              }
          }
     }
  db->FreeSelect(); 
  }

// if is selected e-mails
if( !emails.str().empty()  )
{
LOG( DEBUG_LOG , "Expiration mail TO:  %s type %d" ,  emails.str().c_str()   , type );

// Mailer manager send emailes 
switch( type )
{
case  TYPE_EXDATE_DNS:
      if (!emails.str().empty()) {
        mail_type = db->GetNumericFromTable( "mail_type" , "id" , "name" , "expiration_dns_owner"  );
        mailID =   mm->sendEmail( "" , emails.str() , "",  "expiration_dns_owner",  params , handles , attach );
      }
      if (!tech_emails.str().empty()) {
        tech_mail_type =  db->GetNumericFromTable( "mail_type" , "id" , "name" ,  "expiration_dns_tech" ); 
        tech_mailID = mm->sendEmail( "" , tech_emails.str() , "",  "expiration_dns_tech" ,  params , handles , attach );
      } 
      break; 
case  TYPE_EXDATE_DEL:
      if (!emails.str().empty()) {
        mail_type = db->GetNumericFromTable( "mail_type" , "id" , "name" , "expiration_register_owner" );
        mailID =   mm->sendEmail( "" , emails.str() , "",  "expiration_register_owner",   params , handles , attach );
      } 
      if (!tech_emails.str().empty()) {
        tech_mail_type =  db->GetNumericFromTable( "mail_type" , "id" , "name" ,   "expiration_register_tech" );
        tech_mailID = mm->sendEmail( "" , tech_emails.str() , "",  "expiration_register_tech" ,  params , handles , attach );
      } 
      break; 

case  TYPE_EXDATE_AFTER:
      if (!emails.str().empty()) {
        mail_type = db->GetNumericFromTable( "mail_type" , "id" , "name" , "expiration_notify" ); 
        mailID =   mm->sendEmail( "" , emails.str() , "", "expiration_notify",   params , handles , attach );
      } 
      break;
case  TYPE_VALEXDATE_BEFORE:
      mail_type = db->GetNumericFromTable( "mail_type" , "id" , "name" ,  "expiration_validation_before" );
      if (!emails.str().empty()) {
        mailID =   mm->sendEmail( "" , emails.str() ,  "", "expiration_validation_before",   params , handles , attach );
      } 
      break;
case  TYPE_VALEXDATE_AFTER:
      if (!emails.str().empty()) {
        params["exdate"] = db->GetDomainValExDate(  objectID );
        mail_type = db->GetNumericFromTable( "mail_type" , "id" , "name" ,   "expiration_validation" );
        mailID =   mm->sendEmail( "" , emails.str() ,  "", "expiration_validation",   params , handles , attach );
      } 
      if (!tech_emails.str().empty()) {
        tech_mail_type =  db->GetNumericFromTable( "mail_type" , "id" , "name" ,  "expiration_dns_tech" ); 
        tech_mailID = mm->sendEmail( "" , tech_emails.str() , "",  "expiration_dns_tech" ,  params , handles , attach );
      } 
      break;
     
}




if( type == TYPE_VALEXDATE_AFTER || type == TYPE_EXDATE_DNS || type == TYPE_EXDATE_DEL )
{ 
if( tech_mailID  > 0 )
  {
   db->INSERT( "object_status_notifications_mail_map" );
   db->INTO( "id" );
   db->INTO( "mail_type" );
   db->INTO( "mailID" );
   db->VALUE( (int)  id );
   db->VALUE(  (int) tech_mail_type); // mail templates for  tech-c for  nssetu at domain 
   db->VALUE( (int) tech_mailID );
   if( !db->EXEC() ) return false;
  }

}

if( mailID > 0  )
{
db->INSERT( "object_status_notifications_mail_map" );
db->INTO( "id" );
db->INTO( "mail_type" );
db->INTO( "mailID" );
db->VALUE( (int)  id );
db->VALUE( (int)  mail_type );
db->VALUE( (int) mailID );
if( !db->EXEC() ) return false;
}

return true; // if is OK
}
else return true; // not generated eny e-mails

}


 
// default config file
#ifndef CONFIG_FILE
#define CONFIG_FILE  "/etc/fred/server.conf"
#endif



int main(int argc , char *argv[] )
{
CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
Expiration *exp;
CORBA::Long loginID;
ccReg::Response *ret;
DB db;
int  type=0;
int i ,  num , tr=0 ;
Conf config; // READ CONFIG  file
NameService *ns;



    if (!config.ReadConfigFile(CONFIG_FILE) )
    {
      LOG(  ALERT_LOG ,  "Cannot read config file %s" , CONFIG_FILE);
      exit(-1);
    }


// prepare NameService object witch config
ns = new  NameService(orb , config.GetNameServiceHost() );
MailerManager mm(ns);


// EPP interface
ccReg::EPP_var EPP = ccReg::EPP::_narrow( ns->resolve("EPP")   );

 // stat syslog
#ifdef SYSLOG
    setlogmask ( LOG_UPTO(  config.GetSYSLOGlevel()  )   );
    openlog ( "expiration" , LOG_CONS | LOG_PID | LOG_NDELAY,  config.GetSYSLOGfacility() );
#endif

printf("connect DB string [%s]\n" , config.GetDBconninfo() );
// usage
if( argc == 1 )printf("expiration handling\nusage: %s --valexdate-before OR --valexdate-after OR  --exdate-before OR --exdate-after OR  --exdate-dns OR --exdate-del\n"  , argv[0]   );


if( argc == 2 )
{


            if( strcmp(  argv[1]  , "--exdate-before" )  == 0 ) 
              {
                type = TYPE_EXDATE_BEFORE ;
              }

            if( strcmp(  argv[1]  , "--exdate-after" )  == 0 ) 
              {
                type = TYPE_EXDATE_AFTER ;
              }


            if( strcmp(  argv[1]  , "--exdate-dns" )  == 0 ) 
              {
                type = TYPE_EXDATE_DNS ;
              }


            if( strcmp(  argv[1]  , "--exdate-del" )  == 0 ) 
              {
                type = TYPE_EXDATE_DEL ;
              }


            if( strcmp(  argv[1]  , "--valexdate-after" )  == 0 ) 
              {
                type = TYPE_VALEXDATE_AFTER ;
              }

            if( strcmp(  argv[1]  , "--valexdate-before" )  == 0 ) 
              {
                type = TYPE_VALEXDATE_BEFORE ;
              }


    


  if( type > 0  ) 
    {
         if( db.OpenDatabase(  config.GetDBconninfo()   ) )
          {

                LOG( LOG_DEBUG , "successfully  connect to DATABASE %s"  ,  config.GetDBconninfo() );
 
               if( db.BeginTransaction() )
                {
           
                      exp = new Expiration( &mm , &db , type );
                      num = exp->Process();
                      LOG( LOG_DEBUG , "total process objects %d" , num );
                      if( num > 0 )  tr = CMD_OK;
                      else tr=0;
                      db.QuitTransaction( tr );

                 if( type ==  TYPE_EXDATE_DEL && exp->NumDomains()) // REMOVE domains after protected interval ExDate+45 days 
                 {
                     // DELETE via EPP interface 
                     // get login  parametrs of system registrar from database


                     if( db.ExecSelect( "SELECT handle  , password , cert from registrar , registraracl where registrar.system=\'t\' and  registraracl.registrarid=registrar.id;" ) )
                       {
                          if( db.GetSelectRows() > 0 )
                            {
                                ret =  EPP->ClientLogin(   db.GetFieldValue( 0 , 0 )   ,  db.GetFieldValue( 0 , 1 )   , "" ,  "expiration-login" , "" , loginID ,  db.GetFieldValue( 0 , 2 )  ,  ccReg::EN  );
                                LOG( LOG_NOTICE ,  "Login err code %d msg %s svtrID %s"  , ret->code , ( char *)   ret->msg ,  ( char *) ret->svTRID  );
                            }
                         db.FreeSelect();
                       }


                  if( loginID )
                  {
                     // delete all selected domains via EPP 
                     for( i = 0 ; i <  exp->NumDomains() ; i ++ )
                     {
                         ret =  EPP->DomainDelete( exp->GetDomainName( exp->GetObjectID( i )   ) , loginID , "expiration-handling"  , "<XML>expiration-handling</XML>");
                         LOG( LOG_NOTICE ,  "Delete domain err code %d msg %s svtrID %s"  , ret->code ,  ( char *) ret->msg , ( char *)  ret->svTRID  );
                     }
                   //   logout  from EPP interface
                    ret =  EPP->ClientLogout( loginID , "expiration-logout" , "");
                    LOG( LOG_NOTICE ,  "Logout err code %d msg %s svtrID %s" ,ret->code , ( char *)   ret->msg ,  ( char *)  ret->svTRID  );

                   }
                     
                       
                 }


  
               }


          // disconnect
           db.Disconnect();


           
        }
 
    }

}




#ifdef SYSLOG
  closelog ();
#endif



}
