#include<stdio.h>
#include "gpc.h"

#include "conf.h"
#include "dbsql.h"

#include "log.h"

//  select id , invoiceid , price , account_memo , registarid , zone  from bank_statement_item , banking_invoice_varsym_map where bank_statement_item.account_number=banking_invoice_varsym_map.account_number and bank_statement_item.bank_code=banking_invoice_varsym_map.bank_code and banking_invoice_varsym_map.varsymb=banking_invoice_varsym_map.varsymb  and bank_statement_item.invoiceid is null;

// #define error printf

// TODO konfig
int get_VATNum()
{
return 19;
}

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



int  banking_invoicing(const char *database )
{
DB db;
char sqlString[512];
struct invBANK **ib;
char prefixStr[25];
int num , i;
int counter, zero , id;
int invoiceID;
long credit;

if( db.OpenDatabase( database  ) )
{

LOG( LOG_DEBUG , "successfully  connect to DATABASE %s"  , database);

if(  db.BeginTransaction() )
  {

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
       sprintf( sqlString , "SELECT *  FROM invoice_prefix WHERE zone=%d and typ=1;",  ib[i]->zone );    

       prefixStr[0] = 0 ; 
       counter = 0; 
       if( db.ExecSelect( sqlString )  )
         {
            if(  db.GetSelectRows() == 1 )
              {
                    id = atoi( db.GetFieldValueName("id"  , 0 ) );
                    counter =  atoi( db.GetFieldValueName("counter" , 0 ) );
                    zero =  atoi( db.GetFieldValueName("num" , 0 ) );
                    sprintf( prefixStr , "%s%0*d" , 
                    db.GetFieldValueName("pref" , 0 )  , zero ,  counter );
                    LOG( LOG_DEBUG ,"invoice pref [%s] prefix[%s]" , db.GetFieldValueName("pref" , 0 )  ,   prefixStr  );
              }
           db.FreeSelect();
          }

        if( counter > 0  && id )
          {
            db.UPDATE( "invoice_prefix" );
            db.SET( "counter" , counter +1 );
            db.WHEREID( id );
            if( !db.EXEC() )
              {
                   LOG( LOG_ERR , "SQL EXEC error update invoice_prefix");
                   db.QuitTransaction( 0 );
                   // odpojeni od databaze
                   db.Disconnect();
                   return -8;
               }

          }

        // jestlize byl vytvoren prefix zalohove faktury
        if( strlen( prefixStr  )  )
          {
	  invoiceID = db.GetSequenceID( "invoice" ); 

           db.INSERT( "invoice" );
           db.INTO( "id" );
           db.INTO( "prefix" );
           db.INTO( "typ" );
           db.INTO( "registarid" );
           db.INTO( "price" );
           db.INTO( "numvat" );
           db.INTO( "vat" );
           db.INTO( "total" );
           db.VALUE(  invoiceID );  
           db.VALUE(  prefixStr  );
           db.VALUE( 1 ); // zalohova FA
           db.VALUE( ib[i]->regID );
           db.VALPRICE( ib[i]->price );
          
           db.VALUE( get_VATNum() );
           db.VALPRICE( get_VAT( ib[i]->price  ) );
           credit = ib[i]->price - get_VAT( ib[i]->price) ;
           db.VALPRICE( credit ); // cena bez dane

           if( !db.EXEC() ) 
              {
                LOG( LOG_ERR , "SQL EXEC error invoice");
                db.QuitTransaction( 0 );
                // odpojeni od databaze
                db.Disconnect();
                return  -2;
              }



            // uloz credit
           db.INSERT( "credit_invoice_credit_map" );
           db.INTO( "registrarid" );
           db.INTO( "zone" );
           db.INTO( "invoice_id" );
           db.INTO( "credit" );
           db.INTO( "total" );

           db.VALUE( ib[i]->regID );
           db.VALUE( ib[i]->zone );
           db.VALUE( invoiceID );
           db.VALPRICE( credit ); 
           db.VALPRICE( 0 );
  
           if( !db.EXEC() )
              {
                LOG( LOG_ERR , "SQL EXEC error credit_invoice_credit_map");
                db.QuitTransaction( 0 );
                // odpojeni od databaze
                db.Disconnect();
                return  -3;
              }

            db.UPDATE( "bank_statement_item" );
            db.SET( "invoice_id" , invoiceID );
            db.WHEREID(  ib[i]->id  );
            if( !db.EXEC() )
              {
                   LOG( LOG_ERR , "SQL EXEC error update invoice_statement_item");
                   db.QuitTransaction( 0 );
                   // odpojeni od databaze
                   db.Disconnect();
                   return -5;
               }
 
           
          }

       }

      delete [] ib;
   }
  else
   {
          LOG( LOG_ERR , "nenalezeny zadne platby ke zparovani"  );          
   }

     
     

  db.QuitTransaction( CMD_OK );
  }
// odpojeni od databaze
db.Disconnect();
}

return 0;
}

int banking_statement(const char *database , ST_Head *head , ST_Item  **item , int numrec )
{
DB db;
char sqlString[512];
int lastNum; // poradove cislo posledniho vypisu
int accountID=0;
long lastBalance=0; 
int statemetID=0;
bool retOK=false;
int rc;


if( db.OpenDatabase( database  ) )
{

LOG( LOG_DEBUG , "successfully  connect to DATABASE %s"  , database);

if(  db.BeginTransaction() )
  {  

     sprintf( sqlString , "SELECT  * FROM  bank_account WHERE account_number=\'%s\';" ,  head->account );

     if( db.ExecSelect( sqlString )  )
       {
          if(  db.GetSelectRows() == 1 )
            {
                accountID = atoi( db.GetFieldValueName("id"  , 0 ) ); 
                lastNum = atoi( db.GetFieldValueName("last_num" , 0 ) );
                lastBalance = (long) ( 100.0 *  atof( db.GetFieldValueName("balance" , 0 ) )  );
            }

         db.FreeSelect();
        }
     

     LOG( LOG_DEBUG ,"posledni vypis ucetID %d cislo %d zustatek na uctu %ld" , accountID ,  lastNum , lastBalance);

     if( accountID == 0 )
       {
          LOG( LOG_ERR , "nelze najit ucet na vypisu cislo %s" ,  head->account );          
          db.QuitTransaction( 0 );
          // odpojeni od databaze
           db.Disconnect();
          return -5;
       }

     // test podle cisla vypisu ne pro prvni vypis
    if( head->num > 1  )
      {
        if( lastNum + 1 != head->num )
          {
               LOG( LOG_ERR , "chyba nesedi cislo  vypisu %d  posledni nacteny je %d" , head->num , lastNum );
               db.QuitTransaction( 0 );
                // odpojeni od databaze
                db.Disconnect();
                return -3;
          }      
      }

    // test pokud nesedi zustatek na uctu
   if(  head->oldBalnce  != lastBalance )
     {
         LOG( LOG_ERR , "chyba nesedi zustatek na uctu poslednu zustatek %ld nacitany stav %ld" , lastBalance ,  head->oldBalnce );
               db.QuitTransaction( 0 );
                // odpojeni od databaze
               db.Disconnect();
               return  -4;
     }

     statemetID = db.GetSequenceID( "bank_statement_head" ); 
   // id | account_id | num | create_date | balance_old_date | balance_old | balance_new | balance_credit | balence_debet

                    db.INSERT( "bank_statement_head" );
                    db.INTO( "id" );
                    db.INTO( "account_id" );
                    db.INTO( "num" );
                    db.INTO( "create_date" );
                    db.INTO( "balance_old_date" );
                    db.INTO( "balance_old" );
                    db.INTO( "balance_new" );
                    db.INTO( "balance_credit" );
                    db.INTO( "balance_debet" );
                    db.VALUE(  statemetID );
                    db.VALUE(  accountID );
                    db.VALUE( head->num );
                    db.VALUE(  head->date );
                    db.VALUE(  head->oldDate );
                    db.VALPRICE(  head->oldBalnce );
                    db.VALPRICE(  head->newBalance  );
                    db.VALPRICE(  head->credit );
                    db.VALPRICE(  head->debet );

      if( !db.EXEC() ) 
      {
          LOG( LOG_ERR , "SQL EXEC error bank_statement_head");
                db.QuitTransaction( 0 );
                // odpojeni od databaze
               db.Disconnect();
               return  -6;
     }


     for( rc = 0 ; rc < numrec ; rc ++ )
        {


// id | statement_id | account_number | bank_code | code | konstsym | varsymb | specsymb | price | account_evid | account_date | account_memo
                    db.INSERT( "bank_statement_item" );
                    db.INTO( "statement_id" );
                    db.INTO( "account_number" );
                    db.INTO( "account_evid" );
                    db.INTO( "account_date" );
                    db.INTO( "account_memo" );
                    db.INTO( "bank_code" );
                    db.INTO( "code" );
                    db.INTO( "konstsym" );
                    db.INTO( "varsymb" );
                    db.INTO( "specsymb" );
                    db.INTO( "price" );
                    db.VALUE(  statemetID );
                    db.VALUE(  item[rc]->account );
                    db.VALUE( item[rc]->evid );
                    db.VALUE(  item[rc]->date );
                    db.VALUE(   item[rc]->memo );
                    db.VALUE(   item[rc]->bank );
                    db.VALUE(   item[rc]->code );
                    db.VALUE(   item[rc]->ks );
                    db.VALUE(   item[rc]->vs );
                    db.VALUE(   item[rc]->ss );
                    db.VALPRICE(  item[rc]->price );


              if( !db.EXEC() )
               { 
                   LOG( LOG_ERR , "SQL EXEC error bank_statement_item");
                   db.QuitTransaction( 0 );
                   // odpojeni od databaze
                   db.Disconnect();
                   return -7;
               }
 
              
        }

     //  update  tabulky 
    // UPDATE bank_account set last_date='2006-11-10', last_num=162 , balance='230000.00' where id=1;

           db.UPDATE( "bank_account" );
           db.SET( "last_date" , head->date  );
           db.SET( "last_num" , head->num );
           db.SETPRICE( "balance" , head->newBalance );
           db.WHEREID( accountID );

              if( !db.EXEC() )
               {
                   LOG( LOG_ERR , "SQL EXEC error update bank_account");
                   db.QuitTransaction( 0 );
                   // odpojeni od databaze
                   db.Disconnect();
                   return -8;
               }
             

       retOK =  db.QuitTransaction( CMD_OK ); // potvrdit transakci jako uspesnou OK
    }
      // odpojeni od databaze
      db.Disconnect();

if( retOK ) return 0;
else return 1;
}
else
{
      LOG( ALERT_LOG ,  "Cannot connect to DB %s" , database );
      return -1;
}


}

int main(int argc , char *argv[] )
{
GPC gpc;
ST_Head *head;
ST_Item **item;
int i , numrec , err ;

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



if( argc ==2 )
{
 if( strcmp(  argv[1]  , "--invoice"  )  == 0 )
   {
       err =  banking_invoicing( config.GetDBconninfo() );
       return err;
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
     err = banking_statement( config.GetDBconninfo() , head ,  item ,  numrec );



     // vysledek
     if( err != 0 ) printf("error %d import banking statement\n"  , err );
     else  printf("FILE %s succesfully import to database\n" ,  argv[2] );  

     // uvolni pamet
      delete head;
      delete [] item;

    return err;      
   }

  }

return 0;
}




printf("import banking statement file to database\nusage: %s --bank-gpc file.gpc\ninvoicing: %s --invoice\n"  , argv[0]  , argv[0] );  

#ifdef SYSLOG
  closelog ();
#endif

}

