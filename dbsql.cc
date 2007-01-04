#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sstream>

#include "dbsql.h"

#include "util.h"
#include "action.h"

#include "status.h"

#include "log.h"


// constructor 
DB::DB()
{ 
// nastav memory buffry
svrTRID = NULL;
memHandle=NULL;  
actionID = 0 ;
loginID = 0;
}

// uvolni memory buffry
DB::~DB()
{
if( svrTRID ) 
  {
     LOG( NOTICE_LOG , "delete svrTRID"); 
     delete[] svrTRID; 
   }

if( memHandle ) 
  { 
     LOG( NOTICE_LOG , "delete memHandle");
     delete[] memHandle;
  }

   
}

long DB::GetRegistrarCredit(int regID , int zoneID )
{
long price=0;
char sqlString[128];

sprintf(  sqlString ,  "SELECT sum( credit) FROM invoice  WHERE  registrarid=%d and zone=%d; " , regID , zoneID );

if( ExecSelect( sqlString ) )
 {
 
  if(  GetSelectRows()  == 1 )
     {
       price =  (long) ( 100.0 *  atof( GetFieldValue( 0  , 0 ) )  );
     }

   FreeSelect();
  }

return price;
}

bool  DB::TestRegistrarACL( int  regID , const char *  pass , const char * cert )
{
char sqlString[512];
bool ret =false;

sprintf(  sqlString ,  "SELECT  registrarid FROM registraracl WHERE registrarid=%d and cert=\'%s\' and password=\'%s\'; " ,   regID ,  cert ,  pass );
if( ExecSelect( sqlString ) )
 {

   if(  GetSelectRows()  > 0 ) ret = true;

   FreeSelect();
  }

return ret;
}

// vraci castku za operaci 
long DB::GetPrice(   int operation ,  int zone , int period  )
{
char sqlString[256];
long p ,  price=0; // cena
int per; // cena za periodu


if( period > 0 ) sprintf(  sqlString ,  "SELECT price , period FROM price_list WHERE valid_from < 'now()'  and ( valid_to is NULL or valid_to > 'now()' )  and operation=%d and zone=%d;" , operation , zone );
else  sprintf(  sqlString ,  "SELECT price  FROM price_list WHERE valid_from < 'now()'  and ( valid_to is NULL or valid_to > 'now()' )  AND  operation=%d and zone=%d;" , operation , zone );


if( ExecSelect( sqlString ) )
 {

   if(  GetSelectRows()  == 1 )
     {
       p = get_price(   GetFieldValue( 0 , 0 )    );

       if( period > 0 )  
       {
       per = atoi(  GetFieldValue( 0 , 1 )   );
       // vypocet ceny
       price = period * p  / per;
       }else  price = p;

       LOG( NOTICE_LOG , "GetPrice operation %d zone %d period %d   -> price %ld (units) " , operation , zone , period  , price);
     }
   

   FreeSelect();
  }

return price;
}

// ukladani polozky creditu
bool DB::SaveInvoiceCredit(int regID , int objectID , int operation , int zone  , const char *ExDate , long price  , long price2 ,  int invoiceID   , int invoiceID2) 
{
int id;

if( price > 0 )
{

id = GetSequenceID( "bank_statement_head" );

LOG( DEBUG_LOG , "uctovani creditu objectID %d ExDate [%s] regID %d" , objectID ,  ExDate , regID  );
// uloz zaznam o zuctovanem creditu
INSERT( "invoice_object_registry" );
INTO( "id" );
INTO( "objectid" );
INTO( "registrarid"  );
INTO( "operation" );
INTO( "zone" );
INTOVAL( "ExDate" , ExDate); // pokud je nastaveno EXDate
VALUE( id );
VALUE( objectID );
VALUE( regID );
VALUE( operation );
VALUE( zone );
VAL( ExDate);
if( EXEC() ) 
 {

     LOG( DEBUG_LOG , "uctovani creditu  price %ld  invoiceID %d" , price , invoiceID );

  INSERT( "invoice_object_registry_price_map" );
  INTO( "id");
  INTO( "invoiceID" );
  INTO( "price" );
  VALUE( id );
  VALUE( invoiceID );
  VALPRICE( price);
  if( !EXEC() ) { LOG( CRIT_LOG , "ERROR invoice_object_registry_price_map" ); return false; }
    

  if( price2 ) // uloz druhou cenu
   {  
     LOG( DEBUG_LOG , "uctovani creditu  price2 %ld  invoiceID2 %d" , price2 , invoiceID2 );

     INSERT( "invoice_object_registry_price_map" );
     INTO( "id");
      INTO( "invoiceID" );
      INTO( "price" );
      VALUE( id );
      VALUE( invoiceID2 );
      VALPRICE( price2);
      if( !EXEC() ) { LOG( CRIT_LOG , "ERROR invoice_object_registry_price_map price2" ); return false; }       
   }
 
 

  return true;
  }
else 
  {
     LOG( CRIT_LOG , "ERROR SaveInvoiceCredit invoiceID %d objectid %d " , invoiceID , objectID );
     return false;
  }

}
else return true;
}

// operace registrace domeny  CREATE
bool DB::BillingCreateDomain( int regID , int  zone , int  objectID  )
{
return UpdateInvoiceCredit( regID ,  OPERATION_DomainCreate , zone , 0 , "" , objectID  );
}
// operace prodlouzeni domeny RENEW
bool DB::BillingRenewDomain(  int regID , int  zone , int  objectID , int period  ,  const char *ExDate )
{
return UpdateInvoiceCredit( regID ,  OPERATION_DomainRenew , zone , period  , ExDate , objectID  );
}


// zpracovani creditu strzeni ze zalohe faktury ci dvou faktur
bool DB::UpdateInvoiceCredit( int regID ,   int operation  , int zone   , int period , const char *ExDate ,  int objectID  )
{
char priceStr[16];
// char creditStr[16];
long price , credit ;
char sqlString[256];
int invoiceID;
int invID[2];
long cr[2];
int i , num;

// cena za operaci v registru
price =  GetPrice( operation , zone , period );

// zadna cena (zadarmo) operace se povoluje
if( price == 0 ) return true;

// zjisti na jakych zalohovych fakturach je volny credit vypis vzdy dve po sobe nasledujici
sprintf( sqlString , "SELECT id , credit FROM invoice WHERE registrarid=%d and zone=%d and credit > 0 order by id limit 2;" , regID , zone );

invoiceID=0;

if( ExecSelect( sqlString ) )
 {
    num = GetSelectRows();

    for( i = 0 ; i  < num ; i ++ )
       {
        invID[i]  = atoi( GetFieldValue( i , 0 )  );
        cr[i]  =  (long) ( 100.0 *  atof(  GetFieldValue( i , 1 )   )  );
       }
     FreeSelect();
 }


// nejsou zadne zalohove faktury na zpracovani c
if( num == 0 ) return false;


credit= cr[0];
invoiceID=invID[0];

if( credit - price > 0 )
  {
     get_priceStr(   priceStr , price  );  

     if( SaveInvoiceCredit(  regID , objectID ,   operation  ,  zone   , ExDate , price , 0 , invoiceID , 0  )  )
      {
       sprintf(  sqlString ,  "UPDATE invoice SET  credit=credit-%s  WHERE id=%d;" , priceStr  , invoiceID );
       if( ExecSQL( sqlString )   ) return true;
       else LOG( CRIT_LOG , "error update  invoice  %d" , invoiceID );   
      }

   }
else
  {
   
   if(  num == 2 )   // pokud existuje dalsi faktura
   {
      credit = price  - cr[0];
 
     // cena je to co zbylo
     get_priceStr(   priceStr , cr[0] );
     invoiceID=invID[0];

    
       if( SaveInvoiceCredit(  regID ,objectID , operation  ,  zone , ExDate , cr[0] ,  price  - cr[0]  , invID[0] , invID[1]  )  ) 
         {
        // nastav credit na nulu u prvni zalohove faktury 
        sprintf(  sqlString ,  "UPDATE invoice  SET  credit=0  WHERE id=%d;" , invoiceID );

        if( ExecSQL( sqlString ) )  
        {
             invoiceID=invID[1];                        
             LOG(   DEBUG_LOG , "pozadovana castka credit0 %ld credit1 %ld price %ld credit %ld" , cr[0] ,  cr[1]  , price  , credit );
              
             if( cr[1] -  credit  > 0 )
             {
                     // u druhe zalohove faktury je uctovana cena credit = cena - credit z prvni faktury   
                   get_priceStr(  priceStr , credit ); // zmena o cenu minus credit z predchozi zal faktury
                   sprintf(  sqlString , "UPDATE invoice SET credit=credit-%s WHERE id=%d;" ,   priceStr , invoiceID );
                   if( ExecSQL( sqlString ) ) return true;
                   else    LOG( CRIT_LOG , "error update nasleduji   zalohova faktura %d" , invoiceID );  
                
             }else   LOG( CRIT_LOG , "na dalsi zalohove fakture id %d  neni pozadovana castka  %ld credit %ld" ,   invoiceID ,  price  , cr[1] );

        }else    LOG( CRIT_LOG , "error updatefirst invoice  %d" , invoiceID );  

      } 
 
   }else  LOG( CRIT_LOG , "neni uz dalsi zalohova faktura ke zpracovani ");    
 
 }


return false;
}

int DB::SaveXMLout( const char *svTRID , const char *xml  )
{
int actionID;

actionID = GetNumericFromTable( "action" , "id" ,  "serverTRID" , svTRID  );

if( actionID > 0 )
  {


        if( strlen( xml ) )
           {
                UPDATE("Action_XML");
                SET( "xml_out" , xml );
                WHERE( "actionID" , actionID );
                if(  EXEC() ) return true;
           }

   }

// default
return false;
}

// action
// zpracovani action
bool  DB::BeginAction(int clientID , int action ,const char *clTRID  , const char *xml  )
{



// HACK pro PIF zruseno  ted primo pres ADIF

 // umozni  info funkce pro PIF
/*
if( action == EPP_ContactInfo ||  action == EPP_NSsetInfo ||   action ==  EPP_DomainInfo ) 
{
if( clientID == 0 ) { actionID = 0 ; loginID =  0 ;  return 1;  }
}
*/


// actionID pro logovani
actionID = GetSequenceID("action");
loginID = clientID; // id klienta
historyID = 0 ; // history ID je nula

if( actionID ) 
  {

  // zapis do action tabulky
  INSERT( "ACTION" );
  INTO( "id" );
  if( clientID > 0 ) INTO( "clientID"  );
  INTO( "action" );
  INTO( "clienttrid" );
  
  VALUE(  actionID  );
  if( clientID > 0 ) VALUE( clientID );
  VALUE( action );
  VALUE( clTRID );
  
   if( EXEC() ) 
     {
          // zjisti jazyk clienta v tabulce login
         // pres session  clientLang = ReturnClientLanguage();

        if( strlen( xml ) )
           {
                INSERT("Action_XML");
                VALUE( actionID );
                VALUE( xml );
                if(  EXEC() ) return true;
           }
        else return true;
     }

  }

return false;
}


char * DB:: EndAction(int response )
{
int id;

if( actionID == 0 ) return "no action";
else
{
if( svrTRID == NULL ) 
 {
  LOG( SQL_LOG , "alloc svrTRID");
  svrTRID= new char[MAX_SVTID] ; 
 }

// cislo transakce co vraci server
sprintf( svrTRID , "ccReg-%010d" , actionID );

UPDATE("ACTION");
if( response > 0  ) SET( "response" , response );
SET( "enddate", "now" );
SSET( "servertrid" , svrTRID ); // bez escape
WHEREID( actionID );


// update tabulky
id  =  actionID;
actionID = 0 ; 
LOG( SQL_LOG ,  "EndAction svrTRID: %s" ,  svrTRID );

if( EXEC() ) return   svrTRID;
else return "svrTRID_DATABASE_ERROR";
}
}

char * DB::GetObjectCrDateTime( int id )
{
convert_rfc3339_timestamp( dtStr ,  GetValueFromTable(  "OBJECT_registry" , "CrDate"  , "id" , id ) );
return dtStr;
}


char * DB::GetDomainExDate( int id )
{
convert_rfc3339_date( dtStr , GetValueFromTable( "DOMAIN", "ExDate" , "id" , id ) );
return dtStr;
}
char * DB::GetFieldDateTimeValueName(  const char *fname , int row )
{
convert_rfc3339_timestamp( dtStr ,  GetFieldValueName( (char * ) fname , row ) ) ;
return dtStr;
}

char * DB::GetFieldDateValueName(  const char *fname , int row )
{
convert_rfc3339_date( dtStr ,  GetFieldValueName(  (char * )  fname , row ) ) ;
return dtStr;
}




// zjistuje pocet hostu pro dany nsset
int DB::GetNSSetHosts( int nssetID )
{
char sqlString[128];
int num=0;

sprintf( sqlString , "SELECT id FROM host  WHERE nssetID=%d;" , nssetID );

if( ExecSelect( sqlString ) )
 {
     num =  GetSelectRows();
     LOG( SQL_LOG , "nsset %d num %d" , nssetID , num ); 
     FreeSelect();
 }

return num;
}


// jestli je registrator clientam objektu
bool DB::TestObjectClientID( int id  , int regID )
{
char sqlString[128];
bool ret=false;
sprintf( sqlString , "SELECT id FROM  object WHERE id=%d and clID=%d " , id , regID ); 
if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() == 1 ) ret = true;
     FreeSelect();
 }

return ret;
}

char * DB::GetObjectName( int id )
{
return GetValueFromTable( "object_registry" , "name" , "id" , id );
}
 
 
int  DB::GetContactID( const char *handle )
{
char HANDLE[64];

if( get_CONTACTHANDLE( HANDLE , handle ) == false ) return -1; // preved handle na velka pismena
else return GetObjectID( 1 , HANDLE );
}

int  DB::GetNSSetID( const char *handle )
{
char HANDLE[64];

if( get_NSSETHANDLE( HANDLE , handle ) == false ) return -1 ; // preved handle na velka pismena
else  return  GetObjectID( 2 , HANDLE );
}

 
int DB::GetObjectID( int type , const char *name )
{
char sqlString[512];
int id=0;

sprintf( sqlString , "SELECT object.id FROM object_registry , object WHERE object_registry.type=%d AND object_registry.id=object.id AND object_registry.name=\'%s\';" , type ,  name );
if( ExecSelect( sqlString ) )
 {
  if( GetSelectRows()  ==  1 )
    {
     id = atoi(  GetFieldValue( 0 , 0 )  );
     LOG( SQL_LOG , "GetObjectID   name=\'%s\'  -> ID %d" , name ,  id );
    }

   FreeSelect();
 }

return id;
}


 
// zjistuje pocet  tech pro dany nsset
int DB::GetNSSetContacts( int nssetID )
{
char sqlString[128];
int num=0;

sprintf( sqlString , "SELECT * FROM nsset_contact_map  WHERE nssetID=%d;" , nssetID );

if( ExecSelect( sqlString ) )
  {
     num =  GetSelectRows();
     LOG( SQL_LOG , " nsset_contact_map  num %d"  , num );
     FreeSelect();
  }

return num;
}

bool DB::SaveObjectDelete( int id )
{
   LOG( SQL_LOG , "set delete objectID %d" , id );
           // uloz ze je objekt smazan
           UPDATE( "object_registry" );
           SET( "ErDate" , "now" );
           WHEREID( id );
           return EXEC();
}


bool DB::SaveObjectCreate( int id )
{
 LOG( SQL_LOG , "set create histyoryID for object ID %d historyID %d" , id ,  historyID);
if( historyID )
  {
           UPDATE( "object_registry" );
           SET( "crhistoryid" , historyID );
           WHEREID( id );
           return EXEC();
   }        
else return false;
}

bool DB::TestContactHandleHistory( const char * handle , int days )
{
return TestObjectHistory( handle , days );
}


bool DB::TestNSSetHandleHistory( const char * handle , int days )
{
return TestObjectHistory(  handle , days );
}


bool DB::TestDomainFQDNHistory( const char * fqdn , int days )
{
return TestObjectHistory( fqdn , days );
}

// test na objekty v historii
bool DB::TestObjectHistory(   const char * name , int days )
{
/* TODO predelat
char sqlString[512];
bool ret=false;
int count;

if( days > 0 )
{
// nezalezi mala velka pismena
sprintf( sqlString , "SELECT count( id ) FROM object_delete  WHERE name ILIKE \'%s\' and  deltime  > current_timestamp - interval\'%d days\';"  , name , days );

if( ExecSelect( sqlString ) )
  {
     count = atoi(  GetFieldValue( 0 , 0 ) ) ;
     if( count > 0 )  ret=true ;
     FreeSelect();
  }

return ret;
} else 
*/
return false;

}



int DB::CreateObject( const char *type , int regID , const char *name , const char *authInfoPw )
{
char roid[64];
char pass[PASS_LEN+1];
int id;

id = GetSequenceID( "object_registry" );

 // vytvor roid 
 get_roid( roid, ( char * ) type , id );

 INSERT( "OBJECT_registry" );
 INTO( "id" );
 INTO( "type" );
 INTO( "NAME" ); 
 INTO( "roid" );
 INTO( "CrDate" );
 INTO( "CrID" );

 VALUE( id );

// TYP objektu
switch( type[0] )
  {
    case 'C' : // kontakt
         VALUE( 1 );
         VALUEUPPER( name );
         break;
    case 'N' : // nsset
         VALUE( 2 );
         VALUEUPPER( name );
         break;
    case 'D' : // domena
         VALUE( 3 );
         VALUELOWER( name );
         break;
   default :
         VALUE( 0);
  }
    
 VALUE( roid );
 VALUENOW();
 VALUE( regID );
 


if( EXEC() )
 {
 INSERT( "OBJECT" );
 INTO( "id" );
 INTO( "ClID" );
 INTO( "AuthInfoPw" );

 VALUE( id );
 VALUE( regID );


                     if( strlen ( authInfoPw ) == 0 )
                       {
                          random_pass(  pass  ); // autogenerovane heslo pokud se heslo nezada
                          VVALUE( pass );
                        }
                      else VALUE( authInfoPw );

  if( EXEC() )  return id;
  else return 0; 
 }
else return 0;
}


/*
char *  DB::GetStatusString( int status )
{

char sqlString[128];
char *str;

str = new char[128];

sprintf( sqlString , "SELECT  status  FROM enum_status  WHERE id=%d;" , status );

if( ExecSelect( sqlString ) )
 {
      strcpy( str ,  GetFieldValue( 0 , 0 ) ) ;
      LOG( SQL_LOG , "GetStatustring \'%s\'  ->  %d" ,  str , status );
      FreeSelect(); 
      return str;
 }
else return "{ }" ; // prazdny status string pole
}


switch( status )
{
   case STATUS_ok:
           return "ok";
   case STATUS_inactive:
           return "inactive";
   case STATUS_linked:
           return "linked";
   case STATUS_clientDeleteProhibited:
           return "clientDeleteProhibited";
   case STATUS_serverDeleteProhibited:
           return "serverDeleteProhibited";
   case STATUS_clientHold:
           return "clientHold";
   case STATUS_serverHold:
           return "serverHold";
   case STATUS_clientRenewProhibited:
           return "clientRenewProhibited";
   case STATUS_serverRenewProhibited:
           return "serverRenewProhibited";
   case STATUS_clientTransferProhibited:
           return "clientTransferProhibited";
   case STATUS_serverTransferProhibited:
           return "serverTransferProhibited";
   case STATUS_clientUpdateProhibited:
           return "clientUpdateProhibited";
   case STATUS_serverUpdateProhibited:
           return "serverUpdateProhibited";
   default:
           return "";
}         

return "";
}


int  DB::GetStatusNumber( char  *status )
{
char sqlString[128];
char *handle;
int id =0;

sprintf( sqlString , "SELECT  id  FROM enum_status  WHERE status=\'%s\';" , status );

if( ExecSelect( sqlString ) )
 {
      handle = GetFieldValue( 0 , 0 );
      id = atoi( handle );
      LOG( SQL_LOG , "GetStatusNumeric \'%s\'  ->  (%s) %d" ,   status , handle  , id  );
      FreeSelect();
 }

return id;
}

*/

// testuje pravo registratora na zapis do zony 
bool DB::TestRegistrarZone(int regID , int zone )
{
bool ret = false;
char sqlString[128];
// char zoneArray[64] , str[10] ;
// int z , j  , len ;

sprintf( sqlString , "SELECT zone FROM REGISTRAR WHERE id=%d; " ,  regID  );

if( ExecSelect( sqlString ) )
 {
    if(  GetSelectRows() == 1  )
      {
                     ret=true;
/* TODO predelat databazove schama a zrusit posledni array
                   // pole zone
                   strcpy( zoneArray , GetFieldValue( 0 , 0 ) );

                   // zpracuj pole adres
                   len =  get_array_length( zoneArray  );
                   for( j = 0 ; j < len ; j ++)
                      {
                        get_array_value( zoneArray , str , j );
                        z = atoi( str ); 
                        if( z == zone ) { ret=true; break ; }  // mam pravo                       
                      }

*/
      }
    FreeSelect();
  }

return ret;
}


bool  DB::CheckContactMap(const char * table , int id , int contactid )
{
bool ret = false;
char sqlString[128];

sprintf( sqlString , "SELECT * FROM %s_contact_map WHERE %sid=%d and contactid=%d" , table , table , id , contactid );

if( ExecSelect( sqlString ) )
 {
    if(  GetSelectRows() == 1  ) ret=true; // kontakt existuje
    FreeSelect();
  }

return ret;
}





// pridave contact do pole kontaktu
bool DB::AddContactMap( const char * table , int id , int  contactid )
{
char sqlString[128];

sprintf( sqlString , "INSERT INTO %s_contact_map VALUES ( %d , %d );" ,  table , id , contactid );

return ExecSQL( sqlString );
}
 


bool DB::TestValExDate(const char *dateStr ,  int period  , int interval , int id )
{
char sqlString[256];
char maxDate[32];
bool ret =false;

// default
strcpy( maxDate ,  "" );

if( id)
{
 // vyber mensi ze dvou hodnot
 sprintf( sqlString , "SELECT  date_smaller (  date( exdate + interval\'%d month\' )  , date( current_date + interval\'%d month\' + interval\'%d days\' ) )  FROM enumval WHERE domainid=%d;" , 
                  period , period , interval , id   );
if( ExecSelect( sqlString ) )
 {
    if(  GetSelectRows() == 1  )   strcpy( maxDate ,  GetFieldValue( 0 , 0 )   ) ;
     FreeSelect();
  }
}
else
{
 sprintf( sqlString , "SELECT date( current_date + interval\'%d month\' + interval\'%d days\'  ) ; " , period , interval );
if( ExecSelect( sqlString ) )
 {
    if(  GetSelectRows() == 1  )   strcpy( maxDate ,  GetFieldValue( 0 , 0 )   ) ;
    FreeSelect();
 }
}

// nepodarilose vypocitat maxDate
if( strlen( maxDate  ) == 0 ) return false;


sprintf( sqlString , "SELECT valex , max, min  from (select date\'%s\' as valex , date( '%s' )  as max,  current_date as min ) as tmp WHERE valex > min AND valex < max;" ,
                        dateStr , maxDate  );

if( ExecSelect( sqlString ) )
 {
    if(  GetSelectRows() == 1  )  ret = true;
    FreeSelect();
 }


return ret;
}



// testuje jestli lze prodlouzit domenu pri DomainRenew
bool DB::CountExDate( int domainID , int period  , int max_period )
{
char sqlString[256];
bool ret=false;

sprintf( sqlString , "SELECT  ExDate+interval\'%d month\' FROM DOMAIN WHERE id=%d AND ExDate+interval\'%d month\' < current_date+interval\'%d month\';",
           period , domainID , period , max_period );


if( ExecSelect( sqlString ) )
 {
    if(  GetSelectRows() == 1  )  ret = true;
    FreeSelect();
  }

return ret;
}


bool DB::RenewExDate(  int domainID , int period  )
{
char sqlString[256];

sprintf( sqlString , "UPDATE domain SET  ExDate=ExDate+interval\'%d month\' WHERE id=%d ;" ,  period , domainID );

return ExecSQL(  sqlString );
}


 



// test pri Check funkci na domenu case insensitiv
int DB::GetDomainID( const char *fqdn  , bool enum_zone )
{
// TODOTODOTODOTODOTODOTODO
//char sqlString[512];
//int id=0;
return GetObjectID( 3 , fqdn );
// if( enum_zone )
//sprintf( sqlString , "SELECT id FROM object  WHERE  ( \'%s\' LIKE  \'%%.\'|| name ) OR  (name LIKE  \'%%.\'  || \'%s\' )  OR ( name=\'%s\' );"  , fqdn   , fqdn  , fqdn );
// else 
// sprintf( sqlString , "SELECT id FROM object_registry , object WHERE  object.id=object_registry.id and  object_registry.name=\'%s\';" , fqdn );
/*
if( ExecSelect( sqlString ) )
 {
  if( GetSelectRows()  ==  1 )
    {
     id = atoi(  GetFieldValue( 0 , 0 )  );
     LOG( SQL_LOG , "Check domain   fqdn=\'%s\'  -> ID %d" , fqdn ,  id );
    }

   FreeSelect();
 }

return id;
*/
}


// vraci ID hostu
int DB::GetHostID(  const char *fqdn , int nssetID )
{
char sqlString[128];
int hostID=0;

sprintf( sqlString , "SELECT id FROM HOST WHERE fqdn=\'%s\' AND nssetid=%d;" , fqdn , nssetID ); 

if( ExecSelect( sqlString ) )
 {
   if(  GetSelectRows()  == 1 )
     {
       hostID = atoi(  GetFieldValue( 0 , 0 )  );
       LOG( SQL_LOG , "CheckHost fqdn=\'%s\' nssetid=%d  -> hostID %d" , fqdn , nssetID , hostID );
     }
   
  FreeSelect();
 }

if( hostID == 0 )  LOG( SQL_LOG , "Host fqdn=\'%s\' not found" , fqdn  );
return hostID;
}

bool DB::TestNSSetRelations(int id )
{
bool ret = false;
char sqlString[128];

sprintf( sqlString , "SELECT id from DOMAIN WHERE nsset=%d;" , id );
if( ExecSelect( sqlString ) )
  {
     if(  GetSelectRows() > 0 ) ret=true;  // jestli ma domena definovany nsset 
     FreeSelect();
  }

return ret;
}

bool DB::TestContactRelations(int id )
{
int count=0;
char sqlString[128];

sprintf( sqlString , "SELECT count(id) from DOMAIN WHERE Registrant=%d;" , id );
if( ExecSelect( sqlString ) ) 
  {
     count = atoi(  GetFieldValue( 0 , 0 ) );
     FreeSelect();
  }

if( count > 0 ) return true;

sprintf( sqlString , "SELECT count(nssetID ) from NSSET_CONTACT_MAP WHERE contactid=%d;" , id );
if( ExecSelect( sqlString ) )
  {
     count = atoi(  GetFieldValue( 0 , 0 ) );
     FreeSelect();
  }

if( count > 0 ) return true;

sprintf( sqlString , "SELECT count( domainID)  from DOMAIN_CONTACT_MAP WHERE contactid=%d;" , id );
if( ExecSelect( sqlString ) )
  {
    count = atoi(  GetFieldValue( 0 , 0 ) );

     FreeSelect();
  }

if( count > 0 ) return true;
else return false;
}

// potvrzeni hesla authinfopw v tabulce 
bool  DB::AuthTable(const  char *table , char *auth , int id )
{
bool ret=false;
char *pass;
char sqlString[128];

sprintf( sqlString , "SELECT authinfopw from %s WHERE id=%d" , table  , id );

if( ExecSelect( sqlString ) )
  {
    if( GetSelectRows() == 1 )
      {
           pass = GetFieldValue( 0 , 0 );
           if( strcmp( pass , auth  ) == 0 )  ret = true;
      }
  
   FreeSelect();
  }

return ret;
}


int DB::GetSystemVAT()  // vraci hodnotu DPH pro sluzby registrace
{
char sqlString[128] = "select vat from price_vat where valid_to > now() or valid_to is null;" ;
int dph=0;

if( ExecSelect( sqlString ) )
  {
    if( GetSelectRows() == 1 )
      {
          dph = atoi( GetFieldValue( 0 , 0 ) );
      }
    FreeSelect();
  }

return dph;
} 

// test kodu zemo
/*
bool DB::TestCountryCode(const char *cc)
{
char sqlString[128];
bool ret = false;

if( strlen( cc )  == 0 ) return true; // test neni potreba vysledek je true

sprintf( sqlString , "SELECT id FROM enum_country WHERE id=\'%s\';" , cc );
if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() == 1   ) ret = true;
    FreeSelect();
 } 

return ret;
}
*/

// test zustatku na uctu pro import bankovniho vypisu
int DB::TestBankAccount( char *accountStr , int num , long oldBalance )
{
int accountID=0;
int lastNum=0;
long lastBalance=0;

  if(  SELECTONE( "bank_account" , "account_number" ,  accountStr  ) )
    {
          if(  GetSelectRows() == 1 )
            {
                accountID = atoi( GetFieldValueName("id"  , 0 ) );
                lastNum = atoi( GetFieldValueName("last_num" , 0 ) );
                lastBalance = (long) ( 100.0 *  atof( GetFieldValueName("balance" , 0 ) )  );
            }
       
       FreeSelect();
     }

if(  accountID )
{
    LOG( LOG_DEBUG ,"posledni vypis ucetID %d cislo %d zustatek na uctu %ld" , accountID ,  lastNum , lastBalance);


     // test podle cisla vypisu ne pro prvni vypis
    if( num > 1  )
      {
        if( lastNum + 1 != num ) 
          {
               LOG(  ERROR_LOG  , "chyba nesedi cislo  vypisu %d  posledni nacteny je %d" , num , lastNum );
          }
      }

    // dalsi test pokud sedi zustatek na uctu
   if(  oldBalance  == lastBalance ) return accountID;
   else
     {
         LOG(  ERROR_LOG  , "chyba nesedi zustatek na uctu poslednu zustatek %ld nacitany stav %ld" , lastBalance ,  oldBalance );
     }

}
else 
{
   LOG(  ERROR_LOG  , "nelze najit ucet na vypisu cislo %s" ,  accountStr );
}


return 0;
}


// update zustatku na uctu
bool DB::UpdateBankAccount( int accountID , char *date , int num ,  long newBalance  )
{
    
     //  update  tabulky

           UPDATE( "bank_account" );
           SET( "last_date" , date  );
           SET( "last_num" , num );
           SETPRICE( "balance" , newBalance );
           WHEREID( accountID );

       return EXEC();
}




int DB::SaveBankHead( int accountID ,int num ,  char *date  ,  char *oldDate , long oldBalance , long newBalance , long credit , long debet )
{ 
int statemetID;

     statemetID = GetSequenceID( "bank_statement_head" );
   // id | account_id | num | create_date | balance_old_date | balance_old | balance_new | balance_credit | balence_debet

                    INSERT( "bank_statement_head" );
                    INTO( "id" );
                    INTO( "account_id" );
                    INTO( "num" );
                    INTO( "create_date" );
                    INTO( "balance_old_date" );
                    INTO( "balance_old" );
                    INTO( "balance_new" );
                    INTO( "balance_credit" );
                    INTO( "balance_debet" );
                    VALUE(  statemetID );
                    VALUE(  accountID );
                    VALUE(  num );
                    VALUE(  date );
                    VALUE(  oldDate );
                    VALPRICE(  oldBalance );
                    VALPRICE(  newBalance  );
                    VALPRICE(  credit );
                    VALPRICE(  debet );

     if( EXEC() ) return statemetID;
     else return 0;

}

bool DB::SaveBankItem( int statemetID , char *account  , char *bank , char *evidNum,  char *date , char *memo ,
                       int code  ,  char *konstSymb ,  char *varSymb , char *specsymb  , long price )
{
                    INSERT( "bank_statement_item" );
                    INTO( "statement_id" );
                    INTO( "account_number" );
                    INTO( "bank_code" );
                    INTO( "account_evid" );
                    INTO( "account_date" );
                    INTO( "account_memo" );
                    INTO( "code" );
                    INTO( "konstsym" );
                    INTO( "varsymb" );
                    INTO( "specsymb" );
                    INTO( "price" );
                    VALUE( statemetID );
                    VALUE( account );
                    VALUE( bank );
                    VALUE( evidNum );
                    VALUE( date );
                    VALUE( memo );
                    VALUE( code );
                    VALUE( konstSymb  );
                    VALUE( varSymb );
                    VALUE( specsymb);
                    VALPRICE( price );

return EXEC ();
}


// uloz credit
bool DB::SaveCredit( int invoiceID ,  long credit   )
{

           // uloz credit do tabulky invoice
           UPDATE("invoice");
           SETPRICE("credit"  , credit );
           WHEREID(  invoiceID );
           return EXEC();
}
                
// nastav bankovni vypis jako zpracovany
bool DB::UpdateBankStatementItem( int id , int invoiceID)
{
                  UPDATE( "bank_statement_item" );
                  SET( "invoice_id" , invoiceID );
                  WHEREID(  id  );
                  return EXEC();
}  

// vytvoreni zalohove faktury pro registratora na castku price s vysi DPH vatNum odvedenou dani vat  a castkou bez DPH credit
int  DB::MakeInvoice( const char *prefixStr  , int zone ,  int regID , long price , int vatNum , long vat ,  long credit )
{
int invoiceID;

// castka bez DPh + DPH by se mela rovanat castce s dani 
if( (vat + credit)  == price ) 
{


           invoiceID = GetSequenceID( "invoice" );

           INSERT( "invoice" );
           INTO( "id" );
           INTO( "prefix" );
           INTO( "zone" );
           INTO( "registrarid" );
           INTO( "price" );
           INTO( "vat" );
           INTO( "totalVAT" );           
           INTO( "credit" );
           VALUE( invoiceID );
           VALUE( prefixStr  );
           VALUE( zone );
           VALUE( regID );
           VALPRICE( price );

           VALUE( vatNum );
           VALPRICE( vat  );
           VALPRICE( credit ); // cena bez dane
          
           if(  EXEC() ) return invoiceID;
}
          
return 0;
} 

/*
int  DB::MakeFaktur( const char *prefixStr  , int zone ,  int regID , const char *fromDate , const char *toDate  )
{
int fakturID;
long total=0;
char sqlString[512];

sprintf( sqlString , "SELECT  sum(price ) FROM invoice_object_registry WHERE zone=%d and registrarid=%d and crdate>= date\'%s\' and crdate < date\'%s\'; " ,
              zone , regID , fromDate , toDate  );

if( ExecSelect( sqlString ) )
 {
     total =  (long) ( 100.0 *  atof( GetFieldValue( 0  , 0 ) )  );
      FreeSelect();
 } 

if( total )
{


}

*/


bool DB::GetInvoicePrefix( char *prefixStr , int typ , int zone )
{
char sqlString[512];
int id , counter , zero;

sprintf( sqlString , "SELECT *  FROM invoice_prefix WHERE zone=%d and typ=%d;",  zone , typ);

prefixStr[0] = 0 ;
counter = 0;
id=0;


       if( ExecSelect( sqlString )  )
         {
            if(  GetSelectRows() == 1 )
              {
                    id = atoi( GetFieldValueName("id"  , 0 ) );
                    counter =  atoi( GetFieldValueName("counter" , 0 ) );
                    zero =  atoi( GetFieldValueName("num" , 0 ) );
                    sprintf( prefixStr , "%s%0*d" ,  GetFieldValueName("pref" , 0 )  , zero ,  counter );
                    LOG( LOG_DEBUG ,"generate invoice zone %d typ %d counter %d  prefix[%s]" , zone , typ , counter ,    prefixStr  );
              }
           FreeSelect();
          }

        if( counter > 0  && id )
          {
            UPDATE( "invoice_prefix" );
            SET( "counter" , counter +1 );
            WHEREID( id );
            if( EXEC() ) return true;
           }

 
return false;         
}


// vraci id registratora z domeny
int DB::GetClientDomainRegistrant( int clID , int contactID )
{
int regID=0;
char sqlString[128];

sprintf( sqlString , "SELECT  clID FROM DOMAIN WHERE Registrant=%d AND clID=%d" , contactID , clID );
if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() > 0   )
     {
       regID = atoi(  GetFieldValue( 0 , 0 )  );
       LOG( SQL_LOG , "Get ClientDomainRegistrant  contactID \'%d\' -> regID %d" , contactID , regID );
       FreeSelect();
     }
   }

return regID;  
}  

char *  DB::GetValueFromTable( const char *table , const char *vname , const char *fname , const char *value)
{
char sqlString[512];
int size;
// char *handle;

sprintf( sqlString , "SELECT  %s FROM %s WHERE %s=\'%s\';" , vname ,  table  ,  fname , value );

if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() == 1   ) // pokud je vracen prave jeden zaznam
     {
      size = GetValueLength( 0 , 0 );
//      handle = new char[size+1];  // alokace pameti pro retezec

      if( memHandle )
       {
          delete[] memHandle;
          LOG( SQL_LOG , "re-alloc memHandle");
          memHandle = new char[size+1];   
        }
       else { LOG( SQL_LOG , "alloc memHandle");  memHandle = new char[size+1]; } 
 
      strcpy( memHandle , GetFieldValue( 0 , 0 ) );
      LOG( SQL_LOG , "GetValueFromTable \'%s\' field %s  value  %s ->  %s" , table ,  fname , value  , memHandle );
      FreeSelect();      
      return memHandle;
     }
   else {
     FreeSelect();         	
   	 return "";
   }
  }
else return "";

}

char * DB::GetValueFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric)
{
char value[16];

sprintf( value , "%d" ,  numeric );

return GetValueFromTable( table , vname , fname , value );
}

int DB::GetNumericFromTable( const char *table , const char *vname , const char *fname , const char *value)
{
return atoi( GetValueFromTable( table , vname , fname , value )  );
}

int  DB::GetNumericFromTable( const char *table , const char *vname ,  const char *fname ,  int numeric)
{
char value[16];

sprintf( value , "%d" ,  numeric );

return  GetNumericFromTable( table , vname , fname , value  );
}

// vymaz data z tabulky
bool  DB::DeleteFromTable(char *table , char *fname , int id )
{
char sqlString[128];

LOG( SQL_LOG , "DeleteFromTable %s fname %s id -> %d" , table , fname  , id );

sprintf(  sqlString , "DELETE FROM %s  WHERE %s=%d;" , table , fname , id );
return ExecSQL( sqlString );
}
 

// vymaz data z tabulky
bool  DB::DeleteFromTableMap(char *map ,int  id , int contactid )
{
char sqlString[128];


LOG( SQL_LOG , "DeleteFrom  %s_contact_map  id  %d contactID %d" , map ,id , contactid );

sprintf( sqlString , "DELETE FROM %s_contact_map WHERE  %sid=%d AND contactid=%d;" , map , map ,  id ,  contactid );

return ExecSQL( sqlString );
}



int DB::GetSequenceID( char *sequence )
{
char sqlString[128];
int id=0;


sprintf(  sqlString , "SELECT  NEXTVAL( \'%s_id_seq\'  );" , sequence );

if( ExecSelect( sqlString ) )
  {
     id = atoi(  GetFieldValue( 0 , 0 )  );
     LOG( SQL_LOG , "GetSequence \'%s\' -> ID %d" , sequence , id );
     FreeSelect();
   }

return id;
}

bool DB::SaveNSSetHistory( int id )
{

 //  uloz do historie
 if(   MakeHistory(id)  )
   {

      if( SaveHistory( "NSSET", "id", id ) )        
           if( SaveHistory( "HOST", "nssetid", id ) )
              if( SaveHistory( "HOST_IPADDR_map", "nssetid", id ) ) 
                    if( SaveHistory(  "nsset_contact_map", "nssetid", id ) )  // historie tech kontakty
                           return true;
   }

return 0;         
}

bool DB::DeleteNSSetObject( int id )
{

// na zacatku vymaz technicke kontakty
if( DeleteFromTable( "nsset_contact_map", "nssetid", id ) )
   if( DeleteFromTable( "HOST_IPADDR_map", "nssetid", id ) ) // nejdrive smaz ip adresy
        if( DeleteFromTable( "HOST", "nssetid", id ) )  // vymaz podrizene hosty
             if( DeleteFromTable( "NSSET", "id", id ) )
                          if( DeleteFromTable(  "OBJECT", "id", id ) ) return true;


return false;
}

bool DB::SaveDomainHistory( int id )
{
                          //  uloz do historie
 if( MakeHistory(id)  )
   {
     if( SaveHistory( "DOMAIN" , "id", id ) )  
        if( SaveHistory( "domain_contact_map", "domainID", id ) )       // uloz admin kontakty
                   if( SaveHistory( "enumval",  "domainID", id ) )  // uloz extension
                                   return true;
   }

return 0;
}

bool DB::DeleteDomainObject( int id )
{
    if( DeleteFromTable( "domain_contact_map", "domainID", id ) ) // admin kontakt     
         if( DeleteFromTable( "enumval", "domainID", id ) )      // enumval extension
               if( DeleteFromTable( "DOMAIN", "id", id ) )
                       if( DeleteFromTable(  "OBJECT", "id", id ) ) return true;


return false;
}
                                   
 

bool DB::SaveContactHistory( int id )
{
 //  uloz do historie
 if(   MakeHistory(id) )
   {
     if( SaveHistory(  "Contact", "id", id ) ) return true;
   }

return 0;
}


bool DB::DeleteContactObject( int id )
{

     if( DeleteFromTable( "CONTACT", "id", id ) )
       {
             if( DeleteFromTable(  "OBJECT", "id", id ) ) return true;
       }

return false;
}



int DB::MakeHistory(int objectID) // zapise do tabulky history
{
char sqlString[128];


if( actionID )
 {   
   LOG( SQL_LOG , "MakeHistory actionID -> %d " , actionID);
   historyID = GetSequenceID( "HISTORY" );
   if( historyID )
    {
     LOG( SQL_LOG , "MakeHistory actionID -> %d " , actionID); 
     sprintf(  sqlString , "INSERT INTO HISTORY ( id , action ) VALUES ( %d  , %d );" , historyID , actionID );
     if( ExecSQL(  sqlString ) )
        {
          if( SaveHistory(  "OBJECT" ,  "id", objectID ) )  // uloz take object do object_history
            {
              LOG( SQL_LOG , "Update objectID  %d -> historyID %d " , objectID , historyID );
              sprintf(  sqlString , "UPDATE OBJECT_registry set historyID=%d WHERE id=%d;" , historyID  , objectID );
              if( ExecSQL(  sqlString ) ) return historyID;
            }
        }
    }
 }

// default
return 0;
}


// ulozi radek tabulky s id
bool DB::SaveHistory(char *table , char *fname , int id )
{
int i ,  row ;
bool ret= true; // default


if( historyID )
{
    LOG( SQL_LOG , "SaveHistory historyID - > %d" , historyID ); 
    if( SELECTONE( table , fname , id )  )
    {
     for( row = 0 ; row <  GetSelectRows() ; row ++ )
     {
      // zapis do tabulky historie 
      INSERTHISTORY( table );

       for( i = 0 ; i <  GetSelectCols() ; i ++ )
          {
           // pokud to neni historyID z tabulky object to uz neukladej
           if( IsNotNull( row , i ) && strcasecmp( "historyID" , GetFieldName(i) ) != 0  ) INTO(  GetFieldName( i ) ); 
          }

      VALUE( historyID ); // zapis jako prvni historyID
      for( i = 0 ; i <  GetSelectCols() ; i ++ )
      {
        // uloz pouze ne null hosdnoty
        if( IsNotNull( row , i ) && strcasecmp( "historyID" , GetFieldName(i) ) != 0   ) VALUE(  GetFieldValue( row  , i ) );
       }

        if( EXEC() == false) { ret = false ; break;  } // uloz hostorii a  pokud nastane chyba

      }

     FreeSelect();
   }

}

// default 

return ret;
}

// id object a registrator
bool DB::ObjectUpdate( int id , int regID , const char *authInfo )
{
// datum a cas updatu  plus kdo zmenil zanzma na konec
UPDATE( "OBJECT" );
SSET( "UpDate", "now" );
SET( "UpID", regID );
SET( "AuthInfoPw", authInfo ); 
WHEREID( id );
return EXEC();
}

// UPDATE funkce
// SQL UPDATE funkce
void DB::UPDATE( const char * table )
{
sqlBuffer = new char[MAX_SQLBUFFER];
memset(  sqlBuffer , 0 , MAX_SQLBUFFER );
SQLCat( "UPDATE " );
SQLCat( table );
SQLCat( " SET " );
}

void DB::SQLDelEnd()
{
int len;
len = strlen( sqlBuffer );
// vymaz konec
if( len > 0 )  sqlBuffer[len-1]  = 0;
}

bool DB::SQLTestEnd( char c )
{
int len;
len = strlen( sqlBuffer );

// test
if(  sqlBuffer[len-1]  == c ) return true;
else return false;
}

void DB::SQLCat(const char *str )
{
int len , length ;
len = strlen( str );
length = strlen( sqlBuffer );

//  test na delku retezce
if(  len  + length < MAX_SQLBUFFER ) strcat( sqlBuffer , str );
}


void DB::SQLCatLower( const char *str )
{
SQLCatLW( str , false );
}

void DB::SQLCatUpper(const char *str )
{
SQLCatLW( str , true );
}

// prevadi na mala ci velka pismena 
void DB::SQLCatLW( const char *str , bool lw )
{
int len , length , i ;
len = strlen( str );
length = strlen( sqlBuffer );

//  test na delku retezce
if(  len  + length < MAX_SQLBUFFER ) 
{
  for( i = 0 ; i < len ; i ++ ) 
   {
       if( lw ) sqlBuffer[length+i] = toupper( str[i] );
       else sqlBuffer[length+i] = tolower( str[i] );
   }
// ukonict retezec
sqlBuffer[length+len] = 0;
}

LOG( SQL_LOG , "DB::SQLCatLW lw %d str[%s] sqlBuffer[%s]" ,  lw , str , sqlBuffer );

}


// escape
void DB::SQLCatEscape( const char * value )
{
int length;
char *str;

length = strlen( value );

if( length )
{
    // zvetsi retezec
    // LOG( SQL_LOG , "alloc escape string length  %d" , length*2 );
    str = new char[length*2];
    // DB escape funkce
    Escape( str , value  , length ) ;
    SQLCat( str );
    // LOG( SQL_LOG , "free  escape string" );
    delete[] str;
}

}


void DB::SETPRICE( const char *fname , long price )
{
char priceStr[16];
// castka v halirich
sprintf( priceStr , "%ld.%02ld" , price /100 ,  price %100 );
SETS( fname , priceStr , false  ); // bez ESC
}

// beaz escape 
void DB::SSET( const char *fname , const char * value )
{
SETS( fname , value , false   );
}
// s escape 
void DB::SET( const char *fname , const char * value )
{
SETS( fname , value , true );
}

/*
void DB::NSET( const char *fname , const char * value , bool null )
{
SETS( fname ,  value , true , null );
}
*/
void DB::SETS( const char *fname , const char * value , bool esc /* , bool null  */)
{

if( strlen( value ) )
 {

  SQLCat(" ");
  SQLCat(fname );
  SQLCat( "=" );
 // ODSTRANIT zatim to pouziva take hack na znak \b
//   if( strcmp( value  , NULL_STRING ) ==  0 || value[0] == 0x8 ) // nastavi NULL value pro NULL string z IDL

// if( null )
  if(   value[0] == 0x8 )  //  zatim pouzivat s mod_eppd hack na znak \b
   {     
     SQLCat( "NULL" );           
   }
   else 
   {
    SQLCat( "'" );
    if( esc )  SQLCatEscape(  value ); 
    else SQLCat( value );
    SQLCat(  "'" );
   }

  SQLCat(  " ," ); // carka na konec
 }

}

void DB::SET( const char *fname , int value )
{
char numStr[16];

SQLCat( "  ");
SQLCat(  fname );
SQLCat(  "=" );
sprintf( numStr , "%d" ,  value  );
SQLCat(  numStr );
SQLCat( " ," );
}




void DB::SET( const char *fname , bool  value )
{

SQLCat( "  "); 
SQLCat(  fname );
SQLCat(  "=" );

if( value )  SQLCat( "\'t\'");
else  SQLCat( "\'f\'");

SQLCat(  " ," );
}

// nastavi but t nebo f 
void DB::SETBOOL( const char *fname , char c  )
{
// jedna jako tru 0 jako false -1 nic nemenit
if( c == 't' || c == 'f'  ||  c == 'T' || c == 'F' )
{
SQLCat("  "); 
SQLCat( fname );
SQLCat( "=" );

if(  c == 't' ||  c == 'T' )  SQLCat( "\'t\'");
else  SQLCat( "\'f\'");

SQLCat( " ," );
}

}


void DB::WHERE(const char *fname , const char * value )
{
  if( SQLTestEnd( ',' ) ||  SQLTestEnd( ';' ) ) SQLDelEnd();  // vymaz posledni znak

  SQLCat("  WHERE " );
  SQLCat( fname );
  SQLCat( "='" );
  SQLCatEscape(  value );
  SQLCat( "' ;" );

}


void DB::OPERATOR(  const  char *op )
{
  if( SQLTestEnd( ',' ) ||  SQLTestEnd( ';' ) ) SQLDelEnd();  // vymaz posledni znak
       SQLCat( "  "  );
       SQLCat( op ); // operator AND OR LIKE
       SQLCat( " " );
}


void DB::WHEREOPP(  const  char *op ,  const  char *fname , const  char *p  , const  char * value )
{
  if( SQLTestEnd( ',' ) ||  SQLTestEnd( ';' ) ) SQLDelEnd();  // vymaz posledni znak

       SQLCat( "  "  );
       SQLCat( op ); // operator AND OR LIKE
       SQLCat( " " );
       SQLCat( fname );
       SQLCat(  p );
       SQLCat(  "'" );
       SQLCatEscape(  value );
       SQLCat(  "' ;" ); // konec         

}
void DB::WHERE( const  char *fname , int value )
{
char numStr[16];
sprintf( numStr , "%d" ,  value );
WHERE( fname , numStr ); 
}

// INSERT INTO history
void DB::INSERTHISTORY( const char * table )
{
sqlBuffer = new char[MAX_SQLBUFFER];
memset(  sqlBuffer , 0 , MAX_SQLBUFFER );
SQLCat( "INSERT INTO ");
SQLCat( table );
SQLCat( "_history " );
INTO( "HISTORYID" );
}



// INSERT INTO funkce

void DB::INSERT( const char * table )
{
sqlBuffer = new char[MAX_SQLBUFFER];
memset(  sqlBuffer , 0 , MAX_SQLBUFFER );
SQLCat( "INSERT INTO " );
SQLCat( table );
SQLCat( "  " );
}

void DB::INTO(const char *fname)
{
 
// zacni zavorkou
if( SQLTestEnd(' ') ) SQLCat(  " ( " ); // zavorka

SQLCat( fname );
SQLCat( " ," );

}

void DB::INTOVAL( const char *fname , const char * value )
{
// pokud josu nejaka data
if( strlen( value ) ) INTO( fname );
}

void DB::VAL(  const char * value)
{
if( strlen( value ) ) VALUE( value );
}

void DB::VALUEUPPER( const char * value  )
{
VALUES( value , false , true , 1 );
}

void DB::VALUELOWER( const char * value  )
{
VALUES( value , false , true , -1 );
}


void DB::VALUES( const char * value  , bool esc , bool amp , int uplo )
{
int len ;

len = strlen( sqlBuffer );


if(  SQLTestEnd(  ';' ) ) // ukonceni 
{
SQLDelEnd();
SQLDelEnd();
SQLCat( ",");
} 
else
{

 if(   SQLTestEnd( ',' )  )  
 {
   SQLDelEnd();
   SQLCat( " ) " ); // uzavri zavorku 
  }

  SQLCat( " VALUES ( " );

}


if( amp ) SQLCat( " '" );
else SQLCat( " " );

// esacepe
if( esc) SQLCatEscape(  value );
else 
{
 
  if( uplo == 0 ) SQLCat( value );
  else if(  uplo > 0 ) SQLCatUpper( value );
       else  SQLCatLower( value );
}

if( amp ) SQLCat( "'");
strcat( sqlBuffer , " );" ); // vzdy ukoncit
 
}

// zadani aktualni cas 
void DB::VALUENOW()
{
VALUES("current_timestamp" , false , false , 0 );
}

// zadani null values
void DB::VALUENULL()
{
VALUES("NULL" , false , false , 0 );
}
// zadani aktualnic as puls interval perido v mesicich
void DB::VALUEPERIOD( int period )
{
char str[80];
// spocitej dobu expirace 
sprintf( str , "current_timestamp + interval\'%d month\' " , period );
VALUES( str , false , false , 0 );
}


void DB::VVALUE( const char * value )
{
// nepouzivej ESCAPE
VALUES( value , false , true , 0 );
}

void DB::VALUE( const char * value )
{
// pouzij ESCAPE 
VALUES( value , true , true , 0  ); 
}

void DB::VALUE( int  value )
{
char numStr[16];
sprintf( numStr , "%d" ,  value );
VALUES( numStr , false , false , 0  ); // bez ESC
}

void DB::VALUE( bool  value )
{
// bez ESC
if( value ) VALUES( "t" , false , true , 0 );
else VALUES( "f" , false , true  , 0);
}


void DB::VALPRICE( long price )
{
char priceStr[16];
// castka v halirich
sprintf( priceStr , "%ld.%02ld" , price /100 ,  price %100 );
VALUES( priceStr , false , false , 0  ); // bez ESC
}
bool DB::EXEC()
{
bool ret;
// proved SQL prikaz
ret =  ExecSQL( sqlBuffer );

// uvolni pamet
delete[] sqlBuffer;
// vrat vysledek
return ret;
}

bool DB::SELECT()
{
bool ret;
// provede select SQL 
ret = ExecSelect( sqlBuffer );

// uvolni pamet
delete[] sqlBuffer;
// vrat vysledek
return ret;
}

// SQL SELECT funkce
void DB::SELECTFROM( const char *fname  , const char * table  )
{
sqlBuffer = new char[MAX_SQLBUFFER];
memset(  sqlBuffer , 0 , MAX_SQLBUFFER );
SQLCat( "SELECT " );
if( strlen( fname )  ) SQLCat( fname );
else SQLCat( " * " ) ;
SQLCat( " FROM " );
SQLCat( table );
SQLCat( " ;" ); // ukoncit
}


bool DB::SELECTONE(  const char * table  , const char *fname ,  const char *value )
{
LOG( SQL_LOG , "SELECTONE  table %s fname %s values %s" , table  , fname ,  value  );
SELECTFROM( "" , table );
WHERE( fname , value );
return SELECT();
}

bool DB::SELECTONE(  const char * table  , const char *fname ,  int value )
{
SELECTFROM( "" , table );
WHERE( fname , value );
return SELECT();
}



bool DB::SELECTOBJECTID(  const char *table , const char *fname ,  int id  )
{
char numStr[16];

sqlBuffer = new char[MAX_SQLBUFFER];
memset(  sqlBuffer , 0 , MAX_SQLBUFFER );
SQLCat( "SELECT * " );
SQLCat( " FROM " );
SQLCat( " object_registry ,  object  , ");
SQLCat( table ); // table
SQLCat( " WHERE Object.id= object_registry.id" ); 
SQLCat( " AND Object.id" );
SQLCat( "=" );
sprintf( numStr , "%d" , id );
SQLCat(  numStr );
SQLCat( " AND " );
SQLCat( "Object.id=");
SQLCat( table ); 
SQLCat( ".id" );
SQLCat( " ;" ); // ukoncit
return SELECT();
}


bool DB::SELECTCONTACTMAP( char *map , int id )
{
char sqlString[512];

sprintf( sqlString , "SELECT  object_registry.name  FROM object_registry JOIN  %s_contact_map ON %s_contact_map.contactid=object_registry.id WHERE %s_contact_map.%sid=%d;" ,  map  , map , map  , map , id );
return ExecSelect( sqlString );
}


