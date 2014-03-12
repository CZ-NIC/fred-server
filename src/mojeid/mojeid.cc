/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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

/**
 *  @mojeid.cc
 *  mojeid implementation
 */

#include "mojeid.h"

#include "mojeid/mojeid_identification.h"
#include "mojeid/mojeid_notifier.h"
#include "mojeid/mojeid_contact_states.h"

#include "fredlib/db_settings.h"
#include "fredlib/registry.h"
#include "fredlib/contact.h"
#include "fredlib/public_request/public_request.h"
#include "fredlib/object_states.h"
#include "fredlib/contact_verification/contact.h"
#include "mojeid/request.h"
#include "mojeid/mojeid_contact_states.h"
#include "mojeid/mojeid_disclose_policy.h"
#include "mojeid/public_request_verification_impl.h"
#include "mojeid/mojeid_validators.h"
#include "util/factory_check.h"
#include "util/util.h"
#include "util/xmlgen.h"

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

const std::string create_ctx_name(const std::string &_name)
{
    return str(boost::format("%1%-<%2%>")% _name % Random::integer(0, 10000));
}

namespace Registry
{
    namespace MojeID
    {
        MojeIDImpl::MojeIDImpl(const std::string &_server_name
                , boost::shared_ptr<Fred::Mailer::Manager> _mailer)
        : registry_conf_(CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleRegistryArgs>())
        , server_conf_(CfgArgs::instance()
            ->get_handler_ptr_by_type<HandleMojeIDArgs>())
        , server_name_(_server_name)
        , mojeid_registrar_id_(0)
        , mailer_(_mailer)
        {
            Logging::Context ctx_server(server_name_);
            Logging::Context ctx("init");
            ConnectionReleaser releaser;

            try {
                Database::Connection conn = Database::Manager::acquire();
                Database::Result result = conn.exec_params(
                    "SELECT id FROM registrar WHERE handle = $1::text"
                        , Database::query_param_list
                            (server_conf_->registrar_handle));

                if (result.size() != 1
                        || (mojeid_registrar_id_ = result[0][0]) == 0)
                {
                    throw std::runtime_error(
                            "failed to find dedicated registrar in database");
                }

                // factory_check - required keys are in factory
                FactoryHaveSupersetOfKeysChecker<Fred::PublicRequest::Factory>
                ::KeyVector required_keys = boost::assign::list_of
                    (Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION)
                    (Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION)
                    (Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION)
                    (Fred::PublicRequest::PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER)
                    (Fred::PublicRequest::PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER);


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
        }//MojeIDImpl::MojeIDImpl

        MojeIDImpl::~MojeIDImpl(){}

        const std::string& MojeIDImpl::get_server_name()
        {
            return server_name_;
        }


        unsigned long long MojeIDImpl::contactCreatePrepare(
                const std::string & _contact_username
                , Fred::Contact::Verification::Contact &_contact
                , const char* _trans_id
                , const unsigned long long _request_id
                , std::string & _identification)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("contact-create");
            ConnectionReleaser releaser;
            try
            {
                /* start new request - here for logging into action table - until
                 * fred-logd fully migrated */
                ::MojeID::Request request(
                        204, mojeid_registrar_id_, _request_id, _trans_id);
                Logging::Context ctx_request(Util::make_svtrid(_request_id));

                unsigned long long cid;
                unsigned long long hid;

                DBSharedPtr nodb;
                Fred::Contact::ManagerPtr contact_mgr(
                    Fred::Contact::Manager::create(
                        nodb, registry_conf_->restricted_handles));

                Fred::NameIdPair cinfo;
                Fred::Contact::Manager::CheckAvailType check_result;
                check_result = contact_mgr->checkAvail(_contact_username, cinfo);

                if (check_result != Fred::Contact::Manager::CA_FREE)
                {
                    Fred::Contact::Verification::FieldErrorMap errors;
                    errors[Fred::Contact::Verification::field_username]
                           = Fred::Contact::Verification::NOT_AVAILABLE;
                    throw Fred::Contact::Verification::DataValidationError(errors);
                }

                hid = Fred::Contact::Verification::contact_create(
                        request.get_request_id()
                        , request.get_registrar_id()
                        , _contact);
                cid = _contact.id;

                /* create public request */
                Fred::PublicRequest::Type type = Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION;
                IdentificationRequestPtr new_request(mailer_, type);
                new_request->setRegistrarId(request.get_registrar_id());
                new_request->setRequestId(request.get_request_id());
                new_request->addObject(Fred::PublicRequest::OID(
                            cid, _contact_username, Fred::PublicRequest::OT_CONTACT));
                new_request->save();

                IdentificationRequestManagerPtr request_manager(mailer_);
                _identification = request_manager
                    ->getPublicRequestAuthIdentification(cid, boost::assign::list_of(type));

                /* save contact and request (one transaction) */
                request.end_prepare(_trans_id);

                boost::mutex::scoped_lock td_lock(td_mutex);

                trans_data d(MOJEID_CONTACT_CREATE);
                d.cid = cid;
                d.prid = new_request->getId();
                d.request_id = request.get_request_id();
                transaction_data.insert(std::make_pair(_trans_id, d));

                td_lock.unlock();

                LOGGER(PACKAGE).info(boost::format(
                    "contact saved -- handle: %1%  cid: %2%  history_id: %3%")
                    % _contact_username % cid % hid);
                LOGGER(PACKAGE).info("request completed successfully");

                /* return new contact id */
                return cid;

            }//try
            catch (Fred::Contact::Verification::DataValidationError &_ex)
            {
                LOGGER(PACKAGE).info(_ex.what());
                throw;
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
        }//MojeIDImpl::contactCreatePrepare

        unsigned long long MojeIDImpl::contactTransferPrepare(
                const char* _handle
                , const char* _trans_id
                , unsigned long long _request_id
                , std::string& _identification)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("contact-transfer-request");
            ConnectionReleaser releaser;

            try
            {
                Database::Connection conn = Database::Manager::acquire();
                Database::Transaction tx(conn);

                std::string handle(_handle);

                Fred::NameIdPair cinfo;
                DBSharedPtr nodb;
                Fred::Contact::ManagerPtr contact_mgr(
                        Fred::Contact::Manager::create(nodb
                                , registry_conf_->restricted_handles));

                Fred::Contact::Manager::CheckAvailType check_result;
                check_result = contact_mgr->checkAvail(handle, cinfo);

                if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
                    throw Registry::MojeID::OBJECT_NOT_EXISTS();
                }

                Fred::Contact::Verification::FieldErrorMap errors;

                if (Fred::object_has_state(cinfo.id, Fred::ObjectState::VALIDATED_CONTACT) == true)
                {
                    errors[Fred::Contact::Verification::field_status]
                           = Fred::Contact::Verification::NOT_AVAILABLE;
                }
                else if ((Fred::object_has_state(cinfo.id
                            , Fred::ObjectState::SERVER_TRANSFER_PROHIBITED) == true)
                        || (Fred::object_has_state(cinfo.id
                            , Fred::ObjectState::SERVER_UPDATE_PROHIBITED) == true)) {

                    errors[Fred::Contact::Verification::field_status]
                           = Fred::Contact::Verification::INVALID;
                }
                if (errors.size() > 0) {
                    throw Fred::Contact::Verification
                        ::DataValidationError(errors);
                }

                /* create public request */
                Fred::PublicRequest::Type type;

                if (Fred::object_has_state(cinfo.id, Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT))
                {
                    type = Fred::PublicRequest::PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER;
                }
                else if (Fred::object_has_state(cinfo.id, Fred::ObjectState::IDENTIFIED_CONTACT))
                {
                    type = Fred::PublicRequest::PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER;
                }
                else {
                    type = Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION;
                }



                IdentificationRequestPtr new_request(mailer_, type);
                new_request->setRegistrarId(mojeid_registrar_id_);
                new_request->setRequestId(_request_id);
                new_request->addObject(Fred::PublicRequest::OID(
                            cinfo.id, handle, Fred::PublicRequest::OT_CONTACT));
                new_request->save();

                IdentificationRequestManagerPtr request_manager(mailer_);
                _identification = request_manager->getPublicRequestAuthIdentification(cinfo.id, boost::assign::list_of(type));

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

            }//try
            catch(Registry::MojeID::OBJECT_NOT_EXISTS& _ex)
            {
                LOGGER(PACKAGE).info(_ex.what());
                throw;
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
        }//MojeIDImpl::contactTransferPrepare

        void MojeIDImpl::contactUpdatePrepare(
            const std::string & _contact_username
            , Fred::Contact::Verification::Contact& _contact
            , const char* _trans_id
            , unsigned long long _request_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("contact-update");

            unsigned long long cid;
            try
            {
                /* XXX: not safe interface
                 * due to #8375 we need to load disclose flags from database
                 * should be replaced by separate data structures or better
                 * wrapped core operations approach
                 */
                contact_load_disclose_flags(_contact);
                LOGGER(PACKAGE).info(boost::format("request data --"
                    "  handle: %1%  transaction_id: %2%"
                    "  request_id: %3%")
                    % _contact_username
                    % _trans_id % _request_id);

                /* start new request - here for logging into action table
                 * - until fred-logd fully migrated */
                ::MojeID::Request request(203, mojeid_registrar_id_
                        , _request_id, _trans_id);
                Logging::Context ctx_request(Util::make_svtrid(_request_id));

                if (_contact.id == 0) {
                    throw std::runtime_error("contact.id is null");
                }
                cid = _contact.id;
                std::string handle = boost::to_upper_copy(_contact_username);

                Fred::NameIdPair cinfo;
                DBSharedPtr nodb;
                Fred::Contact::ManagerPtr contact_mgr(
                        Fred::Contact::Manager::create(nodb
                                , registry_conf_->restricted_handles));

                Fred::Contact::Manager::CheckAvailType check_result;
                check_result = contact_mgr->checkAvail(handle, cinfo);

                if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
                    throw Registry::MojeID::OBJECT_NOT_EXISTS();
                }

                if (cinfo.name != handle || cinfo.id != cid) {
                    throw std::runtime_error(str(boost::format(
                        "inconsistency in parameter --"
                        " passed: (id: %1%, handle: %2%), database (id: %3%, handle: %4%)")
                        % cid % handle % cinfo.id % cinfo.name));
                }

                Fred::Contact::Verification::ContactValidator validator
                    = Fred::Contact::Verification::create_contact_update_validator_mojeid();
                validator.check(_contact);

                DiscloseFlagPolicy::PolicyCallbackVector pcv
                    = boost::assign::list_of
                        (DiscloseFlagPolicy::PolicyCallback(SetDiscloseAddrTrueIfOrganization()))
                        (DiscloseFlagPolicy::PolicyCallback(SetDiscloseAddrTrueIfNotIdentified()));

                DiscloseFlagPolicy contact_disclose_policy (pcv);

                if (Fred::object_has_state(cid, Fred::ObjectState::VALIDATED_CONTACT) == true) {
                    if (Fred::Contact::Verification::check_validated_contact_diff(_contact
                            , Fred::Contact::Verification::contact_info(cid)) == false) {
                        /* change contact status to identified */
                        if (Fred::cancel_object_state(cid, Fred::ObjectState::VALIDATED_CONTACT)) {
                            Fred::insert_object_state(cid, Fred::ObjectState::IDENTIFIED_CONTACT);
                        }
                    }
                }

                contact_disclose_policy.apply(_contact);

                unsigned long long hid = Fred::Contact::Verification::contact_update(
                        request.get_request_id(),
                        request.get_registrar_id(),
                        _contact);

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
        }//MojeIDImpl::contactUpdatePrepare

        Fred::Contact::Verification::Contact MojeIDImpl::contactInfo(
                unsigned long long _contact_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("contact-info");
            ConnectionReleaser releaser;
            try
            {
                LOGGER(PACKAGE).info(boost::format("request data -- id: %1%")
                    % _contact_id);

                 Fred::NameIdPair cinfo;
                 DBSharedPtr nodb;
                 Fred::Contact::ManagerPtr contact_mgr(
                         Fred::Contact::Manager::create(nodb
                                 , registry_conf_->restricted_handles));

                 Fred::Contact::Manager::CheckAvailType check_result;
                 check_result = contact_mgr->checkAvail(_contact_id, cinfo);

                 if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
                     throw Registry::MojeID::OBJECT_NOT_EXISTS();
                 }

                 Fred::Contact::Verification::Contact data
                     = Fred::Contact::Verification::contact_info(_contact_id);

                 LOGGER(PACKAGE).info("request completed successfully");
                 return data;
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
        }//MojeIDImpl::contactInfo

        unsigned long long MojeIDImpl::processIdentification(
                const char* _ident_request_id
                , const char* _password
                , unsigned long long _request_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("process-identification");
            ConnectionReleaser releaser;

            try
            {
                LOGGER(PACKAGE).info(boost::format("request data --"
                     " identification_id: %1%  password: %2%  request_id: %3%")
                     % _ident_request_id % _password % _request_id);

                Database::Connection conn = Database::Manager::acquire();
                Database::Transaction trans(conn);

                 IdentificationRequestManagerPtr request_manager(mailer_ );
                 unsigned long long cid = request_manager->processAuthRequest(
                                     _ident_request_id, _password, _request_id);

                 trans.commit();

                 try {
                     if (server_conf_->notify_commands) {
                         Fred::MojeID::notify_contact_transfer(
                                 _request_id, mailer_);
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
            catch (Fred::PublicRequest::NotApplicable &_ex) {
                Fred::Contact::Verification::FieldErrorMap errors;
                errors[Fred::Contact::Verification::field_status] = Fred::Contact::Verification::INVALID;
                throw Fred::Contact::Verification::DataValidationError(errors);
            }
            catch (Fred::PublicRequest::PublicRequestAuth::NotAuthenticated&_ex){
                LOGGER(PACKAGE).info(_ex.what());
                throw;
            }
            catch (Fred::PublicRequest::AlreadyProcessed&_ex){
                LOGGER(PACKAGE).info(_ex.what());
                throw;
            }
            catch (Fred::NOT_FOUND &) {
                LOGGER(PACKAGE).info("identification not found");
                throw;
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
        }//MojeIDImpl::processIdentification

        std::string MojeIDImpl::getIdentificationInfo(
                unsigned long long _contact_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("get-identification");
            ConnectionReleaser releaser;

            try
            {
                LOGGER(PACKAGE).info(boost::format("get_identification --"
                            "  _contact_id: %1% ")
                        % _contact_id);

                Database::Connection conn = Database::Manager::acquire();
                Database::Transaction trans(conn);

                IdentificationRequestManagerPtr request_manager(mailer_  );
                std::vector<Fred::PublicRequest::Type> request_type_list
                    = boost::assign::list_of
                        (Fred::PublicRequest
                            ::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION)
                        (Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION);
                unsigned long long cid = static_cast<unsigned long long>(
                        _contact_id);

                std::string ret = request_manager->getPublicRequestAuthIdentification(
                        cid, request_type_list);
                trans.commit();
                return ret;
            }
            catch (Fred::NOT_FOUND &)
            {
                LOGGER(PACKAGE).info("request failed - object not found");
                throw;
            }
            catch (std::exception &_ex) {
                LOGGER(PACKAGE).error(boost::format("request failed (%1%)")
                    % _ex.what());
                throw;
            }
            catch (...) {
                LOGGER(PACKAGE).error("request failed (unknown error)");
                throw;
            }
        }//MojeIDImpl::getIdentificationInfo

        void MojeIDImpl::commitPreparedTransaction(const char* _trans_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("commit-prepared");

            trans_data tr_data(MOJEID_NOP);
            try {
                    LOGGER(PACKAGE).info(boost::format(
                        "request data -- transaction_id: %1%")
                        % _trans_id);

                if (std::string(_trans_id).empty())
                {
                     throw std::runtime_error("Transaction ID empty");
                }
                boost::mutex::scoped_lock search_lock(td_mutex);

                transaction_data_map_type::iterator it =
                    transaction_data.find(_trans_id);
                if(it == transaction_data.end()) {
                    throw std::runtime_error((boost::format(
                        "cannot retrieve saved transaction data"
                        " using transaction identifier %1%.")
                        % _trans_id).str());
                }
                search_lock.unlock();

                tr_data = it->second;

                LOGGER(PACKAGE).info(boost::format(
                        "Transaction data for %1% retrieved: "
                        "cid %2%, prid %3%, rid %4%")
                        % _trans_id % tr_data.cid % tr_data.prid
                        % tr_data.request_id);

                if(tr_data.cid == 0)
                {
                    throw std::runtime_error((boost::format(
                    "cannot retrieve contact id from transaction"
                    " identifier %1%.") % _trans_id).str());
                }

                Database::Connection conn = Database::Manager::acquire();
                conn.exec("COMMIT PREPARED '" + conn.escape(_trans_id) + "'");

                // successful commit , delete the data from map
                boost::mutex::scoped_lock erase_lock(td_mutex);
                transaction_data.erase(it);
                erase_lock.unlock();
            }
            catch (std::exception &_ex)
            {
                LOGGER(PACKAGE).error(boost::format(
                        "request failed (%1%)") % _ex.what());
                throw;
            }
            catch (...)
            {
                LOGGER(PACKAGE).error("request failed (unknown error)");
                throw;
            }

            // send identification password if operation is contact create
            if(tr_data.op == MOJEID_CONTACT_CREATE
                    || tr_data.op == MOJEID_CONTACT_TRANSFER)
            {
                sendAuthPasswords(tr_data.cid, tr_data.prid);
            }

            if(tr_data.op == MOJEID_CONTACT_UPDATE
                    || tr_data.op == MOJEID_CONTACT_UNIDENTIFY
                    || tr_data.op == MOJEID_CONTACT_CANCEL )
            {
                updateObjectStates(tr_data.cid);
            }

            /* request notification */
            try {
                if (tr_data.request_id && server_conf_->notify_commands)
                {
                    if (tr_data.op == MOJEID_CONTACT_UPDATE)
                    {
                        Fred::MojeID::notify_contact_update(
                                tr_data.request_id, mailer_);
                    }
                    else if (tr_data.op == MOJEID_CONTACT_CREATE)
                    {
                        Fred::MojeID::notify_contact_create(
                                tr_data.request_id, mailer_);
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

        }//MojeIDImpl::commitPreparedTransaction

        void MojeIDImpl::rollbackPreparedTransaction(
                            const char* _trans_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("rollback-prepared");

            trans_data tr(MOJEID_NOP);
            try {
                LOGGER(PACKAGE).info(boost::format(
                        "request data -- transaction_id: %1%")
                        % _trans_id);

                Database::Connection conn = Database::Manager::acquire();
                conn.exec("ROLLBACK PREPARED '"
                        + conn.escape(_trans_id) + "'");

                boost::mutex::scoped_lock erase_lock(td_mutex);
                transaction_data_map_type::iterator it =
                    transaction_data.find(_trans_id);
                if(it != transaction_data.end()) {
                    tr = it->second;
                    transaction_data.erase(it);
                }
                else
                {
                    LOGGER(PACKAGE).warning(boost::format(
                        "rollbackPrepared: Data for "
                        "transaction %1% not found.")
                        % _trans_id);
                    return;
                }
                erase_lock.unlock();
            }
            catch (std::exception &_ex) {
                LOGGER(PACKAGE).error(boost::format("request failed (%1%)")
                    % _ex.what());
                throw;
            }
            catch (...) {
                LOGGER(PACKAGE).error("request failed (unknown error)");
                throw;
            }

            LOGGER(PACKAGE).info("rollback completed");

        }//MojeIDImpl::rollbackPreparedTransaction


        void MojeIDImpl::getValidationPdf(
            unsigned long long _contact_id
            , std::stringstream& outstr)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
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
                Database::Transaction tx(conn);

                Fred::PublicRequest::lock_public_request_lock(
                        Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION,_contact_id);

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
                    " JOIN enum_public_request_type eprt ON eprt.id = pr.request_type "
                    " WHERE pr.resolve_time IS NULL AND pr.status = 0 "
                    " AND eprt.name = $1::text AND object_id = $2::integer",
                    Database::query_param_list
                        (Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION)
                        (_contact_id));
                if (res.size() != 1)
                    throw Registry::MojeID::OBJECT_NOT_EXISTS();
                IdentificationRequestManagerPtr req_man(mailer_);
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

                std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");

                Util::XmlTagPair("mojeid_valid", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("request_date", Util::XmlUnparsedCData(stringify(d.get()))))
                    (Util::XmlTagPair("request_id", Util::XmlUnparsedCData(static_cast<std::string>(res[0][1]))))
                    (Util::XmlTagPair("handle", Util::XmlUnparsedCData(static_cast<std::string>(res[0][7]))))
                    (Util::XmlTagPair("name", Util::XmlUnparsedCData(static_cast<std::string>(res[0][2]))))
                    (Util::XmlTagPair("organization", Util::XmlUnparsedCData(static_cast<std::string>(res[0][3]))))
                    (Util::XmlTagPair("ic", Util::XmlUnparsedCData(identType == 4 ? static_cast<std::string>(res[0][4]) : "")))
                    (Util::XmlTagPair("birth_date", Util::XmlUnparsedCData(identType == 6 ? stringify(birthdate_from_string_to_date(res[0][4])) : "")))
                    (Util::XmlTagPair("address", Util::XmlUnparsedCData(static_cast<std::string>(res[0][6]))))
                )(letter_xml);

                g->getInput() << letter_xml;
                g->closeInput();
                tx.commit();
            }//try
            catch (Registry::MojeID::OBJECT_NOT_EXISTS&)
            {
                LOGGER(PACKAGE).warning("request not exist");
                throw;
            }
            catch (std::exception &_ex)
            {
                LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
                throw;
            }
            catch (...)
            {
                LOGGER(PACKAGE).error("request failed (unknown error)");
                throw;
            }
        }//MojeIDImpl::getValidationPdf

        void MojeIDImpl::createValidationRequest(
            unsigned long long  _contact_id
            , unsigned long long  _request_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("create-validation-request");
            ConnectionReleaser releaser;

            try {
                LOGGER(PACKAGE).info(boost::format("request data --"
                            "  contact_id: %1%  request_id: %2%")
                        % _contact_id % _request_id);

                Database::Connection conn = Database::Manager::acquire();
                Database::Transaction tx(conn);

                Fred::NameIdPair cinfo;
                DBSharedPtr nodb;
                Fred::Contact::ManagerPtr contact_mgr(
                        Fred::Contact::Manager::create(
                            nodb
                            , registry_conf_->restricted_handles));

                Fred::Contact::Manager::CheckAvailType check_result;
                check_result = contact_mgr->checkAvail(_contact_id, cinfo);

                if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
                    /* contact doesn't exists */
                    throw Registry::MojeID::OBJECT_NOT_EXISTS();
                }

                /* create validation request */
                IdentificationRequestManagerPtr req_man(mailer_);
                std::auto_ptr<Fred::PublicRequest::PublicRequest> new_request(
                    req_man->createRequest(
                        Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION
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

                tx.commit();

                LOGGER(PACKAGE).info(boost::format(
                        "validation request created"
                        " -- public_request_id: %1%")
                        % 0);

                LOGGER(PACKAGE).info("request completed successfully");
            }
            catch (Registry::MojeID::OBJECT_NOT_EXISTS&) {
                LOGGER(PACKAGE).error("contact doesn't exists");
                throw;
            }
            catch (Fred::PublicRequest::RequestExists &_ex)
            {
                LOGGER(PACKAGE).error(boost::format("cannot create request (%1%)") % _ex.what());
                throw;
            }
            catch (Fred::PublicRequest::NotApplicable &_ex)
            {
                LOGGER(PACKAGE).error(boost::format("cannot create request (%1%)") % _ex.what());
                throw;
            }
            catch (std::exception &_ex)
            {
                LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
                throw;
            }
            catch (...)
            {
                LOGGER(PACKAGE).error("request failed (unknown error)");
                throw;
            }
        }//MojeIDImpl::createValidationRequest

        std::vector<ContactStateData> MojeIDImpl::getContactsStates(unsigned long _last_hours)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("get-contacts-states");
            ConnectionReleaser releaser;

            try
            {
                Database::Connection conn = Database::Manager::acquire();
                std::vector<ContactStateData> csdv;
                Database::Result rstates = conn.exec_params(
                "SELECT mojeid_contact.id, os.valid_from, CASE "
                 " WHEN bool_or (eos.name = 'validatedContact') "
                  " THEN 'validatedContact' "
                 " WHEN bool_or (eos.name = 'identifiedContact') "
                   " THEN 'identifiedContact' "
                 " WHEN bool_or (eos.name = 'conditionallyIdentifiedContact') "
                  " THEN 'conditionallyIdentifiedContact' "
                 " END AS name "
                " FROM (SELECT c.id AS id "
                  " FROM contact c "
                   " JOIN object o ON c.id=o.id "
                   " JOIN registrar r on o.clid = r.id "
                   " JOIN object_state os ON o.id = os.object_id "
                   " JOIN enum_object_states eos ON eos.id = os.state_id "
                  " WHERE os.valid_to IS NULL "
                   " AND r.handle=$1::text "
                   " AND eos.name = 'mojeidContact') AS mojeid_contact "
                 " JOIN object_state os ON mojeid_contact.id = os.object_id "
                 " JOIN enum_object_states eos ON eos.id = os.state_id "
                " WHERE os.valid_to IS NULL "
                   " AND os.valid_from > now() - $2::interval "
                  " AND eos.name = ANY ('{\"conditionallyIdentifiedContact\", \"identifiedContact\", \"validatedContact\"}') "
                " GROUP BY mojeid_contact.id, os.valid_from "
                , Database::query_param_list
                (server_conf_->registrar_handle)
                (boost::lexical_cast<std::string>(_last_hours) + " hours"));

                for (Database::Result::size_type i = 0; i < rstates.size(); ++i)
                {
                    ContactStateData csd;
                    csd.contact_id = static_cast<unsigned long long>(
                            rstates[i][0]);
                    csd.valid_from = rstates[i][1].isnull()
                            ? boost::gregorian::date()
                            : boost::gregorian::from_string(
                                static_cast<std::string>(rstates[i][1]));
                    csd.state_name = static_cast<std::string>(rstates[i][2]);

                    csdv.push_back(csd);
                }//for

                return csdv;
            }
            catch (std::exception &_ex)
            {
                LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
                throw;
            }
            catch (...)
            {
                LOGGER(PACKAGE).error("request failed (unknown error)");
                throw;
            }
        }//MojeIDImpl::getContactsStates

        ContactStateData MojeIDImpl::getContactState(unsigned long long _contact_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("get-contact-state");
            ConnectionReleaser releaser;

            try {
                Database::Connection conn = Database::Manager::acquire();
                ContactStateData csd;
                Database::Result rstates = conn.exec_params(
                        "SELECT c.id, mcs.valid_from, mcs.name FROM contact c "
                        " LEFT JOIN (SELECT mojeid_contact.id, os.valid_from, CASE "
                        " WHEN bool_or (eos.name = 'validatedContact') "
                        "   THEN 'validatedContact' "
                        "  WHEN bool_or (eos.name = 'identifiedContact') "
                        "   THEN 'identifiedContact' "
                        "  WHEN bool_or (eos.name = 'conditionallyIdentifiedContact') "
                        "   THEN 'conditionallyIdentifiedContact' "
                        "  END AS name "
                        " FROM (SELECT c.id AS id "
                        "   FROM contact c "
                        "    JOIN object o ON c.id=o.id "
                        "    JOIN registrar r on o.clid = r.id "
                        "    JOIN object_state os ON o.id = os.object_id "
                        "    JOIN enum_object_states eos ON eos.id = os.state_id "
                        "   WHERE os.valid_to IS NULL "
                        "    AND r.handle=$1::text "
                        "    AND c.id = $2::bigint "
                        "    AND eos.name = 'mojeidContact') AS mojeid_contact "
                        "  JOIN object_state os ON mojeid_contact.id = os.object_id "
                        "  JOIN enum_object_states eos ON eos.id = os.state_id "
                        " WHERE os.valid_to IS NULL "
                        "   AND eos.name = ANY ('{\"conditionallyIdentifiedContact\", \"identifiedContact\", \"validatedContact\"}') "
                        " GROUP BY mojeid_contact.id, os.valid_from "
                        " ) AS mcs ON mcs.id = c.id "
                        " WHERE c.id = $2::bigint "
                        , Database::query_param_list
                            (server_conf_->registrar_handle)
                            (_contact_id));

                if (rstates.size() == 0) {
                    throw Registry::MojeID::OBJECT_NOT_EXISTS();
                }
                else if (rstates.size() != 1) {
                    throw std::runtime_error("Object appears to be in several exclusive states");
                }

                csd.contact_id = static_cast<unsigned long long>(rstates[0][0]);;
                csd.valid_from = rstates[0][1].isnull()
                        ? boost::gregorian::date()
                        : boost::gregorian::from_string(
                            static_cast<std::string>(rstates[0][1]));
                csd.state_name = static_cast<std::string>(rstates[0][2]);
                return csd;
            }//try
            catch (std::exception &_ex)
            {
                LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
                throw;
            }
            catch (...)
            {
                LOGGER(PACKAGE).error("request failed (unknown error)");
                throw;
            }
        }//MojeIDImpl::getContactState

        unsigned long long MojeIDImpl::getContactId(const std::string& _handle)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("get-contact-id");
            ConnectionReleaser releaser;

            try
            {
                Fred::NameIdPair cinfo;
                DBSharedPtr nodb;
                Fred::Contact::ManagerPtr contact_mgr(
                        Fred::Contact::Manager::create(
                                nodb, registry_conf_->restricted_handles));

                Fred::Contact::Manager::CheckAvailType check_result;
                check_result = contact_mgr->checkAvail(_handle, cinfo);

                if (check_result == Fred::Contact::Manager::CA_REGISTRED) {
                    LOGGER(PACKAGE).info(boost::format(
                                "contact %1% => id=%2%") % _handle % cinfo.id);
                    return cinfo.id;
                }
            }//try
            catch (std::exception &_ex)
            {
                LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
                throw;
            }
            catch (...)
            {
                LOGGER(PACKAGE).error("request failed (unknown error)");
                throw;
            }
            //for the case when try block ends without return
            throw Registry::MojeID::OBJECT_NOT_EXISTS();
        }//MojeIDImpl::getContactId

        std::vector<std::string> MojeIDImpl::getUnregistrableHandles()
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
                    , Database::query_param_list(Fred::Contact::Verification
                            ::USERNAME_PATTERN.str()));
                std::vector<std::string> ret;
                ret.reserve(res.size());
                for (Database::Result::size_type i=0; i<res.size(); ++i)
                {
                    ret.push_back(std::string(res[i][0]));
                }
                return ret;
            }//try
            catch (std::exception &_ex)
            {
                LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
                throw;
            }
            catch (...)
            {
                LOGGER(PACKAGE).error("request failed (unknown error)");
                throw;
            }
        }

        std::string MojeIDImpl::contactAuthInfo(const unsigned long long _contact_id)
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
                    throw Registry::MojeID::OBJECT_NOT_EXISTS();
                }

                return std::string(res[0][0]);

            }//try
            catch (std::exception &_ex) {
                LOGGER(PACKAGE).error(boost::format("request failed (%1%)")
                    % _ex.what());
                throw;
            }
            catch (...) {
                LOGGER(PACKAGE).error("request failed (unknown error)");
                throw;
            }
        }

        ///cancel mojeidContact state
        void MojeIDImpl::contactCancelAccountPrepare(
            unsigned long long _contact_id
             , const char* _trans_id
             , unsigned long long _request_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("contact-cancel-account");
            ConnectionReleaser releaser;

            LOGGER(PACKAGE).info(boost::format("contactCancelAccountPrepare --"
                    "  contact_id: %1%  request_id: %2%")
                        % _contact_id % _request_id);

            try {
                Database::Connection conn = Database::Manager::acquire();
                Database::Transaction tx(conn);

                // check if the contact with ID _contact_id exists
                Fred::NameIdPair cinfo;
                DBSharedPtr nodb;
                Fred::Contact::ManagerPtr contact_mgr(
                    Fred::Contact::Manager::create(
                        nodb
                        , registry_conf_->restricted_handles));

                Fred::Contact::Manager::CheckAvailType check_result;
                check_result = contact_mgr->checkAvail(_contact_id, cinfo);

                if (check_result != Fred::Contact::Manager::CA_REGISTRED) {
                    /* contact doesn't exists */
                    throw Registry::MojeID::OBJECT_NOT_EXISTS();
                }

                if(!isMojeidContact(_contact_id))
                {
                    throw std::runtime_error(
                            "Contact is not registered with MojeID");
                }

                //try lock object states
                Fred::lock_multiple_object_states(_contact_id
                    , Util::vector_of<std::string>
                        (::MojeID::ObjectState::MOJEID_CONTACT)
                        (Fred::ObjectState::SERVER_DELETE_PROHIBITED)
                        (Fred::ObjectState::SERVER_TRANSFER_PROHIBITED)
                        (Fred::ObjectState::SERVER_UPDATE_PROHIBITED)
                        (Fred::ObjectState::VALIDATED_CONTACT) );

                if(!Fred::cancel_object_state(
                    _contact_id, ::MojeID::ObjectState::MOJEID_CONTACT))
                {
                    throw std::runtime_error(
                            "Fred::cancel_object_state mojeidContact failed");
                }

                if(!Fred::cancel_object_state(
                    _contact_id, Fred::ObjectState::SERVER_DELETE_PROHIBITED))
                {
                    throw std::runtime_error(
                        "Fred::cancel_object_state serverDeleteProhibited failed");
                }

                if(!Fred::cancel_object_state(
                    _contact_id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED))
                {
                    throw std::runtime_error(
                        "Fred::cancel_object_state serverTransferProhibited failed");
                }

                if(!Fred::cancel_object_state(
                    _contact_id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED))
                {
                    throw std::runtime_error(
                        "Fred::cancel_object_state serverUpdateProhibited failed");
                }

                if(Fred::object_has_state(_contact_id, Fred::ObjectState::VALIDATED_CONTACT))
                {
                    //cancel validated
                    if(!Fred::cancel_object_state(_contact_id
                            , Fred::ObjectState::VALIDATED_CONTACT))
                    {
                        throw std::runtime_error(
                            "Fred::cancel_object_state validatedContact failed");
                    }

                    //set identified contact
                    Fred::insert_object_state(
                            _contact_id, Fred::ObjectState::IDENTIFIED_CONTACT);
                }

                //lock public requests
                Fred::PublicRequest::lock_public_request_lock(
                    Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION,_contact_id);
                Fred::PublicRequest::lock_public_request_lock(
                    Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION,_contact_id);
                Fred::PublicRequest::lock_public_request_lock(
                    Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION,_contact_id);

                conn.exec_params("UPDATE public_request pr SET status=2 "
                    " , resolve_time = CURRENT_TIMESTAMP "
                    " FROM public_request_objects_map prom WHERE (prom.request_id = pr.id) "
                    " AND pr.resolve_time IS NULL AND pr.status = 0 "
                    " AND pr.request_type IN (12,13,14) AND object_id = $1::integer",
                        Database::query_param_list(_contact_id));


                tx.prepare(_trans_id);

                trans_data tr_data(MOJEID_CONTACT_CANCEL);
                tr_data.cid = _contact_id;

                boost::mutex::scoped_lock td_lock(td_mutex);
                transaction_data.insert(std::make_pair(_trans_id, tr_data));

            }//try
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
        }//MojeIDImpl::contactCancelAccountPrepare

        ///check if contact is mojeid contact
        bool MojeIDImpl::isMojeidContact(unsigned long long _contact_id)
        {
            Database::Connection conn = Database::Manager::acquire();

            Database::Result contact_result = conn.exec_params(
                "SELECT contact_select.id "
                " , contact_select.handle = $1::text "
                "   AS is_mojeid_registrar "
                " , coalesce(bool_or (eos.name = 'mojeidContact' "
                "     AND os.valid_to IS NULL), false) "
                "   AS mojeidContact "
                " , bool_or (eos.name = 'conditionallyIdentifiedContact' "
                "     AND os.valid_to IS NULL) "
                "   AS conditionallyIdentifiedContact "
                " , bool_or (eos.name = 'identifiedContact' "
                "     AND os.valid_to IS NULL) "
                "   AS identifiedContact "
                " , bool_or (eos.name = 'validatedContact' "
                "     AND os.valid_to IS NULL) "
                "   AS validatedContact "
                " FROM (SELECT c.id, r.handle FROM contact c "
                "  JOIN object o ON c.id=o.id "
                "  JOIN registrar r on o.clid = r.id "
                " AND c.id = $2::bigint "
                "  ) AS contact_select "
                " LEFT JOIN (object_state os "
                "  JOIN enum_object_states eos "
                "   ON eos.id = os.state_id AND os.valid_to IS NULL "
                "  ) ON contact_select.id = os.object_id "
                " GROUP BY contact_select.id, contact_select.handle "
                , Database::query_param_list
                (server_conf_->registrar_handle)
                (_contact_id));

            if(contact_result.size() == 0)
            {
                throw std::runtime_error(
                        "MojeIDImpl::isMojeidContact: contact doesn't exist");
            }

            if(std::string(contact_result[0]["mojeidContact"])
                .compare("f") == 0)
            {
                //not mojeidContact - ok
                return false;
            }

            if(std::string(contact_result[0]["is_mojeid_registrar"])
                .compare("f") == 0)
            {
                //contact is in state mojeidContact
                // and do not have mojeid registrar - nok
                throw std::runtime_error(
                    "MojeIDImpl::isMojeidContact: contact is in state "
                    " mojeidContact and don't have mojeid registrar");
            }

            if(
                (std::string(contact_result[0]["conditionallyIdentifiedContact"])
                .compare("t") == 0)
                || (std::string(contact_result[0]["identifiedContact"])
                .compare("t") == 0)
                || (std::string(contact_result[0]["validatedContact"])
                .compare("t") == 0)
            )
            {
                return true;//ok
            }
            else
            {
                //contact lack identification state - nok
                throw std::runtime_error(
                    "MojeIDImpl::isMojeidContact: missing identification state");
            }
        }//MojeIDImpl::isMojeidContact by id

        ///check if contact is mojeid contact
        bool MojeIDImpl::isMojeidContact(const std::string &_contact_handle)
        {
            Database::Connection conn = Database::Manager::acquire();
            Database::Result contact_result = conn.exec_params(
                "SELECT c.id FROM contact c "
                " JOIN object_registry obr ON c.id=obr.id "
                " WHERE obr.name = $1::text "
                , Database::query_param_list(_contact_handle));

            if(contact_result.size() == 0)
            {
                throw std::runtime_error(
                    "MojeIDImpl::isMojeidContact: "
                    " contact handle doesn't exist");
            }
            unsigned long long contact_id (contact_result[0][0]);
            return isMojeidContact(contact_id);
        }//MojeIDImpl::isMojeidContact by handle


        void MojeIDImpl::sendAuthPasswords(unsigned long long cid, unsigned long long prid)
        {
            try
            {
                Database::Connection conn = Database::Manager::acquire();
                Database::Transaction tx(conn);

                IdentificationRequestManagerPtr mgr(mailer_);
                std::auto_ptr<Fred::PublicRequest::List> list(mgr->loadRequest(
                        prid));
                // ownership stays with the list
                Fred::PublicRequest::PublicRequestAuth
                    *new_auth_req =
                    dynamic_cast<Fred::PublicRequest::PublicRequestAuth*>
                        (list->get(0));

                if (new_auth_req == NULL)
                {
                    LOGGER(PACKAGE).error(
                            "unable to create identfication request - wrong type");
                }
                else
                {
                    new_auth_req->sendPasswords();
                }
                tx.commit();
            }
            catch (std::exception &ex)
            {
                LOGGER(PACKAGE).error(boost::format(
                        "error when sending identification password"
                            " (contact_id=%1% public_request_id=%2%): %3%")
                        % cid % prid % ex.what());
            }
            catch (...)
            {
                LOGGER(PACKAGE).error(boost::format(
                        "error when sending identification password"
                            " (contact_id=%1% public_request_id=%2%)")
                        % cid % prid);
            }
        }//MojeIDImpl::sendAuthPasswords

        void updateObjectStates(unsigned long long cid) throw()
        {
            try
            {
                // apply changes
                ::Fred::update_object_states(cid);
            }
            catch (std::exception &ex)
            {
                LOGGER(PACKAGE).error(boost::format(
                    "update_object_states failed for cid %1% (%2%)")
                    % ex.what() % cid);
            }
            catch (...)
            {
                LOGGER(PACKAGE).error(boost::format(
                    "update_object_states failed for cid "
                    "%1% (unknown exception)")
                    % cid);
            }
        }//updateObjectStates

    }//namespace MojeID
}//namespace Registry



