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
#include "test/backend/epp/keyset/fixture.hh"
#include "src/backend/epp/keyset/impl/limits.hh"

namespace Test {
namespace Backend {
namespace Epp {
namespace Keyset {

RegistrarProvider::RegistrarProvider()
{
    ::LibFred::OperationContextCreator ctx;
    registrar_a_   = create_registrar(ctx, "REGISTRAR_A", false);
    registrar_b_   = create_registrar(ctx, "REGISTRAR_B", false);
    sys_registrar_ = create_registrar(ctx, "SYS_REGISTRAR", true);
    ctx.commit_transaction();
}

const ::LibFred::InfoRegistrarData& RegistrarProvider::get_registrar_a()const
{
    return registrar_a_;
}

const ::LibFred::InfoRegistrarData& RegistrarProvider::get_registrar_b()const
{
    return registrar_b_;
}

const ::LibFred::InfoRegistrarData& RegistrarProvider::get_sys_registrar()const
{
    return sys_registrar_;
}

LibFred::InfoRegistrarData RegistrarProvider::create_registrar(
    ::LibFred::OperationContext &ctx,
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
            const ::LibFred::InfoRegistrarData data =
                ::LibFred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
            if (data.system.get_value_or(false) == system_registrar) {
                return data;
            }
        }
        catch (const ::LibFred::InfoRegistrarByHandle::Exception &e) {
            if (!e.is_set_unknown_registrar_handle()) {
                throw;
            }
            ::LibFred::CreateRegistrar(registrar_handle).set_system(system_registrar).exec(ctx);
            return ::LibFred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
        }
    }
}

ContactProvider::ContactProvider(unsigned number_of_contacts,
                                 const RegistrarProvider &registrar_provider)
:   contact_(create_contacts(number_of_contacts, "CONTACT", registrar_provider))
{ }

const ::LibFred::InfoContactData& ContactProvider::get_contact(unsigned idx)const
{
    return contact_[idx];
}

std::vector< ::LibFred::InfoContactData > ContactProvider::create_contacts(unsigned number_of_contacts,
                                                                      const std::string &handle,
                                                                      const RegistrarProvider &registrar_provider)
{
    ::LibFred::OperationContextCreator ctx;
    std::vector< ::LibFred::InfoContactData > result;
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
            result.push_back(::LibFred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data);
        }
        catch (const ::LibFred::InfoContactByHandle::Exception &e) {
            if (!e.is_set_unknown_contact_handle()) {
                throw;
            }
            const int registrar_idx = result.size() % 3;
            const std::string registrar = registrar_idx == 0 ? registrar_provider.get_registrar_a().handle :
                                          registrar_idx == 1 ? registrar_provider.get_registrar_b().handle :
                                                               registrar_provider.get_sys_registrar().handle;
            ::LibFred::CreateContact(contact_handle, registrar).exec(ctx);
            result.push_back(::LibFred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data);
        }
    }
    ctx.commit_transaction();
    return result;
}

ObjectsProvider::ObjectsProvider()
:   RegistrarProvider(),
    ContactProvider(::Epp::Keyset::max_number_of_tech_contacts,
                    static_cast< const RegistrarProvider& >(*this))
{
}

template < >
std::string ObjectsProvider::get_keyset_handle< ::LibFred::Keyset::HandleState::available,
                                                ::LibFred::Keyset::HandleState::valid >(::LibFred::OperationContext &ctx)
{
    for (unsigned cnt = 0; true; ++cnt) {
        std::string handle = "KEYSET";
        if (0 < cnt) {
            std::ostringstream out;
            out << "-" << cnt;
            handle += out.str();
        }
        if (::LibFred::Keyset::get_handle_registrability(ctx, handle) == ::LibFred::Keyset::HandleState::available) {
            return handle;
        }
    }
}

} // namespace Test::Backend::Epp::Keyset
} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test
