/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/accounting/accounting.hh"

#include "src/backend/accounting/exceptions.hh"
#include "src/backend/accounting/impl/accounting.hh"
#include "src/backend/accounting/impl/exceptions.hh"
#include "util/log/context.hh"

namespace Fred {
namespace Backend {
namespace Accounting {

namespace {

std::string create_ctx_function_name(const char* fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

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

#define LOGGING_CONTEXT(CTX_VAR) LogContext CTX_VAR(create_ctx_function_name(__FUNCTION__))

} // namespace Fred::Backend::Accounting::{anonymous}

Fred::Backend::Accounting::Registrar get_registrar_by_payment(
        const PaymentData& _payment_data,
        std::string& _zone)
{
    LOGGING_CONTEXT(log_ctx);
    try
    {
        LibFred::OperationContextCreator ctx;

        _zone = Impl::get_zone_by_payment(
                ctx,
                _payment_data);
        return Impl::get_registrar_by_payment(
                ctx,
                _payment_data);
    }
    catch (const Impl::RegistrarNotFound& e)
    {
        LOGGER.info(e.what());
        throw RegistrarNotFound();
    }
    catch (const Impl::ZoneNotFound& e)
    {
        LOGGER.info(e.what());
        throw InvalidPaymentData();
    }
    catch (const Impl::InvalidAccountData& e)
    {
        LOGGER.info(e.what());
        throw InvalidPaymentData();
    }
    catch (const Impl::InvalidPaymentData& e)
    {
        LOGGER.info(e.what());
        throw InvalidPaymentData();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("an unexpected exception");
        throw;
    }
}

Fred::Backend::Accounting::Registrar get_registrar_by_handle_and_payment(
        const std::string& _registrar_handle,
        const PaymentData& _payment_data,
        std::string& _zone)
{
    LOGGING_CONTEXT(log_ctx);
    try
    {
        LibFred::OperationContextCreator ctx;

        _zone = Impl::get_zone_by_payment(
                ctx,
                _payment_data);
        return Impl::get_registrar_by_handle(
                ctx,
                _registrar_handle);
    }
    catch (const Impl::RegistrarNotFound& e)
    {
        LOGGER.info(e.what());
        throw RegistrarNotFound();
    }
    catch (const Impl::ZoneNotFound& e)
    {
        LOGGER.info(e.what());
        throw InvalidPaymentData();
    }
    catch (const Impl::InvalidAccountData& e)
    {
        LOGGER.info(e.what());
        throw InvalidPaymentData();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("an unexpected exception");
        throw;
    }
}

PaymentInvoices import_payment(
        const PaymentData& _payment_data)
{
    LOGGING_CONTEXT(log_ctx);
    try
    {
        return Impl::import_payment(
                _payment_data);
    }
    catch (const Impl::RegistrarNotFound& e)
    {
        LOGGER.info(e.what());
        throw RegistrarNotFound();
    }
    catch (const Impl::InvalidAccountData& e)
    {
        LOGGER.info(e.what());
        throw InvalidPaymentData();
    }
    catch (const Impl::InvalidPaymentData& e)
    {
        LOGGER.info(e.what());
        throw InvalidPaymentData();
    }
    catch (const Impl::InvalidTaxDateFormat& e)
    {
        LOGGER.info(e.what());
        throw InvalidTaxDateFormat();
    }
    catch (const Impl::PaymentTooOld& e)
    {
        LOGGER.info(e.what());
        throw PaymentTooOld();
    }
    catch (const Impl::PaymentAlreadyProcessed& e)
    {
        LOGGER.info(e.what());
        throw PaymentAlreadyProcessed();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("an unexpected exception");
        throw;
    }
}

PaymentInvoices import_payment_by_registrar_handle(
        const PaymentData& _payment_data,
        const boost::optional<boost::gregorian::date>& _tax_date,
        const std::string& _registrar_handle)
{
    LOGGING_CONTEXT(log_ctx);
    try
    {
        return Impl::import_payment_by_registrar_handle(
                _payment_data,
                _tax_date,
                _registrar_handle);
    }
    catch (const Impl::RegistrarNotFound& e)
    {
        LOGGER.info(e.what());
        throw RegistrarNotFound();
    }
    catch (const Impl::InvalidAccountData& e)
    {
        LOGGER.info(e.what());
        throw InvalidPaymentData();
    }
    catch (const Impl::InvalidPaymentData& e)
    {
        LOGGER.info(e.what());
        throw InvalidPaymentData();
    }
    catch (const Impl::InvalidTaxDateFormat& e)
    {
        LOGGER.info(e.what());
        throw InvalidTaxDateFormat();
    }
    catch (const Impl::TaxDateTooOld& e)
    {
        LOGGER.info(e.what());
        throw TaxDateTooOld();
    }
    catch (const Impl::PaymentAlreadyProcessed& e)
    {
        LOGGER.info(e.what());
        throw PaymentAlreadyProcessed();
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("an unexpected exception");
        throw;
    }
}

std::vector<RegistrarReference> get_registrar_references()
{
    LOGGING_CONTEXT(log_ctx);
    try
    {
        LibFred::OperationContextCreator ctx;

        return Impl::get_registrar_references(ctx);
    }
    catch (const std::exception& e)
    {
        LOGGER.error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("an unexpected exception");
        throw;
    }
}

} // namespace Fred::Backend::Accounting
} // namespace Fred::Backend
} // namespace Fred
