/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> // to support build on FreeBSD

#include <boost/asio/ip/address.hpp>

#include "src/deprecated/util/util.hh"
#include "util/log/logger.hh"
#include "src/bin/corba/epp/action.hh"

#include "util/random_data_generator.hh"

RandomDataGenerator rdg;

// generate randoma password contains characters  [a-z] [A-Z] and [0-9] with length PASS_LEN
void random_pass(
  char *str)
{
  int len=PASS_LEN;
  int i;
  char c;


  for (i = 0; i < len;) {
    c = rdg.xnletter();//32+(int) (96.0*rand()/(RAND_MAX+1.0));

    if (isalnum(c) ) {
      str[i] = c;
      i ++;
    }

  }
  str[i] = 0; // to end
}

bool validateIPV6(const char* ipadd)
{
    boost::asio::ip::address_v6 addr;
    try
    {
        addr = boost::asio::ip::address_v6::from_string(ipadd);
    }
    catch (...)
    {
        return false;
    }

    if (addr.is_unspecified() ||
        addr.is_loopback() ||
        addr.is_multicast())
    {
        return false;
    }

    // TODO for more
    return true;
}

bool validateIPV4(const char* ipadd)
{
  boost::asio::ip::address_v4 addr;
  boost::asio::ip::address_v4::bytes_type bytes;
  // syntax
  try
  {
    addr = boost::asio::ip::address_v4::from_string(ipadd);
    bytes = addr.to_bytes();
  }
  catch (...)
  {
    // syntax check failed
    return false;
  }

  if ((bytes[0] == 0) && (bytes[1] == 0) && (bytes[2] == 0) && (bytes[3] == 0))
  {
    return false;
  }
  //TODO: why is 1.1.1.1 invalid?
  if ((bytes[0] == 1) && (bytes[1] == 1) && (bytes[2] == 1) && (bytes[3] == 1))
  {
    return false;
  }
  // TODO - libboost1.46-dev in Ubuntu 12.04.4 LTS doesn't contain this method
  // if (addr.is_loopback()) {
  if (bytes[0] == 127)
  {
    return false;
  }
  if (bytes[0] == 10)
  {
    return false;
  }
  if ((bytes[0] == 172) && (16 <= bytes[1]) && (bytes[1] < 32))
  {
    return false;
  }
  if ((bytes[0] == 192) && (bytes[1] == 168))
  {
    return false;
  }
  if (addr.is_multicast())
  {
    return false;
  }
  return true;
}

int test_inet_addr(const char *src)
{
  struct sockaddr_in a4;
  struct sockaddr_in6 a6;

  if (inet_pton(AF_INET, src, &a4.sin_addr) == 0)
  // test IPV6
  {
    if (inet_pton(AF_INET6, src, &a6.sin6_addr))
    {
      if (validateIPV6(src))
      {
        return IPV6;
      }
    }
    // TODO local address for ipv6
  }
  else if (validateIPV4(src))
  {
    return IPV4; // validate for local adres
  }
  return 0;
}

// atoh function ascii to hexa
int atoh(const char *String)
{
  int Value = 0, Digit;
  char c;

  while ((c = *String++) != 0) {

    if (c >= '0' && c <= '9')
      Digit = (int) (c - '0');
    else if (c >= 'a' && c <= 'f')
      Digit = (int ) (c - 'a') + 10;
    else if (c >= 'A' && c <= 'F')
      Digit = (int) (c - 'A') + 10;
    else
      break;

    Value = (Value << 4) + Digit;
  }

  return Value;
}

// contact handle
bool get_CONTACTHANDLE(
  char * HANDLE, const char *handle)
{
  return get_handle(HANDLE, handle, 1);
}

// nsset handle
bool get_NSSETHANDLE(
  char * HANDLE, const char *handle)
{
  return get_handle(HANDLE, handle, 2);
}

// keyset handle
bool get_KEYSETHANDLE(
        char *HANDLE, const char *handle)
{
    return get_handle(HANDLE, handle, 4);
}

// general handle
bool get_HANDLE(
  char * HANDLE, const char *handle)
{
  return get_handle(HANDLE, handle, 0);
}

// convert and test validity of the handle
bool get_handle(
  char * HANDLE, const char *handle, int typ)
{
  int i, len;

  len = strlen(handle);

  LOG<Logging::Log::EventImportance::debug>( "get_HANDLE from [%s] len %d" , handle , len );

  // max a min lenght
  if (len > 1 && len <= 40) {
    for (i = 0; i < len; i ++) {

      // TEST
      if (isalnum(handle[i]) || handle[i] == '.' || handle[i] == '-'
          || handle[i] == '_' || handle[i] == ':') {
        // convert tu upper
        HANDLE[i] = toupper(handle[i]);
      } else {

        HANDLE[0] = 0;
        return false;
      }

    }

    HANDLE[i] = 0;

    switch (typ) {
      case 1:
        if (strncmp(HANDLE, "CID:", 4) == 0) {
          LOG<Logging::Log::EventImportance::debug>( "OK CONTACT HANDLE [%s] " , HANDLE );
          return true;
        } else
          return false;
      case 2:
        if (strncmp(HANDLE, "NSSID:", 6) == 0) {
          LOG<Logging::Log::EventImportance::debug>( "OK NSSET HANDLE [%s] " , HANDLE );
          return true;
        } else
          return false;
      case 4:
        if (strncmp(HANDLE, "KEYID:", 6) == 0) {
            LOG<Logging::Log::EventImportance::debug>( "OK KEYSET HANDLE [%s] ", HANDLE);
            return true;
        } else
            return false;
      default:
        LOG<Logging::Log::EventImportance::debug>( "OK HANDLE [%s] " , HANDLE );
        return true;
    }

  }

  else
    return false;
}

// convert  DNS host  to lowwer case
bool convert_hostname(
  char *HOST, const char *fqdn)
{
  int i, len;

  len = strlen(fqdn);

  for (i = 0; i < len; i ++) {
    if (isalnum(fqdn[i]) || fqdn[i] == '-' || fqdn[i] == '.') // test name of the DNS server
    {
      HOST[i] = tolower(fqdn[i]); // to lower case
    } else {
      HOST[0] = 0;
      return false;
    }
  }

  // ont the end
  HOST[i] = 0;
  return true;
}
// test hostname  form DNS
bool TestDNSHost(
  const char *fqdn)
{
  int i, len, dot, num;

  len = strlen(fqdn);

  LOG<Logging::Log::EventImportance::debug>( "test DNSHost %s" , fqdn );

  // if dot on the end
  if (fqdn[len-1] == '.') {
    LOG<Logging::Log::EventImportance::debug>( "ERORR dots on end" );
    return false;
  }

  for (i = len -1; i > 0; i --) {
    if (fqdn[i] == '.')
      break;
    else if (isalpha(fqdn[i]) == 0) {
      LOG<Logging::Log::EventImportance::debug>( "ERORR not tld" );
      return false;
    }
  }

  // min and max lenght
  if (len > 3 && len <= 255) {
    for (i = 0, num = 0, dot = 0; i < len; i ++) {
      if (isalnum(fqdn[i]) || fqdn[i] == '-' || fqdn[i] == '.') {
        if (fqdn[i] == '.') {
          if (num == 0 || num > 63)
            return false; // very long or very short name for domain
          num = 0;
          dot ++;
        } else
          num ++;
      } else
        return false; // bad name of the DNS host
    }

    // minimal one dot
    if (dot >= 1) {
      LOG<Logging::Log::EventImportance::debug>( "test OK dots %d" , dot );
      return true;
    }
  }

  return false;
}

// test inet address for ipv4 and ipv6
bool TestInetAddress(
  const char *address)
{
  int t;

  t = test_inet_addr(address);
  LOG<Logging::Log::EventImportance::debug>( "test InetAddress %s -> %d" , address , t );
  if (t)
    return true;
  else
    return false;
}

/// compare exdate
bool TestExDate(
  const char *curExDate, const char * ExDate)
{
  LOG<Logging::Log::EventImportance::debug>( "test curExDate [%s] ExDate [%s]" , curExDate , ExDate );

  if (strcmp(curExDate, ExDate) == 0) {
    LOG<Logging::Log::EventImportance::debug>( "test OK" );
    return true;
  } else {
    LOG<Logging::Log::EventImportance::debug>( "test fail " );
    return false;
  }
}

// test period validity for renew
int TestPeriodyInterval(
  int period, int min, int max)
{
  int mod;
  LOG<Logging::Log::EventImportance::debug>( "test periody interval perido %d min %d max %d" , period , min , max );

  // period must be in interval
  if (period > 0 && period <= max) {
    mod = period % min;
    if (mod == 0)
      return 0; // is it OK
    else
      return 1; // period  is correct but not in  interval like min , min*2 , min*3 like  12 .. 24 .. 36  months
  } else
    return 2; // period is out of range
}

// convert price of penny without conversion through float
// convert  local currency string
long get_price(
  const char *priceStr)
{
  char str[33];
  long price;
  int i, j, len;

  strncpy(str, priceStr, sizeof(str)-1);
  len = strlen(priceStr);
  for (i = 0; i < len; i ++) {

    // if thousands are separated with blank space
    // test for thousands
    if (str[i] == ' ') {
      for (j = i; j < len -1; j ++)
        str[j] = str[j+1];
      len --;
    }

    if (str[i] == '.' || str[i] == ',') {
      str[i] = str[i+1];
      str[i+1] = str[i+2];
      str[i+2] = 0;
      break;
    }
  }

  price=atol(str);

  LOG<Logging::Log::EventImportance::debug>( "get_price from string[%s] -> %ld hal" , priceStr , price );

  return price;
}

// convert local format time in string to time_t
time_t get_local_format_time_t(
  const char *string)
{
  struct tm dt;
  time_t t;

  memset(&dt, 0, sizeof(dt));
  // local CZ  format in  DD.MM.YYYY HH:MM:SS
  sscanf(string, "%02d.%02d.%4d %02d:%02d:%02d", &dt.tm_mday, &dt.tm_mon,
      &dt.tm_year, &dt.tm_hour, &dt.tm_min, &dt.tm_sec);

  // minus one month
  dt.tm_mon = dt.tm_mon -1;
  // year - 1900
  dt.tm_year = dt.tm_year - 1900;

  // convert local time
  t = mktime( &dt);

  LOG<Logging::Log::EventImportance::debug>( "get_local_time_t from [%s] = %ld" , string , t );
  return t;
}

// convert  timestamp to  unix time sice epoch
time_t get_time_t(
  const char *string)
{
  struct tm dt;
  time_t t;
  memset(&dt, 0, sizeof(dt));
  double sec = 0;

  if (strcmp(string, "NULL") == 0)
    return 0;
  else {
    char sep; // just to support 'T' char in string
    sscanf(string, "%4d-%02d-%02d%c%02d:%02d:%lf", &dt.tm_year, &dt.tm_mon,
        &dt.tm_mday, &sep, &dt.tm_hour, &dt.tm_min, &sec);

    // convert and round sec to lowwe
    dt.tm_sec = (int ) sec ;
    //  minus one month
    dt.tm_mon = dt.tm_mon -1;
    // year - 1900
    dt.tm_year = dt.tm_year - 1900;

    t = timegm(&dt); // convert to GMT
    if (t < 0)
      return 0; // fix if  convert fault

    LOG<Logging::Log::EventImportance::debug>( "get_time_t from [%s] = %ld" , string , t );
    return t;
  }

}

//  convert local date  to  UTC timestamp for SQL
// involve  dayling saving time  for using time zone
time_t get_utctime_from_localdate(
  const char *dateStr)
{
  struct tm dt;
  time_t t;
  int year, month, day;

  memset(&dt, 0, sizeof(dt));

  sscanf(dateStr, "%4d-%02d-%02d", &year, &month, &day);
  dt.tm_mday = day;
  dt.tm_mon = month -1;
  dt.tm_year = year - 1900;
  dt.tm_isdst = -1; // negative if the information is not available ( test dayling saving time )

  LOG<Logging::Log::EventImportance::debug>( "tm_year  %d tm_mon  %d tm_mday %d hour %d min %d sec %d" , dt.tm_year , dt.tm_mon , dt.tm_mday , dt.tm_hour, dt.tm_min , dt.tm_sec );
  t = mktime( &dt);

  return t;
}

//  convert date  from database  ( int UTC date ) to local  date
void convert_rfc3339_date(
  char *dateStr, size_t len, const char *string)
{
  time_t t;

  t = get_time_t(string); // to  GMT
  if (t <= 0)
    dateStr[0] = (char)NULL; //  ERROR

  else
    get_rfc3339_timestamp(t, dateStr, len, true); // return  local date
}

// convert timestamp from database  ( in UTC ) to local date time
void convert_rfc3339_timestamp(
  char *dateStr, size_t len, const char *string)
{
  time_t t;

  t = get_time_t(string);
  if (t <= 0)
    dateStr[0] = (char)0; //  ERROR
    // return string in rfc3339  like a date time with time zone
  else
    get_rfc3339_timestamp(t, dateStr, len, false);
}

void get_rfc3339_timestamp(
  time_t t, char *string, size_t string_len, bool day)
{
  struct tm *dt;
  int diff;
  char sign, tzstr[6];

  //convert time_t in UTC  to local time
  struct tm result;
  dt = localtime_r( &t, &result);

  diff = dt->tm_gmtoff; // test offset from GTM


  if (diff == 0)
    sign = 'Z'; // UTC zulu time is is not local time
  else if (diff < 0) {
    sign = '-';
    diff = -diff;
  } else
    sign = '+';

  // convert only date
  if (day)
    snprintf(string, string_len, "%4d-%02d-%02d", dt->tm_year + 1900, dt->tm_mon + 1,
        dt->tm_mday);
  else {
    snprintf(string, string_len, "%4d-%02d-%02dT%02d:%02d:%02d%c", dt->tm_year + 1900,
        dt->tm_mon + 1, dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec, sign);

    if (diff != 0) {
      snprintf(tzstr, sizeof(tzstr), "%02d:%02d", diff / SECSPERHOUR, (diff % SECSPERHOUR )
          / MINSPERHOUR); // get timezone
      strncat(string, tzstr, string_len-strlen(string)-1);
    }

  }

}

// function forconvert timestamp  time_t -> SQL string
void get_timestamp(
  char *string, size_t len, time_t t)
{
  struct tm *dt;

  //convert time to GTM
  struct tm result;
  dt = gmtime_r( &t, &result);

  // get SQL string
  snprintf(string, len, "%4d-%02d-%02d %02d:%02d:%02d", dt->tm_year+1900, dt->tm_mon
      +1, dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);

}

