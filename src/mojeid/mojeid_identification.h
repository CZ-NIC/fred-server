#ifndef MOJEID_IDENTIFICATION_H_
#define MOJEID_IDENTIFICATION_H_

#include "cfg/config_handler_decl.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_mojeid_args.h"
#include "src/fredlib/registry.h"
#include "src/fredlib/public_request/public_request.h"
#include "src/mojeid/public_request_verification_impl.h"
#include "src/fredlib/mailer.h"
//config for docmanager
#include "cfg/config_handler_decl.h"
#include "cfg/handle_corbanameservice_args.h"

namespace Registry {
namespace MojeID {


/*
 * helper class - public request manager auto pointer
 */
class IdentificationRequestManagerPtr
{
private:
    DBSharedPtr nodb;
    mutable boost::shared_ptr<Fred::Mailer::Manager> mailer_manager_;
    mutable std::auto_ptr<Fred::Manager> registry_manager_;
    mutable std::auto_ptr<Fred::Document::Manager> doc_manager_;
    mutable std::auto_ptr<Fred::PublicRequest::Manager> request_manager_;


public:
    IdentificationRequestManagerPtr(boost::shared_ptr<Fred::Mailer::Manager> _mailer_manager)
    : mailer_manager_(_mailer_manager)
    {
        /* get config temporary pointer */
        HandleRegistryArgs *rconf =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
        HandleMojeIDArgs *mconf =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIDArgs>();

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

        request_manager_->setIdentificationMailAuthHostname(mconf->hostname);
        request_manager_->setDemoMode(mconf->demo_mode);
    }

    IdentificationRequestManagerPtr(const IdentificationRequestManagerPtr &src) :
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
 * helper class - auth. public request auto pointer (mojeid identification process)
 */
class IdentificationRequestPtr
{
private:
    IdentificationRequestManagerPtr request_manager_;
    Fred::PublicRequest::Type type_;
    mutable std::auto_ptr<Fred::PublicRequest::PublicRequestAuth> request_;


public:
    IdentificationRequestPtr(boost::shared_ptr<Fred::Mailer::Manager> _mailer
            , const Fred::PublicRequest::Type &_type)
        : request_manager_(_mailer)
        , type_(_type)
    {
        /* check valid type for mojeid identification */
        if ((type_ != Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION)
            && (type_ != Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION)
            && (type_ != Fred::PublicRequest::PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER)
            && (type_ != Fred::PublicRequest::PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER)
            && (type_ != Fred::PublicRequest::PRT_MOJEID_CONTACT_REIDENTIFICATION))
        {
            throw std::runtime_error("not valid identification request type");
        }


        /* create request and cast it to authenticated */
        std::auto_ptr<Fred::PublicRequest::PublicRequest> tmp(request_manager_->createRequest(type_));
        request_.reset(dynamic_cast<Fred::PublicRequest::PublicRequestAuth*>(tmp.release()));
        if (!request_.get()) {
            throw std::runtime_error("unable to create identfication request");
        }
    }


    IdentificationRequestPtr(const IdentificationRequestPtr &src) :
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



#endif /*MOJEID_IDENTIFICATION*/

