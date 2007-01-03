#include<stdio.h>
#include "gpc.h"

#include "conf.h"
#include "dbsql.h"

#include "log.h"

//  select id , invoiceid , price , account_memo , registarid , zone  from bank_statement_item , banking_invoice_varsym_map where bank_statement_item.account_number=banking_invoice_varsym_map.account_number and bank_statement_item.bank_code=banking_invoice_varsym_map.bank_code and banking_invoice_varsym_map.varsymb=banking_invoice_varsym_map.varsymb  and bank_statement_item.invoiceid is null;

// #define error printf


// prepocita dan z ceny bez dane pre skoeficient
long get_VAT(long price)
{
double p, c=100.0;
double koef = 0.1597; // koeficient ze zakona pro DPH 19%
long ret;

p = price; 
p = p / c;

ret =  (long ) ( (p * koef ) * c ) ;
return ret;
}

// vytvoreni zalohovych faktur z bankovnich vypisu
struct invBANK
{
int id;
int regID;
long price;
int zone;
char prefix[25]; // prefix zalohove faktury
};


// rucni naliti creditu pro registratora
bool credit_invoicing(const char *database  , const char *registrarHandle ,  const char  *zone_fqdn , long  price )
{
DB db;
int regID;
int invoiceID;
int zone;
char prefixStr[25];
long credit;
int ret = 0;
int DPH=0;

if( db.OpenDatabase( database  ) )
{

LOG( LOG_DEBUG , "successfully  connect to DATABASE %s"  , database);

if( db.BeginTransaction() )
 {

  DPH = db.GetSystemVAT();
  if(  (regID = db.GetRegistrarID( (char * )  registrarHandle ) )  ) 
  {

    if( ( zone =  db.GetNumericFromTable( "zone", "id", "fqdn", zone_fqdn ) )  )
     {     
        // cislo zalohove faktury
        if( db.GetInvoicePrefix( prefixStr , 1 ,  zone  )  )
         {
            credit =  price -  get_VAT( price ); // pripocitavany credit (castka be DPH )

            // vytvoreni zalohove faktury a ulozeni creditu
           invoiceID =  db.MakeInvoice( (const char * ) prefixStr ,  zone , regID , price , DPH , get_VAT( price ) , credit );

           if( invoiceID ) if( db.SaveCredit( invoiceID , credit )  ) ret = CMD_OK;
         }

      }
     else LOG( LOG_ERR , "unkow zone %s\n" , zone_fqdn );
  }
  else
  {
     LOG( LOG_ERR , "unkow registrarHandle %s" , registrarHandle );          
  }

    db.QuitTransaction( ret );
}

// odpojeni od databaze
db.Disconnect();
}
 
if( ret ) return true;
else return false;
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

    if( ( accountID = db.TestBankAccount(  head->account , head->num , head->oldBalnce )  ) )
      {   
        if(  ( statemetID = db.SaveBankHead( accountID , head->num ,  head->date ,  head->oldDate ,  head->oldBalnce ,  head->newBalance  ,  head->credit  ,  head->debet ) ) > 0 )
         {
         LOG( LOG_DEBUG , "accountID %d statemetID %d\n" ,  accountID , statemetID );             
          for( rc = 0 ; rc < numrec ; rc ++ )
           {
              if( !db.SaveBankItem( statemetID , item[rc]->account  ,  item[rc]->bank ,  item[rc]->evid ,  item[rc]->date ,   item[rc]->memo , 
                             item[rc]->code , item[rc]->ks , item[rc]->vs ,  item[rc]->ss ,  item[rc]->price ) ) break;
          
           }
         }
         
        // update zustaktu na uctu
       if( db.UpdateBankAccount( accountID , head->date , head->num ,  head->newBalance  ) ) ret = CMD_OK;    
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
int i , numrec ,  num ;

Conf config; // READ CONFIG  file

    if (!config.ReadConfigFile(CONFIG_FILE) ) 
    {
      LOG(  ALERT_LOG ,  "Cannot read config file %s" , CONFIG_FILE);
      exit(-1);
    }

 // stat syslog
#ifdef SYSLOG
    setlogmask ( LOG_UPTO(  config.GetSYSLOGlevel()  )   );
    openlog ( "banking_gpc" , LOG_CONS | LOG_PID | LOG_NDELAY,  config.GetSYSLOGfacility() );
#endif


// usage
if( argc == 1 )printf("import banking statement file to database\nusage: %s --bank-gpc file.gpc\ninvoicing: %s --invoice\ncredit: %s --credit REG_HANDLE zone price\n"  , argv[0]  , argv[0] ,  argv[0]);  

/*
if( argc ==2 )
{
 if( strcmp(  argv[1]  , "--invoice"  )  == 0 )
   {
       banking_invoicing( config.GetDBconninfo() );        
   }
}
*/

if( argc == 5 )
 {
   if( strcmp(  argv[1]  , "--credit" )  == 0 )
     {
       credit_invoicing(  config.GetDBconninfo() ,  argv[2] , argv[3] , atol( argv[4] ) * 100L );
     }
 }
if( argc==3)
{

  if( strcmp(  argv[1]  , "--bank-gpc" ) == 0 )
  {
    // NACTENI GPC souboru
    numrec =  gpc.ReadGPCFile(argv[2] );

    
   if( numrec <  0 ) LOG( ALERT_LOG , "chyba nacitani souboru %s" , argv[2]  );
   else
   {

    head = new ST_Head;     
    item = new ST_Item *[numrec];

   // hlavicka vypisu
    gpc.GetHead( head );

     // nacteni vypisu 
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

             // dalsi polozka
             gpc.NextItem();

        }


     // proved import bankovniho prikazu do databaze
     if( (  num =  banking_statement( config.GetDBconninfo() , head ,  item ,  numrec ) )  < 0 ) 
            printf("error import file %s to database\n" ,  argv[2] );
      else 
           printf("FILE %s succesfully import to database num items %d\n" ,  argv[2] , num );


     // uvolni pamet
      delete head;
      delete [] item;

   }

  }

return 0;
}



#ifdef SYSLOG
  closelog ();
#endif

}

