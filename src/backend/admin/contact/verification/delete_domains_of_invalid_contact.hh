/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  delete domains owned by contact which fails manual verification check
 */

#ifndef ADMIN_CONTACT_VERIFICATION_DELETE_DOMAINS_OF_INVALID_CONTACT_H_585314041545
#define ADMIN_CONTACT_VERIFICATION_DELETE_DOMAINS_OF_INVALID_CONTACT_H_585314041545

#include "src/backend/admin/contact/verification/exceptions.hh"

#include "src/libfred/opcontext.hh"
#include "src/util/optional_value.hh"

namespace Admin {

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
        const uuid&             _check_handle);
}


#endif // #include guard end
