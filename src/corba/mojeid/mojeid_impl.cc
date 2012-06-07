
#include "corba_conversion.h"
#include "mojeid_impl.h"
#include "mojeid_notifier.h"

#include "cfg/config_handler_decl.h"
#include "log/logger.h"
#include "log/context.h"
#include "random.h"
#include "corba_wrapper_decl.h"

#include "fredlib/db_settings.h"
#include "fredlib/registry.h"
#include "fredlib/contact.h"
#include "fredlib/public_request/public_request.h"
#include "fredlib/object_states.h"
#include "fredlib/mojeid/contact.h"
#include "fredlib/mojeid/request.h"
#include "fredlib/mojeid/mojeid_contact_states.h"
#include "fredlib/mojeid/mojeid_data_validation.h"
#include "fredlib/mojeid/mojeid_disclose_policy.h"

#include "corba/connection_releaser.h"

#include "types/birthdate.h"
#include "types/stringify.h"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>



const std::string create_ctx_name(const std::string &_name)
{
    return str(boost::format("%1%-<%2%>") % _name % Random::integer(0, 10000));
}

namespace Registry {
namespace MojeID {


ServerImpl::ServerImpl(const std::string &_server_name)
    : registry_conf_(CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()),
      server_conf_(CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIDArgs>()),
      server_name_(_server_name),
      mojeid_registrar_id_(0),
      mailer_(new MailerManager(CorbaContainer::get_instance()->getNS()))
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx("init");

    try {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result result = conn.exec_params(
                "SELECT id FROM registrar WHERE handle = $1::text",
                Database::query_param_list(server_conf_->registrar_handle));

        if (result.size() != 1 || (mojeid_registrar_id_ = result[0][0]) == 0) {
            throw std::runtime_error("failed to find dedicated registrar in database");
        }
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).alert(_ex.what());
        throw;
    }
}


ServerImpl::~ServerImpl()
{
}

IdentificationRequestPtr ServerImpl::contactCreateWorker(unsigned long long &cid, unsigned long long &hid,
        const Contact &_contact, IdentificationMethod _method, ::MojeID::Request &_request)
{
    Fred::Contact::ManagerPtr contact_mgr(Fred::Contact::Manager::create(
                    DBDisconnectPtr(0), registry_conf_->restricted_handles));

    Fred::NameIdPair cinfo;
    Fred::Contact::Manager::CheckAvailType check_result;
    check_result = contact_mgr->checkAvail(static_cast<std::string> (_contact.username), cinfo);

    if (check_result != Fred::Contact::Manager::CA_FREE) {
        ::MojeID::FieldErrorMap errors;
        errors[::MojeID::field_username] = ::MojeID::NOT_AVAILABLE;
        throw ::MojeID::DataValidationError(errors);
    }

    ::MojeID::Contact data = corba_unwrap_contact(_contact);
    hid = ::MojeID::contact_create(
            _request.get_request_id(), _request.get_registrar_id(), data);
    cid = data.id;

    /* create public request */
    Fred::PublicRequest::Type type;
    if (_method == Registry::MojeID::SMS) {
        type = Fred::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION;
    } else if (_method == Registry::MojeID::LETTER) {
        type = Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION;
    } else if (_method == Registry::MojeID::CERTIFICATE) {
        throw std::runtime_error("not implemented");
    } else {
        throw std::runtime_error("unknown identification method");
    }

    IdentificationRequestPtr new_request(type);
    new_request->setRegistrarId(_request.get_registrar_id());
    new_request->setRequestId(_request.get_request_id());
    new_request->addObject(Fred::PublicRequest::OID(cid, static_cast<std::string> (_contact.username),
            Fred::PublicRequest::OT_CONTACT));
    new_request->save();

    return new_request;
}

CORBA::ULongLong ServerImpl::contactCreate(const Contact &_contact,
        IdentificationMethod _method, const CORBA::ULongLong _request_id) {
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-create");
    ConnectionReleaser releaser;

    try {
        std::string handle = static_cast<std::string> (_contact.username);
        LOGGER(PACKAGE).info(boost::format("request data --"
            "  handle: %1%  identification_method: %2%  request_id: %3%")
                % handle % _method % _request_id);

        /* start new request - here for logging into action table - until
         * fred-logd fully migrated */
        ::MojeID::Request request(204, mojeid_registrar_id_, _request_id);
        Logging::Context ctx_request(Util::make_svtrid(_request_id));

        unsigned long long cid;
        unsigned long long hid;

        IdentificationRequestPtr new_request = contactCreateWorker (cid, hid, _contact, _method, request);

        /* save contact and request (one transaction) */
        request.end_success();

        LOGGER(PACKAGE).info(boost::format(
                "contact saved -- handle: %1%  cid: %2%  history_id: %3%")
                % handle % cid % hid);
        LOGGER(PACKAGE).info("request completed successfully");

        /* send identification passwords */
        try {
            new_request->sendPasswords();
            LOGGER(PACKAGE).info("identification password sent");
        } catch (...) {
            LOGGER(PACKAGE).error(boost::format(
                    "error when sending identification password"
                        " (contact_id=%1% public_request_id=%2%)") % cid
                    % new_request->getId());
        }

        /* notification */

        try {
            if (server_conf_->notify_commands) {
                Fred::MojeID::notify_contact_create(_request_id, mailer_);
            }
        } catch (...) {
            LOGGER(PACKAGE).error("request notification failed");
        }

        /* return new contact id */
        return cid;
    } catch (::MojeID::DataValidationError &_ex) {
        throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                corba_wrap_validation_error_list(_ex.errors));
    } catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)")
                % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    } catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}

CORBA::ULongLong ServerImpl::contactCreatePrepare(const Contact &_contact,
                                           IdentificationMethod _method,
                                           const char * _trans_id,
                                           const CORBA::ULongLong _request_id,
                                           CORBA::String_out _identification)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-create");
    ConnectionReleaser releaser;

    try {
        std::string handle = static_cast<std::string>(_contact.username);
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  handle: %1%  identification_method: %2%  transaction_id: %3% request_id: %4%")
                % handle % _method % _trans_id % _request_id);

        /* start new request - here for logging into action table - until
         * fred-logd fully migrated */
        ::MojeID::Request request(204, mojeid_registrar_id_, _request_id, _trans_id);
        Logging::Context ctx_request(Util::make_svtrid(_request_id));

        unsigned long long cid;
        unsigned long long hid;

        IdentificationRequestPtr new_request = contactCreateWorker (cid, hid, _contact, _method, request);

        IdentificationRequestManagerPtr request_manager;
        std::vector<unsigned int> request_type_list = boost::assign::list_of
            (Fred::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION)
            (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION);
        _identification = corba_wrap_string(
                request_manager->getPublicRequestAuthIdentification(cid, request_type_list).c_str());

        /* save contact and request (one transaction) */
        request.end_prepare(_trans_id);

        boost::mutex::scoped_lock td_lock(td_mutex);

        trans_data d(MOJEID_CONTACT_CREATE);
        d.cid = cid;
        d.prid = new_request->getId();
        d.request_id =   request.get_request_id();
        transaction_data.insert(std::make_pair(_trans_id, d));

        td_lock.unlock();

        LOGGER(PACKAGE).info(boost::format(
                "contact saved -- handle: %1%  cid: %2%  history_id: %3%")
                % handle % cid % hid);
        LOGGER(PACKAGE).info("request completed successfully");


        /* return new contact id */
        return cid;
    }
    catch (::MojeID::DataValidationError &_ex) {
        throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                corba_wrap_validation_error_list(_ex.errors));
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}


CORBA::ULongLong ServerImpl::processIdentification(const char* _ident_request_id,
                                                   const char* _password,
                                                   const CORBA::ULongLong _request_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("process-identification");
    ConnectionReleaser releaser;

    try {
        LOGGER(PACKAGE).info(boost::format("request data --"
                    " identification_id: %1%  password: %2%  request_id: %3%")
                % _ident_request_id % _password % _request_id);

        IdentificationRequestManagerPtr request_manager;
        unsigned long long cid = request_manager->processAuthRequest(
                                    _ident_request_id, _password, _request_id);

        try {
            if (server_conf_->notify_commands) {
                Fred::MojeID::notify_contact_transfer(_request_id, mailer_);
            }
        }
        catch (Fred::RequestNotFound &_ex) {
            LOGGER(PACKAGE).info(_ex.what());
        }
        catch (std::exception &_ex) {
            LOGGER(PACKAGE).error(_ex.what());
        }
        catch (...) {
            LOGGER(PACKAGE).error("request notification failed");
        }

        return cid;

    }
    catch (Fred::NOT_FOUND&) {
        LOGGER(PACKAGE).error(boost::format(
                    "cannot process identification request"
                    " (not found) ident=%1%") % _ident_request_id);
        throw Registry::MojeID::Server::IDENTIFICATION_FAILED();
    }
    catch (Fred::PublicRequest::PublicRequestAuth::NotAuthenticated&) {
        LOGGER(PACKAGE).info(boost::format(
                    "request authentication failed (bad password)"
                    " ident=%1%") % _ident_request_id);
        throw Registry::MojeID::Server::IDENTIFICATION_FAILED();
    }
    catch (::MojeID::DataValidationError &_ex) {
        LOGGER(PACKAGE).error(boost::format(
                    "cannot process identification request"
                    " (contact data cannot be validated) ident=%1%")
                % _ident_request_id);
        throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                corba_wrap_validation_error_list(_ex.errors));
    }
    catch (Fred::PublicRequest::AlreadyProcessed &_ex) {
        LOGGER(PACKAGE).warning(boost::format(
                    "cannot process identification request (%1%)"
                    " ident=%2%") % _ex.what() % _ident_request_id);
        throw Registry::MojeID::Server::IDENTIFICATION_ALREADY_PROCESSED(_ex.success);
    }
    catch (Fred::PublicRequest::ObjectChanged &_ex) {
        LOGGER(PACKAGE).warning(boost::format(
                    "cannot process identification request (%1%)"
                    " ident=%2%") % _ex.what() % _ident_request_id);
        throw Registry::MojeID::Server::OBJECT_CHANGED();
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format(
                    "request failed (%1%) ident=%2%")
                % _ex.what() % _ident_request_id);
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error(boost::format(
                    "request failed (unknown error) ident=%1%")
                % _ident_request_id);
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}

CORBA::ULongLong ServerImpl::contactTransfer(const char *_handle,
                                             IdentificationMethod _method,
                                             const CORBA::ULongLong _request_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-transfer-request");
    ConnectionReleaser releaser;

    try {
        std::string handle = static_cast<std::string>(_handle);
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  handle: %1%  identification_method: %2%  request_id: %3%")
                % handle % _method % _request_id);

        Fred::NameIdPair cinfo;
        Fred::Contact::ManagerPtr contact_mgr(
                Fred::Contact::Manager::create(DBDisconnectPtr(0), registry_conf_->restricted_handles));

        Fred::Contact::Manager::CheckAvailType check_result;
        check_result = contact_mgr->checkAvail(handle, cinfo);

        if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
            throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
        }

        ::MojeID::FieldErrorMap errors;
        /* contact is blocked or prohibits operations:
         *   7 | serverBlocked
         *   3 | serverTransferProhibited
         *   4 | serverUpdateProhibited
         */
        /* already CI || already I || already V */
        if ((Fred::object_has_state(cinfo.id, ::MojeID::CONDITIONALLY_IDENTIFIED_CONTACT) == true)
                || (Fred::object_has_state(cinfo.id, ::MojeID::IDENTIFIED_CONTACT) == true)
                || (Fred::object_has_state(cinfo.id, ::MojeID::VALIDATED_CONTACT) == true)) {

            errors[::MojeID::field_status] = ::MojeID::NOT_AVAILABLE;
        }
        else if ((Fred::object_has_state(cinfo.id, "serverTransferProhibited") == true)
                || (Fred::object_has_state(cinfo.id, "serverUpdateProhibited") == true)) {

            errors[::MojeID::field_status] = ::MojeID::INVALID;
        }
        if (errors.size() > 0) {
            throw ::MojeID::DataValidationError(errors);
        }

        /* create public request */
        Fred::PublicRequest::Type type;
        if (_method == Registry::MojeID::SMS) {
            type = Fred::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION;
        }
        else if (_method == Registry::MojeID::LETTER) {
            type = Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION;
        }
        else if (_method == Registry::MojeID::CERTIFICATE) {
            throw std::runtime_error("not implemented");
        }
        else {
            throw std::runtime_error("unknown identification method");
        }

        IdentificationRequestPtr new_request(type);
        new_request->setRegistrarId(mojeid_registrar_id_);
        new_request->setRequestId(_request_id);
        new_request->addObject(
                Fred::PublicRequest::OID(
                    cinfo.id, handle, Fred::PublicRequest::OT_CONTACT));
        new_request->save();
        new_request->sendPasswords();

        LOGGER(PACKAGE).info(boost::format(
                "identification request with contact transfer saved"
                " -- handle: %1%  id: %2%")
                % handle % cinfo.id);

        LOGGER(PACKAGE).info("request completed successfully");
        return cinfo.id;
    }
    catch (Registry::MojeID::Server::OBJECT_NOT_EXISTS) {
        throw;
    }
    catch (Fred::PublicRequest::NotApplicable &_ex) {
        LOGGER(PACKAGE).error(boost::format(
                    "cannot create transfer request (%1%)") % _ex.what());
        ::MojeID::FieldErrorMap errors;
        errors[::MojeID::field_status] = ::MojeID::NOT_AVAILABLE;
        throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                corba_wrap_validation_error_list(errors));
    }
    catch (::MojeID::DataValidationError &_ex) {
        throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                corba_wrap_validation_error_list(_ex.errors));
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}

CORBA::ULongLong ServerImpl::contactTransferPrepare(const char *_handle,
                                             IdentificationMethod _method,
                                             const char * _trans_id,
                                             const CORBA::ULongLong _request_id,
                                             CORBA::String_out _identification)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-transfer-request");
    ConnectionReleaser releaser;

    try {
        std::string handle = static_cast<std::string>(_handle);
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  handle: %1%  identification_method: %2%  request_id: %3%")
                % handle % _method % _request_id);

        Fred::NameIdPair cinfo;
        Fred::Contact::ManagerPtr contact_mgr(
                Fred::Contact::Manager::create(DBDisconnectPtr(0), registry_conf_->restricted_handles));

        Fred::Contact::Manager::CheckAvailType check_result;
        check_result = contact_mgr->checkAvail(handle, cinfo);

        if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
            throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
        }

        ::MojeID::FieldErrorMap errors;
        /* contact is blocked or prohibits operations:
         *   7 | serverBlocked
         *   3 | serverTransferProhibited
         *   4 | serverUpdateProhibited
         */
        /* already CI || already I || already V */
        if ((Fred::object_has_state(cinfo.id, ::MojeID::CONDITIONALLY_IDENTIFIED_CONTACT) == true)
                || (Fred::object_has_state(cinfo.id, ::MojeID::IDENTIFIED_CONTACT) == true)
                || (Fred::object_has_state(cinfo.id, ::MojeID::VALIDATED_CONTACT) == true)) {

            errors[::MojeID::field_status] = ::MojeID::NOT_AVAILABLE;
        }
        else if ((Fred::object_has_state(cinfo.id, "serverTransferProhibited") == true)
                || (Fred::object_has_state(cinfo.id, "serverUpdateProhibited") == true)) {

            errors[::MojeID::field_status] = ::MojeID::INVALID;
        }
        if (errors.size() > 0) {
            throw ::MojeID::DataValidationError(errors);
        }

        /* create public request */
        Fred::PublicRequest::Type type;
        if (_method == Registry::MojeID::SMS) {
            type = Fred::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION;
        }
        else if (_method == Registry::MojeID::LETTER) {
            type = Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION;
        }
        else if (_method == Registry::MojeID::CERTIFICATE) {
            throw std::runtime_error("not implemented");
        }
        else {
            throw std::runtime_error("unknown identification method");
        }

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        IdentificationRequestPtr new_request(type);
        new_request->setRegistrarId(mojeid_registrar_id_);
        new_request->setRequestId(_request_id);
        new_request->addObject(
                Fred::PublicRequest::OID(
                    cinfo.id, handle, Fred::PublicRequest::OT_CONTACT));
        new_request->save();

        IdentificationRequestManagerPtr request_manager;
        std::vector<unsigned int> request_type_list = boost::assign::list_of
            (Fred::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION)
            (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION);
        _identification = corba_wrap_string(
                request_manager->getPublicRequestAuthIdentification(cinfo.id, request_type_list).c_str());

        tx.prepare(_trans_id);

        boost::mutex::scoped_lock td_lock(td_mutex);

        trans_data d(MOJEID_CONTACT_TRANSFER);
        d.cid = cinfo.id;
        d.prid = new_request->getId();
        d.request_id =   _request_id;
        transaction_data.insert(std::make_pair(_trans_id, d));

        td_lock.unlock();

        LOGGER(PACKAGE).info(boost::format(
                "identification request with contact transfer saved"
                " -- handle: %1%  id: %2%")
                % handle % cinfo.id);

        LOGGER(PACKAGE).info("request prepare completed successfully");
        return cinfo.id;
    }
    catch (Registry::MojeID::Server::OBJECT_NOT_EXISTS) {
        throw;
    }
    catch (Fred::PublicRequest::NotApplicable &_ex) {
        LOGGER(PACKAGE).error(boost::format(
                    "cannot create transfer request (%1%)") % _ex.what());
        ::MojeID::FieldErrorMap errors;
        errors[::MojeID::field_status] = ::MojeID::NOT_AVAILABLE;
        throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                corba_wrap_validation_error_list(errors));
    }
    catch (::MojeID::DataValidationError &_ex) {
        throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                corba_wrap_validation_error_list(_ex.errors));
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}


/*

   states to drop:
          1 | serverDeleteProhibited           
          3 | serverTransferProhibited        
          4 | serverUpdateProhibited         
         21 | conditionallyIdentifiedContact
         22 | identifiedContact            
         23 | validatedContact            
*/


void ServerImpl::contactUnidentifyPrepare(const CORBA::ULongLong _contact_id,
                                   const char * _trans_id,
                                   const CORBA::ULongLong _request_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-unidentify");
    ConnectionReleaser releaser;

    LOGGER(PACKAGE).info(boost::format("contactUnidentifyPrepare --"
            "  contact_id: %1%  request_id: %2%")
                % _contact_id % _request_id);

    try {
        // check if the contact with ID _contact_id exists
        Fred::NameIdPair cinfo;
        Fred::Contact::ManagerPtr contact_mgr(
                Fred::Contact::Manager::create(
                        DBDisconnectPtr(0), registry_conf_->restricted_handles));

        Fred::Contact::Manager::CheckAvailType check_result;
        check_result = contact_mgr->checkAvail(_contact_id, cinfo);

        if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
            /* contact doesn't exists */
            throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
        }

        if (mojeid_registrar_id_ != contact_mgr->findRegistrarId(_contact_id)) {
            throw std::runtime_error("Contact is not registered with MojeID");
        }

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        boost::format lock_state = boost::format(
        " SELECT os.state_id FROM object_state os WHERE os.state_id = ANY ( SELECT id from enum_object_states where name = "
            "ANY( '{ %1%, %2%, %3%, %4%, %5%, %6% }') ) AND (os.valid_to IS NULL OR os.valid_to > CURRENT_TIMESTAMP) AND os.object_id = $1::integer FOR UPDATE")
           % "serverDeleteProhibited" 
           % "serverTransferProhibited"
           % "serverUpdateProhibited"
           % ::MojeID::CONDITIONALLY_IDENTIFIED_CONTACT
           % ::MojeID::IDENTIFIED_CONTACT
           % ::MojeID::VALIDATED_CONTACT;

        boost::format lock_state_request = boost::format(
        " SELECT * FROM object_state os WHERE os.state_id = ANY ( SELECT id from enum_object_states where name = "
            "ANY( '{ %1%, %2%, %3%, %4%, %5%, %6% }') ) AND (os.valid_to IS NULL OR os.valid_to > CURRENT_TIMESTAMP) AND os.object_id = $1::integer FOR UPDATE")
           % "serverDeleteProhibited" 
           % "serverTransferProhibited"
           % "serverUpdateProhibited"
           % ::MojeID::CONDITIONALLY_IDENTIFIED_CONTACT
           % ::MojeID::IDENTIFIED_CONTACT
           % ::MojeID::VALIDATED_CONTACT;

        // fetch the result and convert to strings
        std::vector<int> drop_states_codes;
        
        Database::Result res = conn.exec_params(lock_state.str(),
                Database::query_param_list(_contact_id));

        conn.exec_params(lock_state_request.str(), 
                Database::query_param_list(_contact_id));

        drop_states_codes.reserve(res.size());
        for(unsigned i=0;i<res.size();i++) {
            drop_states_codes.push_back(res[i][0]);
        }

        std::vector<std::string> drop_states = Fred::states_conversion(drop_states_codes);

        // check consistency 
        bool haveDeleteProhibited = false;
        bool haveTransferProhibited = false;
        bool haveUpdateProhibited = false;
        std::string mojeid_state;

        for(std::vector<std::string>::const_iterator it = drop_states.begin();
                it != drop_states.end();
                it++) {
            if(::MojeID::CONDITIONALLY_IDENTIFIED_CONTACT == *it 
                    || ::MojeID::IDENTIFIED_CONTACT == *it
                    || ::MojeID::VALIDATED_CONTACT == *it) {
                if(!mojeid_state.empty()) {
                    throw std::runtime_error("Contact has invalid combination of states");
                }
                mojeid_state = *it;
            } else {
                if(std::string("serverDeleteProhibited") == *it) {
                    haveDeleteProhibited = true;
                } else if(std::string("serverTransferProhibited") == *it) {
                    haveTransferProhibited = true;
                } else if(std::string("serverUpdateProhibited") == *it) {
                    haveUpdateProhibited = true;
                } else {
                    throw std::runtime_error("Programming error: discrepancy between SQL and processing code");
                }
            }
        }
                
        if (!haveDeleteProhibited 
            || !haveTransferProhibited
            || !haveUpdateProhibited
            || mojeid_state.empty()) {
            throw std::runtime_error("Contact is not in all expected states");
        }

        Fred::cancel_multiple_object_states(_contact_id, drop_states);

        conn.exec_params("UPDATE public_request pr SET status=2, resolve_time = CURRENT_TIMESTAMP "
            "FROM public_request_objects_map prom WHERE (prom.request_id = pr.id) "
            "AND pr.resolve_time IS NULL AND pr.status = 0 "
            "AND pr.request_type IN (12,13,14) AND object_id = $1::integer",
                Database::query_param_list(_contact_id));

        //#6813
        ::MojeID::Contact tmp_contact = ::MojeID::contact_info(_contact_id);
        if(tmp_contact.discloseaddress == false)
        {
            tmp_contact.discloseaddress = true;

            unsigned long long hid = ::MojeID::contact_update(
                            _request_id,
                            mojeid_registrar_id_,
                            tmp_contact);

            LOGGER(PACKAGE).info(boost::format(
                            "contact updated -- handle: %1%  id: %2%  history_id: %3%")
                            % tmp_contact.handle % _contact_id % hid);
        }


        tx.prepare(_trans_id);

        trans_data tr_data(MOJEID_CONTACT_UNIDENTIFY);
        tr_data.cid = _contact_id;

        boost::mutex::scoped_lock td_lock(td_mutex);
        transaction_data.insert(std::make_pair(_trans_id, tr_data));

    } catch (Registry::MojeID::Server::OBJECT_NOT_EXISTS) {
        throw;
    } catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    } catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}

void ServerImpl::contactUpdatePrepare(const Contact &_contact,
                                      const char* _trans_id,
                                      const CORBA::ULongLong _request_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-update");

    unsigned long long cid;
    try {
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  handle: %1%  transaction_id: %2%"
                    "  request_id: %3%")  % static_cast<std::string>(_contact.username)
                    % _trans_id % _request_id);

        /* start new request - here for logging into action table - until
         * fred-logd fully migrated */
        ::MojeID::Request request(203, mojeid_registrar_id_, _request_id, _trans_id);
        Logging::Context ctx_request(Util::make_svtrid(_request_id));

        if (_contact.id == 0) {
            throw std::runtime_error("contact.id is null");
        }
        cid = _contact.id->_value();
        std::string handle = boost::to_upper_copy(corba_unwrap_string(_contact.username));

        Fred::NameIdPair cinfo;
        Fred::Contact::ManagerPtr contact_mgr(
                Fred::Contact::Manager::create(DBDisconnectPtr(0), registry_conf_->restricted_handles));

        Fred::Contact::Manager::CheckAvailType check_result;
        check_result = contact_mgr->checkAvail(handle, cinfo);

        if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
            throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
        }

        if (cinfo.name != handle || cinfo.id != cid) {
            throw std::runtime_error(str(boost::format(
                        "inconsistency in parameter --"
                        " passed: (id: %1%, handle: %2%), database (id: %3%, handle: %4%)")
                    % cid % handle % cinfo.id % cinfo.name));
        }

        ::MojeID::Contact data = corba_unwrap_contact(_contact);
        ::MojeID::ContactValidator validator = ::MojeID::create_contact_update_validator();
        validator.check(data);

        if (Fred::object_has_state(cid, ::MojeID::CONDITIONALLY_IDENTIFIED_CONTACT) == true) {

            if (::MojeID::check_conditionally_identified_contact_diff(data, ::MojeID::contact_info(cid)) == false) {
                LOGGER(PACKAGE).info( boost::format(
                        "tried to update conditionally identified contact -- handle: %1%  id: %2% ")
                        % handle % cid);

                ::MojeID::FieldErrorMap errors;
                errors[::MojeID::field_status] = ::MojeID::INVALID;
                throw ::MojeID::DataValidationError(errors);
            }
        }

        DiscloseFlagPolicy::PolicyCallbackVector pcv
            = boost::assign::list_of
                (DiscloseFlagPolicy::PolicyCallback(SetDiscloseAddrTrueIfOrganization()))
                (DiscloseFlagPolicy::PolicyCallback(SetDiscloseAddrTrueIfNotValidated()));

        DiscloseFlagPolicy contact_disclose_policy (pcv);

        if (Fred::object_has_state(cid, ::MojeID::VALIDATED_CONTACT) == true) {
            if (::MojeID::check_validated_contact_diff(data, ::MojeID::contact_info(cid)) == false) {
                /* change contact status to identified */
                if (Fred::cancel_object_state(cid, ::MojeID::VALIDATED_CONTACT)) {
                    Fred::insert_object_state(cid, ::MojeID::IDENTIFIED_CONTACT);
                    contact_disclose_policy.append_policy_callback(SetDiscloseAddrTrue());
                }
            }
        }

        contact_disclose_policy.apply(data);

        unsigned long long hid = ::MojeID::contact_update(
                request.get_request_id(),
                request.get_registrar_id(),
                data);

        LOGGER(PACKAGE).info(boost::format(
                "contact updated -- handle: %1%  id: %2%  history_id: %3%")
                % handle % cid % hid);

        request.end_prepare(_trans_id);

        trans_data tr_data(MOJEID_CONTACT_UPDATE);

        tr_data.cid = cid;
        tr_data.request_id = request.get_request_id();

        boost::mutex::scoped_lock td_lock(td_mutex);
        transaction_data.insert(std::make_pair(_trans_id, tr_data));
        td_lock.unlock();

        LOGGER(PACKAGE).info("request completed successfully");
    }
    catch (Registry::MojeID::Server::OBJECT_NOT_EXISTS) {
        throw;
    }
    catch (::MojeID::DataValidationError &_ex) {
        throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                corba_wrap_validation_error_list(_ex.errors));
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }

}


Contact* ServerImpl::contactInfo(const CORBA::ULongLong _id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-info");
    ConnectionReleaser releaser;

    try {
        LOGGER(PACKAGE).info(boost::format("request data -- id: %1%") % _id);

        Fred::NameIdPair cinfo;
        Fred::Contact::ManagerPtr contact_mgr(
                Fred::Contact::Manager::create(DBDisconnectPtr(0), registry_conf_->restricted_handles));

        Fred::Contact::Manager::CheckAvailType check_result;
        check_result = contact_mgr->checkAvail(_id, cinfo);

        if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
            throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
        }

        Contact *data = corba_wrap_contact(::MojeID::contact_info(_id));

        LOGGER(PACKAGE).info("request completed successfully");
        return data;
    }
    catch (Registry::MojeID::Server::OBJECT_NOT_EXISTS) {
        throw;
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}

/* if cid is not present, don't do anything
 *
 */

void ServerImpl::commitPreparedTransaction(const char* _trans_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("commit-prepared");

    trans_data tr_data(MOJEID_NOP);

    try {
        LOGGER(PACKAGE).info(boost::format("request data -- transaction_id: %1%")
                % _trans_id);

        if (std::string(_trans_id).empty()) {
             throw std::runtime_error("Transaction ID empty");
        }
        boost::mutex::scoped_lock search_lock(td_mutex);
        
        transaction_data_map_type::iterator it =
            transaction_data.find(_trans_id);
        if(it == transaction_data.end()) {
            throw std::runtime_error((boost::format(
                        "cannot retrieve saved transaction data using transaction identifier %1%.")
                    % _trans_id).str());
        }
        search_lock.unlock();

        tr_data = it->second;

        LOGGER(PACKAGE).info(boost::format("Transaction data for %1% retrieved: cid %2%, prid %3%, rid %4%")
                % _trans_id % tr_data.cid % tr_data.prid % tr_data.request_id);

        if(tr_data.cid == 0) {
            throw std::runtime_error((boost::format(
                 "cannot retrieve contact id from transaction identifier %1%.") % _trans_id).str());
        }

        Database::Connection conn = Database::Manager::acquire();
        conn.exec("COMMIT PREPARED '" + conn.escape(_trans_id) + "'");

        // successful commit , delete the data from map
        boost::mutex::scoped_lock erase_lock(td_mutex);
        transaction_data.erase(it);
        erase_lock.unlock();
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }

    // send identification password if operation is contact create
    if(tr_data.op == MOJEID_CONTACT_CREATE || tr_data.op == MOJEID_CONTACT_TRANSFER) {
        sendAuthPasswords(tr_data.cid, tr_data.prid);
    }

    if(tr_data.op == MOJEID_CONTACT_UPDATE || tr_data.op == MOJEID_CONTACT_UNIDENTIFY) {
        updateObjectStates(tr_data.cid);
    }

    /* request notification */
    try {
        if (tr_data.request_id && server_conf_->notify_commands) {
            if (tr_data.op == MOJEID_CONTACT_UPDATE) {
                Fred::MojeID::notify_contact_update(tr_data.request_id, mailer_);
            }
            else if (tr_data.op == MOJEID_CONTACT_CREATE) {
                Fred::MojeID::notify_contact_create(tr_data.request_id, mailer_);
            }
        }
    }
    catch (Fred::RequestNotFound &_ex) {
        LOGGER(PACKAGE).info(_ex.what());
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request notification failed");
    }

    LOGGER(PACKAGE).info("request completed successfully");

}


void ServerImpl::rollbackPreparedTransaction(const char* _trans_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("rollback-prepared");

    trans_data tr(MOJEID_NOP);
    try {
        LOGGER(PACKAGE).info(boost::format("request data -- transaction_id: %1%")
                % _trans_id);

        Database::Connection conn = Database::Manager::acquire();
        conn.exec("ROLLBACK PREPARED '" + conn.escape(_trans_id) + "'");

        boost::mutex::scoped_lock erase_lock(td_mutex);
        transaction_data_map_type::iterator it =
            transaction_data.find(_trans_id);
        if(it != transaction_data.end()) {
            tr = it->second;
            transaction_data.erase(it);
        } else {
            LOGGER(PACKAGE).warning(boost::format("rollbackPrepared: Data for transaction %1% not found.")
                    % _trans_id);
            return;
        }
        erase_lock.unlock();
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }

    LOGGER(PACKAGE).info("rollback completed");
}


char* ServerImpl::getIdentificationInfo(CORBA::ULongLong _contact_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("get-identification");
    ConnectionReleaser releaser;

    try {
        LOGGER(PACKAGE).info(boost::format("get_identification --"
                    "  _contact_id: %1% ")
                % _contact_id);

        IdentificationRequestManagerPtr request_manager;
        std::vector<unsigned int> request_type_list = boost::assign::list_of
            (Fred::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION)
            (Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION);
        unsigned long long cid = static_cast<unsigned long long>(_contact_id);
        return corba_wrap_string(
                request_manager->getPublicRequestAuthIdentification(cid, request_type_list).c_str());
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}

Buffer* ServerImpl::getValidationPdf(const CORBA::ULongLong _contact_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("get-validation-pdf");
    ConnectionReleaser releaser;

    try {
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  contact_id: %1% ")
                % _contact_id );
        // load all required data for last unprocessed validation request
        // this should be eventually done by filter query with combination of
        // loading contact data
        Database::Connection conn = Database::Manager::acquire();
        // TODO: hardcoded ID=14! should be replaced
        Database::Result res = conn.exec_params(
            "SELECT "
            " (pr.create_time::timestamptz AT TIME ZONE 'Europe/Prague')::date,"
            " pr.id, "
            " c.name, c. organization, c.ssn, c.ssntype, "
            " c.street1 || ' ' || COALESCE(c.street2,'') || ' ' ||"
            " COALESCE(c.street3,' ') || ', ' || "
            " c.postalcode || ' ' || c.city || ', ' || c.country, "
            " oreg.name "
            "FROM public_request pr"
            " JOIN public_request_objects_map prom ON (prom.request_id=pr.id) "
            " JOIN contact c ON (c.id = prom.object_id) "
            " JOIN object_registry oreg ON oreg.id = c.id "
            " WHERE pr.resolve_time IS NULL AND pr.status = 0 "
            " AND pr.request_type=14 AND object_id = $1::integer",
            Database::query_param_list(_contact_id));
        if (res.size() != 1)
            throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
        IdentificationRequestManagerPtr req_man;
        std::stringstream outstr;
        std::auto_ptr<Fred::Document::Generator> g(
                req_man->getDocumentManager()->createOutputGenerator(
                        Fred::Document::GT_CONTACT_VALIDATION_REQUEST_PIN3,
                        outstr,
                        "cs"
                )
        );
        unsigned identType = res[0][5];
        Database::Date d = res[0][0];
        //g->getInput().imbue(std::locale(std::locale(""),new date_facet("%x")));
        g->getInput()
            << "<?xml version='1.0' encoding='utf-8'?>"
            << "<mojeid_valid>"
            << "<request_date>"
            << stringify(d.get())
            << "</request_date>"
            << "<request_id>"  << unsigned(res[0][1]) << "</request_id>"
            << "<handle>" << std::string(res[0][7]) << "</handle>"
            << "<name>" << std::string(res[0][2]) << "</name>"
            << "<organization>" << std::string(res[0][3]) << "</organization>"
            << "<ic>"
            << (identType == 4 ? std::string(res[0][4]) : "")
            << "</ic>"
            << "<birth_date>"
            << (identType == 6 ? stringify(birthdate_from_string_to_date(res[0][4])) : "")
            << "</birth_date>"
            << "<address>" << std::string(res[0][6]) << "</address>"
            << "</mojeid_valid>";
        g->closeInput();
        unsigned long size = outstr.str().size();
        CORBA::Octet *b = Buffer::allocbuf(size);
        memcpy(b,outstr.str().c_str(),size);
        Buffer_var ret = new Buffer(size, size, b, 1);
        return ret._retn();
    }
    catch (Registry::MojeID::Server::OBJECT_NOT_EXISTS) {
        LOGGER(PACKAGE).warning("request not exist");
        throw;
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }

}


void ServerImpl::createValidationRequest(const CORBA::ULongLong _contact_id,
                                         const CORBA::ULongLong _request_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("create-validation-request");
    ConnectionReleaser releaser;

    try {
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  contact_id: %1%  request_id: %2%")
                % _contact_id % _request_id);

        Fred::NameIdPair cinfo;
        Fred::Contact::ManagerPtr contact_mgr(
                Fred::Contact::Manager::create(
                        DBDisconnectPtr(0), registry_conf_->restricted_handles));

        Fred::Contact::Manager::CheckAvailType check_result;
        check_result = contact_mgr->checkAvail(_contact_id, cinfo);

        if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
            /* contact doesn't exists */
            throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
        }

        /* create validation request */
        IdentificationRequestManagerPtr req_man;
        std::auto_ptr<Fred::PublicRequest::PublicRequest> new_request(
            req_man->createRequest(
                Fred::PublicRequest::PRT_CONTACT_VALIDATION
            )
        );
        new_request->setRegistrarId(mojeid_registrar_id_);
        new_request->setRequestId(_request_id);
        new_request->addObject(
             Fred::PublicRequest::OID(
                   _contact_id,
                   "",
                   Fred::PublicRequest::OT_CONTACT
             )
        );
        new_request->save();

        LOGGER(PACKAGE).info(boost::format(
                "validation request created"
                " -- public_request_id: %1%")
                % 0);

        LOGGER(PACKAGE).info("request completed successfully");
    }
    catch (Registry::MojeID::Server::OBJECT_NOT_EXISTS) {
        LOGGER(PACKAGE).error("contact doesn't exists");
        throw;
    }
    catch (Fred::PublicRequest::RequestExists &_ex) {
        LOGGER(PACKAGE).error(boost::format("cannot create request (%1%)") % _ex.what());
        throw Registry::MojeID::Server::OBJECT_EXISTS();
    }
    catch (Fred::PublicRequest::NotApplicable &_ex) {
        LOGGER(PACKAGE).error(boost::format("cannot create request (%1%)") % _ex.what());
        throw Registry::MojeID::Server::VALIDATION_ALREADY_PROCESSED();
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}


ContactStateInfoList* ServerImpl::getContactsStates(const CORBA::ULong _last_hours)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("get-contacts-states");
    ConnectionReleaser releaser;

    try {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result rstates = conn.exec_params(
                "SELECT c.id, os.valid_from, eos.name"
                " FROM object_state os"
                " JOIN enum_object_states eos ON eos.id = os.state_id"
                " JOIN contact c ON c.id = os.object_id"
                " WHERE os.valid_to IS NULL"
                " AND os.valid_from > now() - $1::interval"
                " AND eos.name =ANY ($2::text[])",
                Database::query_param_list
                    (boost::lexical_cast<std::string>(_last_hours) + " hours")
                    ("{conditionallyIdentifiedContact, identifiedContact, validatedContact}"));

        ContactStateInfoList_var ret = new ContactStateInfoList;
        ret->length(0);

        for (Database::Result::size_type i = 0; i < rstates.size(); ++i) {
            Registry::MojeID::ContactStateInfo sinfo;
            sinfo.contact_id = static_cast<unsigned long long>(rstates[i][0]);
                sinfo.valid_from = corba_wrap_date(
                    boost::gregorian::from_string(
                        static_cast<std::string>(rstates[i][1])));
            std::string state_name = static_cast<std::string>(rstates[i][2]);
            unsigned int act_size = ret->length();

            if (state_name == "conditionallyIdentifiedContact") {
                ret->length(act_size + 1);
                sinfo.state = Registry::MojeID::CONDITIONALLY_IDENTIFIED;
                ret[act_size] = sinfo;
            }
            else if (state_name == "identifiedContact") {
                ret->length(act_size + 1);
                sinfo.state = Registry::MojeID::IDENTIFIED;
                ret[act_size] = sinfo;
            }
            else if (state_name == "validatedContact") {
                ret->length(act_size + 1);
                sinfo.state = Registry::MojeID::VALIDATED;
                ret[act_size] = sinfo;
            }
        }

        return ret._retn();
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}


ContactStateInfo ServerImpl::getContactState(const CORBA::ULongLong _contact_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("get-contact-state");
    ConnectionReleaser releaser;

    try {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result rstates = conn.exec_params(
                "SELECT c.id, os.valid_from, eos.name"
                " FROM contact c"
                " LEFT JOIN (object_state os"
                " JOIN enum_object_states eos ON eos.id = os.state_id"
                " AND eos.name =ANY ($2::text[]))"
                " ON os.object_id = c.id AND os.valid_to IS NULL"
                " WHERE c.id = $1::integer",
                Database::query_param_list
                    (_contact_id)
                    ("{conditionallyIdentifiedContact, identifiedContact, validatedContact}"));

        if (rstates.size() == 0) {
            throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
        }
        else if (rstates.size() != 1) {
            throw std::runtime_error("Object appears to be in several exclusive states");
        }

        Registry::MojeID::ContactStateInfo_var sinfo;
        sinfo->contact_id = static_cast<unsigned long long>(rstates[0][0]);
        sinfo->valid_from = corba_wrap_date(
                rstates[0][1].isnull() ?
                    boost::gregorian::date()
                    : boost::gregorian::from_string(
                        static_cast<std::string>(rstates[0][1])));
        std::string state_name = static_cast<std::string>(rstates[0][2]);

        if (state_name == "conditionallyIdentifiedContact") {
            sinfo->state = Registry::MojeID::CONDITIONALLY_IDENTIFIED;
        }
        else if (state_name == "identifiedContact") {
            sinfo->state = Registry::MojeID::IDENTIFIED;
        }
        else if (state_name == "validatedContact") {
            sinfo->state = Registry::MojeID::VALIDATED;
        }
        else {
            sinfo->state = Registry::MojeID::NOT_IDENTIFIED;
        }

        return sinfo._retn();
    }
    catch (Registry::MojeID::Server::OBJECT_NOT_EXISTS&) {
        throw;
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}


CORBA::ULongLong ServerImpl::getContactId(const char* _handle)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("get-contact-id");
    ConnectionReleaser releaser;

    try {
        Fred::NameIdPair cinfo;
        Fred::Contact::ManagerPtr contact_mgr(
                Fred::Contact::Manager::create(
                        DBDisconnectPtr(0), registry_conf_->restricted_handles));

        Fred::Contact::Manager::CheckAvailType check_result;
        check_result = contact_mgr->checkAvail(_handle, cinfo);

        if (check_result == Fred::Contact::Manager::CA_REGISTRED) {
            LOGGER(PACKAGE).info(boost::format(
                        "contact %1% => id=%2%") % _handle % cinfo.id);
            return cinfo.id;
        }
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }

    throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
}

ContactHandleList* ServerImpl::getUnregistrableHandles()
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("get-unregistrable-handles");
    ConnectionReleaser releaser;

    try
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec_params(
            "SELECT name FROM object_registry WHERE type = 1 "
            " AND ((erdate is NULL) or (erdate > (CURRENT_TIMESTAMP "
            " - (SELECT val || ' month' FROM enum_parameters "
            "  WHERE name = 'handle_registration_protection_period' "
            " )::interval)) )  AND lower(name) ~ $1::text"
            , Database::query_param_list(::MojeID::USERNAME_PATTERN.str()));
        ContactHandleList_var ret = new ContactHandleList();
        ret->length(res.size());
        for (Database::Result::size_type i=0; i<res.size(); ++i)
        {
            ret[i]  = corba_wrap_string(res[i][0]);
        }
        return ret._retn();
    }//try
    catch (std::exception &_ex)
    {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}

char *ServerImpl::contactAuthInfo(const CORBA::ULongLong _contact_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-authinfo");
    ConnectionReleaser releaser;

    LOGGER(PACKAGE).info(boost::format("contactAuthInfo, contact_id: %1%") % _contact_id);

    try {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec_params(
                "SELECT authinfopw FROM object o JOIN contact c ON c.id = o.id WHERE o.id = $1::integer",
                Database::query_param_list(_contact_id));

        if(res.size() == 0) {
            throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
        }

        return corba_wrap_string(res[0][0]);

    } catch (Registry::MojeID::Server::OBJECT_NOT_EXISTS) {
        throw;
    } catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
    } catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
    }
}

void sendAuthPasswords(unsigned long long cid, unsigned long long prid) {
    try {
        IdentificationRequestManagerPtr mgr;
        std::auto_ptr<Fred::PublicRequest::List> list(mgr->loadRequest(
                prid));
        // ownership stays with the list
        Fred::PublicRequest::PublicRequestAuth
            *new_auth_req =
            dynamic_cast<Fred::PublicRequest::PublicRequestAuth*> (list->get(0));

        if (new_auth_req == NULL) {
            LOGGER(PACKAGE).error(
                    "unable to create identfication request - wrong type");
        } else {
            new_auth_req->sendPasswords();
        }
    } catch (std::exception &ex) {
        LOGGER(PACKAGE).error(boost::format(
                "error when sending identification password"
                    " (contact_id=%1% public_request_id=%2%): %3%")
                % cid % prid % ex.what());
    } catch (...) {
        LOGGER(PACKAGE).error(boost::format(
                "error when sending identification password"
                    " (contact_id=%1% public_request_id=%2%)")
                % cid % prid);
    }
}

void updateObjectStates(unsigned long long cid) throw()
{
    try {
        // apply changes
        ::Fred::update_object_states(cid);
    } catch (std::exception &ex) {
        LOGGER(PACKAGE).error(boost::format("update_object_states failed for cid %1% (%2%)") % ex.what() % cid);
    } catch (...) {
        LOGGER(PACKAGE).error(boost::format("update_object_states failed for cid %1% (unknown exception)") % cid);
    }
}


}
}

