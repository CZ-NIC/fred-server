/*
 * Copyright (C) 2012-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  common part of contact identification implementation
 */

#include "src/deprecated/libfred/contact_verification/contact_identification_impl.hh"
#include "src/deprecated/libfred/contact_verification/contact_verification_state.hh"
#include "src/deprecated/libfred/object_states.hh"
#include "src/deprecated/libfred/public_request/public_request_impl.hh"

namespace LibFred {
namespace Contact {
namespace Verification {

ContactIdentificationImpl::ContactIdentificationImpl(
    LibFred::PublicRequest::PublicRequestAuthImpl* _pra_impl_ptr, ContactValidator cv)
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
    if (!pra_impl_ptr_->getId()) {
        const unsigned long long contact_id = pra_impl_ptr_->getObject(0).id;
        LibFred::Contact::Verification::Contact cdata
            = LibFred::Contact::Verification::contact_info(contact_id);
        contact_validator_.check(cdata);

        const State contact_state = get_contact_verification_state(contact_id);
        if (!contact_state.has_all(State::Civm) ||// missing C
             contact_state.has_all(State::cIvm)) {// already I
            throw LibFred::PublicRequest::NotApplicable("pre_save_check: failed!");
        }
    }
}

void ContactIdentificationImpl::pre_process_check(bool _check [[gnu::unused]])
{
    const unsigned long long contact_id = pra_impl_ptr_->getObject(0).id;
    const State contact_state = get_contact_verification_state(contact_id);
    if (!contact_state.has_all(State::Civm) ||// missing C
         contact_state.has_all(State::cIvm)) {// already I
        throw LibFred::PublicRequest::NotApplicable("pre_process_check: failed!");
    }

    LibFred::Contact::Verification::Contact cdata
        = LibFred::Contact::Verification::contact_info(contact_id);
    contact_validator_.check(cdata);
}

void ContactIdentificationImpl::process_action(bool _check [[gnu::unused]])
{
        LibFred::PublicRequest::insertNewStateRequest(
                pra_impl_ptr_->getId(),
                pra_impl_ptr_->getObject(0).id,
                ObjectState::IDENTIFIED_CONTACT);
}

}}}
