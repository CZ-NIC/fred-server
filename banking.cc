#include<stdio.h>
#include "gpc.h"

#include "conf.h"
#include "dbsql.h"

#define error printf

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
int ok=0;
long lastBalance; 

if( argc==2)
{

   numrec =  gpc.ReadGPCFile(argv[1] );


   if( numrec <  0 ) error("chyba nacitani souboru %s\n" , argv[1]  );
   else
   {
    if (!config.ReadConfigFile(CONFIG_FILE) ) 
    {
      error( "Cannot read config file %s\n" , CONFIG_FILE);
      exit(-1);
    }

   printf("DATABASE: %s " ,  config.GetDBconninfo()  );

   if( !db.OpenDatabase( config.GetDBconninfo()  ) )
    {
      error( "Cannot connect to DB %s\n" , config.GetDBconninfo());
      exit(-1);
    }
   else
   {
   if(  db.BeginTransaction() )
    {  
      debug("successfully  connect to DATABASE" );

    // hlavicka vypisu
     gpc.GetHead( &head );


     sprintf( sqlString , "SELECT  * FROM  bank_account WHERE account_number=\'%s\';" ,  head.account );

     lastNum=0;
     lastBalance=0;
     if( db.ExecSelect( sqlString )  )
       {
          if(  db.GetSelectRows() == 1 )
            {
                lastNum = atoi( db.GetFieldValueName("last_num" , 0 ) );
                lastBalance = (long) ( 100.0 *  atof( db.GetFieldValueName("balance" , 0 ) )  );
            }
         db.FreeSelect();
        }

     debug("posledni vypis cislo %d zustatek na uctu %ld\n" , lastNum , lastBalance);



     debug("head account %s name %s\n" , head.account , head.name );
     debug("balance old %ld new  %ld credit  %ld debet %d\n" , head.oldBalnce   , head.newBalance , head.credit , head.debet );
     debug("cislo vypisu %d datum stareho zustatky %s datum vypisu %s\n" , head.num , head.oldDate , head.date );

     if( lastNum + 1 == head.num &&  head.oldBalnce == lastBalance )
     { 
     ok = CMD_OK;
     for( i = 0 ; i < numrec ; i ++ )
        {
             gpc.GetItem(&item);

             debug("item account %s bank [%s]\n" ,  item.account , item.bank );
             debug("vs %s ss %s ks %s\n" , item.vs , item.ss , item.ks );
             debug("date %s memo [%s]\n" , item.date , item.memo );
             debug("code %d price %ld.%02ld\n" , item.code ,  item.price /100 , item.price %100 );

             // dalsi polozka
             gpc.NextItem();
        }

     }

     else
     {

          if( lastNum + 1 !=  head.num ) error("chyba nesedi cislo  vypisu %d  posledni nacteny je %d\n" , head.num , lastNum );
          else error("chyba nesedi zustatek na uctu poslednu zustatek %ld nacitany stav %ld\n" , lastBalance ,  head.oldBalnce );

     }


      db.QuitTransaction( ok );
    }
      // odpojeni od databaze
      db.Disconnect();
  }
 }

}
else
  printf("import banking statement to database\nusage: %s file.gpc\n"  , argv[0] );  
}

