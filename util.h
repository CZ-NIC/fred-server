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

// test local adres

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

// test spravnosti intervalu periody
bool TestPeriodyInterval( int period , int min , int max );
// test intervalu validace
bool TestValidityExpDate( time_t val , int max );

// preveadi credit registratora na halire bez konverze na float
long get_price( const char *priceStr );

// prevadi cenu v halirich na string
void get_priceStr(char *priceStr  , long price);

// vraci cas v time_t  pevede SQL retezec
time_t get_time_t(char *string );
// prevede time_t do retezce pro SQL
void get_timestamp( time_t t , char *string);

// spocita cas expirace ze zadaneho casu plus period mesice
time_t expiry_time( time_t extime ,  int period );

// porovnava datum co je v databazi podle datumu zadanevo jako curexdate z XML prevedeneho na tim_t v GMT
bool test_expiry_date( time_t dbextime ,   time_t curexptime );


// zjisti velikost pole z retezce
int get_array_length(char *array);
// vrati prvek pole
void get_array_value(char *array ,  char *value , int field );
// vraci numericky prvek pole
int get_array_numeric(char *array , int field );

