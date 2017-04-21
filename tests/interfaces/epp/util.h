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

#ifndef UTIL_H_A4B759EF1350488596C500C9CC949C14
#define UTIL_H_A4B759EF1350488596C500C9CC949C14

#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

#include "src/epp/exception.h"

#include <boost/test/test_tools.hpp>

namespace Test {
namespace Backend {
namespace Epp {

struct autocommitting_context
    : virtual instantiate_db_template
{
    Fred::OperationContextCreator ctx;


    virtual ~autocommitting_context()
    {
        ctx.commit_transaction();
    }


};

struct autorollbacking_context
    : virtual instantiate_db_template
{
    Fred::OperationContextCreator ctx;

};

class RegistrarProvider
{
public:
    static const Fred::InfoRegistrarData& get_registrar_a();


    static const Fred::InfoRegistrarData& get_registrar_b();


    static const Fred::InfoRegistrarData& get_sys_registrar();


private:
    RegistrarProvider();


    explicit RegistrarProvider(Fred::OperationContext&);


    static Fred::InfoRegistrarData create_registrar(
            Fred::OperationContext&,
            const std::string&,
            bool);


    static const RegistrarProvider& get_const_instance();


    const Fred::InfoRegistrarData registrar_a_;
    const Fred::InfoRegistrarData registrar_b_;
    const Fred::InfoRegistrarData sys_registrar_;
};

class ContactProvider
{
public:
    static const Fred::InfoContactData& get_contact(unsigned idx);


private:
    ContactProvider();


    explicit ContactProvider(
            Fred::OperationContext&,
            unsigned);


    static Fred::InfoContactData create_contact(
            Fred::OperationContext&,
            const std::string&,
            const std::string&);


    static const ContactProvider& get_const_instance();


    std::vector<Fred::InfoContactData> contact_;
};

} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test

#endif
