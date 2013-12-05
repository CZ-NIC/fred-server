#ifndef CONTACT_IDENTIFICATION_H_
#define CONTACT_IDENTIFICATION_H_

#include "cfg/config_handler_decl.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_contactverification_args.h"
#include "src/fredlib/registry.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/public_request/public_request.h"

namespace Registry {
namespace Contact {


/*
 * helper class - public request manager auto pointer
 */
class ContactIdentificationRequestManagerPtr
{
private:
    DBSharedPtr nodb;
    mutable boost::shared_ptr<Fred::Mailer::Manager> mailer_manager_;
    mutable std::auto_ptr<Fred::Manager> registry_manager_;
    mutable std::auto_ptr<Fred::Document::Manager> doc_manager_;
    mutable std::auto_ptr<Fred::PublicRequest::Manager> request_manager_;


public:
    ContactIdentificationRequestManagerPtr(boost::shared_ptr<Fred::Mailer::Manager> _mailer_manager)
    : mailer_manager_(_mailer_manager)
    {
        /* get config temporary pointer */
        HandleRegistryArgs *rconf =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
        HandleContactVerificationArgs *cvconf =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleContactVerificationArgs>();

        /* construct managers */
        registry_manager_.reset(Fred::Manager::create(
                    nodb,
                    rconf->restricted_handles));
        doc_manager_ = Fred::Document::Manager::create(
                    rconf->docgen_path,
                    rconf->docgen_template_path,
                    rconf->fileclient_path,
                    //doc_manager config dependence
                    CfgArgs::instance()->get_handler_ptr_by_type<
                        HandleCorbaNameServiceArgs>()
                            ->get_nameservice_host_port());
        request_manager_.reset(Fred::PublicRequest::Manager::create(
                    registry_manager_->getDomainManager(),
                    registry_manager_->getContactManager(),
                    registry_manager_->getNSSetManager(),
                    registry_manager_->getKeySetManager(),
                    mailer_manager_.get(),
                    doc_manager_.get(),
                    registry_manager_->getMessageManager()));

        request_manager_->setIdentificationMailAuthHostname(cvconf->hostname);
        request_manager_->setDemoMode(cvconf->demo_mode);
    }

    ContactIdentificationRequestManagerPtr(const ContactIdentificationRequestManagerPtr &src) :
            mailer_manager_(src.mailer_manager_),
            registry_manager_(src.registry_manager_),
            doc_manager_(src.doc_manager_),
            request_manager_(src.request_manager_)
    {  }


    Fred::PublicRequest::Manager* operator ->()
    {
        return request_manager_.get();
    }

    Fred::Document::Manager* getDocumentManager()
    {
        return doc_manager_.get();
    }

};



/*
 * helper class - auth. public request auto pointer
 */
class ContactIdentificationRequestPtr
{
private:
    ContactIdentificationRequestManagerPtr request_manager_;
    Fred::PublicRequest::Type type_;
    mutable std::auto_ptr<Fred::PublicRequest::PublicRequestAuth> request_;


public:
    ContactIdentificationRequestPtr(boost::shared_ptr<Fred::Mailer::Manager> _mailer
            , const Fred::PublicRequest::Type &_type)
        : request_manager_(_mailer)
        , type_(_type)
    {
        /* check valid type for contact identification */
        if ((type_ != Fred::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION )
            && (type_ != Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION)) {
            throw std::runtime_error("not valid identification request type");
        }


        /* create request and cast it to authenticated */
        std::auto_ptr<Fred::PublicRequest::PublicRequest> tmp(request_manager_->createRequest(type_));
        request_.reset(dynamic_cast<Fred::PublicRequest::PublicRequestAuth*>(tmp.release()));
        if (!request_.get()) {
            throw std::runtime_error("unable to create identfication request");
        }
    }


    ContactIdentificationRequestPtr(const ContactIdentificationRequestPtr &src) :
        request_manager_(src.request_manager_),
        type_(src.type_),
        request_(src.request_)
    { }

    Fred::PublicRequest::PublicRequestAuth* operator->()
    {
        return request_.get();
    }
};


}
}



#endif //CONTACT_IDENTIFICATION_H_

