
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
// pridavani nazvu pole pri create
void create_field_fname( char *string , char *fname , char *value );

void create_field_value( char *string , char *fname , char *value );
 
