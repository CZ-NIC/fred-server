/*
 * Copyright (C) 2012  CZ.NIC, z. s. p. o.
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
 *  @conditional_contact_identification_impl.h
 *  header of conditional contact identification
 */

#ifndef CONTACT_CONDITIONAL_IDENTIFICATION_IMPL_HH_142686BD80BC4B9C8C3B489409C0FFD6
#define CONTACT_CONDITIONAL_IDENTIFICATION_IMPL_HH_142686BD80BC4B9C8C3B489409C0FFD6

#include "src/deprecated/libfred/public_request/public_request_impl.hh"
#include "src/deprecated/libfred/contact_verification/contact_verification_password.hh"
#include "src/deprecated/libfred/contact_verification/contact_verification_validators.hh"


namespace LibFred {
namespace Contact {
namespace Verification {

class ConditionalContactIdentificationImpl
{
    LibFred::PublicRequest::PublicRequestAuthImpl* pra_impl_ptr_;
    LibFred::PublicRequest::ContactVerificationPassword contact_verification_passwd_;
    LibFred::Contact::Verification::ContactValidator contact_validator_;
public:
    ConditionalContactIdentificationImpl(
            LibFred::PublicRequest::PublicRequestAuthImpl* _pra_impl_ptr, ContactValidator cv);
    std::string generate_passwords();
    void pre_save_check();
    void pre_process_check(bool _check);
    void process_action(bool _check);
};

}}}

#endif /*CONTACT_CONDITIONAL_IDENTIFICATION_IMPL_H_*/
