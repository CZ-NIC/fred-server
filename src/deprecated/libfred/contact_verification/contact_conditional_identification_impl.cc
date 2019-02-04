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
 *  @file
 *  common part of conditional contact identification implementation
 */

#include "src/deprecated/libfred/contact_verification/contact_conditional_identification_impl.hh"
#include "src/deprecated/libfred/contact_verification/contact_verification_state.hh"
#include "src/deprecated/libfred/object_states.hh"
#include <string>

namespace LibFred {
namespace Contact {
namespace Verification {

ConditionalContactIdentificationImpl::ConditionalContactIdentificationImpl(
        LibFred::PublicRequest::PublicRequestAuthImpl* _pra_impl_ptr, ContactValidator cv)
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
        const unsigned long long contact_id = pra_impl_ptr_->getObject(0).id;
        LibFred::Contact::Verification::Contact cdata
            = LibFred::Contact::Verification::contact_info(contact_id);
        contact_validator_.check(cdata);

        const State contact_state = get_contact_verification_state(contact_id);
        if (contact_state.has_any(State::CIVm)) {// already C or I or V
            throw LibFred::PublicRequest::NotApplicable("pre_save_check: failed!");
        }
    }
}

void ConditionalContactIdentificationImpl::pre_process_check(bool _check)
{
    /* object should not change */
    if (LibFred::PublicRequest::object_was_changed_since_request_create(
            pra_impl_ptr_->getId())) {
        throw LibFred::PublicRequest::ObjectChanged();
    }

    const unsigned long long contact_id = pra_impl_ptr_->getObject(0).id;
    LibFred::Contact::Verification::Contact cdata
        = LibFred::Contact::Verification::contact_info(contact_id);
    contact_validator_.check(cdata);

    const State contact_state = get_contact_verification_state(contact_id);
    if (contact_state.has_any(State::CIVm)) {// already C or I or V
        throw LibFred::PublicRequest::NotApplicable("pre_process_check: failed!");
    }
}

void ConditionalContactIdentificationImpl::process_action(bool _check)
{
    LibFred::PublicRequest::insertNewStateRequest(
            pra_impl_ptr_->getId(),
            pra_impl_ptr_->getObject(0).id,
            ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);
}


}}}
