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
 *  @contact_identification_impl.cc
 *  common part of contact identification implementation
 */

#include "contact_identification_impl.h"
#include "src/fredlib/object_states.h"
#include "src/fredlib/public_request/public_request_impl.h"

namespace Fred {
namespace Contact {
namespace Verification {

ContactIdentificationImpl::ContactIdentificationImpl(
    Fred::PublicRequest::PublicRequestAuthImpl* _pra_impl_ptr, ContactValidator cv)
: pra_impl_ptr_(_pra_impl_ptr)
, contact_verification_passwd_(_pra_impl_ptr)
, contact_validator_(cv)
{}

std::string ContactIdentificationImpl::generate_passwords()
{
    /* generate pin3 */
    if(pra_impl_ptr_->getPublicRequestManager()
            ->getDemoMode())
    {
        return std::string(contact_verification_passwd_
                .get_password_chunk_length(),'3');//pin3:33333333
    }
    else
    {
        return contact_verification_passwd_.generateRandomPassword();
    }
}

void ContactIdentificationImpl::pre_save_check()
{
    if (!pra_impl_ptr_->getId())
    {
        Fred::Contact::Verification::Contact cdata
            = Fred::Contact::Verification::contact_info(
                    pra_impl_ptr_->getObject(0).id);
        contact_validator_.check(cdata);

        if (((object_has_state(pra_impl_ptr_->getObject(0).id
                , ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT) == false)
            ||
            object_has_one_of_states(
                pra_impl_ptr_->getObject(0).id
                , Util::vector_of<std::string>
                (ObjectState::IDENTIFIED_CONTACT) // already I
                (ObjectState::VALIDATED_CONTACT)) // already V
        ))
        {
            throw Fred::PublicRequest::NotApplicable("pre_save_check: failed!");
        }
    }
}

void ContactIdentificationImpl::pre_process_check(bool _check)
{
    if (object_has_state(pra_impl_ptr_->getObject(0).id,
            ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT) == false)
    {
        throw Fred::PublicRequest::NotApplicable("pre_process_check: failed!");
    }

    Fred::Contact::Verification::Contact cdata
        = Fred::Contact::Verification::contact_info(
                pra_impl_ptr_->getObject(0).id);
    contact_validator_.check(cdata);
}

void ContactIdentificationImpl::process_action(bool _check)
{
        Fred::cancel_object_state(pra_impl_ptr_->getObject(0).id,
                Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);

        Fred::PublicRequest::insertNewStateRequest(
                pra_impl_ptr_->getId(),
                pra_impl_ptr_->getObject(0).id,
                ObjectState::IDENTIFIED_CONTACT);
}

}}}



