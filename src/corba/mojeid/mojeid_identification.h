#ifndef MOJEID_IDENTIFICATION_H_
#define MOJEID_IDENTIFICATION_H_

#include "cfg/config_handler_decl.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_mojeid_args.h"

#include "corba_wrapper_decl.h"
#include "register/register.h"
#include "register/public_request.h"
#include "corba/mailer_manager.h"


namespace Registry {
namespace MojeID {


/*
 * helper class - public request manager auto pointer
 */
class IdentificationRequestManagerPtr
{
private:
    std::auto_ptr<MailerManager> mailer_manager_;
    std::auto_ptr<Register::Manager> register_manager_;
    std::auto_ptr<Register::Document::Manager> doc_manager_;
    std::auto_ptr<Register::PublicRequest::Manager> request_manager_;


public:
    IdentificationRequestManagerPtr()
    {
        /* get config temporary pointer */
        HandleRegistryArgs *rconf =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
        HandleMojeIDArgs *mconf =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIDArgs>();

        /* construct managers */
        mailer_manager_.reset(new MailerManager(CorbaContainer::get_instance()->getNS())),
        register_manager_.reset(Register::Manager::create(
                    0,
                    rconf->restricted_handles));
        doc_manager_.reset(Register::Document::Manager::create(
                    rconf->docgen_path,
                    rconf->docgen_template_path,
                    rconf->fileclient_path,
                    CorbaContainer::get_instance()->getNS()->getHostName()));
        request_manager_.reset(Register::PublicRequest::Manager::create(
                    register_manager_->getDomainManager(),
                    register_manager_->getContactManager(),
                    register_manager_->getNSSetManager(),
                    register_manager_->getKeySetManager(),
                    mailer_manager_.get(),
                    doc_manager_.get(),
                    register_manager_->getMessageManager()));

        request_manager_->setIdentificationMailAuthHostname(mconf->hostname);
        request_manager_->setDemoMode(mconf->demo_mode);
    }


    Register::PublicRequest::Manager* operator ->()
    {
        return request_manager_.get();
    }

    Register::Document::Manager* getDocumentManager()
    {
        return doc_manager_.get();
    }

};



/* 
 * helper class - auth. public request auto pointer (mojeid identification process)
 */
class IdentificationRequestPtr
{
private:
    IdentificationRequestManagerPtr request_manager_;
    Register::PublicRequest::Type type_;
    std::auto_ptr<Register::PublicRequest::PublicRequestAuth> request_;


public:
    IdentificationRequestPtr(const Register::PublicRequest::Type &_type)
        : type_(_type)
    {
        /* check valid type for mojeid identification */
        if ((type_ != Register::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION)
            && (type_ != Register::PublicRequest::PRT_CONTACT_IDENTIFICATION)) {
            throw std::runtime_error("not valid identification request type");
        }


        /* create request and cast it to authenticated */
        std::auto_ptr<Register::PublicRequest::PublicRequest> tmp(request_manager_->createRequest(type_));
        request_.reset(dynamic_cast<Register::PublicRequest::PublicRequestAuth*>(tmp.release()));
        if (!request_.get()) {
            throw std::runtime_error("unable to create identfication request");
        }
    }


    Register::PublicRequest::PublicRequestAuth* operator->()
    {
        return request_.get();
    }
};


}
}



#endif /*MOJEID_IDENTIFICATION*/

