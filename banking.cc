#include<stdio.h>
#include "gpc.h"

#include "conf.h"
#include "dbsql.h"

#include "log.h"

// #define error printf

int main(int argc , char *argv[] )
{
DB db;
GPC gpc;
ST_Head head;
ST_Item item;
int i , numrec ;
int lastNum; // poradove cislo posledniho vypisu
Conf config; // READ CONFIG  file
char sqlString[256];
int accountID=0;
long lastBalance=0; 
int statemetID=0;

if( argc==2)
{

   numrec =  gpc.ReadGPCFile(argv[1] );


   if( numrec <  0 ) LOG( ALERT_LOG , "chyba nacitani souboru %s" , argv[1]  );
   else
   {
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



   if( !db.OpenDatabase( config.GetDBconninfo()  ) )
    {
      LOG( ALERT_LOG ,  "Cannot connect to DB %s" , config.GetDBconninfo());
      exit(-2);
    }
   else
   {
     LOG( LOG_DEBUG , "successfully  connect to DATABASE %s"  , config.GetDBconninfo() );

   if(  db.BeginTransaction() )
    {  

    // hlavicka vypisu
     gpc.GetHead( &head );


     sprintf( sqlString , "SELECT  * FROM  bank_account WHERE account_number=\'%s\';" ,  head.account );

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
     
       
     LOG( LOG_DEBUG ,"posledni vypis cislo %d zustatek na uctu %ld" , lastNum , lastBalance);


     LOG( LOG_DEBUG ,"head account %s name %s" , head.account , head.name );
     LOG( LOG_DEBUG ,"balance old %ld new  %ld credit  %ld debet %ld" , head.oldBalnce   , head.newBalance , head.credit , head.debet );
     LOG( LOG_DEBUG ,"cislo vypisu %d datum stareho zustatky %s datum vypisu %s" , head.num , head.oldDate , head.date );

     if( accountID == 0 )
       {
             LOG( LOG_ERR , "nelze najit ucet na vypisu cislo %s" ,  head.account );
             db.QuitTransaction( 0 );
             // odpojeni od databaze
             db.Disconnect();
             exit(-5);
       }

     // test podle cisla vypisu ne pro prvni vypis
    if( head.num > 1  )
      {
        if( lastNum + 1 != head.num )
          {
               LOG( LOG_ERR , "chyba nesedi cislo  vypisu %d  posledni nacteny je %d" , head.num , lastNum );
               db.QuitTransaction( 0 );
                // odpojeni od databaze
                db.Disconnect();
                exit( -3);
          }      
      }

    // test pokud nesedi zustatek na uctu
   if(  head.oldBalnce  != lastBalance )
     {
         LOG( LOG_ERR , "chyba nesedi zustatek na uctu poslednu zustatek %ld nacitany stav %ld" , lastBalance ,  head.oldBalnce );
               db.QuitTransaction( 0 );
                // odpojeni od databaze
               db.Disconnect();
               exit( -4);
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
                    db.VALUE( head.num );
                    db.VALUE(  head.date );
                    db.VALUE(  head.oldDate );
                    db.VALPRICE(  head.oldBalnce );
                    db.VALPRICE(  head.newBalance  );
                    db.VALPRICE(  head.credit );
                    db.VALPRICE(  head.debet );

      if( !db.EXEC() ) 
      {
          LOG( LOG_ERR , "SQL EXEC error bank_statement_head");
                        db.QuitTransaction( 0 );
                // odpojeni od databaze
               db.Disconnect();
               exit( -6);
     }


     for( i = 0 ; i < numrec ; i ++ )
        {
             gpc.GetItem(&item);

             LOG( LOG_DEBUG ,"item account %s bank [%s]" ,  item.account , item.bank );
             LOG( LOG_DEBUG ,"vs %s ss %s ks %s" , item.vs , item.ss , item.ks );
             LOG( LOG_DEBUG ,"date %s memo [%s]" , item.date , item.memo );
             LOG( LOG_DEBUG ,"code %d price %ld.%02ld" , item.code ,  item.price /100 , item.price %100 );



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
                    db.VALUE(  item.account );
                    db.VALUE( item.evid );
                    db.VALUE(  item.date );
                    db.VALUE(   item.memo );
                    db.VALUE(   item.bank );
                    db.VALUE(   item.code );
                    db.VALUE(   item.ks );
                    db.VALUE(   item.vs );
                    db.VALUE(   item.ss );
                    db.VALPRICE(  item.price );


              if( !db.EXEC() )
               { 
                   LOG( LOG_ERR , "SQL EXEC error bank_statement_item");
                   db.QuitTransaction( 0 );
                   // odpojeni od databaze
                   db.Disconnect();
                   exit( -7);
               }
 
              
             // dalsi polozka
             gpc.NextItem();
        }

     //  update  tabulky 
    // UPDATE bank_account set last_date='2006-11-10', last_num=162 , balance='230000.00' where id=1;

           db.UPDATE( "bank_account" );
           db.SET( "last_date" , head.date  );
           db.SET( "last_num" , head.num );
           db.SETPRICE( "balance" , head.newBalance );
           db.WHEREID( accountID );

              if( !db.EXEC() )
               {
                   LOG( LOG_ERR , "SQL EXEC error update bank_account");
                   db.QuitTransaction( 0 );
                   // odpojeni od databaze
                   db.Disconnect();
                   exit( -8);
               }
             else   db.QuitTransaction( CMD_OK ); // potvrdit transakci jako uspesnou OK

    }
      // odpojeni od databaze
      db.Disconnect();
  }
 }


#ifdef SYSLOG
  closelog ();
#endif

if( CMD_OK )   printf("%s succesfully import to database\n" , argv[1] );
else printf("error during import see syslog\n");
}
else
  printf("import banking statement file to database\nusage: %s file.gpc\n"  , argv[0] );  
}

