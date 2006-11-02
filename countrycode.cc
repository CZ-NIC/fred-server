#include<stdio.h>

#include "countrycode.h"
#include "log.h"

CountryCode::CountryCode(int num)
{
add=0;
num_country = num;
CC = new char[num][MAX_CC+1];
}

bool CountryCode::AddCode( const char *code )
{
if( add < num_country )
{
//    if( !TestCountryCode( code ) )
    {

    CC[add][0] =  code[0];
    CC[add][1] =  code[1];
    CC[add][2] =  0;
    add++;
    return true;
   }
     
}
   
return false;
}

CountryCode::~CountryCode()
{
delete CC;
}


bool CountryCode::TestCountryCode(const char *cc)
{
int i;

for( i = 0 ;  i < num_country ; i ++ )
  {
    if(  CC[i][0] == cc[0] && CC[i][1] == cc[1]) return true;
  }

return false;
}
