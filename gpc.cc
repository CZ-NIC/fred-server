#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "gpc.h"



void convert_date( char *dateStr ,  const char *date )
{
sprintf(  dateStr , "20%c%c-%c%c-%c%c" ,  date[4] ,  date[5] ,    date[2] ,   date[3]  ,  date[0] ,  date[1] ); 
}



int CopyStr( char *dst , char *src , int from , int len , bool erase)
{
int l ;

if(  erase)
{
// no nulls at the beginnig of string
for( l = 0 ; l < len -1; l ++ )
{
if( src[from+l] > '0'  ) break; 
}

}
else l = 0 ;

   strncpy( dst , src+from+l , len -l );
   dst[len-l] =0; 

return from + len;
}


int NonZeroCopyString(  char *dst , char *src , int len )
{
return   CopyStr( dst , src ,  0 , len , true );
}

int CopyString( char *dst , char *src , int from , int len )
{
// delete nulls at the beginning 
return  CopyStr( dst , src ,   from , len , false );
}

/*
int CopyDate( char *dst , char *src , int from  )
{
char ds[GPC_MAX_DATE+1];
CopyStr( ds , src ,   from , GPC_MAX_DATE , false );
// convert date
convert_date( dst  , ds );
return from + GPC_MAX_DATE;
}

int CopyPrice( char *dst , char *src , int from  )
{
int p;

char str[GPC_MAX_PRICE+1];
// copy string
CopyStr(  str , src ,   from , GPC_MAX_PRICE , false );
// convert to int
p = atoi( str );

sprintf( dst , "%d.02d" , p / 100 , p %100 );

return from + GPC_MAX_PRICE;
}


*/
void GPC::GetHead( ST_Head *head )
{
// client account number without starting zeros at the beginning 
NonZeroCopyString( head->account ,  gpc_head.accountNumber , GPC_MAX_ACCOUNT );

strncpy( head->name ,  gpc_head.accountName , GPC_MAX_NAME ); // client name
head->oldBalnce = atol(gpc_head.oldBalance ) ; // old balances
head->newBalance = atol(gpc_head.newBalance ); // new balances
head->credit = atol( gpc_head.credit ); // credit turn
head->debet = atol( gpc_head.debet); // debit turn
convert_date( head->oldDate , gpc_head.dateOldBalance ); // old balances date 
convert_date( head->date , gpc_head.dateList ); // statement date
head->num =  gpc_head.num ; // statement order
}

void GPC::GetItem( ST_Item *item )
{
// without starting zeros
NonZeroCopyString( item->account ,  gpc_item[recItem].accountOther ,   GPC_MAX_ACCOUNT );

strncpy( item->bank ,   gpc_item[recItem].konstSymbol+2  , 4 ); // bank code against account from KS 
item->bank[4] = 0;

strncpy( item->ks ,   gpc_item[recItem].konstSymbol+6 , 4 ); // bank code against account from KS 
item->ks[4] = 0;

strcpy( item->vs , gpc_item[recItem].varSymbol );
strcpy( item->ss , gpc_item[recItem].specSymbol );

strcpy( item->memo , gpc_item[recItem].memo );
strcpy( item->evid , gpc_item[recItem].evidNum );

// convert date
convert_date( item->date ,  gpc_item[recItem].date  );
// price in pennies
item->price = atol(  gpc_item[recItem].price  );

if(  gpc_item[numrecItem].accountCode == '1' ) item->code = DEBET;
else if(  gpc_item[numrecItem].accountCode == '2' ) item->code = CREDIT;
     else item->code =0;

}



void GPC::ParseItem(char *tmp  )
{
int seek;
 
   seek =3;

   seek = CopyString( gpc_item[numrecItem].accountNumber ,  tmp , seek  , GPC_MAX_ACCOUNT );

   seek= CopyString( gpc_item[numrecItem].accountOther , tmp , seek , GPC_MAX_ACCOUNT );

   seek= CopyString( gpc_item[numrecItem].evidNum , tmp , seek , GPC_MAX_EVID );

   seek= CopyString( gpc_item[numrecItem].price , tmp , seek , GPC_MAX_PRICE );

   gpc_item[numrecItem].accountCode =   tmp[seek]; // account code 1 debit 2 credit ( 3 , 4 cancel )
   seek ++;

   seek= CopyString( gpc_item[numrecItem].varSymbol , tmp , seek , GPC_MAX_SYMBOL );

   seek= CopyString(  gpc_item[numrecItem].konstSymbol , tmp , seek , GPC_MAX_SYMBOL  );
   seek= CopyString(   gpc_item[numrecItem].specSymbol , tmp , seek , GPC_MAX_SYMBOL );

   seek= CopyString(  gpc_item[numrecItem].dateValuta , tmp , seek , GPC_MAX_DATE );

   seek= CopyString(   gpc_item[numrecItem].memo , tmp, seek , GPC_MAX_MEMO );

   gpc_item[numrecItem].changeCode =  tmp[seek];
   seek ++;

   strncpy(  gpc_item[numrecItem].dataType , tmp+seek , 4 );
   seek += 4;

   seek= CopyString( gpc_item[numrecItem].date , tmp, seek , GPC_MAX_DATE );


#ifdef DEBUG
   debug("cislo protiuctu: %s\n" ,  gpc_item[numrecItem].accountOther  );
   debug("cislo dokladu  : %s\n" ,  gpc_item[numrecItem].evidNum  );
   debug("castka  %s kod zauctovani[%c]\n" ,  gpc_item[numrecItem].price  , gpc_item[numrecItem].accountCode );
   debug("VS [%s] KS [%s] SS[%s]\n" ,  gpc_item[numrecItem].varSymbol ,  gpc_item[numrecItem].konstSymbol ,  gpc_item[numrecItem].specSymbol );
   debug("valuta date [%s]\n"  , gpc_item[numrecItem].dateValuta );
   debug("doplnujici udaj %s\n" , gpc_item[numrecItem].memo );
  
   debug("kod zmeny [%c]  druh dat[%c%c%c%c] \n" , gpc_item[numrecItem].changeCode ,
         gpc_item[numrecItem].dataType[0] , gpc_item[numrecItem].dataType[1] , gpc_item[numrecItem].dataType[2] , gpc_item[numrecItem].dataType[3] );
   debug("datum zuctovani [%s]\n" , gpc_item[numrecItem].date );
#endif
 



  numrecItem++;

}


void GPC::ParseHead(char *tmp  )
{
char numStr[4];
int seek;

   seek =3;

   seek= CopyString(  gpc_head.accountNumber , tmp , seek , GPC_MAX_ACCOUNT );

   seek= CopyString(  gpc_head.accountName , tmp , seek , GPC_MAX_NAME );

   seek= CopyString( gpc_head.dateOldBalance , tmp ,seek , GPC_MAX_DATE );

   seek= CopyString( gpc_head.oldBalance , tmp , seek , GPC_MAX_BALANCE );
   gpc_head.oldBalanceSign  = tmp[seek];
   seek ++;

   seek= CopyString( gpc_head.newBalance , tmp, seek , GPC_MAX_BALANCE );
   gpc_head.newBalanceSign  = tmp[seek];
   seek ++;
 

   seek= CopyString( gpc_head.debet , tmp, seek , GPC_MAX_BALANCE );
   gpc_head.debetSign  = tmp[seek];
   seek ++;

   seek= CopyString( gpc_head.credit , tmp,seek , GPC_MAX_BALANCE );
   gpc_head.creditSign  = tmp[seek];
   seek ++;

   seek= CopyString( numStr , tmp,seek , 3 );
   gpc_head.num = atoi( numStr ); // ordinal number of statement
 
   seek = CopyString( gpc_head.dateList , tmp,seek , GPC_MAX_DATE );


#ifdef DEBUG
   debug("cislo uctu klienta: %s\n" ,  gpc_head.accountNumber  );
   debug("nazev uctu klienta: %s\n" ,  gpc_head.accountName  );
   debug("stary zustatek: [%c]%s datum [%s]\n"  , gpc_head.oldBalanceSign ,  gpc_head.oldBalance , gpc_head.dateOldBalance );
   debug("novy zustatek: [%c]%s\n" , gpc_head.newBalanceSign ,   gpc_head.newBalance  );
   debug("kreditni obrat: [%c]%s\n"  , gpc_head.creditSign ,   gpc_head.credit  );
   debug("debetni obrat: [%c]%s\n"  ,  gpc_head.debetSign ,  gpc_head.debet  );
   debug("datum vypisu  [%s] poradi %d\n" , gpc_head.dateList , gpc_head.num );
#endif


}

int GPC::ReadGPCFile( char * filename )
{
    FILE *fd;
    int i, l, c, numrec;
    char tmp[GPC_MAX_RECORD+2];

    if( (fd = fopen( filename ,  "r" ) ) == NULL ) return -1;

    // TODO TEST of first line for number 075 (zero seven five)
    for( l = 0 , numrec = -1 ; l < MAX_ITEMS ;  )
    {
        // read record
        for( i = 0 ; i < GPC_MAX_RECORD+2 ; i ++ )
        {
            if( feof(fd) ) { numrec=l  ;  break;  }
            // OS: musíme testovat návratovou hodnotu, protože pokud nastane chyba, tak je výsledek EOF
            c = fgetc(fd);
            if (c == EOF) {
                // OS: došlo k chybě, vrátíme -1
                return -1;
            }
            tmp[i] = c;
        }

        //  end of line
        if( tmp[GPC_MAX_RECORD] == '\r' &&  tmp[GPC_MAX_RECORD+1]  == '\n'  )
        {
            tmp[GPC_MAX_RECORD] = 0 ; // end of string

            if( strncmp( tmp ,  GPC_LIST_HEAD , 3 ) == 0 ) 
            {
                // debug("HEAD:\n%s\n" , tmp );
                ParseHead( tmp  );
            }
            else if( strncmp( tmp ,  GPC_LIST_ITEM , 3 ) == 0 )
            { 
                // debug("ITEM:\n%s\n" , tmp );
                ParseItem( tmp  );
                l ++; 
            } 
        // else { debug("UNKNOWN:\n%s\n" , tmp );  }
        }
        
        if( numrec >=0  ) break ; // if some lines are loaded or nothing 
    }

    // debug("End of reading list items %d\n" , numrec );
    fclose(fd);
    return numrec;
}

