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
 *  @mojeid.h
 *  header of mojeid implementation
 */

#ifndef MOJEID_H_
#define MOJEID_H_

#include "src/fredlib/contact_verification/contact.h"
#include "src/fredlib/contact_verification/contact_verification_validators.h"

#include <string>
#include <vector>
#include <boost/thread/mutex.hpp>

#include "cfg/handle_registry_args.h"
#include "cfg/handle_mojeid_args.h"
#include "src/fredlib/mailer.h"
#include "src/mojeid/request.h"
#include "src/mojeid/mojeid_identification.h"


namespace Registry
{
    namespace MojeID
    {

        enum mojeid_operation_type {
            MOJEID_NOP = 0,
            MOJEID_CONTACT_CREATE = 1,
            MOJEID_CONTACT_UPDATE,
            MOJEID_CONTACT_UNIDENTIFY,
            MOJEID_CONTACT_TRANSFER,
            MOJEID_CONTACT_CANCEL
        };

        struct trans_data {

            explicit trans_data(const mojeid_operation_type &operation) : op(operation), cid(0), prid(0), eppaction_id(0), request_id(0)
            { }

            mojeid_operation_type op;
            unsigned long long cid;
            unsigned long long prid;
            unsigned long long eppaction_id;
            unsigned long long request_id;
        };

        struct OBJECT_NOT_EXISTS : public std::runtime_error
        {
            OBJECT_NOT_EXISTS() : std::runtime_error("object does not exist")
            {}
        };

        struct IDENTIFICATION_REQUEST_NOT_EXISTS : public std::runtime_error
        {
            IDENTIFICATION_REQUEST_NOT_EXISTS() : std::runtime_error("identification request does not exist")
            {}
        };

        struct ContactStateData
        {
            unsigned long long contact_id;
            typedef std::string DateTime;// iso (extended) format: YYYY-MM-DDTHH:MM:SS,fffffffff
            typedef std::map< std::string, DateTime > StateValidFrom;
            StateValidFrom state;

            ContactStateData(): contact_id(0) { }
            StateValidFrom::const_iterator get_sum_state() const;
        };

        typedef std::vector< ContactStateData > ContactStateDataList;

        class MojeIDImpl
        {
            const HandleRegistryArgs *registry_conf_;
            const HandleMojeIDArgs *server_conf_;
            const std::string server_name_;
            unsigned long long mojeid_registrar_id_;

            typedef std::map<std::string, trans_data> transaction_data_map_type;
            transaction_data_map_type transaction_data;
            boost::mutex td_mutex; /// for transaction data
            boost::shared_ptr<Fred::Mailer::Manager> mailer_;


        public:
            MojeIDImpl(const std::string &_server_name
                    , boost::shared_ptr<Fred::Mailer::Manager> _mailer);
            virtual ~MojeIDImpl();

            const std::string& get_server_name();

            unsigned long long contactCreatePrepare(
                const std::string & _contact_username
                , Fred::Contact::Verification::Contact &_contact
                , const char* _trans_id
                , const unsigned long long _request_id
                , std::string & _identification);

            unsigned long long contactTransferPrepare(
                const char* _handle
                , const char* _trans_id
                , unsigned long long _request_id
                , std::string& _identification);

            void contactUpdatePrepare(
                const std::string & _contact_username
                , Fred::Contact::Verification::Contact& _contact
                , const char* _trans_id
                , unsigned long long _request_id);

            Fred::Contact::Verification::Contact contactInfo(
                unsigned long long contact_id);

            unsigned long long processIdentification(
                const char* ident_request_id
                , const char* password
                , unsigned long long request_id);

            std::string getIdentificationInfo(
                unsigned long long _contact_id);

            void commitPreparedTransaction(
                const char* _trans_id);

            void rollbackPreparedTransaction(
                const char* _trans_id);

            void getValidationPdf(
                unsigned long long _contact_id
                , std::stringstream& outstr);

            void createValidationRequest(
                unsigned long long  _contact_id
                , unsigned long long  _request_id);

            ContactStateDataList getContactsStateChanges(unsigned long _last_hours);

            ContactStateData getContactState(unsigned long long _contact_id);

            unsigned long long getContactId(const std::string& _handle);

            std::vector<std::string> getUnregistrableHandles();
            std::string contactAuthInfo(const unsigned long long _contact_id);

            void resendPIN3(
                unsigned long long _contact_id,
                unsigned long long _request_id);

            void contactCancelAccountPrepare(
                unsigned long long _contact_id
                 , const char* _trans_id
                 , unsigned long long _request_id);

            bool isMojeidContact(unsigned long long _contact_id);
            bool isMojeidContact(const std::string &_contact_handle);
private:
            void sendAuthPasswords(unsigned long long cid, unsigned long long prid);

        };//class MojeIDImpl


        void updateObjectStates(unsigned long long cid) throw();


    }//namespace MojeID
}//namespace Registry

#endif // MOJEID_H_
