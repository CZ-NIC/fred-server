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
 *  push back contact verification checks to queue up to it's at maximal length
 */

#ifndef ADMIN_CONTACT_VERIFICATION_CREATE_TEST_IMPL_PROTOTYPES_H_224556435
#define ADMIN_CONTACT_VERIFICATION_CREATE_TEST_IMPL_PROTOTYPES_H_224556435

#include <map>
#include <string>
#include <memory>

#include "src/admin/contact/verification/test_impl/test_interface.h"

#include "src/fredlib/mailer.h"
#include "src/fredlib/documents.h"
#include "src/fredlib/messages/messages_impl.h"

namespace Admin {

    /**
     * @return mapping of testnames to configured test implementation instances
     */
    std::map<std::string, std::shared_ptr<Admin::ContactVerification::Test> > create_test_impl_prototypes(
        std::shared_ptr<Fred::Mailer::Manager>   _mailer_manager,
        std::shared_ptr<Fred::Document::Manager> _document_manager,
        std::shared_ptr<Fred::Messages::Manager> _message_manager,
        const std::string&                         _cz_address_dataset_path
    );
}

#endif // #include guard end
