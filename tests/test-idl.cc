/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *  (at your option) any later version.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

///to check idl build

#include <memory>
#include "corba/Logger.hh"

//IDL interface test implementations

class ccReg_Filters_Base_i: public POA_ccReg::Filters::Base
{
public:
    //dummy impl
    ~ccReg_Filters_Base_i(){}
    char* name() {return 0;}
    CORBA::Boolean neg() {return 0;}
    void neg(CORBA::Boolean newNeg) {(void)(newNeg);}
    CORBA::Boolean isActive() {return 0;}
};

class Registry_FilterBase_i: public POA_Registry::FilterBase
{
public:
    //dummy impl
    void reload(){}
    void clear(){}

};

int main(int argc, char*argv[])
{
    //force linking
    std::auto_ptr<ccReg_Filters_Base_i> filter_base_ptr(new ccReg_Filters_Base_i );
    std::auto_ptr<Registry_FilterBase_i> filterbase_ptr(new Registry_FilterBase_i );

    return 0;
}
