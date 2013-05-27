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
 *  @server_i.cc
 *  implementational code for mojeid IDL interface
 *  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace/enum/idl/idl/MojeID.idl
 */


#include "server_i.h"
#include "corba_wrapper_decl.h"
#include "corba_conversion.h"
#include "mojeid/mojeid.h"
#include "mailer_manager.h"
#include <MojeID.hh>
#include <string>


namespace Registry
{
    namespace MojeID
    {

        ContactHandleList_i::ContactHandleList_i(
                const std::vector<std::string> &_handles)
            : status_(ACTIVE),
              last_used_(boost::posix_time::second_clock::local_time()),
              handles_(_handles),
              it_(handles_.begin())
        {
        }

        ContactHandleList_i::~ContactHandleList_i()
        {
        }

        Registry::MojeID::ContactHandleSeq* ContactHandleList_i::getNext(::CORBA::ULong count)
        {
            try
            {
                if (status_ != ACTIVE) {
                    throw Registry::MojeID::ContactHandleList::NOT_ACTIVE();
                }

                last_used_ = boost::posix_time::second_clock::local_time();

                Registry::MojeID::ContactHandleSeq_var ret = new Registry::MojeID::ContactHandleSeq;
                ret->length(0);

                for (unsigned long i = 0; it_ != handles_.end() && i < count; ++it_, ++i)
                {
                    unsigned int act_size = ret->length();
                    ret->length(act_size +1);
                    ret[act_size] = corba_wrap_string(*it_);
                }

                return ret._retn();
            }
            catch (Registry::MojeID::ContactHandleList::NOT_ACTIVE)
            {
                throw;
            }
            catch (...)
            {
                throw Registry::MojeID::ContactHandleList::INTERNAL_SERVER_ERROR();
            }
        }


        void ContactHandleList_i::destroy()
        {
            this->close();
        }


        void ContactHandleList_i::close()
        {
            status_ = CLOSED;
        }


        const boost::posix_time::ptime& ContactHandleList_i::get_last_used() const
        {
            return last_used_;
        }


        const ContactHandleList_i::Status& ContactHandleList_i::get_status() const
        {
            return status_;
        }




        Server_i::Server_i(const std::string &_server_name)
        : pimpl_(new MojeIDImpl(_server_name
                , boost::shared_ptr<Fred::Mailer::Manager>(
                    new MailerManager(CorbaContainer::get_instance()
                    ->getNS())))),
          contact_handle_list_scavenger_active_(true),
          contact_handle_list_scavenger_(boost::bind(&Server_i::clean_contact_handle_list_objects, this))
        {}

        Server_i::~Server_i()
        {
            contact_handle_list_scavenger_active_ = false;
            contact_handle_list_scavenger_.join();
        }


        void Server_i::clean_contact_handle_list_objects()
        {
            while (contact_handle_list_scavenger_active_)
            {
                LOGGER(PACKAGE).debug("fred-mifd contact handle list scavenger worker iteration");
                {
                    boost::lock_guard<boost::mutex> lock(contact_handle_list_objects_mutex_);

                    std::vector<ContactHandleListPtr>::iterator it = contact_handle_list_objects_.begin();
                    while(it != contact_handle_list_objects_.end())
                    {
                        if ((boost::posix_time::second_clock::local_time() - (*it)->get_last_used()) > seconds(300))
                        {
                            (*it)->close();
                        }

                        if ((*it)->get_status() == ContactHandleList_i::CLOSED)
                        {
                            LOGGER(PACKAGE).debug(boost::format("destroying contact handle list search from %1%") % (*it)->get_last_used());
                            PortableServer::ObjectId_var id = CorbaContainer::get_instance()->root_poa->servant_to_id(it->get());
                            CorbaContainer::get_instance()->root_poa->deactivate_object(id);
                            it = contact_handle_list_objects_.erase(it);
                        }
                        else
                        {
                            it++;
                        }
                    }
                }
                boost::this_thread::sleep(boost::posix_time::seconds(30));
            }
        }


        //   Methods corresponding to IDL attributes and operations
        ::CORBA::ULongLong Server_i::contactCreatePrepare(
            const Registry::MojeID::Contact& _contact
            , const char* _trans_id
            , ::CORBA::ULongLong _request_id
            , ::CORBA::String_out _identification)
        {
            try
            {
                std::string handle(_contact.username);
                Fred::Contact::Verification::Contact verification_contact
                    = corba_unwrap_contact(_contact);
                std::string identification;

                // return new contact id
                unsigned long long cid = pimpl_->contactCreatePrepare(
                    handle
                    , verification_contact
                    , _trans_id
                    , _request_id
                    , identification);

                _identification = corba_wrap_string(identification.c_str());
                return cid;

            }//try
            catch (Fred::Contact::Verification::DataValidationError &_ex)
            {
                throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                    corba_wrap_validation_error_list(_ex.errors));
            }
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//contactCreatePrepare

        ::CORBA::ULongLong Server_i::contactTransferPrepare(
                const char* _handle
                , const char* _trans_id
                , ::CORBA::ULongLong _request_id
                , ::CORBA::String_out _identification)
        {
            try
            {
                std::string identification;

                unsigned long long cid = pimpl_->contactTransferPrepare(
                    _handle
                    , _trans_id
                    , _request_id
                    , identification);

                _identification = corba_wrap_string(identification.c_str());

                return cid;
            }
            catch (Registry::MojeID::OBJECT_NOT_EXISTS &)
            {
                throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
            }
            catch (Fred::PublicRequest::NotApplicable &_ex)
            {
                Fred::Contact::Verification::FieldErrorMap errors;
                errors[Fred::Contact::Verification::field_status]
                       = Fred::Contact::Verification::NOT_AVAILABLE;
                throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                        corba_wrap_validation_error_list(errors));
            }
            catch (Fred::Contact::Verification::DataValidationError &_ex)
            {
                throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                        corba_wrap_validation_error_list(_ex.errors));
            }
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//contactTransferPrepare

        void Server_i::contactUpdatePrepare(
                const Registry::MojeID::Contact& _contact
                , const char* _trans_id
                , ::CORBA::ULongLong _request_id)
        {
            try
            {
                std::string handle(_contact.username);
                Fred::Contact::Verification::Contact contact
                    = corba_unwrap_contact(_contact);

                pimpl_->contactUpdatePrepare(handle
                        , contact
                        , _trans_id
                        , _request_id);
            }
            catch (Registry::MojeID::OBJECT_NOT_EXISTS&)
            {
                throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
            }
            catch (Fred::Contact::Verification::DataValidationError &_ex)
            {
                throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                        corba_wrap_validation_error_list(_ex.errors));
            }
            catch (std::exception &_ex) {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//contactUpdatePrepare

        Registry::MojeID::Contact* Server_i::contactInfo(
                ::CORBA::ULongLong contact_id)
        {
            try
            {
                Contact *data = corba_wrap_contact(
                        pimpl_->contactInfo(contact_id));
                return data;
            }
            catch (Registry::MojeID::OBJECT_NOT_EXISTS&)
            {
                throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//contactInfo

        ::CORBA::ULongLong Server_i::processIdentification(
                const char* ident_request_id
                , const char* password
                , ::CORBA::ULongLong request_id)
        {
            try
            {
                return pimpl_->processIdentification(
                        ident_request_id
                        , password
                        , request_id);
            }
            catch (Fred::NOT_FOUND&) {
                throw Registry::MojeID::Server::IDENTIFICATION_FAILED();
            }
            catch (Fred::PublicRequest::PublicRequestAuth::NotAuthenticated&) {
                throw Registry::MojeID::Server::IDENTIFICATION_FAILED();
            }
            catch (Fred::Contact::Verification::DataValidationError &_ex) {
                throw Registry::MojeID::Server::DATA_VALIDATION_ERROR(
                        corba_wrap_validation_error_list(_ex.errors));
            }
            catch (Fred::PublicRequest::AlreadyProcessed &_ex) {
                throw Registry::MojeID::Server
                    ::IDENTIFICATION_ALREADY_PROCESSED(_ex.success);
            }
            catch (Fred::PublicRequest::ObjectChanged &) {
                throw Registry::MojeID::Server::OBJECT_CHANGED();
            }
            catch (std::exception &_ex) {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...) {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//processIdentification

        char* Server_i::getIdentificationInfo(
                ::CORBA::ULongLong contact_id)
        {
            try
            {
                return corba_wrap_string(pimpl_
                        ->getIdentificationInfo(contact_id).c_str());
            }
            catch (Fred::NOT_FOUND &_ex) {
                throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
            }
            catch (std::exception &_ex) {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...) {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//getIdentificationInfo

        void Server_i::commitPreparedTransaction(
                const char* _trans_id)
        {
            try
            {
                pimpl_->commitPreparedTransaction(_trans_id);
            }
            catch (std::exception &_ex) {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...) {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//commitPreparedTransaction

        void Server_i::rollbackPreparedTransaction(
                const char* _trans_id)
        {
            try
            {
                pimpl_->rollbackPreparedTransaction(_trans_id);
            }
            catch (std::exception &_ex) {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...) {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//rollbackPreparedTransaction

        Registry::MojeID::Buffer* Server_i::getValidationPdf(
                ::CORBA::ULongLong _contact_id)
        {
            try
            {
                std::stringstream outstr;
                    pimpl_->getValidationPdf(_contact_id, outstr);
                unsigned long size = outstr.str().size();
                CORBA::Octet *b = Buffer::allocbuf(size);
                memcpy(b,outstr.str().c_str(),size);
                Buffer_var ret = new Buffer(size, size, b, 1);
                return ret._retn();
            }
            catch (Registry::MojeID::OBJECT_NOT_EXISTS&)
            {
                throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//getValidationPdf

        void Server_i::createValidationRequest(
                ::CORBA::ULongLong _contact_id
                 , ::CORBA::ULongLong _request_id)
        {
            try
            {
                pimpl_->createValidationRequest(
                        _contact_id, _request_id);
            }
            catch (Registry::MojeID::OBJECT_NOT_EXISTS&)
            {
                throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
            }
            catch (Fred::PublicRequest::RequestExists&)
            {
                throw Registry::MojeID::Server::OBJECT_EXISTS();
            }
            catch (Fred::PublicRequest::NotApplicable&)
            {
                throw Registry::MojeID::Server::VALIDATION_ALREADY_PROCESSED();
            }
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//createValidationRequest

        Registry::MojeID::ContactStateInfoList* Server_i::getContactsStates(
                ::CORBA::ULong _last_hours)
        {
            try
            {
                std::vector<ContactStateData> csdv = pimpl_->getContactsStates(_last_hours);
                ContactStateInfoList_var ret = new ContactStateInfoList;
                ret->length(0);

                for (std::vector<ContactStateData>::const_iterator csdvi
                    = csdv.begin(); csdvi != csdv.end(); ++csdvi)
                {
                    Registry::MojeID::ContactStateInfo sinfo;
                    sinfo.contact_id = csdvi->contact_id;
                    sinfo.valid_from = corba_wrap_date(csdvi->valid_from);
                    std::string state_name = csdvi->state_name;
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
                }//for

                return ret._retn();

            }//try
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//getContactsStates

        Registry::MojeID::ContactStateInfo Server_i::getContactState(
                ::CORBA::ULongLong _contact_id)
        {
            try
            {
                ContactStateData csd = pimpl_->getContactState(_contact_id);
                Registry::MojeID::ContactStateInfo_var sinfo;
                sinfo->contact_id = csd.contact_id;
                sinfo->valid_from = corba_wrap_date(csd.valid_from);
                std::string state_name = csd.state_name;

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
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//getContactState

        ::CORBA::ULongLong Server_i::getContactId(
                const char* _handle)
        {
            try
            {
                return pimpl_->getContactId(_handle);
            }
            catch (Registry::MojeID::OBJECT_NOT_EXISTS &_ex)
            {
                throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//getContactId

        void Server_i::contactCancelAccountPrepare(
                ::CORBA::ULongLong _contact_id
                 , const char* _trans_id
                 , ::CORBA::ULongLong _request_id)
        {
            try
            {
                pimpl_->contactCancelAccountPrepare(_contact_id
                        , _trans_id, _request_id);
            }
            catch (Registry::MojeID::OBJECT_NOT_EXISTS&)
            {
                throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//contactCancelAccountPrepare

        Registry::MojeID::ContactHandleList_ptr
        Server_i::getUnregistrableHandles()
        {
            try
            {
                std::vector<std::string> result;
                result = pimpl_->getUnregistrableHandles();

                boost::shared_ptr<ContactHandleList_i> ret(new ContactHandleList_i(result));
                boost::lock_guard<boost::mutex> lock(contact_handle_list_objects_mutex_);
                contact_handle_list_objects_.push_back(ret);
                return ret.get()->_this();
            }
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }//getUnregistrableHandles


        char* Server_i::contactAuthInfo(::CORBA::ULongLong contact_id)
        {
            try
            {
                return corba_wrap_string(pimpl_
                    ->contactAuthInfo(contact_id).c_str());
            }
            catch (Registry::MojeID::OBJECT_NOT_EXISTS&)
            {
                throw Registry::MojeID::Server::OBJECT_NOT_EXISTS();
            }
            catch (std::exception &_ex)
            {
                throw Registry::MojeID::Server
                    ::INTERNAL_SERVER_ERROR(_ex.what());
            }
            catch (...)
            {
                throw Registry::MojeID::Server::INTERNAL_SERVER_ERROR();
            }
        }

    }//namespace MojeID
}//namespace Registry
