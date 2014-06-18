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
 *  @conditional_contact_identification_impl.h
 *  header of conditional contact identification
 */

#ifndef CONTACT_CONDITIONAL_IDENTIFICATION_IMPL_H_
#define CONTACT_CONDITIONAL_IDENTIFICATION_IMPL_H_

#include "src/fredlib/public_request/public_request_impl.h"
#include "src/fredlib/contact_verification/contact_verification_password.h"
#include "src/fredlib/contact_verification/contact_verification_validators.h"


namespace Fred {
namespace Contact {
namespace Verification {

class ConditionalContactIdentificationImpl
{
    Fred::PublicRequest::PublicRequestAuthImpl* pra_impl_ptr_;
    Fred::PublicRequest::ContactVerificationPassword contact_verification_passwd_;
    Fred::Contact::Verification::ContactValidator contact_validator_;
public:
    ConditionalContactIdentificationImpl(
            Fred::PublicRequest::PublicRequestAuthImpl* _pra_impl_ptr, ContactValidator cv);
    std::string generate_passwords();
    void pre_save_check();
    void pre_process_check(bool _check);
    void process_action(bool _check);
};

}}}

#endif /*CONTACT_CONDITIONAL_IDENTIFICATION_IMPL_H_*/
