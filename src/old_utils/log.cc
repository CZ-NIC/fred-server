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
#include <stdarg.h>

#include "log.h"

// #define MAXSTRING 1024 

#ifndef SYSLGOG
void logprintf(
  int level, char *fmt, ...)
{
  char levels[8][8] = { "EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE",
      "INFO", "SQL" };

  printf("%-8s: ", levels[level]);
  va_list args;
  // char message[MAXSTRING]; 
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  // konec radku
  printf("\n");
}
#endif

