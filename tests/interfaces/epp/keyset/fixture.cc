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

#include "tests/interfaces/epp/keyset/fixture.h"
#include "src/epp/keyset/limits.h"

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
                                 const std::string &registrar_a,
                                 const std::string &registrar_b,
                                 const std::string &sys_registrar)
{
    Fred::OperationContextCreator ctx;
    contact_.reserve(number_of_contacts);
    for (unsigned idx = 0; idx < number_of_contacts; ++idx) {
        const std::string registrar = (idx % 3) == 0 ? registrar_a
                                                     : (idx % 3) == 1 ? registrar_b
                                                                      : sys_registrar;
        contact_.push_back(create_contact(ctx, "CONTACT", registrar));
    }
    ctx.commit_transaction();
}

const Fred::InfoContactData& ContactProvider::get_contact(unsigned idx)const
{
    return contact_[idx];
}

Fred::InfoContactData ContactProvider::create_contact(Fred::OperationContext &ctx,
                                                      const std::string &handle,
                                                      const std::string &registrar)
{
    for (int cnt = 0; true; ++cnt) {
        std::string contact_handle;
        try {
            contact_handle = handle;
            if (0 < cnt) {
                std::ostringstream out;
                out << cnt;
                contact_handle += out.str();
            }
            return Fred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data;
        }
        catch (const Fred::InfoContactByHandle::Exception &e) {
            if (!e.is_set_unknown_contact_handle()) {
                throw;
            }
            Fred::CreateContact(contact_handle, registrar).exec(ctx);
            return Fred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data;
        }
    }
}

ObjectsProvider::ObjectsProvider()
:   RegistrarProvider(),
    ContactProvider(Epp::KeySet::max_number_of_tech_contacts,
                    this->RegistrarProvider::get_registrar_a().handle,
                    this->RegistrarProvider::get_registrar_b().handle,
                    this->RegistrarProvider::get_sys_registrar().handle)
{
}

}//namespace Test
