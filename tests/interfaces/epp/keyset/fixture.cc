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

#include "tests/interfaces/epp/keyset/fixture.h"
#include "src/epp/keyset/impl/limits.h"

namespace Test {

RegistrarProvider::RegistrarProvider()
{
    Fred::OperationContextCreator ctx;
    registrar_a_   = create_registrar(ctx, "REGISTRAR_A", false);
    registrar_b_   = create_registrar(ctx, "REGISTRAR_B", false);
    sys_registrar_ = create_registrar(ctx, "SYS_REGISTRAR", true);
    ctx.commit_transaction();
}

const Fred::InfoRegistrarData& RegistrarProvider::get_registrar_a()const
{
    return registrar_a_;
}

const Fred::InfoRegistrarData& RegistrarProvider::get_registrar_b()const
{
    return registrar_b_;
}

const Fred::InfoRegistrarData& RegistrarProvider::get_sys_registrar()const
{
    return sys_registrar_;
}

Fred::InfoRegistrarData RegistrarProvider::create_registrar(
    Fred::OperationContext &ctx,
    const std::string &handle,
    bool system_registrar)
{
    for (int cnt = 0; true; ++cnt) {
        std::string registrar_handle;
        try {
            registrar_handle = handle;
            if (0 < cnt) {
                std::ostringstream out;
                out << cnt;
                registrar_handle += out.str();
            }
            const Fred::InfoRegistrarData data =
                Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
            if (data.system.get_value_or(false) == system_registrar) {
                return data;
            }
        }
        catch (const Fred::InfoRegistrarByHandle::Exception &e) {
            if (!e.is_set_unknown_registrar_handle()) {
                throw;
            }
            Fred::CreateRegistrar(registrar_handle).set_system(system_registrar).exec(ctx);
            return Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
        }
    }
}

ContactProvider::ContactProvider(unsigned number_of_contacts,
                                 const RegistrarProvider &registrar_provider)
:   contact_(create_contacts(number_of_contacts, "CONTACT", registrar_provider))
{ }

const Fred::InfoContactData& ContactProvider::get_contact(unsigned idx)const
{
    return contact_[idx];
}

std::vector< Fred::InfoContactData > ContactProvider::create_contacts(unsigned number_of_contacts,
                                                                      const std::string &handle,
                                                                      const RegistrarProvider &registrar_provider)
{
    Fred::OperationContextCreator ctx;
    std::vector< Fred::InfoContactData > result;
    result.reserve(number_of_contacts);
    for (int cnt = 0; result.size() < number_of_contacts; ++cnt) {
        std::string contact_handle;
        try {
            contact_handle = handle;
            if (0 < cnt) {
                std::ostringstream out;
                out << cnt;
                contact_handle += out.str();
            }
            result.push_back(Fred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data);
        }
        catch (const Fred::InfoContactByHandle::Exception &e) {
            if (!e.is_set_unknown_contact_handle()) {
                throw;
            }
            const int registrar_idx = result.size() % 3;
            const std::string registrar = registrar_idx == 0 ? registrar_provider.get_registrar_a().handle :
                                          registrar_idx == 1 ? registrar_provider.get_registrar_b().handle :
                                                               registrar_provider.get_sys_registrar().handle;
            Fred::CreateContact(contact_handle, registrar).exec(ctx);
            result.push_back(Fred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data);
        }
    }
    ctx.commit_transaction();
    return result;
}

ObjectsProvider::ObjectsProvider()
:   RegistrarProvider(),
    ContactProvider(Epp::Keyset::max_number_of_tech_contacts,
                    static_cast< const RegistrarProvider& >(*this))
{
}

template < >
std::string ObjectsProvider::get_keyset_handle< Fred::Keyset::HandleState::available,
                                                Fred::Keyset::HandleState::valid >(Fred::OperationContext &ctx)
{
    for (unsigned cnt = 0; true; ++cnt) {
        std::string handle = "KEYSET";
        if (0 < cnt) {
            std::ostringstream out;
            out << "-" << cnt;
            handle += out.str();
        }
        if (Fred::Keyset::get_handle_registrability(ctx, handle) == Fred::Keyset::HandleState::available) {
            return handle;
        }
    }
}

}//namespace Test
