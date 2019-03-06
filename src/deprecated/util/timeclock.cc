/*
 * Copyright (C) 2006-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <sys/time.h>
#include <time.h>

#include "util/log/logger.hh"

unsigned long long total_clock;
unsigned long long time_clock;

#define MICROSEC 1000000L

unsigned long long get_time_clock()
{
  struct timeval tv;
  struct timezone tz;
  unsigned long long tc;

  tz.tz_minuteswest=0;
  tz.tz_dsttime=0;

  gettimeofday( &tv, &tz);
  tc=tv.tv_sec;
  tc=tc * MICROSEC;
  tc= tc + tv.tv_usec;
  // LOG< DEBUG_LOG >( "TIMECLOCK get_time_clock %ld sec  %ld usec  return %lld " , tv.tv_sec , tv.tv_usec  , tc);
  return tc;
}

void timeclock_start()
{
  total_clock = 0;
  //LOG< DEBUG_LOG >( "TIMECLOCK CLEAR ");
}

void timeclock_begin()
{
  //LOG< DEBUG_LOG >( "TIMECLOCK START"  );
  time_clock = get_time_clock();
}

void timeclock_end()
{
  unsigned long long total;

  total = get_time_clock() - time_clock;
  total_clock += total;
  LOG<Logging::Log::EventImportance::debug>( "TIMECLOCK  END  %lld (usec)" , total );
}

void timeclock_quit()
{
  LOG<Logging::Log::EventImportance::debug>( "TIMECLOCK TOTAL  %lld (sec) %lld (usec)" , total_clock / MICROSEC , total_clock % MICROSEC);
  total_clock=0;
}
