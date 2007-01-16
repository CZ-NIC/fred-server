#define MAX_LINE 4096 
#define MAX_STRING 1024
// class for import from CSV file

class CSV
{
public:
CSV();
~CSV();

// precte radek ze souboru
bool get_row();
char * get_value(unsigned int col );

bool read_file( char * filename );
long get_numeric( int row , int col );
unsigned char *get_string( int row ,  int col );

void close_file();

char get_separator() { return  separator; };
void set_separator(char s){ separator = s; } ;
int get_rows(){ return  rows ; };
int get_cols(){ return  cols ; };

private:


int get_sepnum(); // projede buffer radky

FILE *fd;
char separator;
int rows , cols; // pocet radek a sloupcu CSV souboru
char buf[MAX_LINE]; // buffer pro radek
char string[MAX_STRING]; // 
};

