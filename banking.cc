#include <sstream>
#include <stdio.h>
#include "gpc.h"

#include "conf.h"
#include "dbsql.h"

#include "util.h"
#include "log.h"
#include "csv.h"


#ifndef CONFIG_FILE
#define CONFIG_FILE "/etc/fred/server.conf"
#endif



// make advane invoice from bank statement
struct invBANK
{
int id;
int regID;
long price;
int zone;
char prefix[25]; // prefix of the advance invoice 
};



// manual update credit of registrar
bool credit_invoicing(const char *database  ,   const char  *dateStr , const char *registrarHandle ,  const char  *zone_fqdn , long  price )
{
DB db;
int regID;
int invoiceID;
int zone;
int ret = 0;

if( db.OpenDatabase( database  ) )
{

LOG( LOG_DEBUG , "successfully  connect to DATABASE %s"  , database);

if( db.BeginTransaction() )
 {

  if(  (regID = db.GetRegistrarID( (char * )  registrarHandle ) )  ) 
  {

    if( ( zone =  db.GetNumericFromTable( "zone", "id", "fqdn", zone_fqdn ) )  )
     {     
              

          // make advance invoice with credit price
           invoiceID =  db.MakeNewInvoiceAdvance( dateStr ,  zone , regID , price , true  );

           if( invoiceID > 0 ) ret = CMD_OK;

      }
     else LOG( LOG_ERR , "unkow zone %s\n" , zone_fqdn );
  }
  else
  {
     LOG( LOG_ERR , "unkow registrarHandle %s" , registrarHandle );          
  }

    db.QuitTransaction( ret );
}

db.Disconnect();
}
 
if( ret ) return true;
else return false;
}

bool factoring_all(  const char *database  ,   const char  *zone_fqdn , char *taxdateStr , char *todateStr )
{
DB db;
int *regID;
int i , num =-1; 
char timestampStr[32];
int invoiceID;
int zone;
int ret = 0;

if( db.OpenDatabase( database  ) )
{

LOG( LOG_DEBUG , "successfully  connect to DATABASE %s"  , database);

if( db.BeginTransaction() )
 {
  get_timestamp( timestampStr  , get_utctime_from_localdate(  todateStr ) );

 if( ( zone =  db.GetNumericFromTable( "zone", "id", "fqdn", zone_fqdn ) ) )
  {
    std::stringstream sql;
    sql << "SELECT r.id FROM registrar r, registrarinvoice i WHERE r.id=i.registrarid "
        << "AND r.system=false AND i.zone=" << zone << " AND i.fromdate<=CURRENT_DATE";
   if( db.ExecSelect(sql.str().c_str() ) )
    {
        num =  db.GetSelectRows();

        if( num > 0 )
        {
          regID= new int[num];

          for( i = 0 ; i < num ; i ++ )
             {
               regID[i] = atoi( db.GetFieldValue( i  , 0 ) );
             }
        }
       db.FreeSelect();


        if( num > 0 )
         {
             for( i = 0 ; i < num ; i ++ )
                {
                    invoiceID = db.MakeFactoring( regID[i] , zone , timestampStr  , taxdateStr );
                    LOG( NOTICE_LOG , "Vygenerovana fa %d pro regID %d" , invoiceID ,  regID[i]  );

                    if( invoiceID >=0 ) ret = CMD_OK;
                    else { ret = 0 ; break; }
                       
                }
           delete[] regID;
        }

    }
  } else LOG( LOG_ERR , "unkow zone %s\n" , zone_fqdn );

  
  db.QuitTransaction( ret );
}

db.Disconnect();
}

if( ret ) return invoiceID;
else return -1; // err
}

// close invoice to registar handle for zone make taxDate to the todateStr
int factoring( const char *database  ,     const char *registrarHandle  ,  const char  *zone_fqdn , char *taxdateStr , char *todateStr )
{
    DB db;
    int regID;
    char timestampStr[32];
    int invoiceID = -1;
    int zone;
    int ret = 0;

    if( db.OpenDatabase( database  ) )
    {

        LOG( LOG_DEBUG , "successfully connected to DATABASE %s"  , database);

        if( db.BeginTransaction() )
        {

            if(  (regID = db.GetRegistrarID( (char * )  registrarHandle ) )  )
            {
                if( ( zone =  db.GetNumericFromTable( "zone", "id", "fqdn", zone_fqdn ) )  )
                {
 
                    get_timestamp( timestampStr  , get_utctime_from_localdate(  todateStr ) );
                    // make invoice
                    invoiceID = db.MakeFactoring( regID , zone , timestampStr  , taxdateStr );
          
                }
                else
                    LOG( LOG_ERR , "unknown zone %s\n" , zone_fqdn );
            }
            else
                LOG( LOG_ERR , "unknown registrarHandle %s" , registrarHandle );
            
            if( invoiceID >=0 ) ret = CMD_OK; // OK succesfully invocing
            
            db.QuitTransaction( ret );
        }
        
        db.Disconnect();
}

if( ret ) return invoiceID;
else return -1; // err
}


// e-banka on-line statement process from CSV file get from https: url
// cz descriptions
int ebanka_invoicing( const char *database  ,   char *filename )
{
DB db;
CSV csv;
int id;
int num=0 , err=0, ret=0;
int regID;
int invoiceID;
int zone;
int status;
int accountID;
char my_accountStr[18],  my_codeStr[5];
char accountStr[18],  codeStr[5];
char identStr[12];
char varSymb[12] , konstSymb[12];
char nameStr[64] , memoStr[64];
time_t t;
long price;
char datetimeString[32];

if( csv.read_file( filename ) == false )
{
LOG( ALERT_LOG ,  "Cannot open CSV file %s" , filename );

return -5;
}
else
{
 // test to 16 cols 
  if( csv.get_cols() !=  16   ) 
    {
       LOG( ALERT_LOG , "not like a CSV from ebanka https");           
       csv.close_file();
       return -16;
     }

}

if( db.OpenDatabase( database  ) )
{

LOG( LOG_DEBUG , "successfully  connect to DATABASE %s"  , database);

if( db.BeginTransaction() )
 {

  if( csv.get_cols() == 16   )
   {
    while(csv.get_row() )
       {



            strcpy( my_accountStr , csv.get_value(8)  );
           strcpy( my_codeStr , csv.get_value(9)  );
   

             // cislo naseho uctu  plus kod banky
          if( ( accountID = db.GetBankAccount(  my_accountStr , my_codeStr  ) )  )
            { 
              LOG( LOG_DEBUG ,"accountID  %d ucet [%s/%s]" , accountID , my_accountStr , my_codeStr  );              

              zone = db.GetBankAccountZone( accountID );
              LOG( LOG_DEBUG ,"account zone %d" , zone );
             
              // pripsana castka preved na log halire
              price  =  get_price( csv.get_value(4) );
                
              t = get_local_format_time_t( csv.get_value(5)  )  ;
              get_timestamp(   datetimeString , t);
 
               // kod banky a cislo protiuctu
              strcpy( accountStr , csv.get_value(6)  );
              strcpy( codeStr , csv.get_value(7)  );
              strcpy( identStr ,csv.get_value(15) ); // identifikator    
                


              strcpy( varSymb ,  csv.get_value(10) );
              strcpy( konstSymb ,  csv.get_value(11) );

              strcpy( nameStr ,  csv.get_value(14) ); // nazev protiuctu 
              strcpy( memoStr ,  csv.get_value(12) ); // poznamka
            
              
              status = atoi( csv.get_value(13) );

              LOG( LOG_DEBUG ,"EBANKA: identifikator [%s]  status %d price %ld ucet [%s/%s] VS=[%s] KS=[%s] name: %s memo: %s" , 
                                identStr ,status, price , accountStr , codeStr , varSymb ,  konstSymb  , nameStr , memoStr );

              if( status  == 2 )  // status platny musi byt dva platba realizovana
                {                   

                        // nas_ucet , identifikator , castka ,  datum a , cislo ucto , kod banky ,  VS , KS  , nazev uctu , poznamka
                      if( ( id = db.SaveEBankaList( accountID , identStr , price , datetimeString ,  accountStr , codeStr , 
                                                      varSymb ,  konstSymb  , nameStr , memoStr ) )  > 0 )
                        {                            
                          
                          regID =   db.GetRegistrarIDbyVarSymbol(   csv.get_value(10)  );

                          if( regID > 0 )
                            { 
                                 LOG( LOG_DEBUG ,"nalezen registator %d handle %s" , regID , db.GetRegistrarHandle( regID ) );
                                 // vytvoreni zalohove faktury a ulozeni creditu
                                  invoiceID =  db.MakeNewInvoiceAdvance( datetimeString ,  zone , regID , price , true  );
                                  if( invoiceID > 0 ) 
                                    {
                                       LOG( LOG_DEBUG , "OK vytvorena zalohova faktura id %d" , invoiceID );

                                      if(   db.UpdateEBankaListInvoice( id , invoiceID )   ) num ++; // ulozeni
                                       else  {   LOG( ERROR_LOG , "chyba update EBankaListInvoice");  err = -1; }
                                    }
                            }
                           else if( regID == 0 ) LOG(  LOG_DEBUG , "nezparovana platba identifikator [%s] price %ld" , identStr   , price ); 
 
                          
                        }
                      else { if(  id == 0 )   LOG( LOG_DEBUG ,"platba identifikator [%s] byla jiz zpracovana"  , identStr );
                             else  { LOG( ERROR_LOG , "chyba ve zpracovani vypisu enbaky");  err = -2;}
                           }

              }
          
           }

        }


       if( err == 0 )ret = CMD_OK;

       db.QuitTransaction( ret ); // potvrdit transakci jako uspesnou OK
    }
      // odpojeni od databaze
      db.Disconnect();

}

if( err <  0  ) return err; // pokud chyba vrat chybu
else return num; // kdyz ne vrat pocet zpracovanych radek

}
else {  LOG( ALERT_LOG ,  "Cannot connect to DB %s" , database ); return -10 ; } 


}

/* PREDELAT 
int banking_invoicing(const char *database )
{
DB db;
char sqlString[512];
struct invBANK **ib;
char prefixStr[25];
int num , i;
int invoiceID;
long credit;
int ret=0;
int DPH=0;
 
if( db.OpenDatabase( database  ) )
{

LOG( LOG_DEBUG , "successfully  connect to DATABASE %s"  , database);

if(  db.BeginTransaction() )
  {

 DPH = GetSystemVAT();
 
     // SQL dotaz na bankovni vypisy a jajich  zparovani podke varsym v  tabulce banking_invoice_varsym_map
     strcpy( sqlString , "SELECT id , registarid, price , zone from bank_statement_item , banking_invoice_varsym_map where bank_statement_item.account_number=banking_invoice_varsym_map.account_number and bank_statement_item.bank_code=banking_invoice_varsym_map.bank_code and banking_invoice_varsym_map.varsymb=banking_invoice_varsym_map.varsymb and bank_statement_item.invoice_id is null" );

     if( db.ExecSelect( sqlString )  )
       {
          num =  db.GetSelectRows();

        if( num > 0 )
        {   
          ib= new  invBANK *[num];
        
          for( i = 0 ; i < num ; i ++ )
            {
                ib[i] =  new  invBANK;
                ib[i]->id = atoi( db.GetFieldValueName("id" , i ) );
                ib[i]->regID = atoi( db.GetFieldValueName("registarid"  , i ) );
                ib[i]->zone = atoi( db.GetFieldValueName("zone" , i ) );
                ib[i]->price = (long) ( 100.0 *  atof( db.GetFieldValueName("price" , i ) ) );
                LOG( LOG_DEBUG ,"baking_items: id %d regID %d zone %d price %ld"  ,  ib[i]->id ,   ib[i]->regID , ib[i]->zone , ib[i]->price );
            }

        }
         db.FreeSelect();
      }


   if( num > 0 )
   {

      for( i = 0 ; i < num ; i ++ )
      {
          // vygenerovani cisla faktury 
         // jestlize byl vytvoren prefix zalohove faktury
          if( db.GetInvoicePrefix( prefixStr , 1 ,  ib[i]->zone  )  )
           {

              credit =  ib[i]->price -  get_VAT( ib[i]->price  ); // pripocitavany credit (castka be DPH )
             
              // vytvoreni zalohove faktury a ulozeni creditu         
             if( ( invoiceID =  db.MakeInvoice( (const char * ) prefixStr , ib[i]->zone , ib[i]->regID , 
                            ib[i]->price  ,  DPH  , get_VAT( ib[i]->price ) , credit ) ) )
             {
                   if( db.SaveCredit(   invoiceID   , credit ) )
                     {
                       if( db.UpdateBankStatementItem( invoiceID ,  ib[i]->id ) ) ret = CMD_OK;
                    //   else { LOG( LOG_ERR , "SQL EXEC error update invoice_statement_item");  ret =  0 ;num -1; break; }  
                     }
                 //  else { LOG( LOG_ERR , "SQL EXEC error update invoice_statement_item"); ret =  0 ; num -1; break;   }
                  
                   
               }
//               else  {    LOG( LOG_ERR , "SQL EXEC error save credit");  ret = 0 ; num -1; break; }


           }
  //       else  { LOG( LOG_ERR , "make invoice prefix" );  ret = 0 ; num -1;  }
                     
       }

    // free mem
    delete [] ib;
   }
  else LOG( NOTICE_LOG , "nenalezeny zadne platby ke zparovani"  );

    
  db.QuitTransaction( ret);
  }
// odpojeni od databaze
db.Disconnect();
}

return num;
}

*/

// make bank statement
bool banking_statement(const char *database , ST_Head *head , ST_Item  **item , int numrec )
{
DB db;
int accountID;
int statemetID;
int rc;
int ret=0;

if( db.OpenDatabase( database  ) )
{

LOG( LOG_DEBUG , "successfully  connect to DATABASE %s"  , database);

if(  db.BeginTransaction() )
  {  
    char bank_code[MAX_CODE];
    bzero(bank_code, MAX_CODE);
    if( ( accountID = db.TestBankAccount(  head->account , head->num , head->oldBalnce, bank_code )  ) )
      {   
        if(  ( statemetID = db.SaveBankHead( accountID , head->num ,  head->date ,  head->oldDate ,  head->oldBalnce ,  head->newBalance  ,  head->credit  ,  head->debet ) ) > 0 )
         {
         LOG( LOG_DEBUG , "accountID %d statemetID %d\n" ,  accountID , statemetID );             
          for( rc = 0 ; rc < numrec ; rc ++ )
           {
             int itemID = 0;
              if (!strcmp("0000",item[rc]->bank)) strncpy(item[rc]->bank,bank_code,MAX_CODE-1);
              if( (itemID = db.SaveBankItem( statemetID , item[rc]->account  ,  item[rc]->bank ,  item[rc]->evid ,  item[rc]->date ,   item[rc]->memo , 
                             item[rc]->code , item[rc]->ks , item[rc]->vs ,  item[rc]->ss ,  item[rc]->price )) <= 0 ) break;
              int regID =   db.GetRegistrarIDbyVarSymbol(item[rc]->vs);
              if( regID > 0 )
              { 
                LOG( LOG_DEBUG ,"nalezen registator %d handle %s" , regID , db.GetRegistrarHandle( regID ) );
                // vytvoreni zalohove faktury a ulozeni creditu
                int zone = db.GetBankAccountZone( accountID );
                int invoiceID =  db.MakeNewInvoiceAdvance(item[rc]->date,  zone , regID , item[rc]->price , true  );
                if( invoiceID > 0 ) 
                {
                  LOG( LOG_DEBUG , "OK vytvorena zalohova faktura id %d" , invoiceID );
                  if(   !db.UpdateBankStatementItem( itemID , invoiceID )   ) 
                   {   LOG( ERROR_LOG , "chyba update EBankaListInvoice");  ret = -1; }
                }
               }
               else if( regID == 0 ) LOG(  LOG_DEBUG , "nezparovana platba identifikator [%s] price %ld" , item[rc]->memo   , item[rc]->price ); 
            }
         }
         
        // update zustaktu na uctu
       if( ret>0 && db.UpdateBankAccount( accountID , head->date , head->num ,  head->newBalance  ) ) ret = CMD_OK;    
      }     

             
        
       db.QuitTransaction( ret ); // potvrdit transakci jako uspesnou OK
    }
      // odpojeni od databaze
      db.Disconnect();

if( ret == CMD_OK ) return true;
}
else LOG( ALERT_LOG ,  "Cannot connect to DB %s" , database );



      return false;

}



int main(int argc , char *argv[] )
{
GPC gpc;
ST_Head *head;
ST_Item **item;
char dateStr[12];
time_t t;
int i , numrec ,  num ;
bool readConfig=false;

Conf config; // READ CONFIG  file


   // read config file
    for( i = 1 ;   i < argc ; i ++ )
    {
        if( strcmp( argv[i] , "-C" )  == 0  ||  strcmp( argv[i] , "--config" ) == 0    )
         {
          if( i +1< argc  ) // if one more param
           {
                 printf( "Read config file %s\n" , argv[i+1] );

                  if (config.ReadConfigFile(argv[i+1])) readConfig=true;
                  else
                    {
                        printf("Cannot read config file: %s\n"  ,  argv[i+1]  ); 
                        exit(-1);
                    }
           }        

         }

    }


if( !readConfig ) // if not set use default config 
{
    if ( config.ReadConfigFile( CONFIG_FILE ) ) readConfig=true ;
    else  {  
            printf("Cannot read default config file: %s" ,  CONFIG_FILE );   
            exit(-1);
           }

}




 // start syslog
#ifdef SYSLOG
    setlogmask ( LOG_UPTO(  config.GetSYSLOGlevel()  )   );
    openlog ( "banking" , LOG_PERROR | LOG_CONS | LOG_PID | LOG_NDELAY,  config.GetSYSLOGfacility() );
#endif

printf("connect DB string [%s]\n" , config.GetDBconninfo() ); 

// print usage
if( argc == 1 )
printf("import banking statement file to database\nusage: %s  -C --config config_file \nusage: %s --bank-gpc file.gpc\ninvoicing: %s --invoice\ncredit: %s --credit REG-HANDLE zone price\nE-Banka: %s --ebanka-csv file.csv\none invoice: %s --factoring REG-NAME zone 2006-12-31  2007-01-01\nall invoice: %s --factoring  zone taxDate endDate\n"  , argv[0] ,   argv[0]  ,    argv[0] ,  argv[0] , argv[0] , argv[0] , argv[0] );  


for( i = 1 ;   i < argc ; i ++ )
{

   if( strcmp(  argv[i]  , "--credit" )  == 0 )
     {
       t = time(NULL );
       get_rfc3339_timestamp( t , dateStr , true ); // get actual local timestamp
        if(  i +3< argc ) credit_invoicing(  config.GetDBconninfo() , dateStr  ,   argv[i+1] , argv[i+2] , atol( argv[i+3] ) * 100L  );
       break;
     }

   if( strcmp(  argv[i]  , "--factoring" )  == 0 )
    {
     if(  i +3< argc ) factoring_all(  config.GetDBconninfo() ,  argv[i+1] , argv[i+2] , argv[i+3]   );
     break;
    }

     if( strcmp(  argv[i]  , "--factoring" )  == 0 )
       {
           if(  i +4< argc )  factoring(   config.GetDBconninfo() ,  argv[i+1] , argv[i+2] , argv[i+3]    ,  argv[i+4] );
           break;
       }

   if( strcmp(  argv[i]  , "--ebanka-csv" ) == 0 )
    {
     if(  i +1< argc  ) ebanka_invoicing(  config.GetDBconninfo() ,  argv[i+1] );
     break;
    }




  if( strcmp(  argv[i]  , "--bank-gpc" ) == 0  &&  i +1< argc  )
  {
    // read file in  GPC format CZ specified
    numrec =  gpc.ReadGPCFile(argv[i+1] );

    
   if( numrec <  0 ) {
	   LOG( ALERT_LOG , "chyba nacitani souboru %s" , argv[i+1]  );
#ifdef SYSLOG
	  closelog ();
#endif
	   exit(-1);

   }
   else
   {

    head = new ST_Head;     
    item = new ST_Item *[numrec];

   // get head of statement
    gpc.GetHead( head );

     // read statament
     LOG( LOG_DEBUG ,"head account %s name %s" ,head->account ,head->name );
     LOG( LOG_DEBUG ,"balance old %ld new  %ld credit  %ld debet %ld" ,head->oldBalnce   ,head->newBalance ,head->credit ,head->debet );
     LOG( LOG_DEBUG ,"cislo vypisu %d datum stareho zustatky %s datum vypisu %s" ,head->num ,head->oldDate ,head->date );

    
     
     for( i = 0 ; i < numrec ; i ++ )
        {
             item[i] =  new ST_Item;

             gpc.GetItem(item[i]);

             LOG( LOG_DEBUG ,"item account %s bank [%s]" ,  item[i]->account , item[i]->bank );
             LOG( LOG_DEBUG ,"vs %s ss %s ks %s" , item[i]->vs , item[i]->ss , item[i]->ks );
             LOG( LOG_DEBUG ,"date %s memo [%s]" , item[i]->date , item[i]->memo );
             LOG( LOG_DEBUG ,"code %d price %ld.%02ld" , item[i]->code ,  item[i]->price /100 , item[i]->price %100 );

             // next bank statement item
             gpc.NextItem();

        }


     // proved import bankovniho prikazu do databaze
     if( (  num =  banking_statement( config.GetDBconninfo() , head ,  item ,  numrec ) )  < 0 ) 
            printf("error import file %s to database\n" ,  argv[i+1] );
      else 
           printf("FILE %s succesfully import to database num items %d\n" ,  argv[i+1] , num );


     // uvolni pamet
      delete head;
      delete [] item;

   break;
   }

 }

}



#ifdef SYSLOG
  closelog ();
#endif

  return 0;
}

