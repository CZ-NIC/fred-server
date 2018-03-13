#include "src/backend/contact_verification/contact_verification_impl.hh"

#include "src/backend/contact_verification/public_request_contact_verification_impl.hh"
#include "src/backend/contact_verification/public_request_contact_verification_wrapper.hh"
#include "src/bin/corba/connection_releaser.hh"
#include "src/libfred/contact_verification/contact.hh"
#include "src/libfred/contact_verification/contact_verification_state.hh"
#include "src/libfred/contact_verification/contact_verification_validators.hh"
#include "src/libfred/db_settings.hh"
#include "src/libfred/object_states.hh"
#include "src/libfred/public_request/public_request_impl.hh"
#include "src/libfred/registrable_object/contact.hh"
#include "src/libfred/registry.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/factory_check.hh"
#include "src/util/log/logger.hh"
#include "src/util/random.hh"
#include "src/util/types/birthdate.hh"
#include "src/util/types/stringify.hh"
#include "src/util/util.hh"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include <stdexcept>
#include <utility>


static const std::string create_ctx_name(const std::string& _name)
{
    return str(boost::format("%1%-<%2%>") % _name % Random::integer(0, 10000));
}


namespace Fred {
namespace Backend {
namespace ContactVerification {

static Fred::Backend::ContactVerification::DATA_VALIDATION_ERROR
create_data_validation_error_not_available()
{
    Fred::Backend::ContactVerification::FIELD_ERROR_MAP errors;
    errors[LibFred::Contact::Verification::field_status]
        = Fred::Backend::ContactVerification::VALIDATION_ERROR::NOT_AVAILABLE;
    return Fred::Backend::ContactVerification::DATA_VALIDATION_ERROR(errors);
}


static void log_data_validation_error(const LibFred::Contact::Verification::DataValidationError& _ex)
{
    std::string msg("Fred::Contact::Verification::DataValidationError:");
    for (LibFred::Contact::Verification::FieldErrorMap::const_iterator it = _ex.errors.begin();
         it != _ex.errors.end();
         ++it)
    {
        msg += std::string("  ") + (it->first);
        msg += std::string(" ") + boost::lexical_cast<std::string>(it->second);
    }
    LOGGER(PACKAGE).warning(msg);
}


static void log_data_validation_error(const Fred::Backend::ContactVerification::DATA_VALIDATION_ERROR& _ex)
{
    std::string msg("Fred::Backend::ContactVerification::DATA_VALIDATION_ERROR:");
    for (Fred::Backend::ContactVerification::FIELD_ERROR_MAP::const_iterator it = _ex.errors.begin();
         it != _ex.errors.end();
         ++it)
    {
        msg += std::string("  ") + (it->first);
        msg += std::string(" ") + boost::lexical_cast<std::string>(it->second);
    }
    LOGGER(PACKAGE).warning(msg);
}


static Fred::Backend::ContactVerification::DATA_VALIDATION_ERROR
translate_data_validation_error(LibFred::Contact::Verification::DataValidationError& _ex)
{
    Fred::Backend::ContactVerification::FIELD_ERROR_MAP fem;
    for (LibFred::Contact::Verification::FieldErrorMap::const_iterator it = _ex.errors.begin()
         ; it != _ex.errors.end(); ++it)
    {
        switch (it->second)
        {
            case LibFred::Contact::Verification::NOT_AVAILABLE:
                fem[it->first] = Fred::Backend::ContactVerification::VALIDATION_ERROR::NOT_AVAILABLE;
                break;

            case LibFred::Contact::Verification::INVALID:
                fem[it->first] = Fred::Backend::ContactVerification::VALIDATION_ERROR::INVALID;
                break;

            case LibFred::Contact::Verification::REQUIRED:
                fem[it->first] = Fred::Backend::ContactVerification::VALIDATION_ERROR::REQUIRED;
                break;

            default:
                throw std::runtime_error("unknown validation error type");
                break;
        }
    }
    return Fred::Backend::ContactVerification::DATA_VALIDATION_ERROR(fem);
}


ContactVerificationImpl::ContactVerificationImpl(
        const std::string& _server_name,
        std::shared_ptr<LibFred::Mailer::Manager> _mailer)
    : registry_conf_(CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()),
      server_name_(_server_name),
      mailer_(_mailer)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx("init");
    ConnectionReleaser releaser;

    try
    {
        // factory_check - required keys are in factory
        FactoryHaveSupersetOfKeysChecker<LibFred::PublicRequest::Factory>
        ::KeyVector required_keys = boost::assign::list_of(
                PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION)(
                PublicRequest::
                PRT_CONTACT_IDENTIFICATION);

        FactoryHaveSupersetOfKeysChecker<LibFred::PublicRequest::Factory>(required_keys).check();

        // factory_check - factory keys are in database
        FactoryHaveSubsetOfKeysChecker<LibFred::PublicRequest::Factory>(
                LibFred::PublicRequest::get_enum_public_request_type()).check();

    }
    catch (std::exception& _ex)
    {
        LOGGER(PACKAGE).alert(_ex.what());
        throw;
    }
}


ContactVerificationImpl::~ContactVerificationImpl()
{
}


const std::string& ContactVerificationImpl::get_server_name()
{
    return server_name_;
}


unsigned long long ContactVerificationImpl::createConditionalIdentification(
        const std::string& contact_handle,
        const std::string& registrar_handle,
        const unsigned long long log_id,
        std::string& request_id)
{
    Logging::Context ctx_server(create_ctx_name(get_server_name()));
    Logging::Context ctx("create-conditional-identification");
    ConnectionReleaser releaser;
    try
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction trans(conn);

        // get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if (res_reg.size() == 0)
        {
            throw Fred::Backend::ContactVerification::REGISTRAR_NOT_EXISTS();
        }

        unsigned long long registrar_id = res_reg[0][0];

        // check contact availability and get id
        DBSharedPtr nodb;
        LibFred::Contact::ManagerPtr contact_mgr(
                LibFred::Contact::Manager::create(
                        nodb,
                        registry_conf_->restricted_handles));
        LibFred::NameIdPair cinfo;
        LibFred::Contact::Manager::CheckAvailType check_result;
        check_result = contact_mgr->checkAvail(
                contact_handle,
                cinfo);
        if (check_result != LibFred::Contact::Manager::CA_REGISTRED)
        {
            LOGGER(PACKAGE).warning("checkAvail CA_REGISTRED");
            throw Fred::Backend::ContactVerification::OBJECT_NOT_EXISTS();
        }

        // create request
        LibFred::PublicRequest::Type type = PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION;
        ContactIdentificationRequestPtr new_request(mailer_, type);
        new_request->setRegistrarId(registrar_id);
        new_request->setRequestId(log_id);
        new_request->addObject(
                LibFred::PublicRequest::OID(
                        cinfo.id,
                        contact_handle,
                        LibFred::PublicRequest::OT_CONTACT));
        new_request->save();

        std::vector<LibFred::PublicRequest::Type> request_type_list
            = Util::vector_of<LibFred::PublicRequest::Type>(
                PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION);

        ContactIdentificationRequestManagerPtr request_manager(mailer_);
        request_id = request_manager->getPublicRequestAuthIdentification(
                cinfo.id,
                request_type_list);

        new_request->sendPasswords();

        trans.commit();
        LOGGER(PACKAGE).info("request completed successfully");

        // return contact id
        return cinfo.id;

    }            // try
    catch (LibFred::Contact::Verification::DataValidationError& _ex)
    {
        log_data_validation_error(_ex);
        throw translate_data_validation_error(_ex);
    }
    catch (const Fred::Backend::ContactVerification::OBJECT_NOT_EXISTS& _ex)
    {
        throw;
    }
    catch (const LibFred::PublicRequest::NotApplicable& _ex)
    {
        LOGGER(PACKAGE).warning(_ex.what());
        throw create_data_validation_error_not_available();
    }
    catch (std::exception& _ex)
    {
        LOGGER(PACKAGE).error(_ex.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}


unsigned long long ContactVerificationImpl::processConditionalIdentification(
        const std::string& request_id,
        const std::string& password,
        const unsigned long long log_id)
{
    Logging::Context ctx_server(create_ctx_name(get_server_name()));
    Logging::Context ctx("process-conditional-identification");
    ConnectionReleaser releaser;
    try
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction trans(conn);

        // lock public request lock by identification
        LibFred::PublicRequest::lock_public_request_lock(request_id);

        // check request type
        Database::Result res_req = conn.exec_params(
                "SELECT eprt.name FROM public_request pr "
                " JOIN enum_public_request_type eprt ON eprt.id = pr.request_type "
                " JOIN public_request_auth pra ON pra.id = pr.id "
                " WHERE pra.identification = $1::text "
                ,
                Database::query_param_list(request_id));

        if (res_req.size() != 1)
        {
            LOGGER(PACKAGE).warning(
                    std::string("unable to find request with identification: ")
                    + request_id);
            throw Fred::Backend::ContactVerification::IDENTIFICATION_FAILED();
        }

        std::string request_type = static_cast<std::string>(res_req[0][0]);

        if (request_type != PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION)
        {
            LOGGER(PACKAGE).warning(
                    std::string("wrong type of request: ")
                    + request_type);
            throw Fred::Backend::ContactVerification::IDENTIFICATION_FAILED();
        }

        LOGGER(PACKAGE).info(
                boost::format(
                        "request data --"
                        " request_id: %1%  password: %2%  log_id: %3%")
                % request_id % password % log_id);

        ContactIdentificationRequestManagerPtr request_manager(mailer_);

        unsigned long long cid = request_manager->processAuthRequest(
                request_id,
                password,
                log_id);

        trans.commit();

        return cid;
    }            // try
    catch (LibFred::Contact::Verification::DataValidationError& _ex)
    {
        log_data_validation_error(_ex);
        throw translate_data_validation_error(_ex);
    }
    catch (LibFred::PublicRequest::PublicRequestAuth::NotAuthenticated&)
    {
        LOGGER(PACKAGE).warning("PublicRequestAuth::NotAuthenticated");
        throw Fred::Backend::ContactVerification::IDENTIFICATION_FAILED();
    }
    catch (LibFred::PublicRequest::AlreadyProcessed& _ex)
    {
        if (_ex.success)
        {
            LOGGER(PACKAGE).warning("PublicRequest::AlreadyProcessed true");
            throw Fred::Backend::ContactVerification::IDENTIFICATION_PROCESSED();
        }
        else
        {
            LOGGER(PACKAGE).warning("PublicRequest::AlreadyProcessed false");
            throw Fred::Backend::ContactVerification::IDENTIFICATION_INVALIDATED();
        }
    }
    catch (LibFred::PublicRequest::ObjectChanged&)
    {
        LOGGER(PACKAGE).warning("Object changed");
        throw Fred::Backend::ContactVerification::OBJECT_CHANGED();
    }
    catch (LibFred::PublicRequest::NotApplicable& _ex)
    {
        LOGGER(PACKAGE).warning(_ex.what());
        throw create_data_validation_error_not_available();
    }
    catch (std::exception& _ex)
    {
        LOGGER(PACKAGE).error(_ex.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}


unsigned long long ContactVerificationImpl::processIdentification(
        const std::string& contact_handle,
        const std::string& password,
        const unsigned long long log_id)
{
    Logging::Context ctx_server(create_ctx_name(get_server_name()));
    Logging::Context ctx("process-identification");
    ConnectionReleaser releaser;
    try
    {
        LOGGER(PACKAGE).info(
                boost::format(
                        "request data --"
                        " contact_handle: %1%  password: %2%  log_id: %3%")
                % contact_handle % password % log_id);

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction trans(conn);

        // check contact availability
        DBSharedPtr nodb;
        LibFred::Contact::ManagerPtr contact_mgr(
                LibFred::Contact::Manager::create(
                        nodb,
                        registry_conf_->restricted_handles));
        LibFred::NameIdPair cinfo;
        LibFred::Contact::Manager::CheckAvailType check_result;
        check_result = contact_mgr->checkAvail(
                contact_handle,
                cinfo);
        if (check_result != LibFred::Contact::Manager::CA_REGISTRED)
        {
            LOGGER(PACKAGE).warning("checkAvail CA_REGISTRED");
            throw Fred::Backend::ContactVerification::OBJECT_NOT_EXISTS();
        }

        ContactIdentificationRequestManagerPtr request_manager(mailer_);

        std::vector<LibFred::PublicRequest::Type> request_type_list =
            Util::vector_of<LibFred::PublicRequest::Type>(PublicRequest::PRT_CONTACT_IDENTIFICATION);

        if (request_manager->checkAlreadyProcessedPublicRequest(
                    cinfo.id,
                    request_type_list))
        {
            LOGGER(PACKAGE).warning("Found already processed request");
            throw Fred::Backend::ContactVerification::IDENTIFICATION_PROCESSED();
        }

        const LibFred::Contact::Verification::State contact_state =
            LibFred::Contact::Verification::get_contact_verification_state(cinfo.id);
        if (!contact_state.has_any(LibFred::Contact::Verification::State::Civm))
        {            // lost conditionallyIdentifiedContact state
            LOGGER(PACKAGE).warning("Lost 'conditionallyIdentifiedContact' state");
            LibFred::PublicRequest::cancel_public_request(
                    cinfo.id,
                    PublicRequest::PRT_CONTACT_IDENTIFICATION,
                    log_id);
            trans.commit();
            throw Fred::Backend::ContactVerification::IDENTIFICATION_INVALIDATED();
        }

        std::string request_id = request_manager->getPublicRequestAuthIdentification(
                cinfo.id,
                request_type_list);

        request_manager->processAuthRequest(
                request_id,
                password,
                log_id);

        trans.commit();

        return cinfo.id;
    }
    catch (const Fred::Backend::ContactVerification::OBJECT_NOT_EXISTS& _ex)
    {
        throw;
    }
    catch (LibFred::Contact::Verification::DataValidationError& _ex)
    {
        log_data_validation_error(_ex);
        throw translate_data_validation_error(_ex);
    }
    catch (const Fred::Backend::ContactVerification::DATA_VALIDATION_ERROR& _ex)
    {
        log_data_validation_error(_ex);
        throw;
    }
    catch (LibFred::NOT_FOUND& _ex)
    {
        LOGGER(PACKAGE).warning(_ex.what());
        throw Fred::Backend::ContactVerification::IDENTIFICATION_FAILED();
    }
    catch (LibFred::PublicRequest::PublicRequestAuth::NotAuthenticated&)
    {
        LOGGER(PACKAGE).warning("PublicRequestAuth::NotAuthenticated");
        throw Fred::Backend::ContactVerification::IDENTIFICATION_FAILED();
    }
    catch (LibFred::PublicRequest::AlreadyProcessed& _ex)
    {
        if (_ex.success)
        {
            LOGGER(PACKAGE).warning("PublicRequest::AlreadyProcessed true");
            throw Fred::Backend::ContactVerification::IDENTIFICATION_PROCESSED();
        }
        else
        {
            LOGGER(PACKAGE).warning("PublicRequest::AlreadyProcessed false");
            throw Fred::Backend::ContactVerification::IDENTIFICATION_INVALIDATED();
        }
    }
    catch (LibFred::PublicRequest::ObjectChanged&)
    {
        LOGGER(PACKAGE).warning("Object changed");
        throw Fred::Backend::ContactVerification::OBJECT_CHANGED();
    }
    catch (LibFred::PublicRequest::NotApplicable& _ex)
    {
        LOGGER(PACKAGE).warning(_ex.what());
        throw create_data_validation_error_not_available();
    }
    catch (const Fred::Backend::ContactVerification::IDENTIFICATION_PROCESSED&)
    {
        /* Exception is throw directly from the block above and already logged as warning.
         * This catch clause just prevents it to be caught by general block below and logged as error.
         */
        throw;
    }
    catch (Fred::Backend::ContactVerification::IDENTIFICATION_INVALIDATED&)
    {
        /* Exception is throw directly from the block above and already logged as warning.
         * This catch clause just prevents it to be caught by general block below and logged as error.
         */
        throw;
    }
    catch (std::exception& _ex)
    {
        LOGGER(PACKAGE).error(_ex.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}            // ContactVerificationImpl::processIdentification


std::string ContactVerificationImpl::getRegistrarName(const std::string& registrar_handle)
{
    Logging::Context ctx_server(create_ctx_name(get_server_name()));
    Logging::Context ctx("get-registrar-name");
    ConnectionReleaser releaser;

    try
    {

        LOGGER(PACKAGE).info(boost::format(" registrar_handle: %1%") % registrar_handle);

        Database::Connection conn = Database::Manager::acquire();

        Database::Result regname_result = conn.exec_params(
                "SELECT name FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));

        std::string registrar_name;
        if (regname_result.size() == 1)
        {
            registrar_name = static_cast<std::string>(regname_result[0][0]);
        }
        else
        {
            LOGGER(PACKAGE).warning("registrar not found");
            throw Fred::Backend::ContactVerification::OBJECT_NOT_EXISTS();
        }
        return registrar_name;
    }
    catch (const Fred::Backend::ContactVerification::OBJECT_NOT_EXISTS& _ex)
    {
        throw;
    }
    catch (std::exception& _ex)
    {
        LOGGER(PACKAGE).error(_ex.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("unknown exception");
        throw;
    }
}

} // namespace Fred::Backend::ContactVerification
} // namespace Fred::Backend
} // namespace Fred
