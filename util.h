#ifndef time_t
#include <time.h>
#endif

#define ZONE_CZ   3
#define ZONE_CENUM 2
#define ZONE_ENUM  1

#define CZ_ZONE  "cz"
#define CENUM_ZONE "0.2.4.c.e164.arpa"
#define ENUM_ZONE   "0.2.4.e164.arpa"


#define IPV4 4
#define IPV6 6


#define   SECSPERMIN 60
#define MINSPERHOUR    60
#define SECSPERHOUR    (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY SECSPERHOUR * 24

#define  MAX_DATE 32 // delka stringu pro datum
#define PASS_LEN 8 // delka hesla
// vygeneruje nahodne heslo delky len obsahujici znaky [a-z] [A-Z] a [0-0]
void random_pass(char *str );

// test inet adres
bool validateIPV6(const char *ipadd);
bool validateIPV4(const char *ipadd);

// test ip adres
int test_inet_addr( const char *src );

// funkce pro prevod hexa
int atoh(const char *String);


// prevadi handle na velka pismena psojena s testem
bool get_HANDLE( char  * HANDLE , const char *handle );

// contact handle
bool get_CONTACTHANDLE(  char  * HANDLE , const char *handle  );

// nsset handle
bool get_NSSETHANDLE(  char  * HANDLE , const char *handle  );

// obecny handle
bool get_HANDLE(  char  * HANDLE , const char *handle  );

// prevod a testovani handlu
bool get_handle( char  * HANDLE , const char *handle  , int typ );

// vytvoreni roid
void get_roid( char *roid , char *prefix , int id );

// prevadeni a test fqdn nazvu domeny 
// vraci zpet cislo zony
// nula pokud je chyba
int get_FQDN( char *FQDN , const char *fqdn );

// zarazeni do zony a kontrola nazvu domeny 
int get_zone( const char * fqdn , bool compare );

// prevadi DNS host  na mala pismena
bool convert_hostname( char *HOST ,const  char *fqdn );

// test spravnosti zadani hosu
bool TestDNSHost( const char *fqdn  );

// test inet addres ipv4 a ipv6
bool TestInetAddress(const char *address );

// test ExDate
bool TestExDate( const char *curExDate , const char * ExDate );

// test spravnosti intervalu periody
int TestPeriodyInterval( int period , int min , int max );

// preveadi credit registratora na halire bez konverze na float
long get_price( const char *priceStr );

// prevadi cenu v halirich na string
void get_priceStr(char *priceStr  , long price);

// prevod lokalniho datumu na UTC timestamp pro SQL
void get_utctime_from_localdate( char *utctime , char *dateStr );

// prevod datau z DB SQL na date
void convert_rfc3339_date( char *dateStr , const char *string );

// preved cas z UTC stringu na lokalni cas dle rfc3339 z casovou zonou
void convert_rfc3339_timestamp( char *dateStr , const char *string );


// prevadi casovy string na zulu date time dle rfc3339
void get_zulu_t(  char *dateStr , const char *string );

// vraci cas v time_t  pevede SQL retezec
time_t get_time_t(const char *string );

// prevede cas na timestamp dle rfc3339 s casovou zonou jako offset
void get_rfc3339_timestamp( time_t t , char *string ,   bool day );

// prevede time_t do retezce pro SQL
void get_timestamp( time_t t , char *string);

