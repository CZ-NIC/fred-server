/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
 *  delete domains owned by contact which fails manual verification check
 */

#ifndef DELETE_DOMAINS_OF_INVALID_CONTACT_HH_B778D2F9BB834FE5A8BCAE9D6BF0DD35
#define DELETE_DOMAINS_OF_INVALID_CONTACT_HH_B778D2F9BB834FE5A8BCAE9D6BF0DD35

#include "src/backend/admin/contact/verification/exceptions.hh"

#include "libfred/opcontext.hh"
#include "util/optional_value.hh"

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

/**
 * Deletes domains owned by contact which MANUAL check FAILED.
 * If check IS NOT MANUAL or the result is NOT FAILED then does nothing.
 *
 * Creates Delete Domain Poll Message as sideeffect and stores the relation between the message and check.
 *
 * @throws Admin::ExceptionUnknownCheckHandle
 * @throws Admin::ExceptionIncorrectTestsuite
 * @throws Admin::ExceptionIncorrectCheckStatus
 * @throws Admin::ExceptionIncorrectContactStatus
 * @throws Admin::ExceptionDomainsAlreadyDeleted
 */
void delete_domains_of_invalid_contact(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle);


} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
