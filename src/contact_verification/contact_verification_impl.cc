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
                        (Fred::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION)
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


        }
    }
}

