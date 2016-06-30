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
#include "src/fredlib/keyset/handle_state.h"

#include <vector>

namespace Test {

class RegistrarProvider
{
public:
    const Fred::InfoRegistrarData& get_registrar_a()const;
    const Fred::InfoRegistrarData& get_registrar_b()const;
    const Fred::InfoRegistrarData& get_sys_registrar()const;
protected:
    RegistrarProvider();
private:
    static Fred::InfoRegistrarData create_registrar(Fred::OperationContext&, const std::string&, bool);
    Fred::InfoRegistrarData registrar_a_;
    Fred::InfoRegistrarData registrar_b_;
    Fred::InfoRegistrarData sys_registrar_;
};

class ContactProvider
{
public:
    const Fred::InfoContactData& get_contact(unsigned idx)const;
protected:
    ContactProvider(unsigned number_of_contacts,
                    const RegistrarProvider &registrar_provider);
private:
    ContactProvider();
    static std::vector< Fred::InfoContactData > create_contacts(unsigned,
                                                                const std::string&,
                                                                const RegistrarProvider&);
    const std::vector< Fred::InfoContactData > contact_;
};

class ObjectsProvider:private Fixture::instantiate_db_template,
                      public RegistrarProvider,
                      public ContactProvider
{
public:
    ObjectsProvider();
    ~ObjectsProvider() { }
    template < Fred::KeySet::HandleState::Registrability REGISTRABILITY >
    static std::string get_keyset_handle(Fred::OperationContext&);
};


}//namespace Test

#endif//FIXTURE_H_9B8B417BFEDF100B23D6A39540F8D033
