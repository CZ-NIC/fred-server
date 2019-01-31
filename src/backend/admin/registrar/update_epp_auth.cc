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

#include "src/backend/admin/registrar/update_epp_auth.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrar/epp_auth/add_registrar_epp_auth.hh"
#include "libfred/registrar/epp_auth/clone_registrar_epp_auth.hh"
#include "libfred/registrar/epp_auth/delete_registrar_epp_auth.hh"
#include "libfred/registrar/epp_auth/exceptions.hh"
#include "libfred/registrar/epp_auth/get_registrar_epp_auth.hh"
#include "libfred/registrar/epp_auth/update_registrar_epp_auth.hh"
#include "util/log/context.hh"
#include "util/log/logger.hh"

#include <set>
#include <string>

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

unsigned long long add_epp_auth(const std::string& _registrar_handle,
        const std::string& _certificate_fingerprint,
        const std::string& _plain_password)
{
    LOGGING_CONTEXT(log_ctx);
    LOGGER.debug("Registrar handle: " + _registrar_handle +
                          ", certificate: " + _certificate_fingerprint +
                          ", password: " + _plain_password);

    LibFred::OperationContextCreator ctx;
    try
    {
        TRACE("[CALL] LibFred::Registrar::EppAuth::AddRegistrarEppAuth()");
        const unsigned long long id = LibFred::Registrar::EppAuth::AddRegistrarEppAuth(_registrar_handle,
                    _certificate_fingerprint,
                    _plain_password).exec(ctx);
        ctx.commit_transaction();
        return id;
    }
    catch (const LibFred::Registrar::EppAuth::NonexistentRegistrar& e)
    {
        LOGGER.warning(e.what());
        throw EppAuthNonexistentRegistrar();
    }
    catch (const LibFred::Registrar::EppAuth::DuplicateCertificate& e)
    {
        LOGGER.warning(e.what());
        throw DuplicateCertificate();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw AddEppAuthException();
    }
}

void update_epp_auth(const EppAuthData& _auth_data)
{
    LOGGING_CONTEXT(log_ctx);
    LOGGER.debug("Registrar handle: " + _auth_data.registrar_handle);

    LibFred::OperationContextCreator ctx;

    TRACE("[CALL] LibFred::Registrar::EppAuth::GetRegistrarEppAuth()");
    const LibFred::Registrar::EppAuth::RegistrarEppAuthData epp_auth_data =
            LibFred::Registrar::EppAuth::GetRegistrarEppAuth(_auth_data.registrar_handle).exec(ctx);
    std::set<unsigned long long> auth_ids;
    std::for_each(epp_auth_data.epp_auth_records.cbegin(),
            epp_auth_data.epp_auth_records.cend(),
            [&auth_ids](const auto& record) { return auth_ids.insert(record.id); });

    for (const auto& record : _auth_data.epp_auth_records)
    {
        unsigned long long auth_id = record.id;
        const std::string password = record.plain_password;
        const std::string certificate = record.certificate_fingerprint;
        const std::string second_cert = record.new_certificate_fingerprint;
        const bool is_new_acl_record = (auth_id == 0);

        std::stringstream debug_info;
        debug_info << "Registrar handle: " << _auth_data.registrar_handle << " auth_id: " << auth_id;
        debug_info << " password: " << password << " cert: " << certificate << " newcert: " << second_cert;
        LOGGER.debug(debug_info.str());

        if (is_new_acl_record)
        {
            const std::string operation_name = "Libfred::Registrar::EppAuth::AddRegistrarEppAuth()";

            const bool has_mandatory_data = !password.empty() && !certificate.empty();
            if (!has_mandatory_data)
            {
                LOGGER.warning(operation_name + " Missing mandatory params: " +
                        (password.empty()? "password ": "") + (certificate.empty()? "certificate": ""));
                throw EppAuthMissingParameters();
            }
            try
            {
                TRACE("[CALL] " + operation_name);
                auth_id = LibFred::Registrar::EppAuth::AddRegistrarEppAuth(
                                _auth_data.registrar_handle,
                                certificate,
                                password)
                            .exec(ctx);
            }
            catch (const LibFred::Registrar::EppAuth::NonexistentRegistrar& e)
            {
                LOGGER.warning(operation_name + e.what());
                throw EppAuthNonexistentRegistrar();
            }
            catch (const LibFred::Registrar::EppAuth::DuplicateCertificate& e)
            {
                LOGGER.warning(operation_name + e.what());
                throw DuplicateCertificate();
            }
            catch (const std::exception& e)
            {
                LOGGER.error(operation_name + e.what());
                throw UpdateEppAuthException();
            }
        }
        else
        {
            const std::string operation_name = "Libfred::Registrar::EppAuth::UpdateRegistrarEppAuth()";
            const bool has_mandatory_data = !password.empty() || !certificate.empty();
            if (!has_mandatory_data)
            {
                LOGGER.info(operation_name + "No data for update.");
                throw EppAuthNoUpdateData();
            }
            try
            {
                TRACE("[CALL] " + operation_name);
                LibFred::Registrar::EppAuth::UpdateRegistrarEppAuth(auth_id)
                        .set_certificate_fingerprint(certificate)
                        .set_plain_password(password)
                        .exec(ctx);
            }
            catch (const LibFred::Registrar::EppAuth::NonexistentRegistrarEppAuth& e)
            {
                LOGGER.warning(operation_name + e.what());
                throw NonexistentEppAuth();
            }
            catch (const LibFred::Registrar::EppAuth::DuplicateCertificate& e)
            {
                LOGGER.warning(operation_name + e.what());
                throw DuplicateCertificate();
            }
            catch (const std::exception& e)
            {
                LOGGER.error(operation_name + e.what());
                throw UpdateEppAuthException();
            }
            auth_ids.erase(auth_id);
        }
        if (!second_cert.empty())
        {
            const std::string operation_name = "Libfred::Registrar::EppAuth::CloneRegistrarEppAuth()";
            try
            {
                TRACE("[CALL] " + operation_name);
                LibFred::Registrar::EppAuth::CloneRegistrarEppAuth(auth_id, second_cert).exec(ctx);
            }
            catch (const LibFred::Registrar::EppAuth::DuplicateCertificate& e)
            {
                LOGGER.warning(operation_name + e.what());
                throw DuplicateCertificate();
            }
            catch (const std::exception& e)
            {
                LOGGER.error(operation_name + e.what());
                throw UpdateEppAuthException();
            }
        }
    }

    for (const auto id : auth_ids)
    {
        const std::string operation_name = "Libfred::Registrar::EppAuth::DeleteRegistrarEppAuth()";
        try
        {
            TRACE("[CALL] " + operation_name);
            LibFred::Registrar::EppAuth::DeleteRegistrarEppAuth(id).exec(ctx);
        }
        catch (const std::exception& e)
        {
            LOGGER.error(operation_name + e.what());
            throw UpdateEppAuthException();
        }
    }
    ctx.commit_transaction();
}

EppAuthRecord::EppAuthRecord()
    : id(0)
{
}

bool EppAuthRecord::operator==(const EppAuthRecord& _other) const
{
    return (id == _other.id &&
            certificate_fingerprint == _other.certificate_fingerprint &&
            plain_password == _other.plain_password &&
            new_certificate_fingerprint == _other.new_certificate_fingerprint);
}

const char* AddEppAuthException::what() const noexcept
{
    return "Failed to add registrar EPP authentication due to an unknown exception.";
}

const char* UpdateEppAuthException::what() const noexcept
{
    return "Failed to update registrar EPP authentication due to an unknown exception.";
}

const char* EppAuthMissingParameters::what() const noexcept
{
    return "Failed to add registrar EPP authentication due to missing mandatory parameter.";
}

const char* EppAuthNoUpdateData::what() const noexcept
{
    return "No data for update registrar EPP authentication.";
}

const char* EppAuthNonexistentRegistrar::what() const noexcept
{
    return "Failed to add registrar EPP authentication because the registrar doesn't exist.";
}

const char* NonexistentEppAuth::what() const noexcept
{
    return "Failed to update registrar EPP authentication because the authentication doesn't exist.";
}

const char* DuplicateCertificate::what() const noexcept
{
    return "Failed to add registrar EPP authentication due to duplicate certificate.";
}

} // namespace Registrar
} // namespace Admin
