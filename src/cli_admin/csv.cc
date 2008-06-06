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

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<ctype.h>
#include<time.h>

#include "csv.h"

CSV::CSV()
{
  separator=';';
  rows=0;
  cols=0;
}

CSV::~CSV()
{
}

int CSV::get_sepnum()
{
  int j, c;
  int len;

  len = strlen(buf);

  for (j = 0, c =0; j < len; j ++) {
    // the end of line test
    if (buf[j] == '\r' || buf[j] == '\n') {
      buf[j] = 0;
      if (c > 0)
        c ++;
      break;
    } else if (buf[j] == separator)
      c ++;
  }

  return c;
}

bool CSV::get_row()
{

  char *s = fgets(buf, MAX_LINE, fd);
  if (feof(fd) || (!s)) {
    fclose(fd) ;
    return false;
  } else {
    if (get_sepnum() > 0)
      return true;
    else
      return false;
  }

}

void CSV::close_file()
{
  fclose(fd);
}

const char * CSV::get_value(
  unsigned int col)
{
  int i, start, j;
  unsigned int c;
  int len;

  len = strlen(buf);

  if (col == 0)
    start=0;
  else {

    for (i = 0, c =0, start=-1; i < len; i ++) {
      if (buf[i] == separator)
        c++;

      if (c == col) {
        start =i+1;
        break;
      }

    }

  }

  if (start >= 0) {

    for (i = start, j =0; i < len; i ++, j ++) {
      if (buf[i] == separator)
        break;
      else
        string[j] = buf[i];
    }

    string[j] = 0;

    return string;
  } else
    return "";

}

bool CSV::read_file(
  const char *filename)
{
  int i, numrec, c;
  int cls = 0; //shouldnt be needed but compiler throws warning

  if ( (fd = fopen(filename, "r") ) != NULL) {

    for (i=0, numrec = 0;; i++) {
      char *s = fgets(buf, MAX_LINE, fd);
      if (feof(fd) || (!s))
        break;

      c = get_sepnum();

      if (c > 0) {
        if (numrec > 0) {
          if (c != cls) {
            fclose(fd) ;
            return false;
          }
        }

        cls = c;
        numrec ++;
      } else
        break;// empty line at th end of file 

    }

    fseek(fd, 0, SEEK_SET);

    rows = numrec;
    cols = cls;

    // debug("Rows in  files is %d cols %d\n" , rows , cols );
    return true;
  } else {
    // debug("Error open file %s\n" , filename);
    return false;
  }

}

#ifdef TEST
int main(int argc , char *argv[] )
{
  CSV csv;
  int c ,r =0;

  if( argc == 2 )
  {
    csv.read_file( argv[1]);

    while(csv.get_row() )
    {

      for( c = 0; c < csv.get_cols(); c ++ )
      {
        printf("[%d,%d] -> [%s]\n" , r ,c , csv.get_value(c) );
      }

      r++;
    }

  }

  if( argc == 1)
  {
    printf("pouziti:\n%s csvfile\n" , argv[0] );
  }

}

#endif

