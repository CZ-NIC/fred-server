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
#ifndef FIXTURE_HH_3EE8595B6FD3495EA94DB11F74D71AAA
#define FIXTURE_HH_3EE8595B6FD3495EA94DB11F74D71AAA

#include "src/backend/epp/keyset/check_keyset_config_data.hh"
#include "src/backend/epp/keyset/create_keyset_config_data.hh"
#include "src/backend/epp/keyset/create_keyset_localized.hh"
#include "src/backend/epp/keyset/delete_keyset_config_data.hh"
#include "src/backend/epp/keyset/info_keyset_config_data.hh"
#include "src/backend/epp/keyset/transfer_keyset_config_data.hh"
#include "src/backend/epp/keyset/update_keyset_config_data.hh"
#include "libfred/registrable_object/keyset/handle_state.hh"
#include "test/backend/epp/contact/fixture.hh"
#include "test/backend/epp/fixture.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <string>
#include <vector>

namespace Test {
namespace Backend {
namespace Epp {
namespace Keyset {

struct DefaultCheckKeysetConfigData : ::Epp::Keyset::CheckKeysetConfigData
{
    DefaultCheckKeysetConfigData()
        : CheckKeysetConfigData(false)
    {
    }
};

struct DefaultInfoKeysetConfigData : ::Epp::Keyset::InfoKeysetConfigData
{
    DefaultInfoKeysetConfigData()
        : InfoKeysetConfigData(false)
    {
    }
};

struct DefaultCreateKeysetConfigData : ::Epp::Keyset::CreateKeysetConfigData
{
    DefaultCreateKeysetConfigData()
        : CreateKeysetConfigData(false)
    {
    }
};

struct DefaultUpdateKeysetConfigData : ::Epp::Keyset::UpdateKeysetConfigData
{
    DefaultUpdateKeysetConfigData()
        : UpdateKeysetConfigData(false)
    {
    }
};

struct DefaultDeleteKeysetConfigData : ::Epp::Keyset::DeleteKeysetConfigData
{
    DefaultDeleteKeysetConfigData()
        : DeleteKeysetConfigData(false)
    {
    }
};

struct DefaultTransferKeysetConfigData : ::Epp::Keyset::TransferKeysetConfigData
{
    DefaultTransferKeysetConfigData()
        : TransferKeysetConfigData(false)
    {
    }
};

struct Keyset {
    std::string handle;
    Keyset(::LibFred::OperationContext& _ctx, const std::string& _registrar_handle) {
        handle = "KEYSET";
        ::LibFred::CreateKeyset(handle, _registrar_handle).exec(_ctx);
    }
};

struct KeysetWithTechContact
{
    Contact::Contact tech_contact;
    std::string handle;

    KeysetWithTechContact(::LibFred::OperationContext& _ctx, const std::string& _registrar_handle)
        : tech_contact(_ctx, _registrar_handle, "KEYSETTECHCONTACT"),
          handle("KEYSET")
    {
        ::LibFred::CreateKeyset(handle, _registrar_handle)
                .set_tech_contacts(boost::assign::list_of(tech_contact.data.handle))
                .exec(_ctx);
    }
};

class RegistrarProvider
{
public:
    const ::LibFred::InfoRegistrarData& get_registrar_a()const;
    const ::LibFred::InfoRegistrarData& get_registrar_b()const;
    const ::LibFred::InfoRegistrarData& get_sys_registrar()const;
protected:
    RegistrarProvider();
private:
    static ::LibFred::InfoRegistrarData create_registrar(::LibFred::OperationContext&, const std::string&, bool);
    ::LibFred::InfoRegistrarData registrar_a_;
    ::LibFred::InfoRegistrarData registrar_b_;
    ::LibFred::InfoRegistrarData sys_registrar_;
};

class ContactProvider
{
public:
    const ::LibFred::InfoContactData& get_contact(unsigned idx)const;
protected:
    ContactProvider(unsigned number_of_contacts,
                    const RegistrarProvider &registrar_provider);
private:
    ContactProvider();
    static std::vector< ::LibFred::InfoContactData > create_contacts(unsigned,
                                                                const std::string&,
                                                                const RegistrarProvider&);
    const std::vector< ::LibFred::InfoContactData > contact_;
};

class ObjectsProvider : private instantiate_db_template,
                        public RegistrarProvider,
                        public ContactProvider
{
public:
    ObjectsProvider();
    ~ObjectsProvider() { }
    template < ::LibFred::Keyset::HandleState::Registrability REGISTRABILITY,
               ::LibFred::Keyset::HandleState::SyntaxValidity VALIDITY >
    static std::string get_keyset_handle(::LibFred::OperationContext&);
};

} // namespace Test::Backend::Epp::Keyset
} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test

#endif
