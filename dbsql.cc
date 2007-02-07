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

#define INVOICE_FA  1 // typ vyuctovaci faktura
#define INVOICE_ZAL 0 // typ zalohova faktura


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
long p ,  price=0; // cena default -1 nepouziva se 
int per; // cena za periodu


if( period > 0 ) sprintf(  sqlString ,  "SELECT price , period FROM price_list WHERE valid_from < 'now()'  and ( valid_to is NULL or valid_to > 'now()' )  and operation=%d and zone=%d;" , operation , zone );
// cena pro jednorazove operace
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
    else price=-1; 
  
   FreeSelect();
return price;
  }
else return -2; // ERROR
}

// ukladani polozky creditu
bool DB::SaveInvoiceCredit(int regID , int objectID , int operation , int zone  , int period , const char *ExDate , long price  , long price2 ,  long balance , long balance2 ,  int invoiceID   , int invoiceID2) 
{
int id;


LOG( DEBUG_LOG , "SaveInvoiceCredit: uctovani creditu objectID %d ExDate [%s] regID %d" , objectID ,  ExDate , regID  );

id = GetSequenceID( "invoice_object_registry" );

// uloz zaznam o zuctovanem creditu
INSERT( "invoice_object_registry" );
INTO( "id" );
INTO( "objectid" );
INTO( "registrarid"  );
INTO( "operation" );
INTO( "zone" );
INTO( "period" );
INTOVAL( "ExDate" , ExDate); // pokud je nastaveno EXDate
VALUE( id );
VALUE( objectID );
VALUE( regID );
VALUE( operation );
VALUE( zone );
VALUE( period );
VAL( ExDate);
if( EXEC() ) 
 {

   LOG( DEBUG_LOG , "SaveInvoiceCredit:   price %ld  balance credit %ld invoiceID %d" , price , balance,  invoiceID );

  INSERT( "invoice_object_registry_price_map" );
  INTO( "id");
  INTO( "invoiceID" );
  INTO( "price" );
  INTO( "price_balance" );
  VALUE( id );
  VALUE( invoiceID );
  VALPRICE( price);
  VALPRICE( balance );
  if( !EXEC() ) { LOG( CRIT_LOG , "ERROR invoice_object_registry_price_map" ); return false; }
    

  if( price2 ) // uloz druhou cenu
   {  
     LOG( DEBUG_LOG , "uctovani creditu  price2 %ld  balance2 %ld invoiceID2 %d" , price2 , balance2 ,  invoiceID2 );

     INSERT( "invoice_object_registry_price_map" );
     INTO( "id");
      INTO( "invoiceID" );
      INTO( "price" );
      INTO( "price_balance" );
      VALUE( id );
      VALUE( invoiceID2 );
      VALPRICE( price2);
      VALPRICE( balance2 );
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

bool DB::InvoiceCountCredit( long  price , int invoiceID  )
{
char sqlString[256];

// pokud nulova cene neupdavovat credit
if( price == 0 ) {  LOG( DEBUG_LOG , "nulova castka nemenim credit u invoiceID %d"  , invoiceID ); return true; } 
else
{
sprintf(  sqlString ,  "UPDATE invoice SET  credit=credit-%ld%c%02ld  WHERE id=%d;" , price /100 , '.' , price %100   , invoiceID );
if( ExecSQL( sqlString )   ) return true;
else { LOG( CRIT_LOG , "error InvoiceCountCredit invoice  %d price %ld" , invoiceID  , price ); return false ; }    
}

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
char sqlString[256];
long price , credit ;
long price1 , price2;
int invoiceID;
int invID[2];
long cr[2];
int i , num;

LOG( DEBUG_LOG , "UpdateInvoiceCredit operation %d objectID %d ExDate [%s]  period %d regID %d" , operation , objectID ,  ExDate , period ,  regID  );

// systemovy registrator pracuje zadarmo
if( GetRegistrarSystem( regID ) == true ) return true;


// zjisti  cenu za operaci v registru musibyt >=0 
price =  GetPrice( operation , zone , period );

if( price == -2 ) return false; // chyba SQL

// neni zadana ( cena je zadarmo) operace se povoluje a neuctuje 
if( price == -1 ) return true;

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


// nejsou zadne zalohove faktury na zpracovani nemuze odecitat CREDIT
if( num == 0 ) return false;


credit= cr[0];
invoiceID=invID[0];

if( credit - price > 0 )
  {

     if( SaveInvoiceCredit(  regID , objectID ,   operation  ,  zone   , period , ExDate , price , 0 , credit - price  , 0 ,  invoiceID , 0  )  )
      {
       return InvoiceCountCredit( price , invoiceID );
      }

   }
else
  {
   
   if(  num == 2 )   // pokud existuje dalsi faktura
   {
    
       price1= cr[0]; // zustatek na prvni zal FA
       price1= price - cr[0]; // zbytek penez ztrhava se z druhe zal FA

       if( SaveInvoiceCredit(  regID ,objectID , operation  ,  zone , period ,  ExDate , price1 ,  price2 ,  0 , cr[1] - price2 , invID[0] , invID[1]  )  ) 
         {
         if( InvoiceCountCredit(  cr[0] , invID[0] )  ) // dopocitad do nuly na prvni zalohove FA  
           {
             invoiceID=invID[1];                        
             LOG(   DEBUG_LOG , "pozadovana castka credit0 %ld credit1 %ld price %ld druha castka %ld" , cr[0] ,  cr[1]  , price1  , price2 );
              
             if( cr[1] -  credit  > 0 )
             {
                     // castka ponizena o zbyvajici credit na privni zalohove FA
                   return InvoiceCountCredit(  price  - cr[0] , invID[1] );
                
             }else   LOG( CRIT_LOG , "na dalsi zalohove fakture id %d  neni pozadovana castka  %ld credit %ld" ,   invoiceID ,  price  , cr[1] );

          }

      } 
 
   }else  LOG( ALERT_LOG , "neni uz dalsi zalohova faktura ke zpracovani ");    
 
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


// systemovy registrator pracuje zadarmo
if( GetRegistrarSystem( regID ) == true ) return true; // ma prava dany objekt menit
else
{
sprintf( sqlString , "SELECT id FROM  object WHERE id=%d and clID=%d " , id , regID ); 
if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() == 1 ) ret = true;
     FreeSelect();
 }

return ret;
}

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


// vraci tru pokud je to systemovy regostrator
bool DB::GetRegistrarSystem( int regID )
{
char sqlString[128];
bool ret=false;

sprintf( sqlString , "SELECT system FROM registrar where id=%d;" , regID );
if( ExecSelect( sqlString ) )
 {
    if(  GetSelectRows() == 1  )
      {
         ret =  GetFieldBooleanValueName( "system" , 0 );

      }
   FreeSelect();
 }

         if( ret ) LOG(  SQL_LOG , "GetRegistrarSystem TRUE" );
         else LOG(  SQL_LOG , "GetRegistrarSystem FALSE");


return ret;
}


// testuje pravo registratora na zapis do zony  podle toho odkdy ma zacatek fakturace pro danou zonu 
// TODO ukonceni cinnosti registratora
bool DB::TestRegistrarZone(int regID , int zone )
{
bool ret = false;
char sqlString[128];

// sytemovy registrator ma prava pro vsechny zony
if( GetRegistrarSystem( regID ) == true ) return true;

sprintf( sqlString , "SELECT  id  FROM  registrarinvoice  WHERE registrarid=%d and zone=%d and  current_timestamp > fromdate;" ,  regID , zone );



if( ExecSelect( sqlString ) )
 {
    if(  GetSelectRows() == 1  )
      {
            ret=true;
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

double DB::GetSystemKOEF() // vraci koeficient pro prepocet DPH
{
char sqlString[128] = "select koef   from price_vat where valid_to > now() or valid_to is null;" ;
double koef;

if( ExecSelect( sqlString ) )
  {
    if( GetSelectRows() == 1 )
      {
         koef = atof( GetFieldValue( 0 , 0 ) );
      }
    FreeSelect();
  }

return koef;
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

int  DB::GetBankAccount( const char *accountStr ,  const char *codeStr   )
{
char sqlString[128];
int accountID=0;

  LOG( LOG_DEBUG ,"GetBankAccount account %s , code %s" ,  accountStr ,codeStr   );
sprintf( sqlString , "SELECT id FROM bank_account WHERE account_number=\'%s\' AND bank_code=\'%s\';" , accountStr ,codeStr   );

if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() == 1   )
    {
        accountID=atoi( GetFieldValue( 0 , 0 ) );
       LOG( LOG_DEBUG ,"get accountId %d" , accountID );             
    }
 FreeSelect();
}
return accountID;
}


int DB::GetBankAccountZone( int accountID )
{
char sqlString[128];
int zone=0;

LOG( LOG_DEBUG ,"GetBankAccountZone accountID %d" , accountID );
sprintf( sqlString , "SELECT  zone  FROM bank_account WHERE id=%d" , accountID );

if( ExecSelect( sqlString ) )
 {
   if( GetSelectRows() == 1   )
    {
        zone = atoi( GetFieldValue( 0 , 0 ) );
        LOG( LOG_DEBUG ,"get zone %d" , zone );
    }
 }

return zone;
}

// test zustatku na uctu pro import bankovniho vypisu
int DB::TestBankAccount( const  char *accountStr , int num , long oldBalance )
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


                
// nastav bankovni vypis jako zpracovany
bool DB::UpdateBankStatementItem( int id , int invoiceID)
{
                  UPDATE( "bank_statement_item" );
                  SET( "invoice_id" , invoiceID );
                  WHEREID(  id  );
                  return EXEC();
}  

int DB::TestEBankaList( const char *ident )
{
char sqlString[128] ;
int id=0;

sprintf( sqlString ,  "SELECT  id    from BANK_EBANKA_LIST where ident=\'%s\'"  , ident );

if( ExecSelect( sqlString ) )
  {
    if( GetSelectRows() == 1 )
      { 
          id=   atoi( GetFieldValue( 0 , 0 ) );

      }
    FreeSelect();
  }

return id;
}
                        // identifikator , castka ,  datum a , cislo ucto , kod banky ,  VS , KS  , nazev uctu , poznamka
int DB::SaveEBankaList( int account_id , const char *ident , long  price , const char *datetimeStr ,  const char *accountStr , const char *codeStr , 
                        const char *varsymb , const char *konstsymb ,  const char *nameStr ,  const char *memoStr )
{
int ID;

// pokud jeste neni nacteny 
if( TestEBankaList( ident ) == false )
  {

     ID = GetSequenceID( "bank_statement_head" );

                    INSERT( "BANK_EBANKA_LIST" );
                    INTO("id");
                    INTO( "account_id" );
                    INTO( "account_number" );
                    INTO( "bank_code" );
                    INTO( "konstsym" );
                    INTO( "varsymb" );
                    INTO( "memo");
                    INTO( "name");
                    INTO( "ident" );
                    INTO( "crdate");
                    INTO( "price" );
                    VALUE( ID );
                    VALUE( account_id );
                    VALUE( accountStr );
                    VALUE( codeStr );
                    VALUE( konstsymb );
                    VALUE( varsymb  );
                    VALUE( memoStr );
                    VALUE( nameStr );
                    VALUE( ident );
                    VALUE( datetimeStr );
                    VALPRICE( price );

   if( EXEC () ) return ID;
   else return -1; // chyba
 }
else return 0;
}

// nastav vypis ebanky jako zpracovany na zalohovou FA
bool DB::UpdateEBankaListInvoice( int id , int invoiceID )
{
                  UPDATE( "BANK_EBANKA_LIST" );
                  SET( "invoice_id" , invoiceID );
                  WHEREID(  id  );
                  return EXEC();

}

int DB::MakeFactoring(  int regID , int zone , const char *timestampStr ,  const char *taxDateStr )
{
char sqlString[512];
int invoiceID=-1;
char fromtimeStr[32];
int count=-1;
long price;


 LOG( NOTICE_LOG , "MakeFactoring regID %d zone %d" , regID , zone );



            // zjisti fromdate nebo lasdate z tabulky registrarinvoice od kdy fakturovat 
            sprintf( sqlString , "SELECT lastdate ,  fromdate  from registrarinvoice  WHERE zone=%d and registrarid=%d;"  , zone , regID );
             if( ExecSelect( sqlString ) )
               {

                   if( IsNotNull( 0 , 0 ) )
                     {
                         strcpy( fromtimeStr , GetFieldValue( 0 , 0 ) );
                      }
                    else strcpy( fromtimeStr ,  GetFieldValue( 0 , 1 ) );

                 FreeSelect();
               }else return -1; // chyba

            LOG( NOTICE_LOG , "Fakturace od %s do %s" , fromtimeStr  , timestampStr );





                // zjisti pocet polozek k fakturaci
               sprintf( sqlString , "SELECT count( id)  from invoice_object_registry  where crdate < \'%s\' AND  zone=%d AND registrarid=%d AND invoiceid IS NULL;" , timestampStr , zone , regID );
               if( ExecSelect( sqlString ) )
                {
                     count = atoi(  GetFieldValue( 0 , 0 ) ) ;
                     FreeSelect();
                }else return -2; // chyba


                // zjisti celkovou fakturovanou  castku pokud je alespon jeden zaznam
               if( count > 0 )
                 {
                     sprintf( sqlString , "SELECT sum( price ) FROM invoice_object_registry , invoice_object_registry_price_map  WHERE   invoice_object_registry_price_map.id=invoice_object_registry.id AND  crdate < \'%s\' AND zone=%d and registrarid=%d AND  invoice_object_registry.invoiceid is null ;" ,  timestampStr , zone , regID );
                     if( ExecSelect( sqlString ) )
                       {
                            price =  (long) ( 100.0 *  atof( GetFieldValue( 0  , 0 ) )  );
                            LOG( NOTICE_LOG , "Celkova castka na fakture %ld" , price );
                            FreeSelect();
                       }
                 }
                else price = 0; // jinak nulova castka


             
            // prazdna faktura zaznam o fakturaci
           // vraci invoiceID nebo nulo pokud nebylo nic vyfakturovano pri chybe vraci zaporne cislo chyby
                if( ( invoiceID = MakeNewInvoice(  taxDateStr , fromtimeStr , timestampStr , zone , regID   , price  , count )  ) >= 0 )
                  {

                    if( count > 0 )    // oznac polozky faktury faktrury
                      {
                          sprintf( sqlString , "UPDATE invoice_object_registry set invoiceid=%d  WHERE crdate < \'%s\' AND zone=%d and registrarid=%d AND invoiceid IS NULL;" ,  invoiceID , timestampStr , zone , regID );
                          if( ExecSQL( sqlString )  == false ) return -3; // chyba

                       }

                    // pokud byla vytvorena faktura 
                     if( invoiceID > 0 )  
                       {
                        // set last date do tabulky registrarinvoice
                         sprintf( sqlString , "UPDATE registrarinvoice SET lastdate=\'%s\' WHERE zone=%d and registrarid=%d;" , timestampStr , zone , regID );
                         if( ExecSQL( sqlString )   == false ) return -4; //chyba
                       }
 

                           
                  }else return -5; // nepodarilo se vytvorit fakturu

               

return invoiceID;                   
}

            


int  DB::MakeNewInvoice(  const char *taxDateStr , const char *fromdateStr , const char *todateStr , int zone ,  int regID ,  long price , unsigned int count  )
{
int invoiceID;
int prefix;
int type;

type = GetPrefixType( taxDateStr , INVOICE_FA , zone );  // id prefixu OSTRA FA VYUCTOVACI 
prefix = GetInvoicePrefix( taxDateStr , INVOICE_FA , zone );  // cislo faktury podle zdanitelneho obdobi
LOG( LOG_DEBUG ,"MakeNewInvoice taxdate[%s]  fromdateStr [%s] todateStr[%s]  zone %d regID %d , price %ld  " , 
taxDateStr , fromdateStr , todateStr ,   zone , regID , price );

if( prefix  > 0  && type > 0 )
{


     
        if( count ) // vytvor fakturu 
          {
           invoiceID = GetSequenceID( "invoice" );

           INSERT( "invoice" );
           INTO( "id" );
           INTO( "prefix" );
           INTO( "zone" );
           INTO( "prefix_type" );
           INTO( "registrarid" );
           INTO( "taxDate" );  
           INTO( "price" );
           INTO( "vat" );
           INTO( "total" );           
           INTO( "totalVAT" );           
           INTO( "credit" );
           VALUE( invoiceID );
           VALUE( prefix  );
           VALUE( zone );
           VALUE( type );  // odkaz do prefixu
           VALUE( regID );
           VALUE( taxDateStr );
           VALPRICE( price ); // celkova castka
           VALUE( 0 ); // dan je nulova
           VALPRICE( price ); // zaklad bez dane total stejny jako price
           VALUE( 0);
           VALUENULL(); // pouze credit je NULL

          
           if(  !EXEC() )  return -1; // chyba SQL insert
          }else invoiceID=0; // prazdna fakturace
           

// zaznam o fakturaci
           INSERT( "invoice_generation");
          
           INTO( "fromdate");
           INTO( "todate");
           INTO( "registrarid" );
           INTO( "zone" );

           INTO( "invoiceID" );
           VALUE( fromdateStr );
           VALUE( todateStr ); 
           VALUE( regID );
           VALUE( zone );
           if( invoiceID  ) VALUE( invoiceID );
           else   VALUENULL(); 

          if( EXEC() )return invoiceID; // chyba v insertu
          else return -3; // chyba 


}else return -2;  // chyba ve vytvoreni  prefixu 


}
// vytvoreni zalohove faktury pro registratora na castku price s vysi DPH vatNum odvedenou dani vat  a castkou bez DPH credit
// taxDateStr  datum zdanitelneho plneni datum kdy castka prisla na nas ucet
int  DB::MakeNewInvoiceAdvance( const char *taxDateStr , int zone ,  int regID ,  long price , bool VAT )
{
int invoiceID;
int prefix;
int dph;
long total; // castka bez DPH == credit
long credit;
long totalVAT; // odvedene DPH;
double koef; // prepocitavaci koeficinet pro DPH
int type; // typ ZAL FA z rady

  if( VAT)
    {
     // zjisti vysi DPH
     dph =GetSystemVAT(); // vyse DPH 19 %
     koef =GetSystemKOEF();// vyse koeficientu
     // cpocte odvedena DPH zaoKROUHLENE MATEMaticky na desetniky

       totalVAT =  count_dph( price , koef );         
       total = price - totalVAT;
       credit =  total;

    }
   else // vytvori zalohovou fakturu bezodvodu DPH
   {
     dph=0;
     totalVAT=0;
     total = price;
     credit = price;
   }


     type = GetPrefixType( taxDateStr , INVOICE_ZAL , zone );
  
     prefix = GetInvoicePrefix( taxDateStr , INVOICE_ZAL , zone );

 LOG( LOG_DEBUG ,"MakeNewInvoiceAdvance taxdate[%s]   zone %d regID %d , price %ld dph %d credit %ld\n" , taxDateStr ,  zone , regID , price , dph , credit );
 
     if( prefix  > 0 )
       {
           invoiceID = GetSequenceID( "invoice" );

           INSERT( "invoice" );
           INTO( "id" );
           INTO( "prefix" );
           INTO( "zone" );
           INTO( "prefix_type" );
           INTO( "registrarid" );
           INTO( "taxDate" );  
           INTO( "price" );
           INTO( "vat" );
           INTO( "total" );           
           INTO( "totalVAT" );           
           INTO( "credit" );
           VALUE( invoiceID );
           VALUE( prefix  );
           VALUE( zone );
           VALUE( type );  // cislo rady z tabulky invoice_prefix
           VALUE( regID );
           VALUE( taxDateStr );
           VALPRICE( price ); // celkova castka
           VALUE( dph ); // vyse dph 19 %
           VALPRICE( total ); // castka bez DPH
           VALPRICE( totalVAT ); // castka bez DPH
           VALPRICE( credit ); // pripocteny credit
          
           if(  EXEC() ) return invoiceID;
           else return -1; // chyba SQL insert
      }
      else return -2;  // chyba prefixu 
          
} 


int DB::GetPrefixType( const char *dateStr , int typ , int zone )
{
char sqlString[512];
int year;
char yearStr[5];
int id=0;

// rok
strncpy( yearStr , dateStr  , 4 );
yearStr[4] = 0 ;
year= atoi( yearStr);
LOG( LOG_DEBUG ,"GetPrefixType  date[%s]  year %d typ %d zone %d\n" , dateStr , year ,  typ ,  zone  );

sprintf( sqlString , "SELECT id  FROM invoice_prefix WHERE zone=%d AND  typ=%d AND year=\'%s\';",  zone , typ, yearStr );
if( ExecSelect( sqlString )  )
  {
            if(  GetSelectRows() == 1 )
              {
                    id = atoi( GetFieldValue( 0  , 0 ) );
                    LOG( LOG_DEBUG ,"invoice_id type-> %d" , id );
               }
     FreeSelect();
 }

return id;
}

int DB::GetInvoicePrefix( const char *dateStr , int typ , int zone )
{
char sqlString[512];
int year;
char yearStr[5];
int prefix=0 , id=0;

// rok
strncpy( yearStr , dateStr  , 4 );
yearStr[4] = 0 ;
year= atoi( yearStr);

LOG( LOG_DEBUG ,"GetInvoicePrefix date[%s]  year %d typ %d zone %d\n" , dateStr , year ,  typ ,  zone  ); 

sprintf( sqlString , "SELECT id , prefix   FROM invoice_prefix WHERE zone=%d AND  typ=%d AND year=\'%s\';",  zone , typ, yearStr );

if( ExecSelect( sqlString )  )
  {
            if(  GetSelectRows() == 1 )
              {
                    id = atoi( GetFieldValueName("id"  , 0 ) );
                    prefix =  atoi( GetFieldValueName("prefix" , 0 ) );
                    LOG( LOG_DEBUG ,"invoice_prefix id %d -> %d" ,  id , prefix  );
              }
            else return -3; // chyba

     FreeSelect();

     UPDATE( "invoice_prefix" );
     SET( "prefix" , prefix +1 );
     WHEREID( id );
     if( EXEC() ) return prefix;
     else return -2; // chyba
          

  }
 else return -1;

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

bool DB::ObjectModify( int id )
{
UPDATE( "OBJECT" );
SSET( "UpDate", "now" ); // MOD date
WHEREID( id );
return EXEC();
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


void DB::SETNULL( const char *fname  )
{
SQLCat( "  ");
SQLCat(  fname );
SQLCat(  "=NULL" );
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


