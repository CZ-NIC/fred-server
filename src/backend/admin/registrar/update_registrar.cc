/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "src/backend/admin/registrar/update_registrar.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/exceptions.hh"
#include "src/libfred/registrar/update_registrar.hh"
#include "src/util/log/context.hh"
#include "src/util/log/logger.hh"

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

const char* UpdateRegistrarException::what() const noexcept
{
    return "Failed to update registrar due to an unknown exception.";
}

const char* UpdateRegistrarNonexistent::what() const noexcept
{
    return "Failed to update registrar because the registrar doesn't exist.";
}

const char* UpdateRegistrarNoUpdateData::what() const noexcept
{
    return "No data for update registrar.";
}

unsigned long long update_registrar(const std::string& _handle,
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
                boost::optional<bool> _vat_payer)
{
    LOGGING_CONTEXT(log_ctx);
    LOGGER(PACKAGE).debug("Registrar handle: " + _handle);
    const std::string operation_name = "LibFred::Registrar::UpdateRegistrar()";

    LibFred::OperationContextCreator ctx;

    try {
        TRACE("[CALL] " + operation_name);
        const unsigned long long id = LibFred::Registrar::UpdateRegistrar(_handle)
            .set_name(_name)
            .set_organization(_organization)
            .set_street1(_street1)
            .set_street2(_street2)
            .set_street3(_street3)
            .set_city(_city)
            .set_state_or_province(_state_or_province)
            .set_postal_code(_postal_code)
            .set_country(_country)
            .set_telephone(_telephone)
            .set_fax(_fax)
            .set_email(_email)
            .set_url(_url)
            .set_system(_system)
            .set_ico(_ico)
            .set_dic(_dic)
            .set_variable_symbol(_variable_symbol)
            .set_payment_memo_regex(_payment_memo_regex)
            .set_vat_payer(_vat_payer)
            .exec(ctx);
        ctx.commit_transaction();
        return id;
    }
    catch (const ::LibFred::Registrar::NoUpdateData& e)
    {
        LOGGER(PACKAGE).info(operation_name + e.what());
        throw UpdateRegistrarNoUpdateData();
    }
    catch (const ::LibFred::Registrar::NonExistentRegistrar& e)
    {
        LOGGER(PACKAGE).warning(operation_name + e.what());
        throw UpdateRegistrarNonexistent();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(operation_name + e.what());
        throw UpdateRegistrarException();
    }
    throw UpdateRegistrarException();
}

} // namespace Admin::Registrar
} // namespace Admin
