/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 */

#ifndef UTIL_HH_63802C73227A4AB2A7CEB864FFA71E62
#define UTIL_HH_63802C73227A4AB2A7CEB864FFA71E62

#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include "src/backend/epp/exception.hh"

#include <boost/test/test_tools.hpp>

namespace Test {
namespace Backend {
namespace Epp {

struct autocommitting_context
    : virtual instantiate_db_template
{
    ::LibFred::OperationContextCreator ctx;


    virtual ~autocommitting_context()
    {
        ctx.commit_transaction();
    }


};

struct autorollbacking_context
    : virtual instantiate_db_template
{
    ::LibFred::OperationContextCreator ctx;

};

class RegistrarProvider
{
public:
    static const ::LibFred::InfoRegistrarData& get_registrar_a();


    static const ::LibFred::InfoRegistrarData& get_registrar_b();


    static const ::LibFred::InfoRegistrarData& get_sys_registrar();


private:
    RegistrarProvider();


    explicit RegistrarProvider(::LibFred::OperationContext&);


    static ::LibFred::InfoRegistrarData create_registrar(
            ::LibFred::OperationContext&,
            const std::string&,
            bool);


    static const RegistrarProvider& get_const_instance();


    const ::LibFred::InfoRegistrarData registrar_a_;
    const ::LibFred::InfoRegistrarData registrar_b_;
    const ::LibFred::InfoRegistrarData sys_registrar_;
};

class ContactProvider
{
public:
    static const ::LibFred::InfoContactData& get_contact(unsigned idx);


private:
    ContactProvider();


    explicit ContactProvider(
            ::LibFred::OperationContext&,
            unsigned);


    static ::LibFred::InfoContactData create_contact(
            ::LibFred::OperationContext&,
            const std::string&,
            const std::string&);


    static const ContactProvider& get_const_instance();


    std::vector<::LibFred::InfoContactData> contact_;
};

} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test

#endif
