
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<ctype.h>
#include<time.h>


#include "csv.h"


CSV::CSV()
{
separator=';' ;
rows=0;
cols=0;
}

CSV::~CSV()
{
}



int CSV::get_sepnum()
{
int j , c ;
int len ;

len =  strlen(buf);

  for( j = 0 , c =0  ; j < len ; j ++ )
     {        
        // ukonceni radku 
        if( buf[j] == '\r' || buf[j] == '\n'  ) { buf[j] = 0 ; if( c > 0 )  c ++; break ; }
        else   if( buf[j] == separator ) c ++ ;
     }


return c;
}

bool CSV::get_row()
{

  fgets( buf , MAX_LINE , fd );
  if( feof(fd) ) {   fclose(fd) ; return false ; } 
  else
  {
   if(  get_sepnum() >  0 )  return true;
   else return false;
  }

}

void CSV::close_file()
{
 fclose(fd);
}

char * CSV::get_value(unsigned int col )
{
int i , start  , j ;
unsigned int c;
int len;


len =   strlen(buf);


if( col == 0 ) start=0;
else
{

 for( i = 0 , c =0  , start=-1; i < len ; i ++ )
    {    
      if( buf[i] == separator )c++;

      if( c == col ){ start =i+1 ;  break; }
       
    }

}

if( start >= 0 )
{


for( i = start , j =0; i < len ; i ++  , j ++ )
   {
   if( buf[i] == separator ) break;
   else string[j] = buf[i];
   }

string[j] = 0; 


return string;
}
else return "";

}

bool CSV::read_file(char *filename)
{
int i ,  numrec , cls  ,c ;

if( ( fd = fopen( filename ,  "r" ) ) != NULL ) 
{

for(i=0 , numrec = 0 ;;i++)
{
  fgets( buf , MAX_LINE , fd );
  if( feof(fd) ) break;

  c = get_sepnum();  



  if( c > 0 ) 
   {
     if( numrec > 0 )
     {
       if( c != cls  ) {  fclose(fd) ; return false ; } 
     }

     cls = c; numrec ++ ; 
   } 
  else  break;// prazdna radka na konci souboru

}

// fclose(fd);
// reset 
fseek( fd, 0, SEEK_SET );

rows = numrec ; cols = cls;

// debug("Rows in  files is %d cols %d\n" , rows , cols );
return true;
}
else
{
   // debug("Error open file %s\n" , filename);
   return false;
 }


}




#ifdef TEST
int main(int argc , char *argv[] )
{
CSV csv; 
int c ,r =0;


if( argc == 2 ) 
{
csv.read_file( argv[1]);

while(csv.get_row() )
{

for(  c = 0  ; c < csv.get_cols() ; c ++ )
  {
       printf("[%d,%d] -> [%s]\n" , r ,c  , csv.get_value(c) );
  }

r++;
}

}

if( argc == 1)
  {
   printf("pouziti:\n%s csvfile\n" ,  argv[0] );
  }


}

#endif

