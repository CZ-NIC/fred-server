#define MAX_LINE 4096 
#define MAX_STRING 1024
// class for import from data  CSV file

class CSV
{
public:
  CSV();
  ~CSV();

  // read row from file
  bool get_row();
  const char * get_value(
    unsigned int col);

  bool read_file(
    const char * filename);
  long get_numeric(
    int row, int col);
  unsigned char *get_string(
    int row, int col);

  void close_file();

  char get_separator()
  {
    return separator;
  }
  ;
  void set_separator(
    char s)
  {
    separator = s;
  }
  ;
  int get_rows()
  {
    return rows;
  }
  ;
  int get_cols()
  {
    return cols;
  }
  ;

private:

  int get_sepnum(); // return number of separator

  FILE *fd;
  char separator;
  int rows, cols; // rows and cols of the CSV file
  char buf[MAX_LINE]; // buffer for read wors
  char string[MAX_STRING]; // string
};

