/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef COUNTRYCODE_H_E104B6D592014DFCA247850CEEFC0522
#define COUNTRYCODE_H_E104B6D592014DFCA247850CEEFC0522

#include <string>
#include <vector>

#define MAX_CC 2

class CountryCode
{
    typedef std::vector<std::string> CountryCodeT;
public:
    CountryCode();
    CountryCode(int num);
    ~CountryCode();

    bool AddCode(const char *code);
    bool TestCountryCode(const char *cc);

    void load();

    int GetNum()
    {
    return cc_.size();
    } // return number of countries

private:
    CountryCodeT cc_;
};//class CountryCode
#endif
