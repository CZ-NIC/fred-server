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
 *  @conditional_contact_identification_impl.cc
 *  common part of conditional contact identification implementation
 */

#include <string>
#include "src/fredlib/contact_verification/contact_conditional_identification_impl.h"
#include "src/fredlib/object_states.h"

namespace Fred {
namespace Contact {
namespace Verification {

ConditionalContactIdentificationImpl::ConditionalContactIdentificationImpl(
        Fred::PublicRequest::PublicRequestAuthImpl* _pra_impl_ptr, ContactValidator cv)
: pra_impl_ptr_(_pra_impl_ptr)
, contact_verification_passwd_(_pra_impl_ptr)

, contact_validator_(cv)
{}

std::string ConditionalContactIdentificationImpl::generate_passwords()
{
    if(pra_impl_ptr_->getPublicRequestManager()->getDemoMode())
    {
        return std::string(contact_verification_passwd_
                .get_password_chunk_length(),'1')//pin1:11111111
            +std::string(contact_verification_passwd_
                .get_password_chunk_length(),'2'); //pin2:22222222
    }
    else
    {
        return std::string(contact_verification_passwd_.generateRandomPassword())
            + std::string(contact_verification_passwd_.generateRandomPassword());
    }
}

void ConditionalContactIdentificationImpl::pre_save_check()
{
    /* insert */
    if (!pra_impl_ptr_->getId())
    {
        Fred::Contact::Verification::Contact cdata
            = Fred::Contact::Verification::contact_info(
                    pra_impl_ptr_->getObject(0).id);
        contact_validator_.check(cdata);

        if (object_has_one_of_states(pra_impl_ptr_->getObject(0).id
            , Util::vector_of<std::string>
            (ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT)// already CI
            (ObjectState::IDENTIFIED_CONTACT)// already I
            (ObjectState::VALIDATED_CONTACT))// already V
        )
        {
            throw Fred::PublicRequest::NotApplicable("pre_save_check: failed!");
        }
    }
}

void ConditionalContactIdentificationImpl::pre_process_check(bool _check)
{
    /* object should not change */
    if (Fred::PublicRequest::object_was_changed_since_request_create(
            pra_impl_ptr_->getId())) {
        throw Fred::PublicRequest::ObjectChanged();
    }

    Fred::Contact::Verification::Contact cdata
        = Fred::Contact::Verification::contact_info(
                pra_impl_ptr_->getObject(0).id);
    contact_validator_.check(cdata);
}

void ConditionalContactIdentificationImpl::process_action(bool _check)
{
    Fred::PublicRequest::insertNewStateRequest(
            pra_impl_ptr_->getId(),
            pra_impl_ptr_->getObject(0).id,
            ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);
}


}}}

