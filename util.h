#ifndef time_t
#include <time.h>
#endif

#define ZONE_CZ   3
#define ZONE_CENUM 2
#define ZONE_ENUM  1

#define CZ_ZONE  "cz"
#define CENUM_ZONE "0.2.4.c.e164.arpa"
#define ENUM_ZONE   "0.2.4.e164.arpa"


// prevadi handle na velka pismena psojena s testem
bool get_HANDLE( char  * HANDLE , const char *handle );

// vytvoreni roid
void get_roid( char *roid , char *prefix , int id );

// test nsset handle fqdn nebo contact id
bool  get_CHECK( char *CHCK , const char *chck , int act );
// prevadeni a test fqdn nazvu domeny 
bool get_FQDN( char *FQDN , const char *handle );

// zarazeni do zony a kontrola nazvu domeny 
int get_zone( const char * fqdn , bool compare );
// vraci cas v time_t  pevede SQL retezec
time_t get_time_t(char *string );
// prevede time_t do retezce pro SQL
void get_timestamp( time_t t , char *string);

// spocita cas expirace ze zadaneho casu plus period mesice
time_t expiry_time( time_t extime ,  int period );

// zjisti velikost pole z retezce
int get_array_length(char *array);
// vrati prvek pole
void get_array_value(char *array ,  char *value , int field );
// vraci numericky prvek pole
int get_array_numeric(char *array , int field );

void add_field_value( char *string , char *fname , char *value );
void add_field_bool(  char *string , char *fname , int value );
// pridavani nazvu pole pri create
void create_field_fname( char *string , char *fname , char *value );

void create_field_value( char *string , char *fname , char *value );
 
