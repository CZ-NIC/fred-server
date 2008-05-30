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

#include "countrycode.h"
#include "old_utils/log.h"

CountryCode::CountryCode(
  int num)
{
  add=0;
  num_country = num;
  CC = new char[num][MAX_CC+1];
}

bool CountryCode::AddCode(
  const char *code)
{
  if (add < num_country) {
    //    if( !TestCountryCode( code ) ) // do not test duplicity
    {

      CC[add][0] = code[0];
      CC[add][1] = code[1];
      CC[add][2] = 0;
      add++;
      return true;
    }

  }

  return false;
}

CountryCode::~CountryCode()
{
  delete CC;
}

bool CountryCode::TestCountryCode(
  const char *cc)
{
  int i;

  for (i = 0; i < num_country; i ++) {
    if (CC[i][0] == cc[0] && CC[i][1] == cc[1])
      return true;
  }

  return false;
}
