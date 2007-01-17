
#include <sys/time.h>
#include <time.h>

#include "log.h"


unsigned long long  total_clock;
unsigned long long time_clock;

#define MICROSEC 1000000L

unsigned long long get_time_clock()
{
struct timeval tv;
struct timezone tz;
unsigned long long  tc;

tz.tz_minuteswest=0;
tz.tz_dsttime=0;

gettimeofday( &tv, &tz);
tc=tv.tv_sec;
tc=tc * MICROSEC;
tc= tc + tv.tv_usec;
// LOG( DEBUG_LOG , "TIMECLOCK get_time_clock %ld sec  %ld usec  return %lld " , tv.tv_sec , tv.tv_usec  , tc);
return tc;
}

void timeclock_start()
{
total_clock=0;
//LOG( DEBUG_LOG , "TIMECLOCK CLEAR ");
}


void timeclock_begin()
{

//LOG( DEBUG_LOG , "TIMECLOCK START"  );
time_clock=get_time_clock();

}

void timeclock_end()
{
unsigned long long  total;

total = get_time_clock()  - time_clock;
total_clock += total;
LOG( DEBUG_LOG , "TIMECLOCK  END  %lld (usec)" ,  total );
}


void timeclock_quit()
{

LOG( DEBUG_LOG , "TIMECLOCK TOTAL  %lld (sec) %lld (usec)" ,   total_clock / MICROSEC , total_clock % MICROSEC);
total_clock=0;
}

