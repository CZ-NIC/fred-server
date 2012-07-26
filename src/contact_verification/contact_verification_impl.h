#ifndef CONTACT_VERIFICATION_IMPL_H__
#define CONTACT_VERIFICATION_IMPL_H__


#include "fredlib/contact_verification/contact.h"
#include "fredlib/contact_verification/contact_verification.h"

#include <string>
#include <vector>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/thread/mutex.hpp>

#include "cfg/handle_registry_args.h"

#include "fredlib/mailer.h"


namespace Registry
{
    namespace Contact
    {
        namespace Verification
        {

            enum contact_verification_operation_type {
                CONTACT_VERIFICATION_NOP = 0,
                CONTACT_VERIFICATION_CONTACT_CREATE = 1,
                CONTACT_VERIFICATION_CONTACT_UPDATE,
                CONTACT_VERIFICATION_CONTACT_UNIDENTIFY,
                CONTACT_VERIFICATION_CONTACT_TRANSFER,
                CONTACT_VERIFICATION_CONTACT_CANCEL
            };

            struct trans_data {
            explicit trans_data(const contact_verification_operation_type &operation) : op(operation), cid(0), prid(0), eppaction_id(0), request_id(0)
            { }

            contact_verification_operation_type op;
            unsigned long long cid;
            unsigned long long prid;
            unsigned long long eppaction_id;
            unsigned long long request_id;
        };


            class ContactVerificationImpl
            {
                const HandleRegistryArgs *registry_conf_;
                const std::string server_name_;

                typedef std::map<std::string, trans_data> transaction_data_map_type;
                transaction_data_map_type transaction_data;
                boost::mutex td_mutex; /// for transaction data
                boost::shared_ptr<Fred::Mailer::Manager> mailer_;

            public:
                ContactVerificationImpl(const std::string &_server_name
                        , boost::shared_ptr<Fred::Mailer::Manager> _mailer);
                virtual ~ContactVerificationImpl();

                const std::string& get_server_name();

            };//class ContactVerificationImpl
        }
    }
}


#endif //CONTACT_VERIFICATION_IMPL_H__

