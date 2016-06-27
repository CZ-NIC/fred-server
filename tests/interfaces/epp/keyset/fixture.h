/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file
 */

#ifndef FIXTURE_H_9B8B417BFEDF100B23D6A39540F8D033//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define FIXTURE_H_9B8B417BFEDF100B23D6A39540F8D033

#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

#include <vector>

namespace Test {

class RegistrarProvider
{
protected:
    RegistrarProvider();
    const Fred::InfoRegistrarData& get_registrar_a()const;
    const Fred::InfoRegistrarData& get_registrar_b()const;
    const Fred::InfoRegistrarData& get_sys_registrar()const;
private:
    static Fred::InfoRegistrarData create_registrar(Fred::OperationContext&, const std::string&, bool);
    Fred::InfoRegistrarData registrar_a_;
    Fred::InfoRegistrarData registrar_b_;
    Fred::InfoRegistrarData sys_registrar_;
};

class ContactProvider
{
protected:
    ContactProvider(unsigned number_of_contacts,
                    const std::string &registrar_a,
                    const std::string &registrar_b,
                    const std::string &sys_registrar);
    const Fred::InfoContactData& get_contact(unsigned idx)const;
private:
    ContactProvider();
    static Fred::InfoContactData create_contact(Fred::OperationContext&, const std::string&, const std::string&);
    std::vector< Fred::InfoContactData > contact_;
};

class ObjectsProvider:private Fixture::instantiate_db_template,
                      protected Fred::OperationContextCreator,
                      protected RegistrarProvider,
                      protected ContactProvider
{
public:
    ObjectsProvider();
    ~ObjectsProvider() { }
};


}//namespace Test

#endif//FIXTURE_H_9B8B417BFEDF100B23D6A39540F8D033
