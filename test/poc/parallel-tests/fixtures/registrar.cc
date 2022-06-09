/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#include "test/poc/parallel-tests/fixtures/registrar.hh"

#include "libfred/registrar/info_registrar.hh"

#include <stdexcept>


namespace {

decltype(auto) make_registrar(LibFred::OperationContext& ctx, LibFred::CreateRegistrar& create)
{
    return LibFred::InfoRegistrarById{create.exec(ctx)}.exec(ctx).info_registrar_data;
}

std::string get_version(int index)
{
   if (index < 0)
   {
       throw std::runtime_error{"negative index is not allowed"};
   }
   static constexpr char number_of_letters = 'Z' - 'A';
   if (index <= number_of_letters)
   {
       return std::string(1, 'A' + index);
   }
   return std::to_string(index - number_of_letters);
}

void set_registrar(LibFred::CreateRegistrar& op, bool system, int index)
{
    const auto version = get_version(index);
    if (system)
   {
       op.set_name(version + " System Registrar")
         .set_organization(version + " System Registrar Ltd")
         .set_email("system-registrar-" + version + "@nic.com")
         .set_url("https://system-registrar-" + version + ".nic.com")
         .set_system(true);
   }
   else
   {
       op.set_name(version + " Registrar")
         .set_organization(version + " Registrar Gmbh.")
         .set_email("registrar-" + version + "@registrar.com")
         .set_url("https://registrar-" + version + ".registrar.com");
   }
   op.set_street1("Street 1 - " + version)
     .set_street2("Street 2 - " + version)
     .set_street3("Street 3 - " + version)
     .set_city("City " + version)
     .set_stateorprovince("State Or Province " + version)
     .set_postalcode("143 21")
     .set_country("CZ")
     .set_telephone("+420.441207848")
     .set_fax("+420.361971091")
     .set_ico("1234" + version)
     .set_dic("5678" + version)
     .set_variable_symbol("VS-" + std::to_string(2 * index + (system ? 1 : 0)))
     .set_payment_memo_regex(version)
     .set_vat_payer(index % 2 != 0);
}

}//namespace {anonymous}

namespace Test {

Registrar::Registrar(LibFred::OperationContext& ctx, LibFred::CreateRegistrar create)
    : data{make_registrar(ctx, create)}
{ }

SystemRegistrar::SystemRegistrar(LibFred::OperationContext& ctx, LibFred::CreateRegistrar create)
    : data{make_registrar(ctx, create.set_system(true))}
{ }

}//namespace Test

using namespace Test::Setter;

LibFred::CreateRegistrar Test::Setter::registrar(LibFred::CreateRegistrar create, int index)
{
    set_registrar(create, false, index);
    return create;
}

LibFred::CreateRegistrar Test::Setter::system_registrar(LibFred::CreateRegistrar create, int index)
{
    set_registrar(create, true, index);
    return create;
}
