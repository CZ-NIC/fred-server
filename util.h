
// vraci cas v time_t  pevede SQL retezec
time_t get_time_t(char *string );
// prevede time_t do retezce pro SQL
void get_timestamp( time_t t , char *string);

// zjisti velikost pole z retezce
int get_array_length(char *array);
// vrati prvek pole
void get_array_value(char *array ,  char *value , int field );

void add_field_value( char *string , char *fname , char *value );
 
