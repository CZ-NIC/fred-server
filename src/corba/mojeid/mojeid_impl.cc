
#include "corba_conversion.h"
#include "mojeid_impl.h"
#include "mojeid_identification.h"

#include "cfg/config_handler_decl.h"
#include "log/logger.h"
#include "log/context.h"
#include "random.h"
#include "corba_wrapper_decl.h"

#include "register/db_settings.h"
#include "register/register.h"
#include "register/contact.h"
#include "register/public_request.h"
#include "register/mojeid/contact.h"
#include "register/mojeid/request.h"

#include "corba/connection_releaser.h"
#include "corba/mailer_manager.h"

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
      mojeid_registrar_id_(0)
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


CORBA::ULongLong ServerImpl::contactCreate(const Contact &_contact,
                                           IdentificationMethod _method,
                                           const CORBA::ULongLong _request_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-create");
    ConnectionReleaser releaser;

    try {
        std::string handle = static_cast<std::string>(_contact.username);
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  handle: %1%  identification_method: %2%  request_id: %3%")
                % handle % _method % _request_id);

        /* start new request - here for logging into action table - until
         * fred-logd fully migrated */
        ::MojeID::Request request(204, mojeid_registrar_id_, _request_id);
        Logging::Context ctx_request(request.get_servertrid());

        try {
            Register::Contact::ManagerPtr contact_mgr(
                    Register::Contact::Manager::create(0, registry_conf_->restricted_handles));
            LOGGER(PACKAGE).debug(boost::format("handle '%1%' availability check")
                    % handle);

            Register::NameIdPair cinfo;
            Register::Contact::Manager::CheckAvailType check_result;
            check_result = contact_mgr->checkAvail(handle, cinfo);

            if (check_result == Register::Contact::Manager::CA_INVALID_HANDLE) {
                throw std::runtime_error(str(boost::format("handle '%1%' is not valid")
                            % handle));
            }
            else if (check_result == Register::Contact::Manager::CA_REGISTRED) {
                throw std::runtime_error(str(boost::format("handle '%1%' is already registered")
                            % handle));
            }
            else if (check_result == Register::Contact::Manager::CA_PROTECTED) {
                throw std::runtime_error(str(boost::format("handle '%1%' is in protection period")
                        % handle));
            }
            LOGGER(PACKAGE).debug(boost::format("handle '%1%' check passed")
                    % handle);
        }
        catch (std::exception &_ex) {
            throw;
        }
        catch (...) {
            throw std::runtime_error("contact handle availability check failed");
        }

        /* TODO: more checking? */
        if (_contact.addresses.length() == 0) {
            throw std::runtime_error("contact has no address");
        }
        if (_contact.emails.length() == 0) {
            throw std::runtime_error("contact has no email");
        }

        ::MojeID::Contact data = corba_unwrap_contact(_contact);
        unsigned long long hid = ::MojeID::contact_create(
                request.get_id(),
                request.get_request_id(),
                request.get_registrar_id(),
                data);
        unsigned long long id = data.id;

        /* create public request */
        Register::PublicRequest::Type type;
        if (_method == Registry::MojeID::SMS) {
            type = Register::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION;
        }
        else if (_method == Registry::MojeID::LETTER) {
            type = Register::PublicRequest::PRT_CONTACT_IDENTIFICATION;
        }
        else if (_method == Registry::MojeID::CERTIFICATE) {
            throw std::runtime_error("not implemented");
        }
        else {
            throw std::runtime_error("unknown identification method");
        }

        IdentificationRequestPtr new_request(type);
        new_request->setRegistrarId(request.get_registrar_id());
        new_request->setRequestId(request.get_request_id());
        new_request->setEppActionId(request.get_id());
        new_request->addObject(
                Register::PublicRequest::OID(
                    id, handle, Register::PublicRequest::OT_CONTACT));
        new_request->save();

        /* save contact and request (one transaction) */
        request.end_success();

        LOGGER(PACKAGE).info(boost::format(
                "contact saved -- handle: %1%  id: %2%  history_id: %3%")
                % handle % id % hid);
        LOGGER(PACKAGE).info("request completed successfully");

        /* send identification passwords */
        try {
            new_request->sendPasswords();
            LOGGER(PACKAGE).info("identification password sent");
        }
        catch (...) {
            LOGGER(PACKAGE).error(boost::format(
                        "error when sending identification password"
                        " (contact_id=%1% public_request_id=%2%)")
                    % id % new_request->getId());
        }

        /* return new contact id */
        return id;
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::ErrorReport();
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
                    "  identification_id: %1%  password: %2%  request_id: %3%")
                % _ident_request_id % _password % _request_id);

        IdentificationRequestManagerPtr request_manager;
        return request_manager->processAuthRequest(_ident_request_id, _password);
    }
    catch (Register::PublicRequest::PublicRequestAuth::NOT_AUTHENTICATED&) {
        LOGGER(PACKAGE).info("request authentication failed (bad password)");
        throw Registry::MojeID::Server::ErrorReport("not authenticated");
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::ErrorReport();
    }

}

CORBA::ULongLong ServerImpl::transferContact(const char* _handle,
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

        Register::NameIdPair cinfo;
        try {
            Register::Contact::ManagerPtr contact_mgr(
                    Register::Contact::Manager::create(0, registry_conf_->restricted_handles));

            LOGGER(PACKAGE).debug(boost::format("handle '%1%' availability check")
                    % handle);

            Register::Contact::Manager::CheckAvailType check_result;
            check_result = contact_mgr->checkAvail(handle, cinfo);

            if (check_result != Register::Contact::Manager::CA_REGISTRED) {
                throw std::runtime_error(str(boost::format("handle '%1%' is not registered")
                            % handle));
            }
            LOGGER(PACKAGE).debug(boost::format("handle '%1%' check passed")
                    % handle);

        }
        catch (std::exception &_ex) {
            throw;
        }
        catch (...) {
            throw std::runtime_error("contact handle availability check failed");
        }

        /* TODO: more checking? */

        /* create public request */
        Register::PublicRequest::Type type;
        if (_method == Registry::MojeID::SMS) {
            type = Register::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION;
        }
        else if (_method == Registry::MojeID::LETTER) {
            type = Register::PublicRequest::PRT_CONTACT_IDENTIFICATION;
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
        new_request->setEppActionId(0);
        new_request->addObject(
                Register::PublicRequest::OID(
                    cinfo.id, handle, Register::PublicRequest::OT_CONTACT));
        new_request->save();
        new_request->sendPasswords();

        LOGGER(PACKAGE).info(boost::format(
                "identification request with contact transfer saved"
                " -- handle: %1%  id: %2%")
                % handle % cinfo.id);

        LOGGER(PACKAGE).info("request completed successfully");
        return cinfo.id;
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::ErrorReport();
    }
}


void ServerImpl::contactUpdatePrepare(const Contact &_contact,
                                      const char* _trans_id,
                                      const CORBA::ULongLong _request_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-update");

    try {
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  handle: %1%  transaction_id: %2%"
                    "  request_id: %3%")  % static_cast<std::string>(_contact.username)
                    % _trans_id % _request_id);

        /* start new request - here for logging into action table - until
         * fred-logd fully migrated */
        ::MojeID::Request request(203, mojeid_registrar_id_, _request_id, _trans_id);
        Logging::Context ctx_request(request.get_servertrid());

        if (_contact.id == 0) {
            throw std::runtime_error("contact.id is null");
        }
        unsigned long long id = _contact.id->_value();
        std::string handle = boost::to_upper_copy(corba_unwrap_string(_contact.username));

        Register::NameIdPair cinfo;
        try {
            Register::Contact::ManagerPtr contact_mgr(
                    Register::Contact::Manager::create(0, registry_conf_->restricted_handles));

            LOGGER(PACKAGE).debug(boost::format("handle '%1%' availability check")
                    % handle);

            Register::Contact::Manager::CheckAvailType check_result;
            check_result = contact_mgr->checkAvail(handle, cinfo);

            if (check_result != Register::Contact::Manager::CA_REGISTRED) {
                throw std::runtime_error(str(boost::format("handle '%1%' is not registered")
                            % handle));
            }
            LOGGER(PACKAGE).debug(boost::format("handle '%1%' check passed")
                    % handle);
        }
        catch (std::exception &_ex) {
            throw;
        }
        catch (...) {
            throw std::runtime_error("contact handle availability check failed");
        }

        if (cinfo.name != handle || cinfo.id != id) {
            throw std::runtime_error(str(boost::format(
                        "inconsistency in parameter --"
                        " passed: (id: %1%, handle: %2%), database (id: %3%, handle: %4%)")
                    % id % handle % cinfo.id % cinfo.name));
        }

        /* TODO: checking for prohibited states */

        ::MojeID::Contact data = corba_unwrap_contact(_contact);
        unsigned long long hid = ::MojeID::contact_update(
                request.get_id(),
                request.get_request_id(),
                data);

        LOGGER(PACKAGE).info(boost::format(
                "contact updated -- handle: %1%  id: %2%  history_id: %3%")
                % handle % id % hid);

        request.end_prepare(_trans_id);
        LOGGER(PACKAGE).info("request completed successfully");
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::ErrorReport();
    }

}


Contact* ServerImpl::contactInfo(const CORBA::ULongLong _id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-info");
    ConnectionReleaser releaser;

    try {
        LOGGER(PACKAGE).info(boost::format("request data -- id: %1%") % _id);

        Contact *data = corba_wrap_contact(::MojeID::contact_info(_id));

        LOGGER(PACKAGE).info("request completed successfully");
        return data;
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::ErrorReport();
    }
}


void ServerImpl::commitPreparedTransaction(const char* _trans_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("commit-prepared");

    try {
        LOGGER(PACKAGE).info(boost::format("request data -- transaction_id: %1%")
                % _trans_id);

        Database::Connection conn = Database::Manager::acquire();
        conn.exec("COMMIT PREPARED '" + conn.escape(_trans_id) + "'");

        /* TEMP: until we finish migration to request logger */
        try {
            Database::Result result = conn.exec_params(
                    "SELECT id FROM action WHERE"
                    " enddate IS NULL AND response IS NULL"
                    " AND clienttrid = $1::text",
                    Database::query_param_list(_trans_id));

            unsigned long long id = 0;
            if (result.size() != 1 || (id = result[0][0]) == 0) {
                LOGGER(PACKAGE).warning("unable to find unique action with given"
                        " transaction id as clienttrid");
            }
            else {
                conn.exec_params("UPDATE action SET response = 1000, enddate = now()"
                        " WHERE id = $1::integer", Database::query_param_list(id));
            }
        }
        catch (...) {
            LOGGER(PACKAGE).error("error occured when updating action response");
        }

        LOGGER(PACKAGE).info("request completed successfully");
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::ErrorReport();
    }
}


void ServerImpl::rollbackPreparedTransaction(const char* _trans_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("rollback-prepared");

    try {
        LOGGER(PACKAGE).info(boost::format("request data -- transaction_id: %1%")
                % _trans_id);

        Database::Connection conn = Database::Manager::acquire();
        conn.exec("ROLLBACK PREPARED '" + conn.escape(_trans_id) + "'");

        /* TEMP: until we finish migration to request logger */
        try {
            Database::Result result = conn.exec_params(
                    "SELECT id FROM action WHERE"
                    " enddate IS NULL AND response IS NULL"
                    " AND clienttrid = $1::text",
                    Database::query_param_list(_trans_id));

            unsigned long long id = 0;
            if (result.size() != 1 || (id = result[0][0]) == 0) {
                LOGGER(PACKAGE).warning("unable to find unique action with given"
                        " transaction id as clienttrid");
            }
            else {
                conn.exec_params("UPDATE action SET response = 2400, enddate = now()"
                        " WHERE id = $1::integer", Database::query_param_list(id));
            }
        }
        catch (...) {
            LOGGER(PACKAGE).error("error occured when updating action response");
        }

        LOGGER(PACKAGE).info("request completed successfully");
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::ErrorReport();
    }
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
        return corba_wrap_string(request_manager->getIdentification(_contact_id).c_str());
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::Server::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::Server::ErrorReport();
    }
}


ContactStateChangeList* ServerImpl::getContactStateChanges(const Date& since)
{
    ContactStateChangeList_var ret = new ContactStateChangeList;
    ret->length(0);
    return ret._retn();
}


}
}

