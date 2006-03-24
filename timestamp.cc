#include<time.h>
#include<stdio.h>


time_t get_gmt_time(char *string )
{
struct tm dt;
double sec;

sscanf(string , "%4d-%02d-%02d %02d:%02d:%lf" ,
                &dt.tm_year ,  &dt.tm_mon , &dt.tm_mday ,
                &dt.tm_hour,   &dt.tm_min , &sec);


printf("sec %lf\n" , sec );
 dt.tm_sec = (int ) sec ;
printf(" %d  %d %d %d %d %d\n" , dt.tm_year , dt.tm_mon , dt.tm_mday , dt.tm_hour , dt.tm_min , dt.tm_sec  );
// mesic o jeden mene
dt.tm_mon = dt.tm_mon -1;
// rok - 1900
dt.tm_year = dt.tm_year - 1900;

printf(" %d  %d %d\n" , dt.tm_year , dt.tm_mon , dt.tm_mday );

return mktime(&dt);
}


/*
main()
{
time_t t;

t = get_gmt_time("2006-03-22 16:45:30.847079");

printf("%s\n" , ctime( &t ) );

t = time(NULL );
printf("aktualni cas %s" , ctime( &t ) );

printf("gmt cas %s" , asctime( gmtime( &t)  ) );

}
*/
