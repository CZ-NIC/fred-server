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

            struct IDENTIFICATION_FAILED : public std::runtime_error
            {
                IDENTIFICATION_FAILED() : std::runtime_error("identification failed")
                {}
            };

            struct IDENTIFICATION_PROCESSED : public std::runtime_error
            {
                IDENTIFICATION_PROCESSED() : std::runtime_error("identification procesed")
                {}
            };

            struct IDENTIFICATION_INVALIDATED : public std::runtime_error
            {
                IDENTIFICATION_INVALIDATED() : std::runtime_error("identification invalidated")
                {}
            };

            struct OBJECT_CHANGED : public std::runtime_error
            {
                OBJECT_CHANGED() : std::runtime_error("object changed")
                {}
            };

            struct OBJECT_NOT_EXISTS : public std::runtime_error
            {
                OBJECT_NOT_EXISTS() : std::runtime_error("object does not exist")
                {}
            };

            struct REGISTRAR_NOT_EXISTS : public std::runtime_error
            {
                REGISTRAR_NOT_EXISTS() : std::runtime_error("registrar does not exist")
                {}
            };

            class ContactVerificationImpl
            {
                const HandleRegistryArgs *registry_conf_;
                const std::string server_name_;

                boost::shared_ptr<Fred::Mailer::Manager> mailer_;

            public:
                ContactVerificationImpl(const std::string &_server_name
                        , boost::shared_ptr<Fred::Mailer::Manager> _mailer);
                virtual ~ContactVerificationImpl();

                const std::string& get_server_name();

                unsigned long long createConditionalIdentification(
                        const std::string & contact_handle
                        , const std::string & registrar_handle
                        , const unsigned long long log_id
                        , std::string & request_id);

                unsigned long long processConditionalIdentification(
                        const std::string & request_id
                        , const std::string & password
                        , const unsigned long long log_id);

                unsigned long long processIdentification(
                        const std::string & contact_handle
                        , const std::string & password
                        , const unsigned long long log_id);

                std::string getRegistrarName(const std::string & registrar_handle);

            };//class ContactVerificationImpl
        }
    }
}


#endif //CONTACT_VERIFICATION_IMPL_H__

