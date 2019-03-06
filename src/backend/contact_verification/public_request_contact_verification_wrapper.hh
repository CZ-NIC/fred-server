/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef PUBLIC_REQUEST_CONTACT_VERIFICATION_WRAPPER_HH_EC031A84BE7647918B85B93686D53575
#define PUBLIC_REQUEST_CONTACT_VERIFICATION_WRAPPER_HH_EC031A84BE7647918B85B93686D53575

#include "src/backend/contact_verification/public_request_contact_verification_impl.hh"
#include "libfred/mailer.hh"
#include "src/deprecated/libfred/public_request/public_request.hh"
#include "src/deprecated/libfred/registry.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_contactverification_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_registry_args.hh"

namespace Fred {
namespace Backend {
namespace ContactVerification {

/*
 * helper class - public request manager auto pointer
 */
class ContactIdentificationRequestManagerPtr
{
private:
    DBSharedPtr nodb;
    mutable std::shared_ptr<LibFred::Mailer::Manager> mailer_manager_;
    mutable std::unique_ptr<LibFred::Manager> registry_manager_;
    mutable std::unique_ptr<LibFred::Document::Manager> doc_manager_;
    mutable std::unique_ptr<LibFred::PublicRequest::Manager> request_manager_;

public:
    ContactIdentificationRequestManagerPtr(std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager)
        : mailer_manager_(_mailer_manager)
    {
        /* get config temporary pointer */
        HandleRegistryArgs* rconf =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
        HandleContactVerificationArgs* cvconf =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleContactVerificationArgs>();

        /* construct managers */
        registry_manager_.reset(
                LibFred::Manager::create(
                        nodb,
                        rconf->restricted_handles));
        doc_manager_ = LibFred::Document::Manager::create(
                rconf->docgen_path,
                rconf->docgen_template_path,
                rconf->fileclient_path,
                // doc_manager config dependence
                CfgArgs::instance()->get_handler_ptr_by_type<
                        HandleCorbaNameServiceArgs>()
                ->get_nameservice_host_port());
        request_manager_.reset(
                LibFred::PublicRequest::Manager::create(
                        registry_manager_->getDomainManager(),
                        registry_manager_->getContactManager(),
                        registry_manager_->getNssetManager(),
                        registry_manager_->getKeysetManager(),
                        mailer_manager_.get(),
                        doc_manager_.get(),
                        registry_manager_->getMessageManager()));

        request_manager_->setIdentificationMailAuthHostname(cvconf->hostname);
        request_manager_->setDemoMode(cvconf->demo_mode);
    }


    ContactIdentificationRequestManagerPtr(const ContactIdentificationRequestManagerPtr& src)
        : mailer_manager_(src.mailer_manager_),
          registry_manager_(std::move(src.registry_manager_)),
          doc_manager_(std::move(src.doc_manager_)),
          request_manager_(std::move(src.request_manager_))
    {
    }

    LibFred::PublicRequest::Manager* operator ->()
    {
        return request_manager_.get();
    }

    LibFred::Document::Manager* getDocumentManager()
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
    LibFred::PublicRequest::Type type_;
    mutable std::unique_ptr<LibFred::PublicRequest::PublicRequestAuth> request_;

public:
    ContactIdentificationRequestPtr(
            std::shared_ptr<LibFred::Mailer::Manager> _mailer,
            const LibFred::PublicRequest::Type& _type)
        : request_manager_(_mailer),
          type_(_type)
    {
        /* check valid type for contact identification */
        if ((type_ != Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION)
            &&
            (type_ != Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION))
        {
            throw std::runtime_error("not valid identification request type");
        }


        /* create request and cast it to authenticated */
        std::unique_ptr<LibFred::PublicRequest::PublicRequest> tmp(request_manager_->createRequest(type_));
        request_.reset(dynamic_cast<LibFred::PublicRequest::PublicRequestAuth*>(tmp.release()));
        if (!request_.get())
        {
            throw std::runtime_error("unable to create identfication request");
        }
    }


    ContactIdentificationRequestPtr(const ContactIdentificationRequestPtr& src)
        : request_manager_(src.request_manager_),
          type_(src.type_),
          request_(std::move(src.request_))
    {
    }

    LibFred::PublicRequest::PublicRequestAuth* operator->()
    {
        return request_.get();
    }

};

} // namespace Fred::Backend::ContactVerification
} // namespace Fred::Backend
} // namespace Fred

#endif // CONTACT_IDENTIFICATION_H_
