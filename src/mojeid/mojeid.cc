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
 *  @file
 *  mojeid implementation
 */

#include "mojeid.h"

#include "src/mojeid/mojeid_identification.h"
#include "src/mojeid/mojeid_notifier.h"
#include "src/mojeid/mojeid_contact_states.h"
#include "src/fredlib/contact_verification/contact_verification_state.h"
#include "src/fredlib/contact_verification/contact_verification_password.h"

#include "src/fredlib/db_settings.h"
#include "src/fredlib/registry.h"
#include "src/fredlib/contact.h"
#include "src/fredlib/public_request/public_request.h"
#include "src/fredlib/public_request/public_request_impl.h"
#include "src/fredlib/object_states.h"
#include "src/fredlib/contact_verification/contact.h"
#include "src/contact_verification/contact_verification_impl.h"
#include "src/mojeid/request.h"
#include "src/mojeid/mojeid_contact_states.h"
#include "src/mojeid/mojeid_disclose_policy.h"
#include "src/mojeid/public_request_verification_impl.h"
#include "src/mojeid/mojeid_validators.h"
#include "util/factory_check.h"
#include "util/util.h"
#include "util/map_at.h"
#include "util/xmlgen.h"

#include "cfg/config_handler_decl.h"
#include "log/logger.h"
#include "random.h"
#include "src/corba/connection_releaser.h"
#include "types/stringify.h"
#include "types/birthdate.h"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>

#include "src/admin/contact/verification/contact_states/delete_all.h"

const std::string create_ctx_name(const std::string &_name)
{
    return str(boost::format("%1%-<%2%>")% _name % Random::integer(0, 10000));
}

namespace Registry
{
    namespace MojeID
    {
        ContactStateData::StateValidFrom::const_iterator ContactStateData::get_sum_state() const
        {
            StateValidFrom::const_iterator state_ptr = state.find(Fred::ObjectState::VALIDATED_CONTACT);
            if (state_ptr != state.end()) {
                return state_ptr;
            }
            state_ptr = state.find(Fred::ObjectState::IDENTIFIED_CONTACT);
            if (state_ptr != state.end()) {
                return state_ptr;
            }
            return state.find(Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);
        }

        namespace
        {
            std::string get_contact_handle(unsigned long long _contact_id, Database::Connection &_conn)
            {
                const Database::Result result = _conn.exec_params(
                    "SELECT obr.name "
                    "FROM contact c "
                    "JOIN object_registry obr ON obr.id=c.id "
                    "WHERE c.id=$1::BIGINT", Database::query_param_list(_contact_id));
                if (0 < result.size()) {
                    return static_cast< std::string >(result[0][0]);
                }
                throw Registry::MojeID::OBJECT_NOT_EXISTS();
            }

            void check_sent_letters_limit(unsigned long long _contact_id,
                                          unsigned _max_sent_letters,
                                          unsigned _watched_period_in_days,
                                          Database::Connection &_conn)
            {
                const Database::Result result = _conn.exec_params(
"WITH comm_type_letter AS (SELECT id FROM comm_type WHERE type='letter'),"
     "message_types AS (SELECT id FROM message_type WHERE type IN ('mojeid_pin3')),"
     "send_states AS (SELECT id FROM enum_send_status WHERE status_name IN ('send_failed',"
                                                                           "'sent',"
                                                                           "'being_sent',"
                                                                           "'undelivered'))"
"SELECT (ma.moddate+($3::TEXT||'DAYS')::INTERVAL)::DATE "
"FROM message_archive ma "
"JOIN message_contact_history_map mc ON mc.message_archive_id=ma.id "
"WHERE ma.message_type_id IN (SELECT id FROM message_types) AND "
      "ma.comm_type_id=(SELECT id FROM comm_type_letter) AND "
      "ma.status_id IN (SELECT id FROM send_states) AND "
      "(NOW()-($3::TEXT||'DAYS')::INTERVAL)::DATE<ma.moddate::DATE AND "
      "mc.contact_object_registry_id=$1::INTEGER "
"ORDER BY 1 DESC OFFSET ($2::INTEGER-1) LIMIT 1",
                    Database::query_param_list(_contact_id)               // used as $1::INTEGER
                                              (_max_sent_letters)         // used as $2::INTEGER
                                              (_watched_period_in_days)); // used as $3::TEXT
                if (result.size() <= 0) {
                    return;
                }
                throw MESSAGE_LIMIT_EXCEEDED(
                          boost::gregorian::from_simple_string(
                              static_cast< std::string >(result[0][0])),
                          _max_sent_letters,
                          _watched_period_in_days);
            }
        }

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
                    (Fred::PublicRequest::PRT_MOJEID_CONTACT_REIDENTIFICATION)
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

                const Fred::Contact::Verification::State contact_state =
                    Fred::Contact::Verification::get_contact_verification_state(cinfo.id);
                if (contact_state.has_all(Fred::Contact::Verification::State::ciVm)) {// already V
                    errors[Fred::Contact::Verification::field_status]
                           = Fred::Contact::Verification::NOT_AVAILABLE;
                }
                else if (Fred::object_has_state(cinfo.id
                            , Fred::ObjectState::SERVER_TRANSFER_PROHIBITED) ||
                         Fred::object_has_state(cinfo.id
                            , Fred::ObjectState::SERVER_UPDATE_PROHIBITED)) {

                    errors[Fred::Contact::Verification::field_status]
                           = Fred::Contact::Verification::INVALID;
                }
                if (0 < errors.size()) {
                    throw Fred::Contact::Verification
                        ::DataValidationError(errors);
                }

                /* create public request */
                Fred::PublicRequest::Type type;

                if (contact_state.has_all(Fred::Contact::Verification::State::cIvm)) {// already I
                    type = Fred::PublicRequest::PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER;
                }
                else if (contact_state.has_all(Fred::Contact::Verification::State::Civm)) {// already C
                    type = Fred::PublicRequest::PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER;
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
            catch (const Registry::Contact::Verification::DATA_VALIDATION_ERROR& _ex)
            {
                LOGGER(PACKAGE).warning(_ex.what());
                throw;
            }
            catch (const Fred::Contact::Verification::DataValidationError& _ex)
            {
                LOGGER(PACKAGE).warning(_ex.what());
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

                const Fred::Contact::Verification::State contact_state =
                    Fred::Contact::Verification::get_contact_verification_state(cid);
                if (contact_state.has_all(Fred::Contact::Verification::State::ciVm)) {// already V
                    if (!Fred::Contact::Verification::check_validated_contact_diff(_contact
                            , Fred::Contact::Verification::contact_info(cid))) {
                        /* drop contact validated status */
                        Fred::cancel_object_state(cid, Fred::ObjectState::VALIDATED_CONTACT);
                    }
                }

                unsigned long long prid = 0; // new public request id
                if (!Fred::Contact::Verification::check_identified_contact_diff(_contact, Fred::Contact::Verification::contact_info(cid)))
                {
                    if (contact_state.has_all(Fred::Contact::Verification::State::cIvm))
                    {
                        /* drop contact identified status */
                        Fred::cancel_object_state(cid, Fred::ObjectState::IDENTIFIED_CONTACT);
                    }

                    Fred::PublicRequest::Type type;
                    if (Fred::PublicRequest::check_public_request(cid, Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION))
                    {
                        Fred::PublicRequest::cancel_public_request(cid, Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION, request.get_request_id());
                        type = Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION;
                    }
                    else
                    {
                        Fred::PublicRequest::cancel_public_request(cid, Fred::PublicRequest::PRT_MOJEID_CONTACT_REIDENTIFICATION, request.get_request_id());
                        type = Fred::PublicRequest::PRT_MOJEID_CONTACT_REIDENTIFICATION;
                    }
                    {
                        Database::Connection conn = Database::Manager::acquire();
                        check_sent_letters_limit(cid,
                                                 server_conf_->letter_limit_count,
                                                 server_conf_->letter_limit_interval,
                                                 conn);
                    }
                    IdentificationRequestPtr new_request(mailer_, type);
                    new_request->setRegistrarId(request.get_registrar_id());
                    new_request->setRequestId(request.get_request_id());
                    new_request->addObject(Fred::PublicRequest::OID(
                                cid, _contact_username, Fred::PublicRequest::OT_CONTACT));
                    new_request->save();
                    prid = new_request->getId();
                }

                DiscloseFlagPolicy::PolicyCallbackVector pcv
                    = boost::assign::list_of
                        (DiscloseFlagPolicy::PolicyCallback(SetDiscloseAddrTrueIfOrganization()))
                        (DiscloseFlagPolicy::PolicyCallback(SetDiscloseAddrTrueIfNotIdentified()));

                DiscloseFlagPolicy contact_disclose_policy (pcv);

                contact_disclose_policy.apply(_contact);

                unsigned long long hid = Fred::Contact::Verification::contact_update(
                        request.get_request_id(),
                        request.get_registrar_id(),
                        _contact);

                // admin contact verification Ticket #10935
                try {
                    Admin::AdminContactVerificationObjectStates::conditionally_cancel_final_states_legacy(cid);
                } catch (...) {
                    LOGGER(PACKAGE).warning("conditionally_delete_final_states_legacy exception");
                    throw;
                }

                LOGGER(PACKAGE).info(boost::format(
                        "contact updated -- handle: %1%  id: %2%  history_id: %3%")
                        % handle % cid % hid);

                request.end_prepare(_trans_id);

                trans_data tr_data(MOJEID_CONTACT_UPDATE);

                tr_data.cid = cid;
                tr_data.prid = prid;
                tr_data.request_id = request.get_request_id();

                boost::mutex::scoped_lock td_lock(td_mutex);
                transaction_data.insert(std::make_pair(_trans_id, tr_data));
                td_lock.unlock();

                LOGGER(PACKAGE).info("request completed successfully");

            }
            catch(Registry::MojeID::OBJECT_NOT_EXISTS& _ex)
            {
                LOGGER(PACKAGE).info(_ex.what());
                throw;
            }
            catch(Registry::MojeID::MESSAGE_LIMIT_EXCEEDED &_ex)
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
            catch (const Registry::Contact::Verification::DATA_VALIDATION_ERROR& _ex)
            {
                LOGGER(PACKAGE).warning(_ex.what());
                throw;
            }
            catch (const Fred::Contact::Verification::DataValidationError& _ex)
            {
                LOGGER(PACKAGE).warning(_ex.what());
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
                        (Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION)
                        (Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION)
                        (Fred::PublicRequest::PRT_MOJEID_CONTACT_REIDENTIFICATION);
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
                    || tr_data.op == MOJEID_CONTACT_TRANSFER
                    || tr_data.op == MOJEID_CONTACT_UPDATE)
            {
                if (tr_data.prid != 0)
                {
                    sendAuthPasswords(tr_data.cid, tr_data.prid);
                }
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

                Fred::PublicRequest::lock_public_request_by_object(_contact_id);

                Database::Result res = conn.exec_params(
                    "SELECT "
                    " pr.id, "
                    " c.name, c.organization, c.ssn, c.ssntype, "
                    " concat_ws(', ', nullif(btrim(c.street1), ''), nullif(btrim(c.street2), ''), "
                    " nullif(btrim(c.street3), ''), nullif(btrim(c.postalcode), ''), "
                    " nullif(btrim(c.city), ''), c.country), "
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
                const unsigned identType = static_cast< unsigned >(res[0][4]);
                //g->getInput().imbue(std::locale(std::locale(""),new date_facet("%x")));

                std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");

                Util::XmlTagPair("mojeid_valid", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("request_id", Util::XmlUnparsedCData(static_cast<std::string>(res[0][0]))))
                    (Util::XmlTagPair("handle", Util::XmlUnparsedCData(static_cast<std::string>(res[0][6]))))
                    (Util::XmlTagPair("name", Util::XmlUnparsedCData(static_cast<std::string>(res[0][1]))))
                    (Util::XmlTagPair("organization", Util::XmlUnparsedCData(static_cast<std::string>(res[0][2]))))
                    (Util::XmlTagPair("ic", Util::XmlUnparsedCData(identType == 4 ? static_cast<std::string>(res[0][3]) : "")))
                    (Util::XmlTagPair("birth_date", Util::XmlUnparsedCData(identType == 6 ? stringify(birthdate_from_string_to_date(res[0][3])) : "")))
                    (Util::XmlTagPair("address", Util::XmlUnparsedCData(static_cast<std::string>(res[0][5]))))
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
                LOGGER(PACKAGE).warning("contact doesn't exists");
                throw;
            }
            catch (Fred::PublicRequest::RequestExists &_ex)
            {
                LOGGER(PACKAGE).warning(boost::format("cannot create request (%1%)") % _ex.what());
                throw;
            }
            catch (Fred::PublicRequest::NotApplicable &_ex)
            {
                LOGGER(PACKAGE).warning(boost::format("cannot create request (%1%)") % _ex.what());
                throw;
            }
            catch (Fred::Contact::Verification::DataValidationError &_ex)
            {
                LOGGER(PACKAGE).info(_ex.what());
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

        namespace
        {
            std::string db_timestamp_to_iso_extended(const Database::Value &_t)
            {
                using namespace boost::posix_time;
                const std::string db_timestamp = static_cast< std::string >(_t);//2014-12-11 09:28:45.741828
                const ptime posix_timestamp = time_from_string(db_timestamp);
                return to_iso_extended_string(posix_timestamp);
            }

            typedef bool IsNotNull;

            IsNotNull add_state(const Database::Value &_valid_from, const std::string &_state,
                                ContactStateData &_contact)
            {
                if (_valid_from.isnull()) {
                    return false;
                }
                _contact.state[_state] = db_timestamp_to_iso_extended(_valid_from);
                return true;
            }
        }

        ContactStateDataList MojeIDImpl::getContactsStateChanges(unsigned long _last_hours)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("get-contacts-state-changes");
            ConnectionReleaser releaser;

            try
            {
                Database::Connection conn = Database::Manager::acquire();
                ContactStateDataList result;
                Database::Result rcontacts = conn.exec_params(
                "WITH cic AS (SELECT id FROM enum_object_states WHERE name='conditionallyIdentifiedContact'),"
                      "ic AS (SELECT id FROM enum_object_states WHERE name='identifiedContact'),"
                      "vc AS (SELECT id FROM enum_object_states WHERE name='validatedContact'),"
                      "mc AS (SELECT id FROM enum_object_states WHERE name='mojeidContact'),"
                      "lc AS (SELECT id FROM enum_object_states WHERE name='linked'),"
/* observed states */"obs AS (SELECT id FROM enum_object_states WHERE name IN ('conditionallyIdentifiedContact',"
                                            "'identifiedContact','validatedContact','mojeidContact','linked')),"
/* contacts whose */  "cc AS (SELECT DISTINCT c.id "
/* observed states */        "FROM contact c "
/* start or stop in */       "JOIN object_state os ON os.object_id=c.id "
/* the course of $2 hours */ "JOIN obs ON os.state_id=obs.id "
                             "WHERE (NOW()-$2::INTERVAL)<=os.valid_from OR (os.valid_to IS NOT NULL AND "
                                   "(NOW()-$2::INTERVAL)<=os.valid_to)) "
                "SELECT cc.id," // [0] - contact id
/* [1] - cic from */   "(SELECT valid_from FROM object_state JOIN cic ON state_id=cic.id "
                        "WHERE object_id=cc.id AND valid_to IS NULL),"
/* [2] - ic from */    "(SELECT valid_from FROM object_state JOIN ic ON state_id=ic.id "
                        "WHERE object_id=cc.id AND valid_to IS NULL),"
/* [3] - vc from */    "(SELECT valid_from FROM object_state JOIN vc ON state_id=vc.id "
                        "WHERE object_id=cc.id AND valid_to IS NULL),"
/* [4] - mc from */    "(SELECT valid_from FROM object_state JOIN mc ON state_id=mc.id "
                        "WHERE object_id=cc.id AND valid_to IS NULL), "
/* [5] - lc from */    "(SELECT valid_from FROM object_state JOIN lc ON state_id=lc.id "
                        "WHERE object_id=cc.id AND valid_to IS NULL) "
                "FROM cc "
                "JOIN object_state os ON os.object_id=cc.id AND os.valid_to IS NULL "
                "JOIN mc ON mc.id=os.state_id "
                "JOIN object o ON o.id=cc.id "
                "JOIN registrar r ON r.id=o.clid AND r.handle=$1::TEXT",
                Database::query_param_list
                    (server_conf_->registrar_handle)
                    (boost::lexical_cast< std::string >(_last_hours) + " HOUR")); // observe interval <now - last_hours, now)

                for (::size_t idx = 0; idx < rcontacts.size(); ++idx) {
                    ContactStateData contact;
                    contact.contact_id = static_cast< ::size_t >(rcontacts[idx][0]);
                    if (!add_state(rcontacts[idx][1], "conditionallyIdentifiedContact", contact)) {
                        std::ostringstream msg;
                        msg << "contact " << contact.contact_id << " hasn't conditionallyIdentifiedContact state";
                        LOGGER(PACKAGE).error(msg.str());
                        continue;
                    }
                    add_state(rcontacts[idx][2], "identifiedContact", contact);
                    add_state(rcontacts[idx][3], "validatedContact", contact);
                    if (!add_state(rcontacts[idx][4], "mojeidContact", contact)) {
                        std::ostringstream msg;
                        msg << "contact " << contact.contact_id << " hasn't mojeidContact state";
                        throw std::runtime_error(msg.str());
                    }
                    add_state(rcontacts[idx][5], "linked", contact);

                    result.push_back(contact);
                }
                return result;
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
        }//MojeIDImpl::getContactsStateChanges

        ContactStateData MojeIDImpl::getContactState(unsigned long long _contact_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("get-contact-state");
            ConnectionReleaser releaser;

            try {
                Database::Connection conn = Database::Manager::acquire();
                Database::Result rcontact = conn.exec_params(
                    "SELECT r.id IS NULL,os.valid_from," // 0,1
                           "(SELECT valid_from FROM object_state " // 2
                            "WHERE object_id=o.id AND valid_to IS NULL AND "
                                  "state_id=(SELECT id FROM enum_object_states WHERE name='conditionallyIdentifiedContact')),"
                           "(SELECT valid_from FROM object_state " // 3
                            "WHERE object_id=o.id AND valid_to IS NULL AND "
                                  "state_id=(SELECT id FROM enum_object_states WHERE name='identifiedContact')),"
                           "(SELECT valid_from FROM object_state " // 4
                            "WHERE object_id=o.id AND valid_to IS NULL AND "
                                  "state_id=(SELECT id FROM enum_object_states WHERE name='validatedContact')),"
                           "(SELECT valid_from FROM object_state " // 5
                            "WHERE object_id=o.id AND valid_to IS NULL AND "
                                  "state_id=(SELECT id FROM enum_object_states WHERE name='linked')) "
                    "FROM contact c "
                    "JOIN object o ON o.id=c.id "
                    "LEFT JOIN registrar r ON r.id=o.clid AND r.handle=$1::TEXT "
                    "LEFT JOIN object_state os ON os.object_id=o.id AND os.valid_to IS NULL AND "
                                                 "os.state_id=(SELECT id FROM enum_object_states WHERE name='mojeidContact') "
                    "WHERE c.id=$2::BIGINT",
                    Database::query_param_list
                        (server_conf_->registrar_handle)
                        (_contact_id));

                if (rcontact.size() == 0) {
                    throw Registry::MojeID::OBJECT_NOT_EXISTS();
                }

                if (1 < rcontact.size()) {
                    std::ostringstream msg;
                    msg << "contact " << _contact_id << " returns multiple (" << rcontact.size() << ") records";
                    throw std::runtime_error(msg.str());
                }

                if (static_cast< bool >(rcontact[0][0])) { // contact's registrar missing
                    throw Registry::MojeID::OBJECT_NOT_EXISTS();
                }

                ContactStateData contact;
                contact.contact_id = _contact_id;
                if (!add_state(rcontact[0][1], "mojeidContact", contact)) {
                    throw Registry::MojeID::OBJECT_NOT_EXISTS();
                }
                if (!add_state(rcontact[0][2], "conditionallyIdentifiedContact", contact)) {
                    std::ostringstream msg;
                    msg << "contact " << _contact_id << " hasn't conditionallyIdentifiedContact state";
                    throw std::runtime_error(msg.str());
                }
                add_state(rcontact[0][3], "identifiedContact", contact);
                add_state(rcontact[0][4], "validatedContact", contact);
                add_state(rcontact[0][5], "linked", contact);
                return contact;
            }//try
            catch(Registry::MojeID::OBJECT_NOT_EXISTS& _ex)
            {
                LOGGER(PACKAGE).warning(_ex.what());
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
            catch(Registry::MojeID::OBJECT_NOT_EXISTS& _ex)
            {
                LOGGER(PACKAGE).info(_ex.what());
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
        }

        void MojeIDImpl::sendNewPIN3(
            unsigned long long _contact_id,
            unsigned long long _request_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("send-new-pin3");
            ConnectionReleaser releaser;

            LOGGER(PACKAGE).info(boost::format("sendNewPIN3 --"
                    "  contact_id: %1%  request_id: %2%")
                        % _contact_id % _request_id);

            try {
                enum { INVALID_CONTACT_ID = 0 };
                if (_contact_id == INVALID_CONTACT_ID) {
                    throw std::runtime_error("_contact_id is invalid");
                }

                Database::Connection conn = Database::Manager::acquire();
                Database::Transaction tx(conn);

                // check if the contact with ID _contact_id exists
                {
                    Fred::NameIdPair cinfo;
                    DBSharedPtr nodb;
                    Fred::Contact::ManagerPtr contact_mgr(
                        Fred::Contact::Manager::create(nodb, registry_conf_->restricted_handles));

                    Fred::Contact::Manager::CheckAvailType check_result;
                    check_result = contact_mgr->checkAvail(_contact_id, cinfo);

                    if (check_result != Fred::Contact::Manager::CA_REGISTRED ||
                        !this->isMojeidContact(_contact_id)) {
                        /* contact doesn't exists */
                        throw Registry::MojeID::OBJECT_NOT_EXISTS();
                    }
                }

                const std::string contact_handle = get_contact_handle(_contact_id, conn);

                {
                    const Fred::Contact::Verification::State contact_state =
                        Fred::Contact::Verification::get_contact_verification_state(_contact_id);
                    if (contact_state.has_all(Fred::Contact::Verification::State::cIvm)) {
                        // nothing to send if contact is identified
                        // IDENTIFICATION_REQUEST_NOT_EXISTS isn't error in frontend
                        throw Registry::MojeID::IDENTIFICATION_REQUEST_NOT_EXISTS();
                    }
                }

                enum { INVALID_PUBLIC_REQUEST_ID = 0 };
                Fred::PublicRequest::Type type = Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION;
                if (Fred::PublicRequest::check_public_request(_contact_id, type) != INVALID_PUBLIC_REQUEST_ID) {
                    Fred::PublicRequest::cancel_public_request(_contact_id, type, _request_id);
                }
                else {
                    type = Fred::PublicRequest::PRT_MOJEID_CONTACT_REIDENTIFICATION;
                    if (Fred::PublicRequest::check_public_request(_contact_id, type) == INVALID_PUBLIC_REQUEST_ID) {
                        throw Registry::MojeID::IDENTIFICATION_REQUEST_NOT_EXISTS();
                    }
                    Fred::PublicRequest::cancel_public_request(_contact_id, type, _request_id);
                }

                {
                    Database::Connection conn = Database::Manager::acquire();
                    check_sent_letters_limit(_contact_id,
                                             server_conf_->letter_limit_count,
                                             server_conf_->letter_limit_interval,
                                             conn);
                }
                IdentificationRequestPtr new_request(mailer_, type);
                new_request->setRegistrarId(mojeid_registrar_id_);
                new_request->setRequestId(_request_id);
                new_request->addObject(Fred::PublicRequest::OID(
                                _contact_id, contact_handle, Fred::PublicRequest::OT_CONTACT));
                new_request->save();
                new_request->sendPasswords();
                tx.commit();

                LOGGER(PACKAGE).info("request completed successfully");

            }
            catch(const Registry::MojeID::OBJECT_NOT_EXISTS &e)
            {
                LOGGER(PACKAGE).info(e.what());
                throw;
            }
            catch(Registry::MojeID::MESSAGE_LIMIT_EXCEEDED &e)
            {
                LOGGER(PACKAGE).info(e.what());
                throw;
            }
            catch(const Registry::MojeID::IDENTIFICATION_REQUEST_NOT_EXISTS &e)
            {
                LOGGER(PACKAGE).info(e.what());
                throw;
            }
            catch (const std::exception &e)
            {
                LOGGER(PACKAGE).error(e.what());
                throw;
            }
            catch (...)
            {
                LOGGER(PACKAGE).error("unknown exception");
                throw;
            }
        }//MojeIDImpl::sendNewPIN3

        namespace
        {
            void send_mojeid_card_letter(unsigned long long _contact_id,
                                         Database::Connection &_conn)
            {
                Fred::PublicRequest::ContactVerificationPassword::MessageData data;
                Fred::PublicRequest::collect_message_data(_contact_id, _conn, data);

                const std::string country_cs_name = map_at(data, "country_cs_name");
                const std::string addr_country = country_cs_name.empty()
                                                 ? map_at(data, "country_name")
                                                 : country_cs_name;

                std::string letter_xml("<?xml version='1.0' encoding='utf-8'?>");

                const std::string lastname = map_at(data, "lastname");
                static const char female_suffix[] = "á"; // utf-8 encoded
                enum { FEMALE_SUFFIX_LEN = sizeof(female_suffix) - 1 };
                const std::string sex = (FEMALE_SUFFIX_LEN <= lastname.length()) &&
                                        (std::strcmp(lastname.c_str() + lastname.length() - FEMALE_SUFFIX_LEN,
                                                     female_suffix) == 0) ? "female" : "male";

                Util::XmlTagPair("contact_auth", Util::vector_of<Util::XmlCallback>
                    (Util::XmlTagPair("user", Util::vector_of<Util::XmlCallback>
                        (Util::XmlTagPair("actual_date", Util::XmlUnparsedCData(map_at(data, "reqdate"))))
                        (Util::XmlTagPair("name", Util::XmlUnparsedCData(map_at(data, "firstname")+ " " + map_at(data, "lastname"))))
                        (Util::XmlTagPair("organization", Util::XmlUnparsedCData(map_at(data, "organization"))))
                        (Util::XmlTagPair("street", Util::XmlUnparsedCData(map_at(data, "street"))))
                        (Util::XmlTagPair("city", Util::XmlUnparsedCData(map_at(data, "city"))))
                        (Util::XmlTagPair("stateorprovince", Util::XmlUnparsedCData(map_at(data, "stateorprovince"))))
                        (Util::XmlTagPair("postal_code", Util::XmlUnparsedCData(map_at(data, "postalcode"))))
                        (Util::XmlTagPair("country", Util::XmlUnparsedCData(addr_country)))
                        (Util::XmlTagPair("account", Util::vector_of<Util::XmlCallback>
                            (Util::XmlTagPair("username", Util::XmlUnparsedCData(map_at(data, "handle"))))
                            (Util::XmlTagPair("first_name", Util::XmlUnparsedCData(map_at(data, "firstname"))))
                            (Util::XmlTagPair("last_name", Util::XmlUnparsedCData(lastname)))
                            (Util::XmlTagPair("sex", Util::XmlUnparsedCData(sex)))
                            (Util::XmlTagPair("email", Util::XmlUnparsedCData(map_at(data, "email"))))
                            (Util::XmlTagPair("sex", Util::XmlUnparsedCData(sex)))
                            (Util::XmlTagPair("mobile", Util::XmlUnparsedCData(map_at(data, "phone"))))
                        ))
                    ))
                )(letter_xml);

                std::stringstream xmldata;
                xmldata << letter_xml;

                HandleRegistryArgs *const rconf =
                    CfgArgs::instance()->get_handler_ptr_by_type< HandleRegistryArgs >();
                std::auto_ptr< Fred::Document::Manager > doc_manager =
                    Fred::Document::Manager::create(
                        rconf->docgen_path,
                        rconf->docgen_template_path,
                        rconf->fileclient_path,
                        CfgArgs::instance()->get_handler_ptr_by_type< HandleCorbaNameServiceArgs >()
                            ->get_nameservice_host_port());
                enum { FILETYPE_MOJEID_CARD = 10 };
                const unsigned long long file_id = doc_manager->generateDocumentAndSave(
                        Fred::Document::GT_MOJEID_CARD,
                        xmldata,
                        "mojeid_card-" +
                            boost::lexical_cast<std::string>(_contact_id) + "-" +
                            boost::lexical_cast<std::string>(::time(NULL)) + ".pdf",
                        FILETYPE_MOJEID_CARD, "");

                Fred::Messages::PostalAddress pa;
                pa.name    = map_at(data, "firstname") + " " + map_at(data, "lastname");
                pa.org     = map_at(data, "organization");
                pa.street1 = map_at(data, "street");
                pa.street2 = std::string("");
                pa.street3 = std::string("");
                pa.city    = map_at(data, "city");
                pa.state   = map_at(data, "stateorprovince");
                pa.code    = map_at(data, "postalcode");
                pa.country = map_at(data, "country_name");

                DBSharedPtr nodb;
                std::auto_ptr< Fred::Manager > registry_manager;
                registry_manager.reset(
                    Fred::Manager::create(
                        nodb,
                        rconf->restricted_handles));
                const unsigned long long message_id = registry_manager->getMessageManager()
                    ->save_letter_to_send(
                        map_at(data, "handle").c_str(),//contact handle
                        pa,
                        file_id,
                        "mojeid_emergency_card",
                        boost::lexical_cast<unsigned long >(map_at(data,
                            "contact_id")),//contact object_registry.id
                        boost::lexical_cast<unsigned long >(map_at(data,
                            "contact_hid")),//contact_history.historyid
                        "letter"
                    );
            }
        }

        //send mojeID (emergency) card
        void MojeIDImpl::sendMojeIDCard(
            unsigned long long _contact_id,
            unsigned long long _request_id)
        {
            Logging::Context ctx_server(create_ctx_name(get_server_name()));
            Logging::Context ctx("send-mojeid-card");
            ConnectionReleaser releaser;

            LOGGER(PACKAGE).info(boost::format("sendMojeIDCard --"
                    "  contact_id: %1%  request_id: %2%")
                        % _contact_id % _request_id);

            try {
                enum { INVALID_CONTACT_ID = 0 };
                if (_contact_id == INVALID_CONTACT_ID) {
                    throw std::runtime_error("_contact_id is invalid");
                }

                Database::Connection conn = Database::Manager::acquire();
                Database::Transaction tx(conn);

                // check if the contact with ID _contact_id exists
                {
                    Fred::NameIdPair cinfo;
                    DBSharedPtr nodb;
                    Fred::Contact::ManagerPtr contact_mgr(
                        Fred::Contact::Manager::create(nodb, registry_conf_->restricted_handles));

                    Fred::Contact::Manager::CheckAvailType check_result;
                    check_result = contact_mgr->checkAvail(_contact_id, cinfo);

                    if (check_result != Fred::Contact::Manager::CA_REGISTRED ||
                        !this->isMojeidContact(_contact_id)) {
                        /* contact doesn't exists */
                        throw Registry::MojeID::OBJECT_NOT_EXISTS();
                    }
                }

                const std::string contact_handle = get_contact_handle(_contact_id, conn);

                {
                    const Fred::Contact::Verification::State contact_state =
                        Fred::Contact::Verification::get_contact_verification_state(_contact_id);
                    if (!contact_state.has_all(Fred::Contact::Verification::State::cIvm)) {
                        Fred::Contact::Verification::FieldErrorMap errors;
                        errors["status"] = Fred::Contact::Verification::INVALID;
                        throw Fred::Contact::Verification::DataValidationError(errors);
                    }
                }

                send_mojeid_card_letter(_contact_id, conn);
                tx.commit();

                LOGGER(PACKAGE).info("request completed successfully");

            }
            catch(const Registry::MojeID::OBJECT_NOT_EXISTS &e)
            {
                LOGGER(PACKAGE).info(e.what());
                throw;
            }
            catch(const Registry::MojeID::IDENTIFICATION_REQUEST_NOT_EXISTS &e)
            {
                LOGGER(PACKAGE).info(e.what());
                throw;
            }
            catch (Fred::Contact::Verification::DataValidationError &e)
            {
                LOGGER(PACKAGE).info(e.what());
                throw;
            }
            catch (const std::exception &e)
            {
                LOGGER(PACKAGE).error(e.what());
                throw;
            }
            catch (...)
            {
                LOGGER(PACKAGE).error("unknown exception");
                throw;
            }
        }//MojeIDImpl::sendMojeIDCard

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

                if (!isMojeidContact(_contact_id)) {
                    throw std::runtime_error(
                            "Contact is not registered with MojeID");
                }

                //lock object states
                Fred::lock_object_state_request_lock(_contact_id);

                if (!Fred::cancel_object_state(
                    _contact_id, ::MojeID::ObjectState::MOJEID_CONTACT)) {
                    throw std::runtime_error(
                            "Fred::cancel_object_state mojeidContact failed");
                }

                if (!Fred::cancel_object_state(
                    _contact_id, Fred::ObjectState::SERVER_DELETE_PROHIBITED)) {
                    throw std::runtime_error(
                        "Fred::cancel_object_state serverDeleteProhibited failed");
                }

                if (!Fred::cancel_object_state(
                    _contact_id, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED)) {
                    throw std::runtime_error(
                        "Fred::cancel_object_state serverTransferProhibited failed");
                }

                if (!Fred::cancel_object_state(
                    _contact_id, Fred::ObjectState::SERVER_UPDATE_PROHIBITED)) {
                    throw std::runtime_error(
                        "Fred::cancel_object_state serverUpdateProhibited failed");
                }

                if (Fred::object_has_state(_contact_id, Fred::ObjectState::VALIDATED_CONTACT)) {
                    //cancel validated
                    if (!Fred::cancel_object_state(_contact_id
                            , Fred::ObjectState::VALIDATED_CONTACT)) {
                        throw std::runtime_error(
                            "Fred::cancel_object_state validatedContact failed");
                    }
                }

                //lock public requests
                Fred::PublicRequest::lock_public_request_by_object(_contact_id);

                conn.exec_params(
                    "UPDATE public_request pr "
                    "SET status=(SELECT id FROM enum_public_request_status WHERE name='invalidated'),"
                        "resolve_time=CURRENT_TIMESTAMP "
                    "FROM public_request_objects_map prom "
                    "WHERE prom.object_id=$1::integer AND "
                          "pr.id=prom.request_id AND "
                          "pr.resolve_time IS NULL AND "
                          "pr.status=(SELECT id FROM enum_public_request_status WHERE name='new') AND "
                          "pr.request_type IN "
                              "(SELECT id FROM enum_public_request_type "
                               "WHERE name IN ($2::text,$3::text,$4::text,$5::text))",
                    Database::query_param_list(_contact_id)
                        (Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION)
                        (Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION)
                        (Fred::PublicRequest::PRT_MOJEID_CONTACT_REIDENTIFICATION)
                        (Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION));

                Fred::Contact::Verification::Contact contact = Fred::Contact::Verification::contact_info(_contact_id);
                contact.addresses.clear();
                Fred::Contact::Verification::contact_update(_request_id, this->mojeid_registrar_id_, contact);
                Fred::Contact::Verification::contact_delete_not_linked(_contact_id);
                tx.prepare(_trans_id);

                trans_data tr_data(MOJEID_CONTACT_CANCEL);
                tr_data.cid = _contact_id;

                boost::mutex::scoped_lock td_lock(td_mutex);
                transaction_data.insert(std::make_pair(_trans_id, tr_data));

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

            if (contact_result.size() == 0) {
                throw std::runtime_error(
                        "MojeIDImpl::isMojeidContact: contact doesn't exist");
            }

            if (std::string(contact_result[0]["mojeidContact"]) == "f") {
                //not mojeidContact - ok
                return false;
            }

            if (std::string(contact_result[0]["is_mojeid_registrar"]) == "f") {
                //contact is in state mojeidContact
                // and do not have mojeid registrar - nok
                throw std::runtime_error(
                    "MojeIDImpl::isMojeidContact: contact is in state "
                    " mojeidContact and don't have mojeid registrar");
            }

            if ((std::string(contact_result[0]["conditionallyIdentifiedContact"]) == "t") ||
                (std::string(contact_result[0]["identifiedContact"]) == "t") ||
                (std::string(contact_result[0]["validatedContact"]) == "t")) {
                return true;//ok
            }
            //contact lack identification state - nok
            throw std::runtime_error(
                "MojeIDImpl::isMojeidContact: missing identification state");
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

            if (contact_result.size() == 0) {
                throw std::runtime_error(
                    "MojeIDImpl::isMojeidContact: "
                    " contact handle doesn't exist");
            }
            const unsigned long long contact_id(contact_result[0][0]);
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
                    LOGGER(PACKAGE).error("unable to create identfication request - wrong type");
                }
                else {
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
