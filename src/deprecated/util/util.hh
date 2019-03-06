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
#ifndef UTIL_HH_9884C7134F124DEB99A810284B66123C
#include <time.h>
#endif

#define IPV4 4 
#define IPV6 6

#define SECSPERMIN 60
#define MINSPERHOUR    60
#define SECSPERHOUR    (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY SECSPERHOUR * 24

#define  MAX_DATE 32 // max date string 
#define PASS_LEN 8 // max length of  password 
// generate random password 
void random_pass(
  char *str);

// test inet address for IPV4 and IPV6
bool validateIPV6(
  const char *ipadd);
bool validateIPV4(
  const char *ipadd);

// test ip address return IPV4 or IPV6 
int test_inet_addr(
  const char *src);

// function for hexa convertion
int atoh(
  const char *String);

// convert handle to uppercase connected with test 
bool get_HANDLE(
  char * HANDLE, const char *handle);

// contact handle
bool get_CONTACTHANDLE(
  char * HANDLE, const char *handle);

// nsset handle
bool get_NSSETHANDLE(
  char * HANDLE, const char *handle);

// keyset handle
bool get_KEYSETHANDLE(
        char *HANDLE, const char *handle);

// obecny handle
bool get_HANDLE(
  char * HANDLE, const char *handle);

// convertion and testing of handle
bool get_handle(
  char * HANDLE, const char *handle, int typ);

// convert  DNS host  to lower case 
bool convert_hostname(
  char *HOST, const char *fqdn);

// test validity of DNS host
bool TestDNSHost(
  const char *fqdn);

// test inet address ipv4 and ipv6
bool TestInetAddress(
  const char *address);

// test if current ExDate is same with ExDate
bool TestExDate(
  const char *curExDate, const char * ExDate);

// test validity of the renew period in month ( min minimal renew interval ) max ( maximal length of the  renew )
int TestPeriodyInterval(
  int period, int min, int max);


// convert registrar credit for pennies without conversion to float it takes also decimal comma or dot
// convert currency  string for example   1.200,00  to logn value 120000
long get_price(
  const char *priceStr);

// convert local date  to  UTC timestamp for SQL
time_t get_utctime_from_localdate(
  const char *dateStr);

// convert dateg from SQL result to  date
void convert_rfc3339_date(
  char *dateStr, size_t len, const char *string);

// convert UTC string timestamop to local time  rfc3339 with time zon
void convert_rfc3339_timestamp(
  char *dateStr, size_t len, const char *string);

// convert local  format of date date (CZ) DD.MM.YYYY to UTC time  
time_t get_local_format_time_t(
  const char *string);

// return time_t from  SQL  result 
time_t get_time_t(
  const char *string);

// convert time_t to  timestamp   rfc3339 with timezone ( true day ) for conver only date 
void get_rfc3339_timestamp(
  time_t t, char *string, size_t string_len, bool day);

// convert  time_t to SQL  string  timestamp
void get_timestamp(
  char *string, size_t len, time_t t);

