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

#ifndef FIXTURE_H_266C91BC21244C9C83BA2220FF3A82DB
#define FIXTURE_H_266C91BC21244C9C83BA2220FF3A82DB

#include "tests/interfaces/epp/fixture.h"
#include "src/epp/keyset/check_keyset_config_data.h"
#include "src/epp/keyset/create_keyset_config_data.h"
#include "src/epp/keyset/create_keyset_localized.h"
#include "src/epp/keyset/delete_keyset_config_data.h"
#include "src/epp/keyset/info_keyset_config_data.h"
#include "src/epp/keyset/transfer_keyset_config_data.h"
#include "src/epp/keyset/update_keyset_config_data.h"
#include "src/fredlib/keyset/handle_state.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

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
    Keyset(Fred::OperationContext& _ctx, const std::string& _registrar_handle) {
        handle = "KEYSET";
        Fred::CreateKeyset(handle, _registrar_handle).exec(_ctx);
    }
};

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

class ObjectsProvider : private instantiate_db_template,
                        public RegistrarProvider,
                        public ContactProvider
{
public:
    ObjectsProvider();
    ~ObjectsProvider() { }
    template < Fred::Keyset::HandleState::Registrability REGISTRABILITY,
               Fred::Keyset::HandleState::SyntaxValidity VALIDITY >
    static std::string get_keyset_handle(Fred::OperationContext&);
};

} // namespace Test::Backend::Epp::Keyset
} // namespace Test::Backend::Epp
} // namespace Test::Backend
} // namespace Test

#endif
