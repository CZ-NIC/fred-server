/*
 * Copyright (C) 2018-2022  CZ.NIC, z. s. p. o.
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
#include "src/backend/admin/registrar/create_registrar.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrar/create_registrar.hh"
#include "util/log/context.hh"
#include "util/log/logger.hh"

namespace Admin {
namespace Registrar {

namespace {

class LogContext
{
public:
    explicit LogContext(const std::string& _op_name)
            : ctx_operation_(_op_name)
    {
    }

private:
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR) LogContext CTX_VAR(__FUNCTION__);

} // namespace Admin::Registrar::{anonymous}

const char* CreateRegistrarException::what() const noexcept
{
    return "Failed to create registrar due to an unknown exception.";
}

const char* RegistrarAlreadyExists::what() const noexcept
{
    return "Registrar already exists in database.";
}

const char* VariableSymbolAlreadyExists::what() const noexcept
{
    return "Variable symbol already exists in database.";
}

unsigned long long create_registrar(const std::string& _handle,
                const boost::optional<std::string>& _name,
                const boost::optional<std::string>& _organization,
                const boost::optional<std::string>& _street1,
                const boost::optional<std::string>& _street2,
                const boost::optional<std::string>& _street3,
                const boost::optional<std::string>& _city,
                const boost::optional<std::string>& _state_or_province,
                const boost::optional<std::string>& _postal_code,
                const boost::optional<std::string>& _country,
                const boost::optional<std::string>& _telephone,
                const boost::optional<std::string>& _fax,
                const boost::optional<std::string>& _email,
                const boost::optional<std::string>& _url,
                boost::optional<bool> _system,
                const boost::optional<std::string>& _ico,
                const boost::optional<std::string>& _dic,
                const boost::optional<std::string>& _variable_symbol,
                const boost::optional<std::string>& _payment_memo_regex,
                boost::optional<bool> _vat_payer,
                bool _internal)
{
    LOGGING_CONTEXT(log_ctx);
    LOGGER.debug("Registrar handle: " + _handle);
    const std::string operation_name = "LibFred::CreateRegistrar() ";

    LibFred::OperationContextCreator ctx;

    TRACE("[CALL] " + operation_name);
    LibFred::CreateRegistrar registrar(_handle);
    if (_name != boost::none)
    {
        registrar.set_name(_name.get());
    }
    if (_organization != boost::none)
    {
        registrar.set_organization(_organization.get());
    }
    if (_street1 != boost::none)
    {
        registrar.set_street1(_street1.get());
    }
    if (_street2 != boost::none)
    {
        registrar.set_street2(_street2.get());
    }
    if (_street3 != boost::none)
    {
        registrar.set_street3(_street3.get());
    }
    if (_city != boost::none)
    {
        registrar.set_city(_city.get());
    }
    if (_state_or_province != boost::none)
    {
        registrar.set_stateorprovince(_state_or_province.get());
    }
    if (_postal_code != boost::none)
    {
        registrar.set_postalcode(_postal_code.get());
    }
    if (_country != boost::none)
    {
        registrar.set_country(_country.get());
    }
    if (_telephone != boost::none)
    {
        registrar.set_telephone(_telephone.get());
    }
    if (_fax != boost::none)
    {
        registrar.set_fax(_fax.get());
    }
    if (_email != boost::none)
    {
        registrar.set_email(_email.get());
    }
    if (_url != boost::none)
    {
        registrar.set_url(_url.get());
    }
    if (_system != boost::none)
    {
        registrar.set_system(_system.get());
    }
    if (_ico != boost::none)
    {
        registrar.set_ico(_ico.get());
    }
    if (_dic != boost::none)
    {
        registrar.set_dic(_dic.get());
    }
    if (_variable_symbol != boost::none)
    {
        registrar.set_variable_symbol(_variable_symbol.get());
    }
    if (_payment_memo_regex != boost::none)
    {
        registrar.set_payment_memo_regex(_payment_memo_regex.get());
    }
    if (_vat_payer != boost::none)
    {
        registrar.set_vat_payer(_vat_payer.get());
    }
    registrar.set_internal(_internal);

    try {
        const unsigned long long id = registrar.exec(ctx);
        ctx.commit_transaction();
        return id;
    }
    catch (const ::LibFred::CreateRegistrar::Exception& e)
    {
        if (e.is_set_invalid_registrar_handle())
        {
            LOGGER.warning(operation_name + e.what());
            throw RegistrarAlreadyExists();
        }
        if (e.is_set_invalid_registrar_varsymb())
        {
            LOGGER.warning(operation_name + e.what());
            throw VariableSymbolAlreadyExists();
        }
        LOGGER.error(operation_name + e.what());
        throw CreateRegistrarException();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(operation_name + e.what());
        throw CreateRegistrarException();
    }
    throw CreateRegistrarException();

}

} // namespace Admin::Registrar
} // namespace Admin
