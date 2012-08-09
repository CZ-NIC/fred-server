
#include "contact_verification/contact_identification.h"

#include "contact_verification/contact_verification_impl.h"
#include "contact_verification/public_request_impl.h"

#include "fredlib/db_settings.h"
#include "fredlib/registry.h"
#include "fredlib/contact.h"
#include "fredlib/public_request/public_request.h"
#include "fredlib/object_states.h"
#include "fredlib/contact_verification/contact.h"
#include "fredlib/contact_verification/contact_verification.h"
#include "fredlib/contact_verification/data_validation.h"
#include "util/factory_check.h"
#include "util/util.h"

#include "cfg/config_handler_decl.h"
#include "log/logger.h"
#include "random.h"
#include "corba/connection_releaser.h"
#include "types/stringify.h"
#include "types/birthdate.h"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>


static const std::string create_ctx_name(const std::string &_name)
{
    return str(boost::format("%1%-<%2%>")% _name % Random::integer(0, 10000));
}

namespace Registry
{
    namespace Contact
    {
        namespace Verification
        {
            ContactVerificationImpl::ContactVerificationImpl(const std::string &_server_name
                    , boost::shared_ptr<Fred::Mailer::Manager> _mailer)
                : registry_conf_(CfgArgs::instance()
                    ->get_handler_ptr_by_type<HandleRegistryArgs>())
                , server_name_(_server_name)
                , mailer_(_mailer)
            {
                Logging::Context ctx_server(server_name_);
                Logging::Context ctx("init");
                ConnectionReleaser releaser;

                try {
                    // factory_check - required keys are in factory
                    FactoryHaveSupersetOfKeysChecker<Fred::PublicRequest::Factory>
                    ::KeyVector required_keys = boost::assign::list_of
                        (Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION )
                        (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION);

                    FactoryHaveSupersetOfKeysChecker<Fred::PublicRequest::Factory>
                        (required_keys).check();

                    // factory_check - factory keys are in database
                    FactoryHaveSubsetOfKeysChecker<Fred::PublicRequest::Factory>
                        (Fred::PublicRequest::get_enum_public_request_type()).check();

                }
                catch (std::exception &_ex) {
                    LOGGER(PACKAGE).alert(_ex.what());
                    throw;
                }
            }//ContactVerificationImpl::ContactVerificationImpl

            ContactVerificationImpl::~ContactVerificationImpl(){}

            const std::string& ContactVerificationImpl::get_server_name()
            {
                return server_name_;
            }

            unsigned long long ContactVerificationImpl::createConditionalIdentification(
                    const std::string & contact_handle
                    , const std::string & registrar_handle
                    , const unsigned long long log_id
                    , std::string & request_id)
            {
                Logging::Context ctx_server(create_ctx_name(get_server_name()));
                Logging::Context ctx("create-conditional-identification");
                ConnectionReleaser releaser;
                try
                {
                    Database::Connection conn = Database::Manager::acquire();
                    Database::Transaction trans(conn);

                    //get registrar id
                    Database::Result res_reg = conn.exec_params(
                            "SELECT id FROM registrar WHERE handle=$1::text",
                            Database::query_param_list(registrar_handle));
                    if(res_reg.size() == 0) {
                        throw std::runtime_error("Registrar does not exist");
                    }

                    unsigned long long registrar_id = res_reg[0][0];

                    //check contact availability and get id
                    Fred::Contact::ManagerPtr contact_mgr(
                        Fred::Contact::Manager::create(
                            DBDisconnectPtr(0), registry_conf_->restricted_handles));
                    Fred::NameIdPair cinfo;
                    Fred::Contact::Manager::CheckAvailType check_result;
                    check_result = contact_mgr->checkAvail(contact_handle, cinfo);
                    if (check_result != Fred::Contact::Manager::CA_REGISTRED)
                    {
                        LOGGER(PACKAGE).error("checkAvail CA_REGISTRED");
                        throw Registry::Contact::Verification::OBJECT_NOT_EXISTS();
                    }

                    //create request
                    Fred::PublicRequest::Type type = Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION ;
                    ContactIdentificationRequestPtr new_request(mailer_,type);
                    new_request->setRegistrarId(registrar_id);
                    new_request->setRequestId(log_id);
                    new_request->addObject(
                            Fred::PublicRequest::OID(
                                cinfo.id, contact_handle, Fred::PublicRequest::OT_CONTACT));
                    new_request->save();

                    std::vector<Fred::PublicRequest::Type> request_type_list
                        = Util::vector_of<Fred::PublicRequest::Type>
                    (Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION );

                    ContactIdentificationRequestManagerPtr request_manager(mailer_);
                    request_id = request_manager->getPublicRequestAuthIdentification(
                            cinfo.id, request_type_list);

                    new_request->sendPasswords();

                    trans.commit();
                    LOGGER(PACKAGE).info("request completed successfully");

                    // return contact id
                    return cinfo.id;

                }//try
                catch (Fred::Contact::Verification::DataValidationError& _ex)
                {
                    std::string msg("Fred::Contact::Verification::DataValidationError:");
                    for (Fred::Contact::Verification::FieldErrorMap::const_iterator it = _ex.errors.begin()
                            ; it != _ex.errors.end(); ++it) {
                        msg+=std::string("  ")+(it->first);
                        msg+=std::string(" ")+ boost::lexical_cast<std::string>(it->second);
                    }
                    LOGGER(PACKAGE).error(msg);
                    throw;
                }
                catch (Fred::PublicRequest::NotApplicable &_ex)
                {
                    LOGGER(PACKAGE).error(_ex.what());
                    Fred::Contact::Verification::FieldErrorMap errors;
                    errors[Fred::Contact::Verification::field_status]
                           = Fred::Contact::Verification::NOT_AVAILABLE;
                    throw Fred::Contact::Verification::DataValidationError((errors));
                }
                catch (std::exception &_ex)
                {
                    LOGGER(PACKAGE).error(_ex.what());
                    throw;
                }
                catch (...)
                {
                    LOGGER(PACKAGE).error("unknown exception");
                    throw;
                }
            }//ContactVerificationImpl::createConditionalIdentification

            unsigned long long ContactVerificationImpl::processConditionalIdentification(
                    const std::string & request_id
                    , const std::string & password
                    , const unsigned long long  log_id)
            {
                Logging::Context ctx_server(create_ctx_name(get_server_name()));
                Logging::Context ctx("process-conditional-identification");
                ConnectionReleaser releaser;
                try
                {
                    Database::Connection conn = Database::Manager::acquire();
                    Database::Transaction trans(conn);

                    //check request type
                    Database::Result res_req = conn.exec_params(
                            "SELECT eprt.name FROM public_request pr "
                            " JOIN enum_public_request_type eprt ON eprt.id = pr.request_type "
                            " JOIN public_request_auth pra ON pra.id = pr.id "
                            " WHERE pra.identification = $1::text "
                            , Database::query_param_list(request_id));

                    if(res_req.size() != 1) {
                        throw std::runtime_error("Request does not exist");
                    }

                    std::string request_type = static_cast<std::string>(res_req[0][0]);

                    if(request_type != Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION )
                    {
                        throw std::runtime_error("Wrong type of request");
                    }

                    LOGGER(PACKAGE).info(boost::format("request data --"
                         " request_id: %1%  password: %2%  log_id: %3%")
                         % request_id % password % log_id);

                     ContactIdentificationRequestManagerPtr request_manager(mailer_);

                     unsigned long long cid = request_manager->processAuthRequest(
                             request_id, password, log_id);

                     trans.commit();

                     return cid;
                }//try
                catch (Fred::PublicRequest::PublicRequestAuth::NotAuthenticated&)
                {
                    LOGGER(PACKAGE).error("PublicRequestAuth::NotAuthenticated");
                    throw Registry::Contact::Verification::IDENTIFICATION_FAILED();
                }
                catch (Fred::PublicRequest::AlreadyProcessed &_ex)
                {
                    if(_ex.success)
                    {
                        LOGGER(PACKAGE).error("PublicRequest::AlreadyProcessed true");
                        throw Registry::Contact::Verification::IDENTIFICATION_PROCESSED();
                    }
                    else
                    {
                        LOGGER(PACKAGE).error("PublicRequest::AlreadyProcessed false");
                        throw Registry::Contact::Verification::IDENTIFICATION_INVALIDATED();
                    }
                }
                catch (Fred::PublicRequest::ObjectChanged &)
                {
                    LOGGER(PACKAGE).error("Object changed");
                    throw Registry::Contact::Verification::OBJECT_CHANGED();
                }
                catch (Fred::PublicRequest::NotApplicable &_ex)
                {
                    LOGGER(PACKAGE).error(_ex.what());
                    Fred::Contact::Verification::FieldErrorMap errors;
                    errors[Fred::Contact::Verification::field_status]
                           = Fred::Contact::Verification::NOT_AVAILABLE;
                    throw Fred::Contact::Verification::DataValidationError((errors));
                }
                catch (std::exception &_ex)
                {
                    LOGGER(PACKAGE).error(_ex.what());
                    throw;
                }
                catch (...)
                {
                    LOGGER(PACKAGE).error("unknown exception");
                    throw;
                }
            }//ContactVerificationImpl::processConditionalIdentification

            unsigned long long ContactVerificationImpl::processIdentification(
                    const std::string & contact_handle
                    , const std::string & password
                    , const unsigned long long log_id)
            {
                Logging::Context ctx_server(create_ctx_name(get_server_name()));
                Logging::Context ctx("process-identification");
                ConnectionReleaser releaser;
                try
                {
                    LOGGER(PACKAGE).info(boost::format("request data --"
                         " contact_handle: %1%  password: %2%  log_id: %3%")
                         % contact_handle % password % log_id);

                    Database::Connection conn = Database::Manager::acquire();
                    Database::Transaction trans(conn);

                    //check contact availability
                    Fred::Contact::ManagerPtr contact_mgr(
                        Fred::Contact::Manager::create(
                            DBDisconnectPtr(0), registry_conf_->restricted_handles));
                    Fred::NameIdPair cinfo;
                    Fred::Contact::Manager::CheckAvailType check_result;
                    check_result = contact_mgr->checkAvail(contact_handle, cinfo);
                    if (check_result != Fred::Contact::Manager::CA_REGISTRED)
                    {
                        LOGGER(PACKAGE).error("checkAvail CA_REGISTRED");
                        throw Registry::Contact::Verification::OBJECT_NOT_EXISTS();
                    }

                     ContactIdentificationRequestManagerPtr request_manager(mailer_);

                     std::vector<Fred::PublicRequest::Type> request_type_list
                                     = Util::vector_of<Fred::PublicRequest::Type>
                                 (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION);

                     std::string request_id = request_manager->getPublicRequestAuthIdentification(
                             cinfo.id, request_type_list);

                     request_manager->processAuthRequest(
                             request_id, password, log_id);

                     trans.commit();

                     return cinfo.id;
                }//try
                catch (Fred::NOT_FOUND& _ex)
                {
                    LOGGER(PACKAGE).error(_ex.what());
                    throw Registry::Contact::Verification::OBJECT_NOT_EXISTS();
                }
                catch (Fred::PublicRequest::PublicRequestAuth::NotAuthenticated&)
                {
                    LOGGER(PACKAGE).error("PublicRequestAuth::NotAuthenticated");
                    throw Registry::Contact::Verification::IDENTIFICATION_FAILED();
                }
                catch (Fred::PublicRequest::AlreadyProcessed &_ex)
                {
                    if(_ex.success)
                    {
                        LOGGER(PACKAGE).error("PublicRequest::AlreadyProcessed true");
                        throw Registry::Contact::Verification::IDENTIFICATION_PROCESSED();
                    }
                    else
                    {
                        LOGGER(PACKAGE).error("PublicRequest::AlreadyProcessed false");
                        throw Registry::Contact::Verification::IDENTIFICATION_INVALIDATED();
                    }
                }
                catch (Fred::PublicRequest::ObjectChanged &)
                {
                    LOGGER(PACKAGE).error("Object changed");
                    throw Registry::Contact::Verification::OBJECT_CHANGED();
                }
                catch (Fred::PublicRequest::NotApplicable &_ex)
                {
                    LOGGER(PACKAGE).error(_ex.what());
                    Fred::Contact::Verification::FieldErrorMap errors;
                    errors[Fred::Contact::Verification::field_status]
                           = Fred::Contact::Verification::NOT_AVAILABLE;
                    throw Fred::Contact::Verification::DataValidationError((errors));
                }
                catch (std::exception &_ex)
                {
                    LOGGER(PACKAGE).error(_ex.what());
                    throw;
                }
                catch (...)
                {
                    LOGGER(PACKAGE).error("unknown exception");
                    throw;
                }
            }//ContactVerificationImpl::processIdentification

            std::string ContactVerificationImpl::getRegistrarName(const std::string & registrar_handle)
            {
                Logging::Context ctx_server(create_ctx_name(get_server_name()));
                Logging::Context ctx("get-registrar-name");
                ConnectionReleaser releaser;

                LOGGER(PACKAGE).info(boost::format(
                     " registrar_handle: %1%") % registrar_handle);

                Database::Connection conn = Database::Manager::acquire();

                Database::Result regname_result = conn.exec_params(
                    "select name from registrar where handle=$1::text"
                    , Database::query_param_list(registrar_handle));

                std::string registrar_name;
                if (regname_result.size() == 1)
                {
                    registrar_name = static_cast<std::string>(regname_result[0][0]);
                }
                else
                {
                    LOGGER(PACKAGE).error("registrar not found");
                    throw Registry::Contact::Verification::OBJECT_NOT_EXISTS();
                }
                return registrar_name;
            }
        }
    }
}

