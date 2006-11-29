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

// vraci castku za operaci 
long DB::GetPrice(   int action  ,  int zone , int period  )
{
char sqlString[256];
long p ,  price=0; // cena
int per; // cena za periodu


sprintf(  sqlString ,  "SELECT * FROM price WHERE valid_from < 'now()'  and ( valid_to is NULL or valid_to > 'now()' )  and action=%d and zone=%d;" , action , zone );

if( ExecSelect( sqlString ) )
 {

   if(  GetSelectRows()  == 1 )
     {
       p = get_price( GetFieldValueName( "price" , 0 ) );
       per = atoi( GetFieldValueName( "period" , 0 ) );
       // vypocet ceny
       price = period * p  / per;

       LOG( NOTICE_LOG , "GetPrice action %d zone %d period %d   -> price %ld (units) " , action , zone , period  , price);
     }


   FreeSelect();
  }

return price;
}

// zpracovani creditu strzeni ze zalohe faktury ci dvou faktur
long DB::UpdateInvoiceCredit( int regID ,   int action  , int zone   , int period  )
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
price =  GetPrice( action , zone , period );

// zadna cena (zadarmo) operace se povoluje
if( price == 0 ) return 0;

// zjisti na jakych zalohovych fakturach je volny credit vypis vzdy dve po sobe nasledujici
sprintf( sqlString , "SELECT * FROM credit_invoice_credit_map WHERE registrarid=%d and zone=%d and credit > 0 order by invoice_id limit 2;" , regID , zone );

invoiceID=0;

if( ExecSelect( sqlString ) )
 {
    num = GetSelectRows();

    for( i = 0 ; i  < num ; i ++ )
       {
        invID[i]  = atoi( GetFieldValueName( "invoice_id"  , i ) );
        cr[i]  =  (long) ( 100.0 *  atof( GetFieldValueName("credit" , i ) )  );
       }
     FreeSelect();
 }

if( num  > 0 )
{
credit= cr[0];
invoiceID=invID[0];

if( credit - price > 0 )
  {
     get_priceStr(   priceStr , price  );  
     
     sprintf(  sqlString ,  "UPDATE credit_invoice_credit_map set credit=credit-%s , total=total+%s  where invoice_id=%d;" , priceStr , priceStr , invoiceID );
     if( !ExecSQL( sqlString )   ) {  LOG( CRIT_LOG , "error update   zalohova faktura %d" , invoiceID );  return -1; }  // chyba
   }
else
  {
   if(  num == 2 )   
    {
     get_priceStr(   priceStr , cr[0] );
     invoiceID=invID[0];
     sprintf(  sqlString ,  "UPDATE credit_invoice_credit_map set credit=0 , total=total+%s where invoice_id=%d;" , priceStr , invoiceID );
     if( !ExecSQL( sqlString ) )  { LOG( CRIT_LOG , "error update prvni  zalohova faktura %d" , invoiceID );  return -2; }
     else
        {
             invoiceID=invID[1];                        
             credit = price  - cr[0];
             LOG(   DEBUG_LOG , "pozadovana castka credit0 %ld credit1 %ld price %ld credi %ld" , cr[0] ,  cr[1]  , price  , credit );
             if( cr[1] -  credit  > 0 )
             {
               get_priceStr(  priceStr , credit ); // zmena o cenu minus credit z predchozi zal faktury
               sprintf(  sqlString , "UPDATE credit_invoice_credit_map set credit=credit-%s , total=total+%s  where invoice_id=%d;" , priceStr , priceStr , invoiceID );
               if( !ExecSQL( sqlString ) ) {  LOG( CRIT_LOG , "error update nasleduji   zalohova faktura %d" , invoiceID ); return -3; }
             }
             else {  LOG( CRIT_LOG , "na dalsi zalohove fakture id %d  neni pozadovana castka  %ld credit %ld" ,   invoiceID ,  price  , cr[1] );  return -5 ; }

        }
    }
   else { LOG( CRIT_LOG , "neni uz dalsi zalohova faktura ke zpracovani ");  return -4;  }

  }


}
else { LOG( CRIT_LOG , "nejsou zadne zalohova faktura ke ke zpracovani ");  return -10; }

// vrat cenu pokud vse proslo
return price;
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
int  DB::BeginAction(int clientID , int action ,const char *clTRID  , const char *xml  )
{



// HACK pro PIF

 // umozni  info funkce pro PIF
if( action == EPP_ContactInfo ||  action == EPP_NSsetInfo ||   action ==  EPP_DomainInfo ) 
{
if( clientID == 0 ) { actionID = 0 ; loginID =  0 ;  return 1;  }
}


// actionID pro logovani
actionID = GetSequenceID("action");
loginID = clientID; // id klienta
clientLang = LANG_EN; // default
registrarID=0;

if( actionID ) 
  {
   //  zjisti id prihlaseneho registratora z tabulky Login
   if( clientID ) registrarID= GetNumericFromTable( "LOGIN" , "registrarid" , "id" , clientID ); ;

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
           clientLang = ReturnClientLanguage();

        if( strlen( xml ) )
           {
                INSERT("Action_XML");
                VALUE( actionID );
                VALUE( xml );
                if(  EXEC() ) return registrarID;
           }
        else return registrarID;
     }

  }

return 0;
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
registrarID=0;
LOG( SQL_LOG ,  "EndAction svrTRID: %s" ,  svrTRID );

if( EXEC() ) return   svrTRID;
/* {

   sprintf( sqlString , "SELECT  servertrid FROM Action  WHERE id=%d;"  , id );
   if( ExecSelect( sqlString ) )
    {
      svr =  GetFieldValue( 0 , 0 );
      LOG( SQL_LOG , "GetsvrTRID %s" , svr );   
     //  FreeSelect(); 
    }
  return svr;
 }
*/

else return "svrTRID ERROR";
}
}



// vraci jazyk klienta z tabulky login

int DB::ReturnClientLanguage()
{
int lang = LANG_EN; // default
char sqlString[128];

sprintf( sqlString , "SELECT  lang  FROM  login  WHERE id=%d;" , loginID );

if( ExecSelect( sqlString ) )
 {
     if( GetSelectRows() == 1 ) 
       {
        if(  strcmp(   GetFieldValue( 0 , 0 ) , "cs" ) == 0 ) lang =LANG_CS;
       }
     FreeSelect();
 }

return lang;
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

int  DB::GetContactID( const char *handle )
{
char HANDLE[64];

if( get_HANDLE( HANDLE , handle ) ) // preved handle na velka pismena
 return  GetNumericFromTable( "Contact", "id", "handle", HANDLE );
else return  0 ;
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

bool DB::TestContactHandleHistory( const char * handle , int period )
{
return TestObjectHistory( "CONTACT" , "HANDLE" , handle , period );
}


bool DB::TestNSSetHandleHistory( const char * handle , int period )
{
return TestObjectHistory( "NSSET" , "HANDLE" , handle , period );
}


bool DB::TestDomainFQDNHistory( const char * fqdn , int period )
{
return TestObjectHistory( "DOMAIN" , "FQDN" , fqdn , period );
}

// test na objekty v historii
bool DB::TestObjectHistory( const char *table , const char *fname ,  const char * name , int period )
{
char sqlString[512];
bool ret=false;
int count;

if( period > 0 )
{
sprintf( sqlString , "SELECT count(*)  FROM %s_history , history WHERE \
  %s_history.%s=\'%s\' AND %s_history.historyid=history.id  AND \
  history.moddate > current_timestamp - interval\'%d month';" , 
     table , table , fname , name , table,  period );

if( ExecSelect( sqlString ) )
  {
     count = atoi(  GetFieldValue( 0 , 0 ) ) ;
     if( count > 0 )  ret=true ;
     FreeSelect();
  }

return ret;
}
else return false;

}



int DB::CreateObject( const char *type , int regID , const char *name , const char *authInfoPw )
{
char roid[64];
char pass[PASS_LEN+1];
int id;

id = GetSequenceID( "object" );

 // vytvor roid 
 get_roid( roid, ( char * ) type , id );

 INSERT( "OBJECT" );
 INTO( "id" );
 INTO( "type" );
 INTO( "roid" );
 INTO( "NAME" );
 INTO( "CrDate" );
 INTO( "CrID" );
 INTO( "ClID" );
 INTO( "AuthInfoPw" );

 VALUE( id );

// TYP objektu
switch( type[0] )
  {
    case 'C' :
         VALUE( 1 );
         break;
    case 'D' :
         VALUE( 3 );
         break;
    case 'N' :
         VALUE( 2 );
         break;
   default :
         VALUE( 0);
  }
    
 VALUE( roid );
 VALUE( name  );
 VALUENOW();
 VALUE( regID );
 VALUE( regID );

                     if( strlen ( authInfoPw ) == 0 )
                       {
                          random_pass(  pass  ); // autogenerovane heslo pokud se heslo nezada
                          VVALUE( pass );
                        }
                      else VALUE( authInfoPw );


if( EXEC() )return id;
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


bool  DB::CheckContactMap(char * table , int id , int contactid )
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



bool DB::GetExpDate(char *dateStr , int domainID , int period  , int max_period )
{
char sqlString[256];
bool ret=false;
strcpy( dateStr , "" ); // default datum

sprintf( sqlString , "SELECT  ExDate+interval\'%d month\' FROM DOMAIN WHERE id=%d AND ExDate+interval\'%d month\' < current_date+interval\'%d month\';",
           period , domainID , period , max_period );


if( ExecSelect( sqlString ) )
 {
    if(  GetSelectRows() == 1  )
      {
        convert_rfc3339_timestamp( dateStr ,     GetFieldValue( 0 , 0 ) );        
        ret = true;
      }
    FreeSelect();
  }

return ret;
}


int DB::CheckNSSet(const char *handle )
{
return CheckHandle( "NSSET" , handle );
}

int DB::CheckContact(const char *handle )
{
return CheckHandle( "CONTACT" , handle );
}



// test pri Check funkci  na handle nssetu ci kontaktu
// vracena hodnota: -1 chyba 0 objekt eexistuje 1 onbjekt existuje
int DB::CheckHandle( const char *table ,     const char *handle )
{
char sqlString[128];
int  id=0 ;

sprintf( sqlString , "SELECT id FROM %s  WHERE handle=\'%s\';" , table ,  handle );

if( ExecSelect( sqlString ) )
 {


   if(  GetSelectRows()  == 1 )
     {
       id = atoi(  GetFieldValue( 0 , 0 )  );
       LOG( SQL_LOG , "Check table %s handle=\'%s\'  -> ID %d" , table , handle , id );
     }


   FreeSelect();
  }

return id;
}

// pro select domeny pri info 
bool DB::SELECTDOMAIN(   const char *fqdn , int zone , bool enum_zone )
{
int id;

if( enum_zone ) 
{
id = CheckDomain(  fqdn ,  zone , true );
return SELECTONE( "DOMAIN" , "id" , id );
}
else return  SELECTONE( "DOMAIN" , "fqdn" , fqdn  );

}  

// test pri Check funkci na domenu case insensitiv
int DB::CheckDomain(   const char *fqdn , int zone , bool enum_zone )
{
char sqlString[512];
int id=0;



if( enum_zone )sprintf( sqlString , "SELECT id FROM domain  WHERE  ( \'%s\' LIKE  \'%%\'||fqdn) OR  (fqdn LIKE  \'%%\'  || \'%s\' ) AND zone=%d;"  , fqdn   , fqdn  , zone );
else  sprintf( sqlString , "SELECT id FROM domain  WHERE   \'%s\'  ILIKE fqdn;" , fqdn );
 

if( ExecSelect( sqlString ) )
 {
   id = atoi(  GetFieldValue( 0 , 0 )  );
   LOG( SQL_LOG , "Check domain  fqdn=\'%s\'  -> ID %d" , fqdn ,  id );

   FreeSelect();
  }

return id;
}


// vraci ID hostu
int DB::CheckHost(  const char *fqdn , int nssetID )
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
bool ret = false;
char sqlString[128];

sprintf( sqlString , "SELECT id from DOMAIN WHERE Registrant=%d;" , id );
if( ExecSelect( sqlString ) ) 
  {
     if(  GetSelectRows() > 0 ) ret=true;  // nejaka domena kterou ma kontakt registrovany
     FreeSelect();
  }

sprintf( sqlString , "SELECT * from DOMAIN_CONTACT_MAP WHERE contactid=%d;" , id );
if( ExecSelect( sqlString ) )
  {
     if(  GetSelectRows() > 0 ) ret=true; // kontakt je admin kontakt nejake domeny 
     FreeSelect();
  }

sprintf( sqlString , "SELECT * from NSSET_CONTACT_MAP WHERE contactid=%d;" , id );
if( ExecSelect( sqlString ) )
  {
     if(  GetSelectRows() > 0 ) ret=true; // kontakt je tech kontakt nejakeho nssetu
     FreeSelect();
  }


return ret;
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
bool DB::SaveCredit(  int regID  , int zone  ,  long credit  ,int invoiceID )
{

            // uloz credit
           INSERT( "credit_invoice_credit_map" );
           INTO( "registrarid" );
           INTO( "zone" );
           INTO( "credit" );
           INTO( "total" );
           if( invoiceID ) INTO( "invoice_id" );

           VALUE( regID );
           VALUE( zone );
           VALPRICE( credit );
           VALPRICE( 0 );
           if( invoiceID ) VALUE( invoiceID );

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
int  DB::MakeAInvoice( const char *prefixStr  , int regID , long price , int vatNum , long vat ,  long credit )
{
int invoiceID;

// castka bez DPh + DPH by se mela rovanat castce s dani 
if( (vat + credit)  == price ) 
{

           invoiceID = GetSequenceID( "invoice" );

           INSERT( "invoice" );
           INTO( "id" );
           INTO( "prefix" );
           INTO( "typ" );
           INTO( "registarid" );
           INTO( "price" );
           INTO( "numvat" );
           INTO( "vat" );
           INTO( "total" );
           VALUE( invoiceID );
           VALUE( prefixStr  );
           VALUE( 1 ); // zalohova FA
           VALUE( regID );
           VALPRICE( price );

           VALUE( vatNum );
           VALPRICE( price  );
           VALPRICE( credit ); // cena bez dane
          
           if(  EXEC() ) return invoiceID;
}
          
return 0;
} 



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

int DB::SaveNSSetHistory( int id )
{
int history_ID;

 //  uloz do historie
 if(  ( history_ID =MakeHistory() ) )
   {

      if( SaveHistory( "NSSET", "id", id ) )        
           if( SaveHistory( "HOST", "nssetid", id ) )
              if( SaveHistory( "HOST_IPADDR_map", "nssetid", id ) ) 
                    if( SaveHistory(  "nsset_contact_map", "nssetid", id ) )  // historie tech kontakty
                           return history_ID;
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

int DB::SaveDomainHistory( int id )
{
int history_ID;
                          //  uloz do historie
 if(  ( history_ID =MakeHistory() ) )
   {
     if( SaveHistory( "DOMAIN" , "id", id ) )  
        if( SaveHistory( "domain_contact_map", "domainID", id ) )       // uloz admin kontakty
                   if( SaveHistory( "enumval",  "domainID", id ) )  // uloz extension
                                   return history_ID;
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
                                   
 

int DB::SaveContactHistory( int id )
{
int history_ID;
                          //  uloz do historie
 if(  ( history_ID =MakeHistory() ) )
   {
     if( SaveHistory(  "Contact", "id", id ) )   return history_ID;
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



int DB::MakeHistory() // zapise do tabulky history
{
char sqlString[128];

historyID = 0 ;

if( actionID )
 {   
   LOG( SQL_LOG , "MakeHistory actionID -> %d " , actionID);
   historyID = GetSequenceID( "HISTORY" );
   if( historyID )
    {
     LOG( SQL_LOG , "MakeHistory actionID -> %d " , actionID); 
     sprintf(  sqlString , "INSERT INTO HISTORY ( id , action ) VALUES ( %d  , %d );" , historyID , actionID );
     if( ExecSQL(  sqlString ) ) return historyID;
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
           if( IsNotNull( row , i ) ) INTO(  GetFieldName( i ) ); 
          }

      VALUE( historyID ); // zapis jako prvni historyID
      for( i = 0 ; i <  GetSelectCols() ; i ++ )
      {
        // uloz pouze ne null hosdnoty
        if( IsNotNull( row , i ) ) VALUE(  GetFieldValue( row  , i ) );
       }

        if( EXEC() == false) { ret = false ; break;  } // uloz hostorii a  pokud nastane chyba

      }

     FreeSelect();
   }

}

// default 

return ret;
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

void DB::WHEREOPP(  const  char *op ,  const  char *fname , const  char *p  , const  char * value )
{


  if(  SQLTestEnd( ';' )  )
    {
       SQLDelEnd(); // umaz posledni strednik
       SQLCat( "  "  );
       SQLCat(  op ); // operator AND OR LIKE
       SQLCat( " " );
       SQLCat( fname );
       SQLCat(  p );
       SQLCat(  "'" );
       SQLCatEscape(  value );
       SQLCat(  "' ;" ); // konec         
    }

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


void DB::VALUES( const char * value  , bool esc , bool amp )
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
else  SQLCat( value );

if( amp ) SQLCat( "'");
strcat( sqlBuffer , " );" ); // vzdy ukoncit
 
}

// zadani aktualni cas 
void DB::VALUENOW()
{
VALUES("current_timestamp" , false , false );
}

// zadani null values
void DB::VALUENULL()
{
VALUES("NULL" , false , false );
}
// zadani aktualnic as puls interval perido v mesicich
void DB::VALUEPERIOD( int period )
{
char str[80];
// spocitej dobu expirace 
sprintf( str , "current_timestamp + interval\'%d month\' " , period );
VALUES( str , false , false );
}


void DB::VVALUE( const char * value )
{
// nepouzivej ESCAPE
VALUES( value , false , true);
}

void DB::VALUE( const char * value )
{
// pouzij ESCAPE 
VALUES( value , true , true ); 
}

void DB::VALUE( int  value )
{
char numStr[16];
sprintf( numStr , "%d" ,  value );
VALUES( numStr , false , false ); // bez ESC
}

void DB::VALUE( bool  value )
{
// bez ESC
if( value ) VALUES( "t" , false , true);
else VALUES( "f" , false , true );
}


void DB::VALPRICE( long price )
{
char priceStr[16];
// castka v halirich
sprintf( priceStr , "%ld.%02ld" , price /100 ,  price %100 );
VALUES( priceStr , false , false ); // bez ESC
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


bool DB::SELECTCONTACTMAP( char *map , int id )
{
char sqlString[512];

sprintf( sqlString , "SELECT  handle FROM CONTACT  JOIN  %s_contact_map ON %s_contact_map.contactid=contact.id WHERE %s_contact_map.%sid=%d;" ,  map  , map , map  , map , id );
return ExecSelect( sqlString );
}


